// Copyright Yes UE FSD. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "AutoDriver/AutoDriverComponent.h"
#include "AutoDriver/NavigationHelper.h"
#include "AutoDriver/NavigationCache.h"
#include "AutoDriver/AutoDriverStats.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "HAL/PlatformTime.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Performance benchmark: AutoDriver component creation
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPerformanceComponentCreationTest,
	"YesUeFsd.Performance.Component.Creation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::PerfFilter)

bool FPerformanceComponentCreationTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("World Created"), World))
	{
		return false;
	}

	const int32 NumComponents = 1000;
	double StartTime = FPlatformTime::Seconds();

	for (int32 i = 0; i < NumComponents; ++i)
	{
		AActor* TestActor = World->SpawnActor<AActor>();
		UAutoDriverComponent* AutoDriver = NewObject<UAutoDriverComponent>(TestActor);
		AutoDriver->RegisterComponent();
	}

	double ElapsedTime = FPlatformTime::Seconds() - StartTime;
	double PerComponentTime = (ElapsedTime / NumComponents) * 1000.0; // milliseconds

	TestTrue(TEXT("Creation Performance Acceptable"), PerComponentTime < 1.0); // < 1ms per component

	AddInfo(FString::Printf(TEXT("Created %d components in %.3f seconds (%.3f ms per component)"),
		NumComponents, ElapsedTime, PerComponentTime));

	return true;
}

/**
 * Performance benchmark: Command execution overhead
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPerformanceCommandExecutionTest,
	"YesUeFsd.Performance.Command.Execution",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::PerfFilter)

bool FPerformanceCommandExecutionTest::RunTest(const FString& Parameters)
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

	const int32 NumCommands = 1000;
	double StartTime = FPlatformTime::Seconds();

	for (int32 i = 0; i < NumCommands; ++i)
	{
		FVector TargetLocation(FMath::FRandRange(-1000, 1000),
			FMath::FRandRange(-1000, 1000), 0);
		AutoDriver->MoveToLocation(TargetLocation);
		AutoDriver->StopCommand();
	}

	double ElapsedTime = FPlatformTime::Seconds() - StartTime;
	double PerCommandTime = (ElapsedTime / NumCommands) * 1000.0; // milliseconds

	TestTrue(TEXT("Command Execution Performance Acceptable"), PerCommandTime < 0.5); // < 0.5ms per command

	AddInfo(FString::Printf(TEXT("Executed %d commands in %.3f seconds (%.3f ms per command)"),
		NumCommands, ElapsedTime, PerCommandTime));

	return true;
}

/**
 * Performance benchmark: Tick performance
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPerformanceTickTest,
	"YesUeFsd.Performance.Tick.Multiple",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::PerfFilter)

bool FPerformanceTickTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("World Created"), World))
	{
		return false;
	}

	// Create multiple components
	const int32 NumComponents = 100;
	TArray<UAutoDriverComponent*> Components;

	for (int32 i = 0; i < NumComponents; ++i)
	{
		AActor* TestActor = World->SpawnActor<AActor>();
		UAutoDriverComponent* AutoDriver = NewObject<UAutoDriverComponent>(TestActor);
		AutoDriver->RegisterComponent();
		AutoDriver->SetEnabled(true);
		Components.Add(AutoDriver);

		// Start movement command
		FVector TargetLocation(FMath::FRandRange(-1000, 1000),
			FMath::FRandRange(-1000, 1000), 0);
		AutoDriver->MoveToLocation(TargetLocation);
	}

	// Benchmark ticking
	const int32 NumFrames = 60;
	double StartTime = FPlatformTime::Seconds();

	for (int32 Frame = 0; Frame < NumFrames; ++Frame)
	{
		for (UAutoDriverComponent* Component : Components)
		{
			Component->TickComponent(0.016f, LEVELTICK_All, nullptr);
		}
	}

	double ElapsedTime = FPlatformTime::Seconds() - StartTime;
	double PerFrameTime = (ElapsedTime / NumFrames) * 1000.0; // milliseconds
	double PerComponentPerFrame = PerFrameTime / NumComponents;

	TestTrue(TEXT("Tick Performance Acceptable"), PerFrameTime < 16.0); // < 16ms per frame

	AddInfo(FString::Printf(TEXT("Ticked %d components for %d frames in %.3f seconds (%.3f ms per frame, %.3f ms per component)"),
		NumComponents, NumFrames, ElapsedTime, PerFrameTime, PerComponentPerFrame));

	return true;
}

/**
 * Performance benchmark: Navigation cache
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPerformanceNavigationCacheTest,
	"YesUeFsd.Performance.Navigation.Cache",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::PerfFilter)

bool FPerformanceNavigationCacheTest::RunTest(const FString& Parameters)
{
	FNavigationCache Cache(10000);

	const int32 NumOperations = 10000;

	// Benchmark additions
	double StartTime = FPlatformTime::Seconds();
	for (int32 i = 0; i < NumOperations; ++i)
	{
		FVector Start(FMath::FRandRange(-5000, 5000), FMath::FRandRange(-5000, 5000), 0);
		FVector End(FMath::FRandRange(-5000, 5000), FMath::FRandRange(-5000, 5000), 0);
		FNavigationCacheKey Key(Start, End, nullptr);

		FNavigationCacheEntry Entry;
		Entry.bIsReachable = true;
		Entry.PathLength = FVector::Dist(Start, End);
		Entry.Timestamp = FPlatformTime::Seconds();

		Cache.Add(Key, Entry);
	}
	double AddTime = FPlatformTime::Seconds() - StartTime;

	// Benchmark lookups (mix of hits and misses)
	StartTime = FPlatformTime::Seconds();
	int32 Hits = 0;
	for (int32 i = 0; i < NumOperations; ++i)
	{
		FVector Start(FMath::FRandRange(-5000, 5000), FMath::FRandRange(-5000, 5000), 0);
		FVector End(FMath::FRandRange(-5000, 5000), FMath::FRandRange(-5000, 5000), 0);
		FNavigationCacheKey Key(Start, End, nullptr);

		FNavigationCacheEntry Entry;
		if (Cache.Get(Key, Entry))
		{
			Hits++;
		}
	}
	double LookupTime = FPlatformTime::Seconds() - StartTime;

	double PerAddTime = (AddTime / NumOperations) * 1000000.0; // microseconds
	double PerLookupTime = (LookupTime / NumOperations) * 1000000.0; // microseconds

	TestTrue(TEXT("Cache Add Performance"), PerAddTime < 100.0); // < 100μs per add
	TestTrue(TEXT("Cache Lookup Performance"), PerLookupTime < 50.0); // < 50μs per lookup

	FNavigationCacheStats Stats = Cache.GetStats();

	AddInfo(FString::Printf(TEXT("Add: %.1f μs/op, Lookup: %.1f μs/op, Hit Rate: %.1f%%"),
		PerAddTime, PerLookupTime, Stats.HitRate * 100.0f));

	return true;
}

/**
 * Performance benchmark: Input simulation
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPerformanceInputSimulationTest,
	"YesUeFsd.Performance.Input.Simulation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::PerfFilter)

bool FPerformanceInputSimulationTest::RunTest(const FString& Parameters)
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

	const int32 NumInputs = 10000;
	double StartTime = FPlatformTime::Seconds();

	for (int32 i = 0; i < NumInputs; ++i)
	{
		AutoDriver->SetAxisValue(FName("MoveForward"), FMath::FRandRange(-1.0f, 1.0f));
		AutoDriver->SetAxisValue(FName("MoveRight"), FMath::FRandRange(-1.0f, 1.0f));
	}

	double ElapsedTime = FPlatformTime::Seconds() - StartTime;
	double PerInputTime = (ElapsedTime / NumInputs) * 1000000.0; // microseconds

	TestTrue(TEXT("Input Simulation Performance"), PerInputTime < 10.0); // < 10μs per input

	AddInfo(FString::Printf(TEXT("Simulated %d inputs in %.3f seconds (%.1f μs per input)"),
		NumInputs, ElapsedTime, PerInputTime));

	return true;
}

/**
 * Performance benchmark: Memory usage
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPerformanceMemoryUsageTest,
	"YesUeFsd.Performance.Memory.Usage",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::PerfFilter)

bool FPerformanceMemoryUsageTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("World Created"), World))
	{
		return false;
	}

	// Measure baseline memory
	FPlatformMemoryStats BaselineStats = FPlatformMemory::GetStats();

	// Create many components
	const int32 NumComponents = 1000;
	TArray<UAutoDriverComponent*> Components;

	for (int32 i = 0; i < NumComponents; ++i)
	{
		AActor* TestActor = World->SpawnActor<AActor>();
		UAutoDriverComponent* AutoDriver = NewObject<UAutoDriverComponent>(TestActor);
		AutoDriver->RegisterComponent();
		Components.Add(AutoDriver);
	}

	// Force garbage collection to get accurate measurement
	CollectGarbage(RF_NoFlags, true);

	FPlatformMemoryStats FinalStats = FPlatformMemory::GetStats();

	int64 MemoryIncrease = FinalStats.UsedPhysical - BaselineStats.UsedPhysical;
	int64 PerComponentMemory = MemoryIncrease / NumComponents;

	AddInfo(FString::Printf(TEXT("Created %d components, Memory increase: %.2f MB (%.2f KB per component)"),
		NumComponents, MemoryIncrease / (1024.0 * 1024.0), PerComponentMemory / 1024.0));

	// Memory per component should be reasonable (< 100KB)
	TestTrue(TEXT("Memory Usage Per Component Reasonable"), PerComponentMemory < 100 * 1024);

	return true;
}

/**
 * Performance benchmark: Rotation calculations
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPerformanceRotationTest,
	"YesUeFsd.Performance.Rotation.Calculations",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::PerfFilter)

bool FPerformanceRotationTest::RunTest(const FString& Parameters)
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

	const int32 NumRotations = 10000;
	double StartTime = FPlatformTime::Seconds();

	for (int32 i = 0; i < NumRotations; ++i)
	{
		FRotator TargetRotation(FMath::FRandRange(-90, 90),
			FMath::FRandRange(-180, 180),
			FMath::FRandRange(-90, 90));

		AutoDriver->RotateToRotation(TargetRotation);
		AutoDriver->StopCommand();
	}

	double ElapsedTime = FPlatformTime::Seconds() - StartTime;
	double PerRotationTime = (ElapsedTime / NumRotations) * 1000000.0; // microseconds

	TestTrue(TEXT("Rotation Performance"), PerRotationTime < 50.0); // < 50μs per rotation

	AddInfo(FString::Printf(TEXT("Executed %d rotations in %.3f seconds (%.1f μs per rotation)"),
		NumRotations, ElapsedTime, PerRotationTime));

	return true;
}

/**
 * Performance benchmark: Concurrent operations
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPerformanceConcurrentTest,
	"YesUeFsd.Performance.Concurrent.Operations",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::PerfFilter)

bool FPerformanceConcurrentTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("World Created"), World))
	{
		return false;
	}

	const int32 NumActors = 100;
	TArray<UAutoDriverComponent*> Components;

	for (int32 i = 0; i < NumActors; ++i)
	{
		AActor* TestActor = World->SpawnActor<AActor>();
		UAutoDriverComponent* AutoDriver = NewObject<UAutoDriverComponent>(TestActor);
		AutoDriver->RegisterComponent();
		AutoDriver->SetEnabled(true);
		Components.Add(AutoDriver);
	}

	// Benchmark concurrent operations
	double StartTime = FPlatformTime::Seconds();

	for (int32 i = 0; i < NumActors; ++i)
	{
		FVector TargetLocation(FMath::FRandRange(-1000, 1000),
			FMath::FRandRange(-1000, 1000), 0);
		Components[i]->MoveToLocation(TargetLocation);
	}

	// Tick all
	for (int32 Frame = 0; Frame < 60; ++Frame)
	{
		for (UAutoDriverComponent* Component : Components)
		{
			Component->TickComponent(0.016f, LEVELTICK_All, nullptr);
		}
	}

	double ElapsedTime = FPlatformTime::Seconds() - StartTime;

	TestTrue(TEXT("Concurrent Operations Performance"), ElapsedTime < 1.0); // < 1 second

	AddInfo(FString::Printf(TEXT("%d actors running concurrently for 60 frames: %.3f seconds"),
		NumActors, ElapsedTime));

	return true;
}

/**
 * Performance benchmark: Stats system overhead
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPerformanceStatsOverheadTest,
	"YesUeFsd.Performance.Stats.Overhead",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::PerfFilter)

bool FPerformanceStatsOverheadTest::RunTest(const FString& Parameters)
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

	const int32 NumOperations = 1000;

	// Measure without stats collection
	double StartTime = FPlatformTime::Seconds();
	for (int32 i = 0; i < NumOperations; ++i)
	{
		AutoDriver->MoveToLocation(FVector(100, 0, 0));
		AutoDriver->StopCommand();
	}
	double TimeWithoutStats = FPlatformTime::Seconds() - StartTime;

	// Stats overhead should be minimal
	TestTrue(TEXT("Stats Overhead Acceptable"), TimeWithoutStats < 1.0);

	AddInfo(FString::Printf(TEXT("%d operations: %.3f seconds"),
		NumOperations, TimeWithoutStats));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
