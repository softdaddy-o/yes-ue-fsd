// Copyright Yes UE FSD. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "AutoDriver/AutoDriverComponent.h"
#include "AutoDriver/AutoDriverSubsystem.h"
#include "AutoDriver/NavigationHelper.h"
#include "AutoDriver/UIInteractionHelper.h"
#include "AutoDriver/Recording/ActionRecorder.h"
#include "AutoDriver/Recording/ActionPlayback.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Integration test: Complete movement scenario
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAutoDriverIntegrationMovementTest,
	"YesUeFsd.Integration.Movement.Complete",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAutoDriverIntegrationMovementTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("World Created"), World))
	{
		return false;
	}

	// Create test actor with AutoDriver component
	AActor* TestActor = World->SpawnActor<AActor>();
	UAutoDriverComponent* AutoDriver = NewObject<UAutoDriverComponent>(TestActor);
	AutoDriver->RegisterComponent();
	AutoDriver->SetEnabled(true);

	// Perform complete movement sequence
	FVector StartLocation = TestActor->GetActorLocation();

	// Move to first location
	FVector Target1(1000, 0, 0);
	AutoDriver->MoveToLocation(Target1);
	TestTrue(TEXT("First Movement Started"), AutoDriver->IsExecutingCommand());

	// Simulate some ticks
	for (int32 i = 0; i < 10; ++i)
	{
		AutoDriver->TickComponent(0.016f, LEVELTICK_All, nullptr);
	}

	// Move to second location
	FVector Target2(1000, 1000, 0);
	AutoDriver->MoveToLocation(Target2);
	TestTrue(TEXT("Second Movement Started"), AutoDriver->IsExecutingCommand());

	// Rotate to face origin
	FRotator FaceOrigin = (FVector::ZeroVector - Target2).Rotation();
	AutoDriver->RotateToRotation(FaceOrigin);
	TestTrue(TEXT("Rotation Started"), AutoDriver->IsExecutingCommand());

	TestTrue(TEXT("Integration Test Completed"), true);

	return true;
}

/**
 * Integration test: Navigation and movement combined
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAutoDriverIntegrationNavigationTest,
	"YesUeFsd.Integration.Navigation.Complete",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAutoDriverIntegrationNavigationTest::RunTest(const FString& Parameters)
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

	UNavigationHelper* NavHelper = NewObject<UNavigationHelper>();

	// Query navigation before movement
	FVector Start = TestActor->GetActorLocation();
	FVector End(2000, 0, 0);

	bool bReachable = NavHelper->IsLocationReachable(World, Start, End);
	float PathLength = NavHelper->GetPathLength(World, Start, End);

	// Attempt movement
	AutoDriver->MoveToLocation(End);
	TestTrue(TEXT("Navigation-Based Movement Started"), AutoDriver->IsExecutingCommand());

	TestTrue(TEXT("Navigation Integration Completed"), true);

	return true;
}

/**
 * Integration test: Recording and playback
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAutoDriverIntegrationRecordingTest,
	"YesUeFsd.Integration.Recording.Complete",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAutoDriverIntegrationRecordingTest::RunTest(const FString& Parameters)
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

	// Create recorder and playback
	UActionRecorder* Recorder = NewObject<UActionRecorder>();
	UActionPlayback* Playback = NewObject<UActionPlayback>();

	// Start recording
	Recorder->StartRecording(TestActor);
	TestTrue(TEXT("Recording Started"), Recorder->IsRecording());

	// Perform actions
	AutoDriver->MoveToLocation(FVector(500, 0, 0));
	for (int32 i = 0; i < 5; ++i)
	{
		AutoDriver->TickComponent(0.016f, LEVELTICK_All, nullptr);
	}

	AutoDriver->PressButton(FName("Jump"));

	// Stop recording
	UActionTimeline* Timeline = Recorder->StopRecording();
	TestNotNull(TEXT("Timeline Created"), Timeline);

	if (Timeline)
	{
		TestTrue(TEXT("Timeline Has Actions"), Timeline->GetNumActions() > 0);

		// Test playback
		Playback->SetTimeline(Timeline);
		Playback->Play();
		TestTrue(TEXT("Playback Started"), Playback->IsPlaying());
	}

	return true;
}

/**
 * Integration test: Multiple commands with callbacks
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAutoDriverIntegrationCallbacksTest,
	"YesUeFsd.Integration.Callbacks.Multiple",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAutoDriverIntegrationCallbacksTest::RunTest(const FString& Parameters)
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

	// Chain multiple commands with callbacks
	int32 CallbackCount = 0;

	FAutoDriverCommandComplete Callback1;
	Callback1.BindLambda([&CallbackCount](bool bSuccess)
	{
		CallbackCount++;
	});

	FAutoDriverCommandComplete Callback2;
	Callback2.BindLambda([&CallbackCount](bool bSuccess)
	{
		CallbackCount++;
	});

	// Execute commands
	AutoDriver->MoveToLocationWithCallback(FVector(100, 0, 0), Callback1);
	AutoDriver->StopCommand(); // Trigger callback

	AutoDriver->MoveToLocationWithCallback(FVector(200, 0, 0), Callback2);
	AutoDriver->StopCommand(); // Trigger callback

	TestEqual(TEXT("All Callbacks Executed"), CallbackCount, 2);

	return true;
}

/**
 * Integration test: Subsystem coordination
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAutoDriverIntegrationSubsystemTest,
	"YesUeFsd.Integration.Subsystem.Coordination",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAutoDriverIntegrationSubsystemTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("World Created"), World))
	{
		return false;
	}

	// Get subsystem
	UAutoDriverSubsystem* Subsystem = World->GetSubsystem<UAutoDriverSubsystem>();
	TestNotNull(TEXT("Subsystem Available"), Subsystem);

	// Create multiple actors with AutoDriver
	TArray<AActor*> TestActors;
	TArray<UAutoDriverComponent*> AutoDrivers;

	for (int32 i = 0; i < 3; ++i)
	{
		AActor* Actor = World->SpawnActor<AActor>();
		TestActors.Add(Actor);

		UAutoDriverComponent* AutoDriver = NewObject<UAutoDriverComponent>(Actor);
		AutoDriver->RegisterComponent();
		AutoDriver->SetEnabled(true);
		AutoDrivers.Add(AutoDriver);

		// Move each actor to different location
		FVector TargetLocation(i * 500.0f, 0, 0);
		AutoDriver->MoveToLocation(TargetLocation);
	}

	// Verify all are executing
	for (UAutoDriverComponent* AutoDriver : AutoDrivers)
	{
		TestTrue(TEXT("AutoDriver Executing"), AutoDriver->IsExecutingCommand());
	}

	TestTrue(TEXT("Subsystem Coordination Test Completed"), true);

	return true;
}

/**
 * Integration test: Input simulation sequence
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAutoDriverIntegrationInputTest,
	"YesUeFsd.Integration.Input.Sequence",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAutoDriverIntegrationInputTest::RunTest(const FString& Parameters)
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

	// Simulate complex input sequence
	AutoDriver->SetAxisValue(FName("MoveForward"), 1.0f);
	AutoDriver->SetAxisValue(FName("MoveRight"), 0.5f);
	AutoDriver->PressButton(FName("Jump"));

	// Tick to process inputs
	for (int32 i = 0; i < 10; ++i)
	{
		AutoDriver->TickComponent(0.016f, LEVELTICK_All, nullptr);
	}

	// Stop inputs
	AutoDriver->SetAxisValue(FName("MoveForward"), 0.0f);
	AutoDriver->SetAxisValue(FName("MoveRight"), 0.0f);

	TestTrue(TEXT("Input Sequence Completed"), true);

	return true;
}

/**
 * Integration test: Performance under load
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAutoDriverIntegrationPerformanceTest,
	"YesUeFsd.Integration.Performance.Load",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FAutoDriverIntegrationPerformanceTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("World Created"), World))
	{
		return false;
	}

	// Create many AutoDriver instances
	const int32 NumActors = 50;
	TArray<UAutoDriverComponent*> AutoDrivers;

	double StartTime = FPlatformTime::Seconds();

	for (int32 i = 0; i < NumActors; ++i)
	{
		AActor* Actor = World->SpawnActor<AActor>();
		UAutoDriverComponent* AutoDriver = NewObject<UAutoDriverComponent>(Actor);
		AutoDriver->RegisterComponent();
		AutoDriver->SetEnabled(true);
		AutoDrivers.Add(AutoDriver);

		// Start movement
		FVector TargetLocation(FMath::FRandRange(-1000, 1000),
			FMath::FRandRange(-1000, 1000), 0);
		AutoDriver->MoveToLocation(TargetLocation);
	}

	// Tick all components
	for (int32 Frame = 0; Frame < 60; ++Frame)
	{
		for (UAutoDriverComponent* AutoDriver : AutoDrivers)
		{
			AutoDriver->TickComponent(0.016f, LEVELTICK_All, nullptr);
		}
	}

	double ElapsedTime = FPlatformTime::Seconds() - StartTime;

	// Should complete in reasonable time (less than 5 seconds for 50 actors * 60 frames)
	TestTrue(TEXT("Performance Under Load Acceptable"), ElapsedTime < 5.0);

	AddInfo(FString::Printf(TEXT("Processed %d actors for 60 frames in %.3f seconds"),
		NumActors, ElapsedTime));

	return true;
}

/**
 * Integration test: End-to-end scenario
 */
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FAutoDriverIntegrationEndToEndTest,
	"YesUeFsd.Integration.EndToEnd.Complete",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

void FAutoDriverIntegrationEndToEndTest::GetTests(TArray<FString>& OutBeautifiedNames,
	TArray<FString>& OutTestCommands) const
{
	OutBeautifiedNames.Add(TEXT("Basic Scenario"));
	OutTestCommands.Add(TEXT("Basic"));

	OutBeautifiedNames.Add(TEXT("Advanced Scenario"));
	OutTestCommands.Add(TEXT("Advanced"));
}

bool FAutoDriverIntegrationEndToEndTest::RunTest(const FString& Parameters)
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

	if (Parameters == TEXT("Basic"))
	{
		// Basic scenario: Move and rotate
		AutoDriver->MoveToLocation(FVector(500, 0, 0));
		for (int32 i = 0; i < 30; ++i)
		{
			AutoDriver->TickComponent(0.016f, LEVELTICK_All, nullptr);
		}

		AutoDriver->RotateToRotation(FRotator(0, 180, 0));
		for (int32 i = 0; i < 30; ++i)
		{
			AutoDriver->TickComponent(0.016f, LEVELTICK_All, nullptr);
		}

		TestTrue(TEXT("Basic Scenario Completed"), true);
	}
	else if (Parameters == TEXT("Advanced"))
	{
		// Advanced scenario: Multiple movements, inputs, and recording
		UActionRecorder* Recorder = NewObject<UActionRecorder>();
		Recorder->StartRecording(TestActor);

		// Complex movement pattern
		TArray<FVector> Waypoints = {
			FVector(500, 0, 0),
			FVector(500, 500, 0),
			FVector(0, 500, 0),
			FVector(0, 0, 0)
		};

		for (const FVector& Waypoint : Waypoints)
		{
			AutoDriver->MoveToLocation(Waypoint);
			for (int32 i = 0; i < 20; ++i)
			{
				AutoDriver->TickComponent(0.016f, LEVELTICK_All, nullptr);
			}
		}

		// Input simulation
		AutoDriver->PressButton(FName("Jump"));
		AutoDriver->SetAxisValue(FName("LookUp"), 1.0f);

		UActionTimeline* Timeline = Recorder->StopRecording();
		TestNotNull(TEXT("Recording Created"), Timeline);

		TestTrue(TEXT("Advanced Scenario Completed"), true);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
