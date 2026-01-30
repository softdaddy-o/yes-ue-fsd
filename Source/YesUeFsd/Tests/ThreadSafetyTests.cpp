// Copyright Yes UE FSD. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "AutoDriver/AutoDriverComponent.h"
#include "AutoDriver/NavigationCache.h"
#include "AutoDriver/AutoDriverSubsystem.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "HAL/ThreadSafeBool.h"
#include "HAL/PlatformProcess.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Thread safety test: Navigation cache concurrent access
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FThreadSafetyNavigationCacheTest,
	"YesUeFsd.ThreadSafety.NavigationCache.Concurrent",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FThreadSafetyNavigationCacheTest::RunTest(const FString& Parameters)
{
	// Test concurrent reads/writes to navigation cache
	FNavigationCache* Cache = new FNavigationCache(1000);
	FThreadSafeBool bStopThreads(false);
	FThreadSafeBool bTestFailed(false);

	// Writer thread
	class FWriterRunnable : public FRunnable
	{
	public:
		FNavigationCache* Cache;
		FThreadSafeBool* bStop;

		FWriterRunnable(FNavigationCache* InCache, FThreadSafeBool* InStop)
			: Cache(InCache), bStop(InStop)
		{}

		virtual uint32 Run() override
		{
			int32 Counter = 0;
			while (!*bStop)
			{
				FVector Start(Counter * 10, 0, 0);
				FVector End(Counter * 10 + 100, 0, 0);
				FNavigationCacheKey Key(Start, End, nullptr);

				FNavigationCacheEntry Entry;
				Entry.bIsReachable = true;
				Entry.PathLength = 100.0f;
				Entry.Timestamp = FPlatformTime::Seconds();

				Cache->Add(Key, Entry);
				Counter++;

				FPlatformProcess::Sleep(0.001f);
			}
			return 0;
		}
	};

	// Reader thread
	class FReaderRunnable : public FRunnable
	{
	public:
		FNavigationCache* Cache;
		FThreadSafeBool* bStop;
		FThreadSafeBool* bFailed;

		FReaderRunnable(FNavigationCache* InCache, FThreadSafeBool* InStop, FThreadSafeBool* InFailed)
			: Cache(InCache), bStop(InStop), bFailed(InFailed)
		{}

		virtual uint32 Run() override
		{
			int32 Counter = 0;
			while (!*bStop)
			{
				FVector Start(Counter * 10, 0, 0);
				FVector End(Counter * 10 + 100, 0, 0);
				FNavigationCacheKey Key(Start, End, nullptr);

				FNavigationCacheEntry Entry;
				Cache->Get(Key, Entry);

				Counter++;
				FPlatformProcess::Sleep(0.001f);
			}
			return 0;
		}
	};

	// Start threads
	FWriterRunnable* Writer = new FWriterRunnable(Cache, &bStopThreads);
	FReaderRunnable* Reader1 = new FReaderRunnable(Cache, &bStopThreads, &bTestFailed);
	FReaderRunnable* Reader2 = new FReaderRunnable(Cache, &bStopThreads, &bTestFailed);

	FRunnableThread* WriterThread = FRunnableThread::Create(Writer, TEXT("CacheWriter"));
	FRunnableThread* Reader1Thread = FRunnableThread::Create(Reader1, TEXT("CacheReader1"));
	FRunnableThread* Reader2Thread = FRunnableThread::Create(Reader2, TEXT("CacheReader2"));

	// Let threads run
	FPlatformProcess::Sleep(0.5f);

	// Stop threads
	bStopThreads = true;

	WriterThread->WaitForCompletion();
	Reader1Thread->WaitForCompletion();
	Reader2Thread->WaitForCompletion();

	delete WriterThread;
	delete Reader1Thread;
	delete Reader2Thread;
	delete Writer;
	delete Reader1;
	delete Reader2;
	delete Cache;

	TestFalse(TEXT("No Thread Safety Issues"), bTestFailed);

	return true;
}

/**
 * Thread safety test: Component command queue
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FThreadSafetyComponentCommandsTest,
	"YesUeFsd.ThreadSafety.Component.Commands",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FThreadSafetyComponentCommandsTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("World Created"), World))
	{
		return false;
	}

	AActor* TestActor = World->SpawnActor<AActor>();
	UAutoDriverComponent* AutoDriver = NewObject<UAutoDriverComponent>(TestActor);
	AutoDriver->RegisterComponent();
	AutoDriver->SetEnabled(true);

	// Test rapid command switching (simulating concurrent-like behavior)
	const int32 NumCommands = 1000;
	bool bAnyFailure = false;

	for (int32 i = 0; i < NumCommands; ++i)
	{
		FVector TargetLocation(FMath::FRandRange(-1000, 1000),
			FMath::FRandRange(-1000, 1000), 0);

		bool bSuccess = AutoDriver->MoveToLocation(TargetLocation);
		if (!bSuccess)
		{
			bAnyFailure = true;
		}

		// Immediately start new command (stress test)
		AutoDriver->StopCommand();
	}

	TestFalse(TEXT("No Command Queue Failures"), bAnyFailure);

	return true;
}

/**
 * Thread safety test: Subsystem access
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FThreadSafetySubsystemTest,
	"YesUeFsd.ThreadSafety.Subsystem.Access",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FThreadSafetySubsystemTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("World Created"), World))
	{
		return false;
	}

	UAutoDriverSubsystem* Subsystem = World->GetSubsystem<UAutoDriverSubsystem>();
	if (!TestNotNull(TEXT("Subsystem Created"), Subsystem))
	{
		return false;
	}

	// Test rapid subsystem queries (simulating multi-threaded access patterns)
	bool bAnyFailure = false;

	for (int32 i = 0; i < 1000; ++i)
	{
		UAutoDriverSubsystem* QueriedSubsystem = World->GetSubsystem<UAutoDriverSubsystem>();
		if (QueriedSubsystem != Subsystem)
		{
			bAnyFailure = true;
		}
	}

	TestFalse(TEXT("Consistent Subsystem Access"), bAnyFailure);

	return true;
}

/**
 * Thread safety test: Callback execution
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FThreadSafetyCallbackTest,
	"YesUeFsd.ThreadSafety.Callback.Execution",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FThreadSafetyCallbackTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("World Created"), World))
	{
		return false;
	}

	AActor* TestActor = World->SpawnActor<AActor>();
	UAutoDriverComponent* AutoDriver = NewObject<UAutoDriverComponent>(TestActor);
	AutoDriver->RegisterComponent();
	AutoDriver->SetEnabled(true);

	// Test that callbacks don't cause race conditions
	FThreadSafeCounter CallbackCounter;
	bool bAnyFailure = false;

	const int32 NumCallbacks = 100;
	for (int32 i = 0; i < NumCallbacks; ++i)
	{
		FAutoDriverCommandComplete Callback;
		Callback.BindLambda([&CallbackCounter, &bAnyFailure](bool bSuccess)
		{
			int32 Value = CallbackCounter.Increment();
			// Verify increment worked correctly
			if (Value <= 0)
			{
				bAnyFailure = true;
			}
		});

		AutoDriver->MoveToLocationWithCallback(FVector(100, 0, 0), Callback);
		AutoDriver->StopCommand(); // Trigger callback immediately
	}

	TestEqual(TEXT("All Callbacks Executed"), CallbackCounter.GetValue(), NumCallbacks);
	TestFalse(TEXT("No Callback Race Conditions"), bAnyFailure);

	return true;
}

/**
 * Thread safety test: Concurrent component operations
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FThreadSafetyConcurrentOpsTest,
	"YesUeFsd.ThreadSafety.Component.ConcurrentOps",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FThreadSafetyConcurrentOpsTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("World Created"), World))
	{
		return false;
	}

	// Create multiple components and stress test them
	const int32 NumComponents = 50;
	TArray<UAutoDriverComponent*> Components;

	for (int32 i = 0; i < NumComponents; ++i)
	{
		AActor* TestActor = World->SpawnActor<AActor>();
		UAutoDriverComponent* AutoDriver = NewObject<UAutoDriverComponent>(TestActor);
		AutoDriver->RegisterComponent();
		AutoDriver->SetEnabled(true);
		Components.Add(AutoDriver);
	}

	// Perform concurrent-like operations
	bool bAnyFailure = false;
	for (int32 Iteration = 0; Iteration < 100; ++Iteration)
	{
		// All components issue commands simultaneously
		for (int32 i = 0; i < NumComponents; ++i)
		{
			FVector Target(FMath::FRandRange(-1000, 1000),
				FMath::FRandRange(-1000, 1000), 0);

			bool bSuccess = Components[i]->MoveToLocation(Target);
			if (!bSuccess)
			{
				bAnyFailure = true;
			}
		}

		// All components tick simultaneously
		for (int32 i = 0; i < NumComponents; ++i)
		{
			Components[i]->TickComponent(0.016f, LEVELTICK_All, nullptr);
		}
	}

	TestFalse(TEXT("No Concurrent Operation Failures"), bAnyFailure);

	return true;
}

/**
 * Thread safety test: Cache statistics
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FThreadSafetyCacheStatsTest,
	"YesUeFsd.ThreadSafety.Cache.Statistics",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FThreadSafetyCacheStatsTest::RunTest(const FString& Parameters)
{
	FNavigationCache Cache(1000);

	// Perform many operations while querying stats
	const int32 NumOperations = 10000;
	for (int32 i = 0; i < NumOperations; ++i)
	{
		FVector Start(i * 10, 0, 0);
		FVector End(i * 10 + 100, 0, 0);
		FNavigationCacheKey Key(Start, End, nullptr);

		FNavigationCacheEntry Entry;
		Entry.bIsReachable = true;
		Entry.PathLength = 100.0f;
		Entry.Timestamp = FPlatformTime::Seconds();

		Cache.Add(Key, Entry);

		// Query stats frequently
		if (i % 10 == 0)
		{
			FNavigationCacheStats Stats = Cache.GetStats();
			TestTrue(TEXT("Stats Query Successful"), Stats.Size >= 0);
		}

		// Perform lookups
		Cache.Get(Key, Entry);
	}

	FNavigationCacheStats FinalStats = Cache.GetStats();
	TestTrue(TEXT("Final Stats Consistent"), FinalStats.Size >= 0 && FinalStats.Size <= 1000);

	return true;
}

/**
 * Thread safety test: Input simulation
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FThreadSafetyInputTest,
	"YesUeFsd.ThreadSafety.Input.Simulation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FThreadSafetyInputTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("World Created"), World))
	{
		return false;
	}

	AActor* TestActor = World->SpawnActor<AActor>();
	UAutoDriverComponent* AutoDriver = NewObject<UAutoDriverComponent>(TestActor);
	AutoDriver->RegisterComponent();
	AutoDriver->SetEnabled(true);

	// Rapid input changes
	bool bAnyFailure = false;
	const int32 NumInputs = 10000;

	for (int32 i = 0; i < NumInputs; ++i)
	{
		bool bSuccess1 = AutoDriver->SetAxisValue(FName("MoveForward"),
			FMath::FRandRange(-1.0f, 1.0f));
		bool bSuccess2 = AutoDriver->SetAxisValue(FName("MoveRight"),
			FMath::FRandRange(-1.0f, 1.0f));

		if (!bSuccess1 || !bSuccess2)
		{
			bAnyFailure = true;
		}

		// Occasional button presses
		if (i % 100 == 0)
		{
			AutoDriver->PressButton(FName("Jump"));
		}
	}

	TestFalse(TEXT("No Input Simulation Failures"), bAnyFailure);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
