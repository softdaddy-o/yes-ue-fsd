// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoDriver/Commands/MoveToLocationCommand.h"
#include "AutoDriver/AutoDriverStats.h"
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
	SCOPE_CYCLE_COUNTER(STAT_AutoDriver_CommandExecution);

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
	SCOPE_CYCLE_COUNTER(STAT_AutoDriver_CommandTick);

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
	if (!PlayerController || !Character)
	{
		Result = FAutoDriverCommandResult(EAutoDriverCommandStatus::Failed, TEXT("No player controller or character"));
		return false;
	}

	UWorld* World = PlayerController->GetWorld();
	if (!World)
	{
		Result = FAutoDriverCommandResult(EAutoDriverCommandStatus::Failed, TEXT("No world"));
		return false;
	}

	// Get or create AI controller for navigation
	AAIController* AIController = nullptr;

	// Check if character already has AI controller
	AIController = Cast<AAIController>(Character->GetController());

	// If no existing AI controller, try to reuse cached one
	if (!AIController && CachedAIController && IsValid(CachedAIController))
	{
		AIController = CachedAIController;
		AIController->Possess(Character);
		INC_DWORD_STAT(STAT_AutoDriver_AIControllersReused);
		UE_LOG(LogTemp, Log, TEXT("MoveToLocationCommand: Reusing cached AI controller"));
	}

	// Only create new controller if we don't have one cached
	if (!AIController)
	{
		// Try to create temporary AI controller
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AIController = World->SpawnActor<AAIController>(AAIController::StaticClass(), SpawnParams);
		if (AIController)
		{
			// Cache for future use
			CachedAIController = AIController;

			// Temporarily possess for navigation
			AIController->Possess(Character);

			INC_DWORD_STAT(STAT_AutoDriver_AIControllersCreated);
			UE_LOG(LogTemp, Log, TEXT("MoveToLocationCommand: Created new AI controller (will be cached)"));
		}
	}

	if (!AIController)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveToLocationCommand: Could not create AI controller, falling back to direct movement"));
		MovementMode = EAutoDriverMovementMode::Direct;
		return ExecuteDirectMovement();
	}

	// Use AI MoveTo for navigation
	EPathFollowingRequestResult::Type MoveResult = AIController->MoveToLocation(
		TargetLocation,
		AcceptanceRadius,
		true,  // Stop on overlap
		true,  // Use pathfinding
		false, // Allow partial path
		true   // Project destination to navigation
	);

	if (MoveResult == EPathFollowingRequestResult::RequestSuccessful ||
		MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		UE_LOG(LogTemp, Log, TEXT("MoveToLocationCommand: Navigation movement started"));
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveToLocationCommand: Navigation failed (result: %d), falling back to direct movement"),
			static_cast<int32>(MoveResult));
		MovementMode = EAutoDriverMovementMode::Direct;
		return ExecuteDirectMovement();
	}
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
