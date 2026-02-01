// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_AutoDriverBase.generated.h"

class UAutoDriverComponent;
class AAIController;

/**
 * Base class for AutoDriver Behavior Tree tasks
 * Provides common functionality for accessing AutoDriver components
 */
UCLASS(Abstract)
class YESUEFSD_API UBTTask_AutoDriverBase : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_AutoDriverBase();

protected:
	/** Get AutoDriver component from the controlled pawn */
	UAutoDriverComponent* GetAutoDriverComponent(UBehaviorTreeComponent& OwnerComp) const;

	/** Get AI controller from the behavior tree */
	AAIController* GetAIController(UBehaviorTreeComponent& OwnerComp) const;

	/** Override: Handle task abortion (UE 5.7 signature) */
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** Timeout for AutoDriver commands (0 = no timeout) */
	UPROPERTY(EditAnywhere, Category = "AutoDriver", meta = (ClampMin = "0.0"))
	float CommandTimeout;

	/** Should log execution details */
	UPROPERTY(EditAnywhere, Category = "AutoDriver")
	bool bLogExecution;

private:
	/** Handle for command timeout */
	FTimerHandle TimeoutHandle;

	/** Cached owner component */
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;

	/** Start timeout timer */
	void StartTimeoutTimer(UBehaviorTreeComponent& OwnerComp);

	/** Clear timeout timer */
	void ClearTimeoutTimer();

	/** Handle timeout */
	void HandleTimeout();
};
