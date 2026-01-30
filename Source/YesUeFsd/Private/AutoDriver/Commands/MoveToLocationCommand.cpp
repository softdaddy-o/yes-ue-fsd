// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoDriver/Commands/MoveToLocationCommand.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

void UMoveToLocationCommand::Initialize_Implementation(UObject* InContext)
{
	PlayerController = Cast<APlayerController>(InContext);
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("MoveToLocationCommand: Invalid context - expected PlayerController"));
		return;
	}

	Character = Cast<ACharacter>(PlayerController->GetPawn());
	if (!Character)
	{
		UE_LOG(LogTemp, Error, TEXT("MoveToLocationCommand: PlayerController does not have a Character pawn"));
	}
}

bool UMoveToLocationCommand::Execute_Implementation()
{
	if (!PlayerController || !Character)
	{
		Result = FAutoDriverCommandResult(EAutoDriverCommandStatus::Failed, TEXT("Invalid controller or character"));
		return false;
	}

	bIsRunning = true;
	ExecutionTime = 0.0f;
	Result = FAutoDriverCommandResult(EAutoDriverCommandStatus::Running, TEXT("Moving to location"));

	// Execute based on movement mode
	bool bStarted = false;
	switch (MovementMode)
	{
		case EAutoDriverMovementMode::Navigation:
			bStarted = ExecuteNavigationMovement();
			break;

		case EAutoDriverMovementMode::Direct:
			bStarted = ExecuteDirectMovement();
			break;

		case EAutoDriverMovementMode::InputSimulation:
			bStarted = ExecuteInputSimulation();
			break;

		default:
			Result = FAutoDriverCommandResult(EAutoDriverCommandStatus::Failed, TEXT("Unknown movement mode"));
			bIsRunning = false;
			return false;
	}

	if (!bStarted)
	{
		bIsRunning = false;
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("MoveToLocationCommand: Started moving to %s (Mode: %d)"),
		*TargetLocation.ToString(), static_cast<int32>(MovementMode));

	return true;
}

void UMoveToLocationCommand::Tick_Implementation(float DeltaTime)
{
	if (!bIsRunning)
	{
		return;
	}

	ExecutionTime += DeltaTime;

	// Check timeout
	if (Timeout > 0.0f && ExecutionTime > Timeout)
	{
		Result = FAutoDriverCommandResult(EAutoDriverCommandStatus::Failed,
			FString::Printf(TEXT("Movement timed out after %.1f seconds"), ExecutionTime));
		bIsRunning = false;
		UE_LOG(LogTemp, Warning, TEXT("MoveToLocationCommand: Timed out"));
		return;
	}

	// Check if we've reached the target
	if (HasReachedTarget())
	{
		Result = FAutoDriverCommandResult(EAutoDriverCommandStatus::Success,
			FString::Printf(TEXT("Reached target in %.2f seconds"), ExecutionTime));
		Result.ExecutionTime = ExecutionTime;
		bIsRunning = false;
		UE_LOG(LogTemp, Log, TEXT("MoveToLocationCommand: Completed successfully"));
		return;
	}

	// For direct movement mode, continuously update movement direction
	if (MovementMode == EAutoDriverMovementMode::Direct)
	{
		if (Character && Character->GetCharacterMovement())
		{
			FVector Direction = (TargetLocation - Character->GetActorLocation()).GetSafeNormal();
			Character->AddMovementInput(Direction, SpeedMultiplier);
		}
	}
}

void UMoveToLocationCommand::Cancel_Implementation()
{
	if (!bIsRunning)
	{
		return;
	}

	bIsRunning = false;
	Result = FAutoDriverCommandResult(EAutoDriverCommandStatus::Cancelled, TEXT("Movement cancelled"));

	// Stop character movement
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->StopMovementImmediately();
	}

	UE_LOG(LogTemp, Log, TEXT("MoveToLocationCommand: Cancelled"));
}

bool UMoveToLocationCommand::IsRunning_Implementation() const
{
	return bIsRunning;
}

FAutoDriverCommandResult UMoveToLocationCommand::GetResult_Implementation() const
{
	return Result;
}

FString UMoveToLocationCommand::GetDescription_Implementation() const
{
	return FString::Printf(TEXT("Move to %s (Radius: %.1f, Mode: %d)"),
		*TargetLocation.ToString(), AcceptanceRadius, static_cast<int32>(MovementMode));
}

UMoveToLocationCommand* UMoveToLocationCommand::CreateMoveToLocationCommand(
	UObject* WorldContextObject,
	FVector InTargetLocation,
	float InAcceptanceRadius)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UMoveToLocationCommand* Command = NewObject<UMoveToLocationCommand>();
	Command->TargetLocation = InTargetLocation;
	Command->AcceptanceRadius = InAcceptanceRadius;

	return Command;
}

bool UMoveToLocationCommand::ExecuteNavigationMovement()
{
	// TODO: Implement navigation-based movement using AIController or NavigationSystem
	// For now, fall back to direct movement
	UE_LOG(LogTemp, Warning, TEXT("MoveToLocationCommand: Navigation movement not yet implemented, using direct movement"));
	MovementMode = EAutoDriverMovementMode::Direct;
	return ExecuteDirectMovement();
}

bool UMoveToLocationCommand::ExecuteDirectMovement()
{
	if (!Character || !Character->GetCharacterMovement())
	{
		Result = FAutoDriverCommandResult(EAutoDriverCommandStatus::Failed, TEXT("No character or movement component"));
		return false;
	}

	// Direct movement will be handled in Tick
	return true;
}

bool UMoveToLocationCommand::ExecuteInputSimulation()
{
	// TODO: Implement input simulation-based movement
	UE_LOG(LogTemp, Warning, TEXT("MoveToLocationCommand: Input simulation not yet implemented, using direct movement"));
	MovementMode = EAutoDriverMovementMode::Direct;
	return ExecuteDirectMovement();
}

bool UMoveToLocationCommand::HasReachedTarget() const
{
	return GetDistanceToTarget() <= AcceptanceRadius;
}

float UMoveToLocationCommand::GetDistanceToTarget() const
{
	if (!Character)
	{
		return MAX_FLT;
	}

	return FVector::Dist(Character->GetActorLocation(), TargetLocation);
}
