// Copyright Yes UE FSD. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "AutoDriver/AutoDriverComponent.h"
#include "AutoDriver/AutoDriverSubsystem.h"
#include "AutoDriver/Commands/MoveToLocationCommand.h"
#include "AutoDriver/Commands/RotateToCommand.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAutoDriverComponentCreationTest,
	"YesUeFsd.AutoDriver.Component.Creation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAutoDriverComponentCreationTest::RunTest(const FString& Parameters)
{
	// Test component creation and initialization
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("World Created"), World))
	{
		return false;
	}

	AActor* TestActor = World->SpawnActor<AActor>();
	if (!TestNotNull(TEXT("Actor Created"), TestActor))
	{
		return false;
	}

	UAutoDriverComponent* AutoDriver = NewObject<UAutoDriverComponent>(TestActor);
	if (!TestNotNull(TEXT("AutoDriverComponent Created"), AutoDriver))
	{
		return false;
	}

	AutoDriver->RegisterComponent();
	TestTrue(TEXT("Component Registered"), AutoDriver->IsRegistered());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAutoDriverComponentEnableDisableTest,
	"YesUeFsd.AutoDriver.Component.EnableDisable",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAutoDriverComponentEnableDisableTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("World Created"), World))
	{
		return false;
	}

	AActor* TestActor = World->SpawnActor<AActor>();
	UAutoDriverComponent* AutoDriver = NewObject<UAutoDriverComponent>(TestActor);
	AutoDriver->RegisterComponent();

	// Test enabling/disabling
	AutoDriver->SetEnabled(true);
	TestTrue(TEXT("AutoDriver Enabled"), AutoDriver->IsEnabled());

	AutoDriver->SetEnabled(false);
	TestFalse(TEXT("AutoDriver Disabled"), AutoDriver->IsEnabled());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAutoDriverMoveToLocationTest,
	"YesUeFsd.AutoDriver.Commands.MoveToLocation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAutoDriverMoveToLocationTest::RunTest(const FString& Parameters)
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

	// Test MoveToLocation command
	FVector TargetLocation(1000.0f, 0.0f, 0.0f);
	bool bSuccess = AutoDriver->MoveToLocation(TargetLocation);
	TestTrue(TEXT("MoveToLocation Command Started"), bSuccess);
	TestTrue(TEXT("Command Is Executing"), AutoDriver->IsExecutingCommand());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAutoDriverRotateToTest,
	"YesUeFsd.AutoDriver.Commands.RotateTo",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAutoDriverRotateToTest::RunTest(const FString& Parameters)
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

	// Test RotateTo command
	FRotator TargetRotation(0.0f, 90.0f, 0.0f);
	bool bSuccess = AutoDriver->RotateToRotation(TargetRotation);
	TestTrue(TEXT("RotateTo Command Started"), bSuccess);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAutoDriverStopCommandTest,
	"YesUeFsd.AutoDriver.Commands.Stop",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAutoDriverStopCommandTest::RunTest(const FString& Parameters)
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

	// Start a command
	FVector TargetLocation(1000.0f, 0.0f, 0.0f);
	AutoDriver->MoveToLocation(TargetLocation);
	TestTrue(TEXT("Command Started"), AutoDriver->IsExecutingCommand());

	// Stop the command
	AutoDriver->StopCommand();
	TestFalse(TEXT("Command Stopped"), AutoDriver->IsExecutingCommand());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAutoDriverMultipleCommandsTest,
	"YesUeFsd.AutoDriver.Commands.MultipleSequential",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAutoDriverMultipleCommandsTest::RunTest(const FString& Parameters)
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

	// Test that starting a new command stops the previous one
	FVector Location1(100.0f, 0.0f, 0.0f);
	AutoDriver->MoveToLocation(Location1);
	TestTrue(TEXT("First Command Started"), AutoDriver->IsExecutingCommand());

	FVector Location2(200.0f, 0.0f, 0.0f);
	AutoDriver->MoveToLocation(Location2);
	TestTrue(TEXT("Second Command Started"), AutoDriver->IsExecutingCommand());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAutoDriverSubsystemTest,
	"YesUeFsd.AutoDriver.Subsystem.Creation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAutoDriverSubsystemTest::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("World Created"), World))
	{
		return false;
	}

	UAutoDriverSubsystem* Subsystem = World->GetSubsystem<UAutoDriverSubsystem>();
	TestNotNull(TEXT("Subsystem Exists"), Subsystem);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAutoDriverInputSimulationTest,
	"YesUeFsd.AutoDriver.Input.Simulation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAutoDriverInputSimulationTest::RunTest(const FString& Parameters)
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

	// Test input simulation
	bool bSuccess = AutoDriver->PressButton(FName("Jump"));
	TestTrue(TEXT("Button Press Simulated"), bSuccess);

	bSuccess = AutoDriver->SetAxisValue(FName("MoveForward"), 1.0f);
	TestTrue(TEXT("Axis Value Set"), bSuccess);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAutoDriverCommandCallbackTest,
	"YesUeFsd.AutoDriver.Commands.Callback",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAutoDriverCommandCallbackTest::RunTest(const FString& Parameters)
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

	// Test command completion callback
	bool bCallbackExecuted = false;
	FAutoDriverCommandComplete Callback;
	Callback.BindLambda([&bCallbackExecuted](bool bSuccess)
	{
		bCallbackExecuted = true;
	});

	FVector TargetLocation(100.0f, 0.0f, 0.0f);
	AutoDriver->MoveToLocationWithCallback(TargetLocation, Callback);

	// Stop command to trigger callback
	AutoDriver->StopCommand();

	TestTrue(TEXT("Callback Was Called"), bCallbackExecuted);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAutoDriverSpeedModifiersTest,
	"YesUeFsd.AutoDriver.Commands.SpeedModifiers",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAutoDriverSpeedModifiersTest::RunTest(const FString& Parameters)
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

	// Test with different speed modifiers
	FVector TargetLocation(1000.0f, 0.0f, 0.0f);

	bool bSuccess = AutoDriver->MoveToLocationAtSpeed(TargetLocation, 0.5f);
	TestTrue(TEXT("Slow Speed Command"), bSuccess);

	AutoDriver->StopCommand();

	bSuccess = AutoDriver->MoveToLocationAtSpeed(TargetLocation, 2.0f);
	TestTrue(TEXT("Fast Speed Command"), bSuccess);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
