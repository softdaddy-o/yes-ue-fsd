// Copyright Epic Games, Inc. All Rights Reserved.

#include "BehaviorTree/BTTask_AutoDriverRotate.h"
#include "AutoDriver/AutoDriverComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Rotator.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"

UBTTask_AutoDriverRotate::UBTTask_AutoDriverRotate()
{
	NodeName = "AutoDriver Rotate";
	RotationSpeed = 180.0f;
	AcceptanceAngle = 5.0f;

	// Setup blackboard key filters
	TargetRotationKey.AddRotatorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_AutoDriverRotate, TargetRotationKey));
	LookAtLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_AutoDriverRotate, LookAtLocationKey));
	LookAtActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_AutoDriverRotate, LookAtActorKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_AutoDriverRotate::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverComponent(OwnerComp);
	if (!AutoDriver)
	{
		UE_LOG(LogTemp, Error, TEXT("BTTask_AutoDriverRotate: No AutoDriver component found"));
		return EBTNodeResult::Failed;
	}

	FRotator TargetRotation;
	if (!GetTargetRotation(OwnerComp, TargetRotation))
	{
		UE_LOG(LogTemp, Error, TEXT("BTTask_AutoDriverRotate: Failed to get target rotation from blackboard"));
		return EBTNodeResult::Failed;
	}

	if (bLogExecution)
	{
		UE_LOG(LogTemp, Log, TEXT("BTTask_AutoDriverRotate: Rotating to %s"), *TargetRotation.ToString());
	}

	// Execute rotation command
	bool bSuccess = AutoDriver->RotateToRotation(TargetRotation);

	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_AutoDriverRotate: Failed to start rotation command"));
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBTTask_AutoDriverRotate::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverComponent(OwnerComp);
	if (AutoDriver)
	{
		AutoDriver->StopCurrentCommand();
	}

	return EBTNodeResult::Aborted;
}

void UBTTask_AutoDriverRotate::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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
		// Command completed
		if (bLogExecution)
		{
			UE_LOG(LogTemp, Log, TEXT("BTTask_AutoDriverRotate: Rotation completed"));
		}

		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

FString UBTTask_AutoDriverRotate::GetStaticDescription() const
{
	FString Description;

	if (TargetRotationKey.SelectedKeyName != NAME_None)
	{
		Description = FString::Printf(TEXT("Rotate to %s"), *TargetRotationKey.SelectedKeyName.ToString());
	}
	else if (LookAtLocationKey.SelectedKeyName != NAME_None)
	{
		Description = FString::Printf(TEXT("Look at %s"), *LookAtLocationKey.SelectedKeyName.ToString());
	}
	else if (LookAtActorKey.SelectedKeyName != NAME_None)
	{
		Description = FString::Printf(TEXT("Look at %s"), *LookAtActorKey.SelectedKeyName.ToString());
	}
	else
	{
		Description = TEXT("Rotate (No target set)");
	}

	Description += FString::Printf(TEXT("\nSpeed: %.0f°/s, Tolerance: %.1f°"), RotationSpeed, AcceptanceAngle);

	return Description;
}

bool UBTTask_AutoDriverRotate::GetTargetRotation(UBehaviorTreeComponent& OwnerComp, FRotator& OutRotation) const
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return false;
	}

	AAIController* AIController = GetAIController(OwnerComp);
	APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
	if (!Pawn)
	{
		return false;
	}

	// Try to get rotation directly
	if (TargetRotationKey.SelectedKeyName != NAME_None)
	{
		OutRotation = BlackboardComp->GetValueAsRotator(TargetRotationKey.SelectedKeyName);
		return true;
	}

	// Try to get rotation from look-at location
	if (LookAtLocationKey.SelectedKeyName != NAME_None)
	{
		FVector LookAtLocation = BlackboardComp->GetValueAsVector(LookAtLocationKey.SelectedKeyName);
		if (!LookAtLocation.IsZero())
		{
			FVector Direction = LookAtLocation - Pawn->GetActorLocation();
			OutRotation = Direction.Rotation();
			return true;
		}
	}

	// Try to get rotation from look-at actor
	if (LookAtActorKey.SelectedKeyName != NAME_None)
	{
		if (AActor* LookAtActor = Cast<AActor>(BlackboardComp->GetValueAsObject(LookAtActorKey.SelectedKeyName)))
		{
			FVector Direction = LookAtActor->GetActorLocation() - Pawn->GetActorLocation();
			OutRotation = Direction.Rotation();
			return true;
		}
	}

	return false;
}
