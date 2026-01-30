// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoDriver/Commands/RotateToCommand.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"

void URotateToCommand::Initialize_Implementation(UObject* InContext)
{
	PlayerController = Cast<APlayerController>(InContext);
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("RotateToCommand: Invalid context - expected PlayerController"));
		return;
	}

	Pawn = PlayerController->GetPawn();
	if (!Pawn)
	{
		UE_LOG(LogTemp, Error, TEXT("RotateToCommand: PlayerController does not have a pawn"));
	}
}

bool URotateToCommand::Execute_Implementation()
{
	if (!PlayerController || !Pawn)
	{
		Result = FAutoDriverCommandResult(EAutoDriverCommandStatus::Failed, TEXT("Invalid controller or pawn"));
		return false;
	}

	bIsRunning = true;
	ExecutionTime = 0.0f;
	Result = FAutoDriverCommandResult(EAutoDriverCommandStatus::Running, TEXT("Rotating to target"));

	UE_LOG(LogTemp, Log, TEXT("RotateToCommand: Started rotating to %s"), *TargetRotation.ToString());

	return true;
}

void URotateToCommand::Tick_Implementation(float DeltaTime)
{
	if (!bIsRunning || !Pawn)
	{
		return;
	}

	ExecutionTime += DeltaTime;

	// Check timeout
	if (Timeout > 0.0f && ExecutionTime > Timeout)
	{
		Result = FAutoDriverCommandResult(EAutoDriverCommandStatus::Failed,
			FString::Printf(TEXT("Rotation timed out after %.1f seconds"), ExecutionTime));
		bIsRunning = false;
		UE_LOG(LogTemp, Warning, TEXT("RotateToCommand: Timed out"));
		return;
	}

	// Check if we've reached the target rotation
	if (HasReachedRotation())
	{
		Result = FAutoDriverCommandResult(EAutoDriverCommandStatus::Success,
			FString::Printf(TEXT("Reached target rotation in %.2f seconds"), ExecutionTime));
		Result.ExecutionTime = ExecutionTime;
		bIsRunning = false;
		UE_LOG(LogTemp, Log, TEXT("RotateToCommand: Completed successfully"));
		return;
	}

	// Interpolate rotation
	FRotator CurrentRotation = Pawn->GetActorRotation();
	FRotator NewRotation = UKismetMathLibrary::RInterpTo_Constant(
		CurrentRotation,
		TargetRotation,
		DeltaTime,
		RotationSpeed
	);

	// Apply rotation through controller
	if (PlayerController)
	{
		PlayerController->SetControlRotation(NewRotation);
	}
}

void URotateToCommand::Cancel_Implementation()
{
	if (!bIsRunning)
	{
		return;
	}

	bIsRunning = false;
	Result = FAutoDriverCommandResult(EAutoDriverCommandStatus::Cancelled, TEXT("Rotation cancelled"));

	UE_LOG(LogTemp, Log, TEXT("RotateToCommand: Cancelled"));
}

bool URotateToCommand::IsRunning_Implementation() const
{
	return bIsRunning;
}

FAutoDriverCommandResult URotateToCommand::GetResult_Implementation() const
{
	return Result;
}

FString URotateToCommand::GetDescription_Implementation() const
{
	return FString::Printf(TEXT("Rotate to %s (Speed: %.1f deg/s)"),
		*TargetRotation.ToString(), RotationSpeed);
}

URotateToCommand* URotateToCommand::CreateRotateToRotation(
	UObject* WorldContextObject,
	FRotator InTargetRotation,
	float InRotationSpeed)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	URotateToCommand* Command = NewObject<URotateToCommand>();
	Command->TargetRotation = InTargetRotation;
	Command->RotationSpeed = InRotationSpeed;

	return Command;
}

URotateToCommand* URotateToCommand::CreateLookAtLocation(
	UObject* WorldContextObject,
	FVector TargetLocation,
	float InRotationSpeed)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC || !PC->GetPawn())
	{
		return nullptr;
	}

	// Calculate rotation to look at target
	FVector Direction = TargetLocation - PC->GetPawn()->GetActorLocation();
	FRotator LookAtRotation = Direction.Rotation();

	return CreateRotateToRotation(WorldContextObject, LookAtRotation, InRotationSpeed);
}

bool URotateToCommand::HasReachedRotation() const
{
	return GetAngleDifferenceFromTarget() <= AcceptanceAngle;
}

float URotateToCommand::GetAngleDifferenceFromTarget() const
{
	if (!Pawn)
	{
		return MAX_FLT;
	}

	FRotator CurrentRotation = Pawn->GetActorRotation();
	FRotator Delta = (TargetRotation - CurrentRotation).GetNormalized();

	// Calculate total angular difference
	float YawDiff = FMath::Abs(Delta.Yaw);
	float PitchDiff = FMath::Abs(Delta.Pitch);

	return FMath::Max(YawDiff, PitchDiff);
}
