// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTask_AutoDriverBase.h"
#include "BTTask_AutoDriverWaitForWidget.generated.h"

/**
 * Behavior Tree Task: Wait For Widget
 *
 * Waits for a widget to appear or disappear in the UI hierarchy.
 * Useful for synchronizing with dynamic UI updates.
 */
UCLASS()
class YESUEFSD_API UBTTask_AutoDriverWaitForWidget : public UBTTask_AutoDriverBase
{
	GENERATED_BODY()

public:
	UBTTask_AutoDriverWaitForWidget();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	/** Widget name (static) */
	UPROPERTY(EditAnywhere, Category = "Widget", meta = (EditCondition = "!bUseBlackboard"))
	FString WidgetName;

	/** Use blackboard key for widget name */
	UPROPERTY(EditAnywhere, Category = "Widget")
	bool bUseBlackboard;

	/** Blackboard key containing widget name */
	UPROPERTY(EditAnywhere, Category = "Widget", meta = (EditCondition = "bUseBlackboard"))
	FBlackboardKeySelector WidgetNameKey;

	/** Wait for widget to appear (true) or disappear (false) */
	UPROPERTY(EditAnywhere, Category = "Widget")
	bool bWaitForAppear;

	/** Maximum wait time */
	UPROPERTY(EditAnywhere, Category = "Widget", meta = (ClampMin = "0.1"))
	float Timeout;

private:
	/** Get the widget name from static or blackboard */
	FString GetWidgetName(UBehaviorTreeComponent& OwnerComp) const;
};
