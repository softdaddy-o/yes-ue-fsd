// Copyright Epic Games, Inc. All Rights Reserved.

#include "Examples/EnhancedInputExample.h"
#include "AutoDriver/InputSimulator.h"
#include "AutoDriver/EnhancedInputAdapter.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

AEnhancedInputExample::AEnhancedInputExample()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEnhancedInputExample::BeginPlay()
{
	Super::BeginPlay();

	// Auto-setup can be triggered here or manually via Blueprint
	// Example_BasicSetup();
}

APlayerController* AEnhancedInputExample::GetPlayerController() const
{
	return GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
}

void AEnhancedInputExample::SetupCommonMappings()
{
	if (!EnhancedInputAdapter || !EnhancedInputAdapter->IsInitialized())
	{
		UE_LOG(LogTemp, Error, TEXT("Enhanced Input Adapter not initialized"));
		return;
	}

	TArray<FEnhancedInputActionMapping> Mappings;

	// Jump action
	if (IA_Jump)
	{
		FEnhancedInputActionMapping& Jump = Mappings.AddDefaulted_GetRef();
		Jump.ActionName = "Jump";
		Jump.InputAction = IA_Jump;
		Jump.MappingContext = IMC_Default;
		Jump.Priority = 0;
	}

	// Move action
	if (IA_Move)
	{
		FEnhancedInputActionMapping& Move = Mappings.AddDefaulted_GetRef();
		Move.ActionName = "Move";
		Move.InputAction = IA_Move;
		Move.MappingContext = IMC_Default;
		Move.Priority = 0;
	}

	// Look action
	if (IA_Look)
	{
		FEnhancedInputActionMapping& Look = Mappings.AddDefaulted_GetRef();
		Look.ActionName = "Look";
		Look.InputAction = IA_Look;
		Look.MappingContext = IMC_Default;
		Look.Priority = 0;
	}

	// Interact action
	if (IA_Interact)
	{
		FEnhancedInputActionMapping& Interact = Mappings.AddDefaulted_GetRef();
		Interact.ActionName = "Interact";
		Interact.InputAction = IA_Interact;
		Interact.MappingContext = IMC_Default;
		Interact.Priority = 0;
	}

	EnhancedInputAdapter->RegisterActionMappings(Mappings);
	UE_LOG(LogTemp, Log, TEXT("Registered %d action mappings"), Mappings.Num());
}

// ========================================
// Example 1: Basic Setup
// ========================================

void AEnhancedInputExample::Example_BasicSetup()
{
	UE_LOG(LogTemp, Display, TEXT("=== Example 1: Basic Setup ==="));

	// Get player controller
	APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("No PlayerController found"));
		return;
	}

	// Create input simulator with auto-detection
	InputSimulator = UInputSimulator::CreateInputSimulator(this, PC);

	if (!InputSimulator)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create InputSimulator"));
		return;
	}

	// Check if Enhanced Input is being used
	if (InputSimulator->IsUsingEnhancedInput())
	{
		UE_LOG(LogTemp, Display, TEXT("Enhanced Input System detected and active"));

		// Get the adapter
		EnhancedInputAdapter = InputSimulator->GetEnhancedInputAdapter();

		if (EnhancedInputAdapter)
		{
			UE_LOG(LogTemp, Display, TEXT("Enhanced Input Adapter ready"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Using legacy input mode"));
	}
}

// ========================================
// Example 2: Action Registration
// ========================================

void AEnhancedInputExample::Example_RegisterActions()
{
	UE_LOG(LogTemp, Display, TEXT("=== Example 2: Register Actions ==="));

	if (!EnhancedInputAdapter)
	{
		Example_BasicSetup();
	}

	SetupCommonMappings();

	// Verify registration
	UInputAction* FoundAction = EnhancedInputAdapter->FindInputAction("Jump");
	if (FoundAction)
	{
		UE_LOG(LogTemp, Display, TEXT("Successfully registered and found Jump action: %s"),
			*FoundAction->GetName());
	}

	// Log all registered mappings
	const TArray<FEnhancedInputActionMapping>& Mappings = EnhancedInputAdapter->GetActionMappings();
	UE_LOG(LogTemp, Display, TEXT("Total registered actions: %d"), Mappings.Num());

	for (const FEnhancedInputActionMapping& Mapping : Mappings)
	{
		UE_LOG(LogTemp, Display, TEXT("  - %s -> %s"),
			*Mapping.ActionName.ToString(),
			Mapping.InputAction ? *Mapping.InputAction->GetName() : TEXT("NULL"));
	}
}

// ========================================
// Example 3: Inject Buttons
// ========================================

void AEnhancedInputExample::Example_InjectButtons()
{
	UE_LOG(LogTemp, Display, TEXT("=== Example 3: Inject Buttons ==="));

	if (!InputSimulator)
	{
		Example_BasicSetup();
	}

	// Press jump
	UE_LOG(LogTemp, Display, TEXT("Pressing Jump button..."));
	InputSimulator->PressButton("Jump");

	// Wait a moment, then release
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		UE_LOG(LogTemp, Display, TEXT("Releasing Jump button..."));
		InputSimulator->ReleaseButton("Jump");
	}, 0.2f, false);

	// Press and auto-release after duration
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		UE_LOG(LogTemp, Display, TEXT("Press and hold Jump for 0.5 seconds..."));
		InputSimulator->PressAndHoldButton("Jump", 0.5f);
	}, 1.0f, false);
}

// ========================================
// Example 4: Inject Axis
// ========================================

void AEnhancedInputExample::Example_InjectAxis()
{
	UE_LOG(LogTemp, Display, TEXT("=== Example 4: Inject Axis ==="));

	if (!InputSimulator)
	{
		Example_BasicSetup();
	}

	// Move forward
	UE_LOG(LogTemp, Display, TEXT("Moving forward (value: 1.0)"));
	InputSimulator->SetAxisValue("MoveForward", 1.0f);

	// Clear after a moment
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		UE_LOG(LogTemp, Display, TEXT("Stopping forward movement"));
		InputSimulator->ClearAxisValue("MoveForward");
	}, 2.0f, false);
}

// ========================================
// Example 5: Inject Movement (2D)
// ========================================

void AEnhancedInputExample::Example_InjectMovement()
{
	UE_LOG(LogTemp, Display, TEXT("=== Example 5: Inject 2D Movement ==="));

	if (!InputSimulator)
	{
		Example_BasicSetup();
	}

	// Move forward-right
	FVector2D MoveVector(0.707f, 0.707f); // 45-degree angle
	UE_LOG(LogTemp, Display, TEXT("Moving at 45-degree angle: (%.2f, %.2f)"),
		MoveVector.X, MoveVector.Y);

	InputSimulator->SetAxis2DValue("Move", MoveVector);

	// Clear after movement
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		UE_LOG(LogTemp, Display, TEXT("Stopping movement"));
		InputSimulator->SetAxis2DValue("Move", FVector2D::ZeroVector);
	}, 1.5f, false);
}

// ========================================
// Example 6: Manage Contexts
// ========================================

void AEnhancedInputExample::Example_ManageContexts()
{
	UE_LOG(LogTemp, Display, TEXT("=== Example 6: Manage Contexts ==="));

	if (!EnhancedInputAdapter)
	{
		Example_BasicSetup();
	}

	// Add default context
	if (IMC_Default)
	{
		UE_LOG(LogTemp, Display, TEXT("Adding Default mapping context"));
		EnhancedInputAdapter->AddMappingContext(IMC_Default, 0);
	}

	// Get active contexts
	TArray<UInputMappingContext*> ActiveContexts = EnhancedInputAdapter->GetActiveMappingContexts();
	UE_LOG(LogTemp, Display, TEXT("Active contexts: %d"), ActiveContexts.Num());

	for (UInputMappingContext* Context : ActiveContexts)
	{
		if (Context)
		{
			UE_LOG(LogTemp, Display, TEXT("  - %s"), *Context->GetName());
		}
	}
}

// ========================================
// Example 7: Context Switching
// ========================================

void AEnhancedInputExample::Example_ContextSwitching()
{
	UE_LOG(LogTemp, Display, TEXT("=== Example 7: Context Switching ==="));

	if (!EnhancedInputAdapter)
	{
		Example_BasicSetup();
	}

	// Start with default context
	if (IMC_Default)
	{
		UE_LOG(LogTemp, Display, TEXT("Switching to Default context"));
		EnhancedInputAdapter->AddMappingContext(IMC_Default, 0);
	}

	// Switch to UI context after 2 seconds
	FTimerHandle Timer1;
	GetWorld()->GetTimerManager().SetTimer(Timer1, [this]()
	{
		if (IMC_UI)
		{
			UE_LOG(LogTemp, Display, TEXT("Switching to UI context (higher priority)"));
			EnhancedInputAdapter->AddMappingContext(IMC_UI, 10);
		}
	}, 2.0f, false);

	// Switch to Combat context after 4 seconds
	FTimerHandle Timer2;
	GetWorld()->GetTimerManager().SetTimer(Timer2, [this]()
	{
		if (IMC_Combat)
		{
			UE_LOG(LogTemp, Display, TEXT("Switching to Combat context"));
			// Remove UI first
			if (IMC_UI)
			{
				EnhancedInputAdapter->RemoveMappingContext(IMC_UI);
			}
			EnhancedInputAdapter->AddMappingContext(IMC_Combat, 5);
		}
	}, 4.0f, false);

	// Clear all after 6 seconds
	FTimerHandle Timer3;
	GetWorld()->GetTimerManager().SetTimer(Timer3, [this]()
	{
		UE_LOG(LogTemp, Display, TEXT("Clearing all contexts"));
		EnhancedInputAdapter->ClearAllMappingContexts();
	}, 6.0f, false);
}

// ========================================
// Example 8: Record Input
// ========================================

void AEnhancedInputExample::Example_RecordInput()
{
	UE_LOG(LogTemp, Display, TEXT("=== Example 8: Record Input ==="));

	if (!EnhancedInputAdapter)
	{
		Example_BasicSetup();
		Example_RegisterActions();
	}

	// Start recording
	UE_LOG(LogTemp, Display, TEXT("Starting recording..."));
	EnhancedInputAdapter->StartRecording();

	// Simulate some actions
	InputSimulator->PressButton("Jump");

	FTimerHandle Timer1;
	GetWorld()->GetTimerManager().SetTimer(Timer1, [this]()
	{
		InputSimulator->ReleaseButton("Jump");
		InputSimulator->SetAxis2DValue("Move", FVector2D(1.0f, 0.5f));
	}, 0.5f, false);

	FTimerHandle Timer2;
	GetWorld()->GetTimerManager().SetTimer(Timer2, [this]()
	{
		InputSimulator->SetAxis2DValue("Move", FVector2D::ZeroVector);
	}, 2.0f, false);

	// Stop recording and export
	FTimerHandle Timer3;
	GetWorld()->GetTimerManager().SetTimer(Timer3, [this]()
	{
		UE_LOG(LogTemp, Display, TEXT("Stopping recording..."));
		EnhancedInputAdapter->StopRecording();

		LogRecordingStats();

		// Export to JSON
		FString JSONString = EnhancedInputAdapter->ExportRecordingToJSON();
		UE_LOG(LogTemp, Display, TEXT("Exported recording (%d characters)"), JSONString.Len());

		// Save to file
		RecordingFilePath = FPaths::ProjectSavedDir() / TEXT("RecordedInput.json");
		if (FFileHelper::SaveStringToFile(JSONString, *RecordingFilePath))
		{
			UE_LOG(LogTemp, Display, TEXT("Saved recording to: %s"), *RecordingFilePath);
		}
	}, 3.0f, false);
}

// ========================================
// Example 9: Import Recording
// ========================================

void AEnhancedInputExample::Example_ImportRecording()
{
	UE_LOG(LogTemp, Display, TEXT("=== Example 9: Import Recording ==="));

	if (!EnhancedInputAdapter)
	{
		Example_BasicSetup();
	}

	// Load from file
	FString JSONString;
	FString FilePath = FPaths::ProjectSavedDir() / TEXT("RecordedInput.json");

	if (FFileHelper::LoadFileToString(JSONString, *FilePath))
	{
		UE_LOG(LogTemp, Display, TEXT("Loaded recording from: %s"), *FilePath);

		// Clear existing recording
		EnhancedInputAdapter->ClearRecordedActions();

		// Import
		if (EnhancedInputAdapter->ImportRecordingFromJSON(JSONString))
		{
			UE_LOG(LogTemp, Display, TEXT("Successfully imported recording"));
			LogRecordingStats();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to import recording"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No recording file found at: %s"), *FilePath);
		UE_LOG(LogTemp, Display, TEXT("Run Example_RecordInput first to create a recording"));
	}
}

// ========================================
// Example 10: Automated Sequence
// ========================================

void AEnhancedInputExample::Example_AutomatedSequence()
{
	UE_LOG(LogTemp, Display, TEXT("=== Example 10: Automated Sequence ==="));

	if (!InputSimulator)
	{
		Example_BasicSetup();
		Example_RegisterActions();
	}

	UE_LOG(LogTemp, Display, TEXT("Starting automated movement sequence..."));

	int32 Step = 0;

	// Create repeating timer for sequence steps
	GetWorld()->GetTimerManager().SetTimer(SequenceTimerHandle, [this, Step]() mutable
	{
		switch (Step)
		{
		case 0:
			UE_LOG(LogTemp, Display, TEXT("Step 1: Move forward"));
			InputSimulator->SetMoveForward(1.0f);
			break;

		case 1:
			UE_LOG(LogTemp, Display, TEXT("Step 2: Jump while moving"));
			InputSimulator->Jump();
			break;

		case 2:
			UE_LOG(LogTemp, Display, TEXT("Step 3: Turn right"));
			InputSimulator->SetMoveForward(0.0f);
			InputSimulator->SetLookRight(1.0f);
			break;

		case 3:
			UE_LOG(LogTemp, Display, TEXT("Step 4: Move right"));
			InputSimulator->SetLookRight(0.0f);
			InputSimulator->SetMoveRight(1.0f);
			break;

		case 4:
			UE_LOG(LogTemp, Display, TEXT("Step 5: Stop all movement"));
			InputSimulator->ClearAllInput();
			GetWorld()->GetTimerManager().ClearTimer(SequenceTimerHandle);
			UE_LOG(LogTemp, Display, TEXT("Sequence complete!"));
			break;
		}

		Step++;
	}, 1.0f, true, 0.0f);
}

// ========================================
// Example 11: Complex Movement
// ========================================

void AEnhancedInputExample::Example_ComplexMovement()
{
	UE_LOG(LogTemp, Display, TEXT("=== Example 11: Complex Movement Pattern ==="));

	if (!InputSimulator)
	{
		Example_BasicSetup();
	}

	UE_LOG(LogTemp, Display, TEXT("Executing circle movement pattern..."));

	float Time = 0.0f;
	const float Duration = 5.0f; // 5 second circle
	const float Radius = 1.0f;

	// Create timer for smooth circular movement
	FTimerHandle CircleTimer;
	GetWorld()->GetTimerManager().SetTimer(CircleTimer, [this, Time, Duration, Radius]() mutable
	{
		if (Time >= Duration)
		{
			// Stop movement
			InputSimulator->SetAxis2DValue("Move", FVector2D::ZeroVector);
			UE_LOG(LogTemp, Display, TEXT("Circle pattern complete"));
			return;
		}

		// Calculate position on circle
		float Angle = (Time / Duration) * 2.0f * PI;
		FVector2D MoveDir(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius);

		InputSimulator->SetAxis2DValue("Move", MoveDir);

		Time += 0.1f;

	}, 0.1f, true);
}

// ========================================
// Helper Methods
// ========================================

void AEnhancedInputExample::LogRecordingStats() const
{
	if (!EnhancedInputAdapter)
	{
		return;
	}

	const TArray<FInputActionRecord>& Records = EnhancedInputAdapter->GetRecordedActions();

	UE_LOG(LogTemp, Display, TEXT("Recording Statistics:"));
	UE_LOG(LogTemp, Display, TEXT("  Total events: %d"), Records.Num());

	if (Records.Num() > 0)
	{
		UE_LOG(LogTemp, Display, TEXT("  Duration: %.2f seconds"),
			Records.Last().Timestamp);

		// Count by type
		int32 TriggeredCount = 0;
		int32 StartedCount = 0;
		int32 CompletedCount = 0;

		for (const FInputActionRecord& Record : Records)
		{
			if (Record.bTriggered) TriggeredCount++;
			if (Record.bStarted) StartedCount++;
			if (Record.bCompleted) CompletedCount++;
		}

		UE_LOG(LogTemp, Display, TEXT("  Triggered events: %d"), TriggeredCount);
		UE_LOG(LogTemp, Display, TEXT("  Started events: %d"), StartedCount);
		UE_LOG(LogTemp, Display, TEXT("  Completed events: %d"), CompletedCount);
	}
}
