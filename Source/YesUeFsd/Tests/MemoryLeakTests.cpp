// Copyright Yes UE FSD. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "AutoDriver/AutoDriverComponent.h"
#include "AutoDriver/AutoDriverSubsystem.h"
#include "AutoDriver/Recording/ActionRecorder.h"
#include "AutoDriver/Recording/ActionPlayback.h"
#include "AutoDriver/NavigationCache.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "HAL/PlatformMemory.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Memory leak test: Component creation and destruction
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMemoryLeakComponentLifecycleTest,
	"YesUeFsd.MemoryLeak.Component.Lifecycle",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMemoryLeakComponentLifecycleTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("World Created"), World))
	{
		return false;
	}

	// Get baseline memory
	CollectGarbage(RF_NoFlags, true);
	FPlatformMemoryStats BaselineStats = FPlatformMemory::GetStats();

	// Create and destroy many components
	const int32 NumIterations = 100;
	for (int32 Iteration = 0; Iteration < NumIterations; ++Iteration)
	{
		TArray<AActor*> Actors;
		for (int32 i = 0; i < 10; ++i)
		{
			AActor* TestActor = World->SpawnActor<AActor>();
			UAutoDriverComponent* AutoDriver = NewObject<UAutoDriverComponent>(TestActor);
			AutoDriver->RegisterComponent();
			AutoDriver->SetEnabled(true);

			// Use the component
			AutoDriver->MoveToLocation(FVector(100, 0, 0));
			AutoDriver->StopCommand();

			Actors.Add(TestActor);
		}

		// Destroy actors
		for (AActor* Actor : Actors)
		{
			Actor->Destroy();
		}

		// Periodic GC
		if (Iteration % 10 == 0)
		{
			CollectGarbage(RF_NoFlags, true);
		}
	}

	// Final garbage collection
	CollectGarbage(RF_NoFlags, true);

	FPlatformMemoryStats FinalStats = FPlatformMemory::GetStats();
	int64 MemoryDelta = FinalStats.UsedPhysical - BaselineStats.UsedPhysical;
	float MemoryDeltaMB = MemoryDelta / (1024.0f * 1024.0f);

	// Memory delta should be minimal (< 10MB)
	TestTrue(TEXT("No Significant Memory Leak"), FMath::Abs(MemoryDeltaMB) < 10.0f);

	AddInfo(FString::Printf(TEXT("Memory delta after %d iterations: %.2f MB"),
		NumIterations, MemoryDeltaMB));

	return true;
}

/**
 * Memory leak test: Command execution cycles
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMemoryLeakCommandExecutionTest,
	"YesUeFsd.MemoryLeak.Command.Execution",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMemoryLeakCommandExecutionTest::RunTest(const FString& Parameters)
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

	CollectGarbage(RF_NoFlags, true);
	FPlatformMemoryStats BaselineStats = FPlatformMemory::GetStats();

	// Execute many commands
	const int32 NumCommands = 10000;
	for (int32 i = 0; i < NumCommands; ++i)
	{
		FVector TargetLocation(FMath::FRandRange(-1000, 1000),
			FMath::FRandRange(-1000, 1000), 0);
		AutoDriver->MoveToLocation(TargetLocation);
		AutoDriver->StopCommand();

		if (i % 1000 == 0)
		{
			CollectGarbage(RF_NoFlags, true);
		}
	}

	CollectGarbage(RF_NoFlags, true);
	FPlatformMemoryStats FinalStats = FPlatformMemory::GetStats();
	int64 MemoryDelta = FinalStats.UsedPhysical - BaselineStats.UsedPhysical;
	float MemoryDeltaMB = MemoryDelta / (1024.0f * 1024.0f);

	TestTrue(TEXT("No Command Execution Memory Leak"), FMath::Abs(MemoryDeltaMB) < 5.0f);

	AddInfo(FString::Printf(TEXT("Memory delta after %d commands: %.2f MB"),
		NumCommands, MemoryDeltaMB));

	return true;
}

/**
 * Memory leak test: Navigation cache growth
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMemoryLeakNavigationCacheTest,
	"YesUeFsd.MemoryLeak.NavigationCache.Growth",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMemoryLeakNavigationCacheTest::RunTest(const FString& Parameters)
{
	CollectGarbage(RF_NoFlags, true);
	FPlatformMemoryStats BaselineStats = FPlatformMemory::GetStats();

	// Create cache and fill it multiple times
	FNavigationCache* Cache = new FNavigationCache(1000);

	const int32 NumCycles = 100;
	for (int32 Cycle = 0; Cycle < NumCycles; ++Cycle)
	{
		// Fill cache beyond capacity to trigger LRU eviction
		for (int32 i = 0; i < 2000; ++i)
		{
			FVector Start(FMath::FRandRange(-5000, 5000),
				FMath::FRandRange(-5000, 5000), 0);
			FVector End(FMath::FRandRange(-5000, 5000),
				FMath::FRandRange(-5000, 5000), 0);
			FNavigationCacheKey Key(Start, End, nullptr);

			FNavigationCacheEntry Entry;
			Entry.bIsReachable = true;
			Entry.PathLength = FVector::Dist(Start, End);
			Entry.Timestamp = FPlatformTime::Seconds();

			Cache->Add(Key, Entry);
		}

		// Clear cache periodically
		if (Cycle % 10 == 0)
		{
			Cache->Clear();
		}
	}

	delete Cache;

	CollectGarbage(RF_NoFlags, true);
	FPlatformMemoryStats FinalStats = FPlatformMemory::GetStats();
	int64 MemoryDelta = FinalStats.UsedPhysical - BaselineStats.UsedPhysical;
	float MemoryDeltaMB = MemoryDelta / (1024.0f * 1024.0f);

	TestTrue(TEXT("No Navigation Cache Memory Leak"), FMath::Abs(MemoryDeltaMB) < 5.0f);

	AddInfo(FString::Printf(TEXT("Memory delta after cache cycles: %.2f MB"), MemoryDeltaMB));

	return true;
}

/**
 * Memory leak test: Recording and playback
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMemoryLeakRecordingTest,
	"YesUeFsd.MemoryLeak.Recording.Lifecycle",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMemoryLeakRecordingTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("World Created"), World))
	{
		return false;
	}

	AActor* TestActor = World->SpawnActor<AActor>();

	CollectGarbage(RF_NoFlags, true);
	FPlatformMemoryStats BaselineStats = FPlatformMemory::GetStats();

	// Create and discard many recordings
	const int32 NumRecordings = 100;
	for (int32 i = 0; i < NumRecordings; ++i)
	{
		UActionRecorder* Recorder = NewObject<UActionRecorder>();
		Recorder->StartRecording(TestActor);

		// Record some actions
		for (int32 j = 0; j < 10; ++j)
		{
			// Simulate recording actions
		}

		UActionTimeline* Timeline = Recorder->StopRecording();

		// Play back
		UActionPlayback* Playback = NewObject<UActionPlayback>();
		Playback->SetTimeline(Timeline);
		Playback->Play();
		Playback->Stop();

		if (i % 10 == 0)
		{
			CollectGarbage(RF_NoFlags, true);
		}
	}

	CollectGarbage(RF_NoFlags, true);
	FPlatformMemoryStats FinalStats = FPlatformMemory::GetStats();
	int64 MemoryDelta = FinalStats.UsedPhysical - BaselineStats.UsedPhysical;
	float MemoryDeltaMB = MemoryDelta / (1024.0f * 1024.0f);

	TestTrue(TEXT("No Recording Memory Leak"), FMath::Abs(MemoryDeltaMB) < 5.0f);

	AddInfo(FString::Printf(TEXT("Memory delta after %d recordings: %.2f MB"),
		NumRecordings, MemoryDeltaMB));

	return true;
}

/**
 * Memory leak test: Callback retention
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMemoryLeakCallbackTest,
	"YesUeFsd.MemoryLeak.Callback.Retention",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMemoryLeakCallbackTest::RunTest(const FString& Parameters)
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

	CollectGarbage(RF_NoFlags, true);
	FPlatformMemoryStats BaselineStats = FPlatformMemory::GetStats();

	// Execute many commands with callbacks
	const int32 NumCallbacks = 1000;
	for (int32 i = 0; i < NumCallbacks; ++i)
	{
		FAutoDriverCommandComplete Callback;
		Callback.BindLambda([i](bool bSuccess)
		{
			// Callback captures index
		});

		AutoDriver->MoveToLocationWithCallback(FVector(100, 0, 0), Callback);
		AutoDriver->StopCommand(); // Trigger callback

		if (i % 100 == 0)
		{
			CollectGarbage(RF_NoFlags, true);
		}
	}

	CollectGarbage(RF_NoFlags, true);
	FPlatformMemoryStats FinalStats = FPlatformMemory::GetStats();
	int64 MemoryDelta = FinalStats.UsedPhysical - BaselineStats.UsedPhysical;
	float MemoryDeltaMB = MemoryDelta / (1024.0f * 1024.0f);

	TestTrue(TEXT("No Callback Retention Memory Leak"), FMath::Abs(MemoryDeltaMB) < 5.0f);

	AddInfo(FString::Printf(TEXT("Memory delta after %d callbacks: %.2f MB"),
		NumCallbacks, MemoryDeltaMB));

	return true;
}

/**
 * Memory leak test: Subsystem lifecycle
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMemoryLeakSubsystemTest,
	"YesUeFsd.MemoryLeak.Subsystem.Lifecycle",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMemoryLeakSubsystemTest::RunTest(const FString& Parameters)
{
	CollectGarbage(RF_NoFlags, true);
	FPlatformMemoryStats BaselineStats = FPlatformMemory::GetStats();

	// Create and destroy worlds with subsystems
	const int32 NumWorlds = 10;
	for (int32 i = 0; i < NumWorlds; ++i)
	{
		UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
		if (!World)
		{
			continue;
		}

		UAutoDriverSubsystem* Subsystem = World->GetSubsystem<UAutoDriverSubsystem>();

		// Use subsystem
		AActor* TestActor = World->SpawnActor<AActor>();
		UAutoDriverComponent* AutoDriver = NewObject<UAutoDriverComponent>(TestActor);
		AutoDriver->RegisterComponent();
		AutoDriver->SetEnabled(true);

		AutoDriver->MoveToLocation(FVector(100, 0, 0));
		AutoDriver->StopCommand();

		CollectGarbage(RF_NoFlags, true);
	}

	CollectGarbage(RF_NoFlags, true);
	FPlatformMemoryStats FinalStats = FPlatformMemory::GetStats();
	int64 MemoryDelta = FinalStats.UsedPhysical - BaselineStats.UsedPhysical;
	float MemoryDeltaMB = MemoryDelta / (1024.0f * 1024.0f);

	TestTrue(TEXT("No Subsystem Memory Leak"), FMath::Abs(MemoryDeltaMB) < 10.0f);

	AddInfo(FString::Printf(TEXT("Memory delta after %d worlds: %.2f MB"),
		NumWorlds, MemoryDeltaMB));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
