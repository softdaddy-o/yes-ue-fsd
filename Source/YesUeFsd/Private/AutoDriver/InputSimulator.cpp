// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoDriver/InputSimulator.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UInputSimulator::Initialize(APlayerController* InPlayerController)
{
	if (!InPlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("InputSimulator: Cannot initialize with null PlayerController"));
		return;
	}

	PlayerController = InPlayerController;
	UE_LOG(LogTemp, Log, TEXT("InputSimulator: Initialized for PlayerController: %s"), *PlayerController->GetName());
}

void UInputSimulator::PressButton(FName ActionName)
{
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("InputSimulator: Not initialized"));
		return;
	}

	ActiveButtons.Add(ActionName);

	// TODO: Implement actual input injection via Enhanced Input or legacy input system
	UE_LOG(LogTemp, Log, TEXT("InputSimulator: PressButton - %s"), *ActionName.ToString());

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

	// TODO: Implement actual input release
	UE_LOG(LogTemp, Log, TEXT("InputSimulator: ReleaseButton - %s"), *ActionName.ToString());

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

	// TODO: Implement actual axis input injection
	UE_LOG(LogTemp, Verbose, TEXT("InputSimulator: SetAxisValue - %s = %.2f"), *AxisName.ToString(), Value);

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
	// For 2D axes, we'll store them as separate X and Y components
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
