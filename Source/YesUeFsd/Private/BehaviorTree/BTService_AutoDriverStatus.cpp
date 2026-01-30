// Copyright Epic Games, Inc. All Rights Reserved.

#include "BehaviorTree/BTService_AutoDriverStatus.h"
#include "AutoDriver/AutoDriverComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"

UBTService_AutoDriverStatus::UBTService_AutoDriverStatus()
{
	NodeName = "AutoDriver Status Monitor";
	Interval = 0.5f;
	RandomDeviation = 0.1f;

	IsExecutingCommandKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_AutoDriverStatus, IsExecutingCommandKey));
	IsLocationReachableKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_AutoDriverStatus, IsLocationReachableKey));
	TargetLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_AutoDriverStatus, TargetLocationKey));
}

void UBTService_AutoDriverStatus::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return;
	}

	AAIController* AIController = Cast<AAIController>(OwnerComp.GetOwner());
	if (!AIController)
	{
		return;
	}

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn)
	{
		return;
	}

	UAutoDriverComponent* AutoDriver = Pawn->FindComponentByClass<UAutoDriverComponent>();
	if (!AutoDriver)
	{
		return;
	}

	// Update execution status
	if (IsExecutingCommandKey.SelectedKeyName != NAME_None)
	{
		bool bIsExecuting = AutoDriver->IsExecutingCommand();
		BlackboardComp->SetValueAsBool(IsExecutingCommandKey.SelectedKeyName, bIsExecuting);
	}

	// Update reachability status
	if (IsLocationReachableKey.SelectedKeyName != NAME_None && TargetLocationKey.SelectedKeyName != NAME_None)
	{
		FVector TargetLocation = BlackboardComp->GetValueAsVector(TargetLocationKey.SelectedKeyName);
		if (!TargetLocation.IsZero())
		{
			bool bIsReachable = AutoDriver->IsLocationReachable(TargetLocation);
			BlackboardComp->SetValueAsBool(IsLocationReachableKey.SelectedKeyName, bIsReachable);
		}
	}
}

FString UBTService_AutoDriverStatus::GetStaticDescription() const
{
	FString Description = TEXT("Monitor AutoDriver Status\n");

	if (IsExecutingCommandKey.SelectedKeyName != NAME_None)
	{
		Description += FString::Printf(TEXT("Executing -> %s\n"), *IsExecutingCommandKey.SelectedKeyName.ToString());
	}

	if (IsLocationReachableKey.SelectedKeyName != NAME_None)
	{
		Description += FString::Printf(TEXT("Reachable -> %s"), *IsLocationReachableKey.SelectedKeyName.ToString());
	}

	return Description;
}
