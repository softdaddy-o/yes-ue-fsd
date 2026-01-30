// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTask_AutoDriverBase.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTTask_AutoDriverRotate.generated.h"

/**
 * Behavior Tree task for rotating to a target rotation or looking at a target using AutoDriver
 */
UCLASS()
class YESUEFSD_API UBTTask_AutoDriverRotate : public UBTTask_AutoDriverBase
{
	GENERATED_BODY()

public:
	UBTTask_AutoDriverRotate();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;

protected:
	/** Blackboard key for target rotation */
	UPROPERTY(EditAnywhere, Category = "AutoDriver")
	FBlackboardKeySelector TargetRotationKey;

	/** Blackboard key for look-at location */
	UPROPERTY(EditAnywhere, Category = "AutoDriver")
	FBlackboardKeySelector LookAtLocationKey;

	/** Blackboard key for look-at actor */
	UPROPERTY(EditAnywhere, Category = "AutoDriver")
	FBlackboardKeySelector LookAtActorKey;

	/** Rotation speed in degrees per second */
	UPROPERTY(EditAnywhere, Category = "AutoDriver", meta = (ClampMin = "1.0", ClampMax = "720.0"))
	float RotationSpeed;

	/** Acceptance angle in degrees */
	UPROPERTY(EditAnywhere, Category = "AutoDriver", meta = (ClampMin = "0.1", ClampMax = "45.0"))
	float AcceptanceAngle;

private:
	/** Get target rotation from blackboard */
	bool GetTargetRotation(UBehaviorTreeComponent& OwnerComp, FRotator& OutRotation) const;
};
