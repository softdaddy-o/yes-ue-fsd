// Copyright Epic Games, Inc. All Rights Reserved.

#include "BehaviorTree/BTTask_AutoDriverMove.h"
#include "AutoDriver/AutoDriverComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "GameFramework/Actor.h"
#include "AIController.h"

UBTTask_AutoDriverMove::UBTTask_AutoDriverMove()
{
	NodeName = "AutoDriver Move To";
	AcceptanceRadius = 50.0f;
	SpeedMultiplier = 1.0f;
	bShouldSprint = false;
	MovementMode = EAutoDriverMovementMode::Navigation;

	// Setup blackboard key filters
	TargetLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_AutoDriverMove, TargetLocationKey));
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_AutoDriverMove, TargetActorKey), AActor::StaticClass());
	ArrivalStatusKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_AutoDriverMove, ArrivalStatusKey));
}

EBTNodeResult::Type UBTTask_AutoDriverMove::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverComponent(OwnerComp);
	if (!AutoDriver)
	{
		UE_LOG(LogTemp, Error, TEXT("BTTask_AutoDriverMove: No AutoDriver component found"));
		return EBTNodeResult::Failed;
	}

	FVector TargetLocation;
	if (!GetTargetLocation(OwnerComp, TargetLocation))
	{
		UE_LOG(LogTemp, Error, TEXT("BTTask_AutoDriverMove: Failed to get target location from blackboard"));
		return EBTNodeResult::Failed;
	}

	if (bLogExecution)
	{
		UE_LOG(LogTemp, Log, TEXT("BTTask_AutoDriverMove: Moving to %s"), *TargetLocation.ToString());
	}

	// Execute movement command
	FAutoDriverMoveParams Params;
	Params.TargetLocation = TargetLocation;
	Params.AcceptanceRadius = AcceptanceRadius;
	Params.SpeedMultiplier = SpeedMultiplier;
	Params.bShouldSprint = bShouldSprint;
	Params.MovementMode = MovementMode;
	bool bSuccess = AutoDriver->MoveToLocation(Params);

	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_AutoDriverMove: Failed to start movement command"));
		return EBTNodeResult::Failed;
	}

	// Task will complete when command finishes (checked in TickTask)
	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBTTask_AutoDriverMove::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverComponent(OwnerComp);
	if (AutoDriver)
	{
		AutoDriver->StopCurrentCommand();
	}

	return EBTNodeResult::Aborted;
}

void UBTTask_AutoDriverMove::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverComponent(OwnerComp);
	if (!AutoDriver)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Check if command is still executing
	if (!AutoDriver->IsExecutingCommand())
	{
		// Command completed - check if we reached the target
		FVector TargetLocation;
		if (GetTargetLocation(OwnerComp, TargetLocation))
		{
			AAIController* AIController = GetAIController(OwnerComp);
			APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
			if (Pawn)
			{
				float Distance = FVector::Dist(Pawn->GetActorLocation(), TargetLocation);
				bool bReachedTarget = Distance <= AcceptanceRadius;

				// Update blackboard if key is set
				if (ArrivalStatusKey.SelectedKeyName != NAME_None)
				{
					OwnerComp.GetBlackboardComponent()->SetValueAsBool(ArrivalStatusKey.SelectedKeyName, bReachedTarget);
				}

				if (bLogExecution)
				{
					UE_LOG(LogTemp, Log, TEXT("BTTask_AutoDriverMove: Completed. Distance: %.2f, Success: %s"),
						Distance, bReachedTarget ? TEXT("Yes") : TEXT("No"));
				}

				FinishLatentTask(OwnerComp, bReachedTarget ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
				return;
			}
		}

		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
}

FString UBTTask_AutoDriverMove::GetStaticDescription() const
{
	FString Description = FString::Printf(TEXT("Move to %s"),
		TargetLocationKey.SelectedKeyName != NAME_None ? *TargetLocationKey.SelectedKeyName.ToString() :
		TargetActorKey.SelectedKeyName != NAME_None ? *TargetActorKey.SelectedKeyName.ToString() :
		TEXT("Unknown"));

	Description += FString::Printf(TEXT("\nRadius: %.1f, Speed: %.1fx"), AcceptanceRadius, SpeedMultiplier);

	if (bShouldSprint)
	{
		Description += TEXT(", Sprint");
	}

	return Description;
}

bool UBTTask_AutoDriverMove::GetTargetLocation(UBehaviorTreeComponent& OwnerComp, FVector& OutLocation) const
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return false;
	}

	// Try to get location directly
	if (TargetLocationKey.SelectedKeyName != NAME_None)
	{
		OutLocation = BlackboardComp->GetValueAsVector(TargetLocationKey.SelectedKeyName);
		return !OutLocation.IsZero();
	}

	// Try to get location from actor
	if (TargetActorKey.SelectedKeyName != NAME_None)
	{
		if (AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName)))
		{
			OutLocation = TargetActor->GetActorLocation();
			return true;
		}
	}

	return false;
}
