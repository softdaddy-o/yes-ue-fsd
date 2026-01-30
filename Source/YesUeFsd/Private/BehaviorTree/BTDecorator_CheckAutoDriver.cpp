// Copyright Epic Games, Inc. All Rights Reserved.

#include "BehaviorTree/BTDecorator_CheckAutoDriver.h"
#include "AutoDriver/AutoDriverComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"

UBTDecorator_CheckAutoDriver::UBTDecorator_CheckAutoDriver()
{
	NodeName = "Check AutoDriver";
	CheckType = EAutoDriverCheckType::HasAutoDriver;
	TargetDistance = 100.0f;
	bInvertCondition = false;

	TargetLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_CheckAutoDriver, TargetLocationKey));
}

bool UBTDecorator_CheckAutoDriver::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIController = Cast<AAIController>(OwnerComp.GetOwner());
	if (!AIController)
	{
		return bInvertCondition ? true : false;
	}

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn)
	{
		return bInvertCondition ? true : false;
	}

	UAutoDriverComponent* AutoDriver = Pawn->FindComponentByClass<UAutoDriverComponent>();

	bool bResult = false;

	switch (CheckType)
	{
	case EAutoDriverCheckType::HasAutoDriver:
		bResult = (AutoDriver != nullptr);
		break;

	case EAutoDriverCheckType::IsExecuting:
		bResult = AutoDriver ? AutoDriver->IsExecutingCommand() : false;
		break;

	case EAutoDriverCheckType::IsReachable:
		if (AutoDriver && TargetLocationKey.SelectedKeyName != NAME_None)
		{
			UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
			if (BlackboardComp)
			{
				FVector TargetLocation = BlackboardComp->GetValueAsVector(TargetLocationKey.SelectedKeyName);
				bResult = AutoDriver->IsLocationReachable(TargetLocation);
			}
		}
		break;

	case EAutoDriverCheckType::WithinDistance:
		if (AutoDriver && TargetLocationKey.SelectedKeyName != NAME_None)
		{
			UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
			if (BlackboardComp)
			{
				FVector TargetLocation = BlackboardComp->GetValueAsVector(TargetLocationKey.SelectedKeyName);
				float Distance = FVector::Dist(Pawn->GetActorLocation(), TargetLocation);
				bResult = Distance <= TargetDistance;
			}
		}
		break;
	}

	return bInvertCondition ? !bResult : bResult;
}

FString UBTDecorator_CheckAutoDriver::GetStaticDescription() const
{
	FString Description;

	switch (CheckType)
	{
	case EAutoDriverCheckType::HasAutoDriver:
		Description = TEXT("Has AutoDriver");
		break;
	case EAutoDriverCheckType::IsExecuting:
		Description = TEXT("Is Executing Command");
		break;
	case EAutoDriverCheckType::IsReachable:
		Description = FString::Printf(TEXT("Is %s Reachable"),
			TargetLocationKey.SelectedKeyName != NAME_None ? *TargetLocationKey.SelectedKeyName.ToString() : TEXT("Target"));
		break;
	case EAutoDriverCheckType::WithinDistance:
		Description = FString::Printf(TEXT("Within %.1f of %s"),
			TargetDistance,
			TargetLocationKey.SelectedKeyName != NAME_None ? *TargetLocationKey.SelectedKeyName.ToString() : TEXT("Target"));
		break;
	}

	if (bInvertCondition)
	{
		Description = TEXT("NOT ") + Description;
	}

	return Description;
}
