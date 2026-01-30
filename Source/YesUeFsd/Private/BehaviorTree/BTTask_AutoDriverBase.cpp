// Copyright Epic Games, Inc. All Rights Reserved.

#include "BehaviorTree/BTTask_AutoDriverBase.h"
#include "AutoDriver/AutoDriverComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Pawn.h"

UBTTask_AutoDriverBase::UBTTask_AutoDriverBase()
{
	NodeName = "AutoDriver Base Task";
	CommandTimeout = 10.0f;
	bLogExecution = false;
	bNotifyTick = true;
	bNotifyTaskFinished = true;
}

UAutoDriverComponent* UBTTask_AutoDriverBase::GetAutoDriverComponent(UBehaviorTreeComponent& OwnerComp) const
{
	AAIController* AIController = GetAIController(OwnerComp);
	if (!AIController)
	{
		return nullptr;
	}

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn)
	{
		return nullptr;
	}

	return Pawn->FindComponentByClass<UAutoDriverComponent>();
}

AAIController* UBTTask_AutoDriverBase::GetAIController(UBehaviorTreeComponent& OwnerComp) const
{
	return Cast<AAIController>(OwnerComp.GetOwner());
}

void UBTTask_AutoDriverBase::AbortTask(UBehaviorTreeComponent& OwnerComp, EBTNodeResult::Type Result)
{
	ClearTimeoutTimer();

	if (UAutoDriverComponent* AutoDriver = GetAutoDriverComponent(OwnerComp))
	{
		AutoDriver->StopCurrentCommand();
	}

	FinishLatentTask(OwnerComp, Result);
}

void UBTTask_AutoDriverBase::StartTimeoutTimer(UBehaviorTreeComponent& OwnerComp)
{
	if (CommandTimeout <= 0.0f)
	{
		return;
	}

	CachedOwnerComp = &OwnerComp;

	if (UWorld* World = OwnerComp.GetWorld())
	{
		World->GetTimerManager().SetTimer(
			TimeoutHandle,
			FTimerDelegate::CreateUObject(this, &UBTTask_AutoDriverBase::HandleTimeout),
			CommandTimeout,
			false
		);
	}
}

void UBTTask_AutoDriverBase::ClearTimeoutTimer()
{
	if (TimeoutHandle.IsValid())
	{
		if (CachedOwnerComp.IsValid())
		{
			if (UWorld* World = CachedOwnerComp->GetWorld())
			{
				World->GetTimerManager().ClearTimer(TimeoutHandle);
			}
		}
		TimeoutHandle.Invalidate();
	}
}

void UBTTask_AutoDriverBase::HandleTimeout()
{
	if (CachedOwnerComp.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("AutoDriver BT Task timed out: %s"), *GetNodeName());
		AbortTask(*CachedOwnerComp.Get(), EBTNodeResult::Failed);
	}
}
