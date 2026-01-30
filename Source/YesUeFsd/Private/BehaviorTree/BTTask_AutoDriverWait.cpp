// Copyright Epic Games, Inc. All Rights Reserved.

#include "BehaviorTree/BTTask_AutoDriverWait.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_AutoDriverWait::UBTTask_AutoDriverWait()
{
	NodeName = "AutoDriver Wait";
	WaitDuration = 1.0f;
	RandomDeviation = 0.0f;
	bNotifyTick = true;

	WaitDurationKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_AutoDriverWait, WaitDurationKey));
}

EBTNodeResult::Type UBTTask_AutoDriverWait::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTWaitTaskMemory* Memory = reinterpret_cast<FBTWaitTaskMemory*>(NodeMemory);

	float Duration = WaitDuration;

	// Try to get duration from blackboard if key is set
	if (WaitDurationKey.SelectedKeyName != NAME_None)
	{
		UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
		if (BlackboardComp)
		{
			Duration = BlackboardComp->GetValueAsFloat(WaitDurationKey.SelectedKeyName);
		}
	}

	// Apply random deviation
	if (RandomDeviation > 0.0f)
	{
		Duration += FMath::RandRange(-RandomDeviation, RandomDeviation);
	}

	Duration = FMath::Max(0.0f, Duration);
	Memory->RemainingTime = Duration;

	if (bLogExecution)
	{
		UE_LOG(LogTemp, Log, TEXT("BTTask_AutoDriverWait: Waiting for %.2f seconds"), Duration);
	}

	if (Duration <= 0.0f)
	{
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_AutoDriverWait::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTWaitTaskMemory* Memory = reinterpret_cast<FBTWaitTaskMemory*>(NodeMemory);

	Memory->RemainingTime -= DeltaSeconds;

	if (Memory->RemainingTime <= 0.0f)
	{
		if (bLogExecution)
		{
			UE_LOG(LogTemp, Log, TEXT("BTTask_AutoDriverWait: Wait completed"));
		}

		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

FString UBTTask_AutoDriverWait::GetStaticDescription() const
{
	FString Description;

	if (WaitDurationKey.SelectedKeyName != NAME_None)
	{
		Description = FString::Printf(TEXT("Wait %s seconds"), *WaitDurationKey.SelectedKeyName.ToString());
	}
	else
	{
		Description = FString::Printf(TEXT("Wait %.1f seconds"), WaitDuration);
	}

	if (RandomDeviation > 0.0f)
	{
		Description += FString::Printf(TEXT(" ±%.1fs"), RandomDeviation);
	}

	return Description;
}

uint16 UBTTask_AutoDriverWait::GetInstanceMemorySize() const
{
	return sizeof(FBTWaitTaskMemory);
}
