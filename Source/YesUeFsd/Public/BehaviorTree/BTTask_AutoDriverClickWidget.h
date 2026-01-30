// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTask_AutoDriverBase.h"
#include "AutoDriver/AutoDriverUITypes.h"
#include "BTTask_AutoDriverClickWidget.generated.h"

/**
 * Behavior Tree Task: Click Widget
 *
 * Finds and clicks a widget in the UI hierarchy.
 * Can use widget name or blackboard key containing the name.
 */
UCLASS()
class YESUEFSD_API UBTTask_AutoDriverClickWidget : public UBTTask_AutoDriverBase
{
	GENERATED_BODY()

public:
	UBTTask_AutoDriverClickWidget();

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

	/** Click type */
	UPROPERTY(EditAnywhere, Category = "Widget")
	EUIClickType ClickType;

	/** Number of clicks */
	UPROPERTY(EditAnywhere, Category = "Widget", meta = (ClampMin = "1"))
	int32 ClickCount;

	/** Retry attempts if widget not found */
	UPROPERTY(EditAnywhere, Category = "Widget", meta = (ClampMin = "0"))
	int32 RetryAttempts;

	/** Delay between retry attempts */
	UPROPERTY(EditAnywhere, Category = "Widget", meta = (ClampMin = "0.1"))
	float RetryDelay;

private:
	/** Get the widget name from static or blackboard */
	FString GetWidgetName(UBehaviorTreeComponent& OwnerComp) const;
};
