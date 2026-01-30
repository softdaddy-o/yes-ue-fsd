// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoDriver/InputSimulator.h"
#include "AutoDriver/EnhancedInputAdapter.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"

void UInputSimulator::Initialize(APlayerController* InPlayerController, EInputSimulatorMode Mode)
{
	if (!InPlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("InputSimulator: Cannot initialize with null PlayerController"));
		return;
	}

	PlayerController = InPlayerController;
	CurrentMode = DetermineInputMode(Mode);

	// If using Enhanced Input, create adapter
	if (CurrentMode == EInputSimulatorMode::EnhancedInput)
	{
		EnhancedInputAdapter = UEnhancedInputAdapter::CreateEnhancedInputAdapter(InPlayerController, InPlayerController);
		if (!EnhancedInputAdapter)
		{
			UE_LOG(LogTemp, Warning, TEXT("InputSimulator: Failed to create Enhanced Input Adapter, falling back to legacy mode"));
			CurrentMode = EInputSimulatorMode::Legacy;
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("InputSimulator: Initialized with Enhanced Input System"));
		}
	}

	UE_LOG(LogTemp, Log, TEXT("InputSimulator: Initialized for PlayerController: %s (Mode: %d)"),
		*PlayerController->GetName(), static_cast<int32>(CurrentMode));
}

void UInputSimulator::PressButton(FName ActionName)
{
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("InputSimulator: Not initialized"));
		return;
	}

	ActiveButtons.Add(ActionName);

	// Use Enhanced Input if available
	if (CurrentMode == EInputSimulatorMode::EnhancedInput && EnhancedInputAdapter)
	{
		if (EnhancedInputAdapter->InjectButtonPress(ActionName))
		{
			UE_LOG(LogTemp, Log, TEXT("InputSimulator: PressButton (Enhanced Input) - %s"), *ActionName.ToString());
			return;
		}
		else
		{
			UE_LOG(LogTemp, Verbose, TEXT("InputSimulator: Enhanced Input injection failed for %s, trying legacy"), *ActionName.ToString());
		}
	}

	// Legacy fallback
	UE_LOG(LogTemp, Log, TEXT("InputSimulator: PressButton (Legacy) - %s"), *ActionName.ToString());

	// Handle common actions
	if (ActionName == "Jump")
	{
		Jump();
	}
}

void UInputSimulator::ReleaseButton(FName ActionName)
{
	if (!PlayerController)
	{
		return;
	}

	ActiveButtons.Remove(ActionName);

	// Use Enhanced Input if available
	if (CurrentMode == EInputSimulatorMode::EnhancedInput && EnhancedInputAdapter)
	{
		if (EnhancedInputAdapter->InjectButtonRelease(ActionName))
		{
			UE_LOG(LogTemp, Log, TEXT("InputSimulator: ReleaseButton (Enhanced Input) - %s"), *ActionName.ToString());
			return;
		}
	}

	// Legacy fallback
	UE_LOG(LogTemp, Log, TEXT("InputSimulator: ReleaseButton (Legacy) - %s"), *ActionName.ToString());

	// Handle common actions
	if (ActionName == "Jump")
	{
		StopJumping();
	}
}

void UInputSimulator::PressAndHoldButton(FName ActionName, float Duration)
{
	PressButton(ActionName);

	if (Duration > 0.0f)
	{
		FTimedButtonRelease Release;
		Release.ActionName = ActionName;
		Release.TimeRemaining = Duration;
		TimedReleases.Add(Release);
	}
}

void UInputSimulator::SetAxisValue(FName AxisName, float Value)
{
	if (!PlayerController)
	{
		return;
	}

	ActiveAxes.Add(AxisName, Value);

	// Use Enhanced Input if available
	if (CurrentMode == EInputSimulatorMode::EnhancedInput && EnhancedInputAdapter)
	{
		if (EnhancedInputAdapter->InjectAxisValue(AxisName, Value))
		{
			UE_LOG(LogTemp, Verbose, TEXT("InputSimulator: SetAxisValue (Enhanced Input) - %s = %.2f"), *AxisName.ToString(), Value);
			return;
		}
	}

	// Legacy fallback
	UE_LOG(LogTemp, Verbose, TEXT("InputSimulator: SetAxisValue (Legacy) - %s = %.2f"), *AxisName.ToString(), Value);

	// Handle common axes
	if (AxisName == "MoveForward")
	{
		SetMoveForward(Value);
	}
	else if (AxisName == "MoveRight")
	{
		SetMoveRight(Value);
	}
	else if (AxisName == "LookUp")
	{
		SetLookUp(Value);
	}
	else if (AxisName == "LookRight" || AxisName == "Turn")
	{
		SetLookRight(Value);
	}
}

void UInputSimulator::SetAxis2DValue(FName AxisName, FVector2D Value)
{
	// Use Enhanced Input if available (native 2D axis support)
	if (CurrentMode == EInputSimulatorMode::EnhancedInput && EnhancedInputAdapter)
	{
		if (EnhancedInputAdapter->InjectAxis2DValue(AxisName, Value))
		{
			UE_LOG(LogTemp, Verbose, TEXT("InputSimulator: SetAxis2DValue (Enhanced Input) - %s = (%.2f, %.2f)"),
				*AxisName.ToString(), Value.X, Value.Y);
			return;
		}
	}

	// For legacy mode, we'll store them as separate X and Y components
	FName XAxisName = FName(*(AxisName.ToString() + TEXT("_X")));
	FName YAxisName = FName(*(AxisName.ToString() + TEXT("_Y")));

	SetAxisValue(XAxisName, Value.X);
	SetAxisValue(YAxisName, Value.Y);
}

void UInputSimulator::ClearAxisValue(FName AxisName)
{
	ActiveAxes.Remove(AxisName);
	SetAxisValue(AxisName, 0.0f);
}

void UInputSimulator::SetMoveForward(float Value)
{
	if (!PlayerController)
	{
		return;
	}

	if (ACharacter* Character = Cast<ACharacter>(PlayerController->GetPawn()))
	{
		Character->AddMovementInput(Character->GetActorForwardVector(), Value);
	}
}

void UInputSimulator::SetMoveRight(float Value)
{
	if (!PlayerController)
	{
		return;
	}

	if (ACharacter* Character = Cast<ACharacter>(PlayerController->GetPawn()))
	{
		Character->AddMovementInput(Character->GetActorRightVector(), Value);
	}
}

void UInputSimulator::SetLookUp(float Value)
{
	if (!PlayerController)
	{
		return;
	}

	PlayerController->AddPitchInput(Value);
}

void UInputSimulator::SetLookRight(float Value)
{
	if (!PlayerController)
	{
		return;
	}

	PlayerController->AddYawInput(Value);
}

void UInputSimulator::Jump()
{
	if (!PlayerController)
	{
		return;
	}

	if (ACharacter* Character = Cast<ACharacter>(PlayerController->GetPawn()))
	{
		Character->Jump();
		UE_LOG(LogTemp, Log, TEXT("InputSimulator: Jump"));
	}
}

void UInputSimulator::StopJumping()
{
	if (!PlayerController)
	{
		return;
	}

	if (ACharacter* Character = Cast<ACharacter>(PlayerController->GetPawn()))
	{
		Character->StopJumping();
	}
}

void UInputSimulator::ClearAllInput()
{
	ActiveButtons.Empty();
	ActiveAxes.Empty();
	TimedReleases.Empty();

	UE_LOG(LogTemp, Log, TEXT("InputSimulator: Cleared all input"));
}

UInputSimulator* UInputSimulator::CreateInputSimulator(UObject* WorldContextObject, APlayerController* InPlayerController)
{
	if (!WorldContextObject || !InPlayerController)
	{
		return nullptr;
	}

	UInputSimulator* Simulator = NewObject<UInputSimulator>();
	Simulator->Initialize(InPlayerController);

	return Simulator;
}

void UInputSimulator::TickTimedReleases(float DeltaTime)
{
	for (int32 i = TimedReleases.Num() - 1; i >= 0; --i)
	{
		TimedReleases[i].TimeRemaining -= DeltaTime;

		if (TimedReleases[i].TimeRemaining <= 0.0f)
		{
			ReleaseButton(TimedReleases[i].ActionName);
			TimedReleases.RemoveAt(i);
		}
	}
}

void UInputSimulator::Tick(float DeltaTime)
{
	TickTimedReleases(DeltaTime);
}

EInputSimulatorMode UInputSimulator::DetermineInputMode(EInputSimulatorMode RequestedMode)
{
	// If specific mode requested, try to use it
	if (RequestedMode != EInputSimulatorMode::Auto)
	{
		// Verify Enhanced Input is available if requested
		if (RequestedMode == EInputSimulatorMode::EnhancedInput)
		{
			if (PlayerController && Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
			{
				return EInputSimulatorMode::EnhancedInput;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("InputSimulator: Enhanced Input requested but not available, using legacy"));
				return EInputSimulatorMode::Legacy;
			}
		}
		return RequestedMode;
	}

	// Auto mode: detect available system
	if (PlayerController && Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
	{
		UE_LOG(LogTemp, Log, TEXT("InputSimulator: Auto-detected Enhanced Input System"));
		return EInputSimulatorMode::EnhancedInput;
	}

	UE_LOG(LogTemp, Log, TEXT("InputSimulator: Using legacy input mode"));
	return EInputSimulatorMode::Legacy;
}
