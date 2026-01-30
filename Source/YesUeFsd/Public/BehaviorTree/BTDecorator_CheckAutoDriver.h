// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTDecorator_CheckAutoDriver.generated.h"

/**
 * Check type for AutoDriver decorator
 */
UENUM(BlueprintType)
enum class EAutoDriverCheckType : uint8
{
	/** Check if AutoDriver is currently executing a command */
	IsExecuting,
	/** Check if a location is reachable */
	IsReachable,
	/** Check if within distance of target */
	WithinDistance,
	/** Check if AutoDriver component exists */
	HasAutoDriver
};

/**
 * Behavior Tree decorator for AutoDriver condition checks
 */
UCLASS()
class YESUEFSD_API UBTDecorator_CheckAutoDriver : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_CheckAutoDriver();

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;

protected:
	/** Type of check to perform */
	UPROPERTY(EditAnywhere, Category = "AutoDriver")
	EAutoDriverCheckType CheckType;

	/** Target location for distance/reachability checks */
	UPROPERTY(EditAnywhere, Category = "AutoDriver", meta = (EditCondition = "CheckType == EAutoDriverCheckType::IsReachable || CheckType == EAutoDriverCheckType::WithinDistance"))
	FBlackboardKeySelector TargetLocationKey;

	/** Target distance for distance checks */
	UPROPERTY(EditAnywhere, Category = "AutoDriver", meta = (ClampMin = "0.0", EditCondition = "CheckType == EAutoDriverCheckType::WithinDistance"))
	float TargetDistance;

	/** Invert the condition */
	UPROPERTY(EditAnywhere, Category = "AutoDriver")
	bool bInvertCondition;
};
