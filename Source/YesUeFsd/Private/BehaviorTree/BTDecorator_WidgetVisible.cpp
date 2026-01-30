// Copyright Epic Games, Inc. All Rights Reserved.

#include "BehaviorTree/BTDecorator_WidgetVisible.h"
#include "AutoDriver/AutoDriverComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"

UBTDecorator_WidgetVisible::UBTDecorator_WidgetVisible()
{
	NodeName = "Widget Visible";
	bUseBlackboard = false;
	bInvertCondition = false;
}

bool UBTDecorator_WidgetVisible::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	FString TargetWidgetName = GetWidgetName(OwnerComp);
	if (TargetWidgetName.IsEmpty())
	{
		return false;
	}

	// Get AutoDriver component
	AAIController* AIController = Cast<AAIController>(OwnerComp.GetOwner());
	if (!AIController)
	{
		return false;
	}

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn)
	{
		return false;
	}

	UAutoDriverComponent* AutoDriver = ControlledPawn->FindComponentByClass<UAutoDriverComponent>();
	if (!AutoDriver)
	{
		return false;
	}

	// Check widget visibility
	bool bIsVisible = AutoDriver->IsWidgetVisible(TargetWidgetName);

	// Apply inversion if needed
	return bInvertCondition ? !bIsVisible : bIsVisible;
}

FString UBTDecorator_WidgetVisible::GetStaticDescription() const
{
	FString Condition = bInvertCondition ? TEXT("Not Visible") : TEXT("Visible");

	if (bUseBlackboard)
	{
		return FString::Printf(TEXT("Widget %s from BB Key: %s"), *Condition, *WidgetNameKey.SelectedKeyName.ToString());
	}
	else
	{
		return FString::Printf(TEXT("Widget %s: %s"), *Condition, *WidgetName);
	}
}

FString UBTDecorator_WidgetVisible::GetWidgetName(UBehaviorTreeComponent& OwnerComp) const
{
	if (bUseBlackboard)
	{
		UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
		if (BlackboardComp)
		{
			return BlackboardComp->GetValueAsString(WidgetNameKey.SelectedKeyName);
		}
		return FString();
	}
	else
	{
		return WidgetName;
	}
}
