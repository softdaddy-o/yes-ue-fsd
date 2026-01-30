// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTask_AutoDriverBase.h"
#include "AutoDriver/AutoDriverTypes.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTTask_AutoDriverMove.generated.h"

/**
 * Behavior Tree task for moving to a target location using AutoDriver
 */
UCLASS()
class YESUEFSD_API UBTTask_AutoDriverMove : public UBTTask_AutoDriverBase
{
	GENERATED_BODY()

public:
	UBTTask_AutoDriverMove();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;

protected:
	/** Blackboard key for target location */
	UPROPERTY(EditAnywhere, Category = "AutoDriver")
	FBlackboardKeySelector TargetLocationKey;

	/** Blackboard key for target actor (alternative to location) */
	UPROPERTY(EditAnywhere, Category = "AutoDriver")
	FBlackboardKeySelector TargetActorKey;

	/** Acceptance radius - how close to get to the target */
	UPROPERTY(EditAnywhere, Category = "AutoDriver", meta = (ClampMin = "0.0"))
	float AcceptanceRadius;

	/** Speed multiplier for movement */
	UPROPERTY(EditAnywhere, Category = "AutoDriver", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float SpeedMultiplier;

	/** Should sprint during movement */
	UPROPERTY(EditAnywhere, Category = "AutoDriver")
	bool bShouldSprint;

	/** Movement mode to use */
	UPROPERTY(EditAnywhere, Category = "AutoDriver")
	EAutoDriverMovementMode MovementMode;

	/** Update blackboard with arrival status */
	UPROPERTY(EditAnywhere, Category = "AutoDriver")
	FBlackboardKeySelector ArrivalStatusKey;

private:
	/** Get target location from blackboard */
	bool GetTargetLocation(UBehaviorTreeComponent& OwnerComp, FVector& OutLocation) const;
};
