// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_WidgetVisible.generated.h"

/**
 * Behavior Tree Decorator: Widget Visible
 *
 * Decorator that checks if a widget is visible before allowing subtree execution.
 * Useful for UI-driven behavior tree logic.
 */
UCLASS()
class YESUEFSD_API UBTDecorator_WidgetVisible : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_WidgetVisible();

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;

protected:
	/** Widget name to check */
	UPROPERTY(EditAnywhere, Category = "Widget", meta = (EditCondition = "!bUseBlackboard"))
	FString WidgetName;

	/** Use blackboard key for widget name */
	UPROPERTY(EditAnywhere, Category = "Widget")
	bool bUseBlackboard;

	/** Blackboard key containing widget name */
	UPROPERTY(EditAnywhere, Category = "Widget", meta = (EditCondition = "bUseBlackboard"))
	FBlackboardKeySelector WidgetNameKey;

	/** Invert the condition (check if NOT visible) */
	UPROPERTY(EditAnywhere, Category = "Widget")
	bool bInvertCondition;

private:
	/** Get the widget name from static or blackboard */
	FString GetWidgetName(UBehaviorTreeComponent& OwnerComp) const;
};
