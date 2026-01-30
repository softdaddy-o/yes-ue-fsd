// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTask_AutoDriverBase.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTTask_AutoDriverWait.generated.h"

/**
 * Behavior Tree task for waiting a specified duration
 */
UCLASS()
class YESUEFSD_API UBTTask_AutoDriverWait : public UBTTask_AutoDriverBase
{
	GENERATED_BODY()

public:
	UBTTask_AutoDriverWait();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;
	virtual uint16 GetInstanceMemorySize() const override;

protected:
	/** Wait duration in seconds */
	UPROPERTY(EditAnywhere, Category = "AutoDriver", meta = (ClampMin = "0.0"))
	float WaitDuration;

	/** Blackboard key for dynamic wait duration */
	UPROPERTY(EditAnywhere, Category = "AutoDriver")
	FBlackboardKeySelector WaitDurationKey;

	/** Allow random deviation from wait duration */
	UPROPERTY(EditAnywhere, Category = "AutoDriver", meta = (ClampMin = "0.0"))
	float RandomDeviation;

private:
	struct FBTWaitTaskMemory
	{
		float RemainingTime;
	};
};
