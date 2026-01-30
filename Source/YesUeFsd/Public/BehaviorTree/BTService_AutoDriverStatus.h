// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTService_AutoDriverStatus.generated.h"

/**
 * Behavior Tree service that monitors AutoDriver status and updates blackboard
 */
UCLASS()
class YESUEFSD_API UBTService_AutoDriverStatus : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_AutoDriverStatus();

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;

protected:
	/** Blackboard key to update with execution status */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsExecutingCommandKey;

	/** Blackboard key to update with reachability status */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsLocationReachableKey;

	/** Target location to check reachability */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetLocationKey;
};
