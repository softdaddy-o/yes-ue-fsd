// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTask_AutoDriverBase.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTTask_AutoDriverInput.generated.h"

/**
 * Input action type
 */
UENUM(BlueprintType)
enum class EAutoDriverInputType : uint8
{
	/** Press and immediately release */
	Press,
	/** Press and hold for duration */
	Hold,
	/** Press and hold indefinitely */
	HoldIndefinite
};

/**
 * Behavior Tree task for triggering input actions using AutoDriver
 */
UCLASS()
class YESUEFSD_API UBTTask_AutoDriverInput : public UBTTask_AutoDriverBase
{
	GENERATED_BODY()

public:
	UBTTask_AutoDriverInput();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;
	virtual uint16 GetInstanceMemorySize() const override;

protected:
	/** Name of the input action to trigger */
	UPROPERTY(EditAnywhere, Category = "AutoDriver")
	FName InputActionName;

	/** Blackboard key for dynamic input action name */
	UPROPERTY(EditAnywhere, Category = "AutoDriver")
	FBlackboardKeySelector InputActionKey;

	/** Type of input action */
	UPROPERTY(EditAnywhere, Category = "AutoDriver")
	EAutoDriverInputType InputType;

	/** Duration to hold the input (for Hold type) */
	UPROPERTY(EditAnywhere, Category = "AutoDriver", meta = (ClampMin = "0.0", EditCondition = "InputType == EAutoDriverInputType::Hold"))
	float HoldDuration;

	/** Wait for input to complete before finishing task */
	UPROPERTY(EditAnywhere, Category = "AutoDriver")
	bool bWaitForCompletion;

private:
	struct FBTInputTaskMemory
	{
		float RemainingHoldTime;
	};
};
