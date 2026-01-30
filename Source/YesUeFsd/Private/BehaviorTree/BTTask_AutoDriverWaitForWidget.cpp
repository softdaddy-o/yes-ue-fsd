// Copyright Epic Games, Inc. All Rights Reserved.

#include "BehaviorTree/BTTask_AutoDriverWaitForWidget.h"
#include "AutoDriver/AutoDriverComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTTask_AutoDriverWaitForWidget::UBTTask_AutoDriverWaitForWidget()
{
	NodeName = "Wait For Widget";
	bUseBlackboard = false;
	bWaitForAppear = true;
	Timeout = 10.0f;
	CommandTimeout = 0.0f; // No timeout for this task
}

EBTNodeResult::Type UBTTask_AutoDriverWaitForWidget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverComponent(OwnerComp);
	if (!AutoDriver)
	{
		if (bLogExecution)
		{
			UE_LOG(LogTemp, Warning, TEXT("BTTask_AutoDriverWaitForWidget: No AutoDriver component found"));
		}
		return EBTNodeResult::Failed;
	}

	FString TargetWidgetName = GetWidgetName(OwnerComp);
	if (TargetWidgetName.IsEmpty())
	{
		if (bLogExecution)
		{
			UE_LOG(LogTemp, Warning, TEXT("BTTask_AutoDriverWaitForWidget: Widget name is empty"));
		}
		return EBTNodeResult::Failed;
	}

	bool Success = false;

	if (bWaitForAppear)
	{
		if (bLogExecution)
		{
			UE_LOG(LogTemp, Log, TEXT("BTTask_AutoDriverWaitForWidget: Waiting for widget '%s' to appear (timeout: %.1fs)"),
				*TargetWidgetName, Timeout);
		}

		Success = AutoDriver->WaitForWidget(TargetWidgetName, Timeout);
	}
	else
	{
		if (bLogExecution)
		{
			UE_LOG(LogTemp, Log, TEXT("BTTask_AutoDriverWaitForWidget: Waiting for widget '%s' to disappear (timeout: %.1fs)"),
				*TargetWidgetName, Timeout);
		}

		Success = AutoDriver->WaitForWidgetToDisappear(TargetWidgetName, Timeout);
	}

	if (Success)
	{
		if (bLogExecution)
		{
			UE_LOG(LogTemp, Log, TEXT("BTTask_AutoDriverWaitForWidget: Widget '%s' condition met"),
				*TargetWidgetName);
		}
		return EBTNodeResult::Succeeded;
	}
	else
	{
		if (bLogExecution)
		{
			UE_LOG(LogTemp, Warning, TEXT("BTTask_AutoDriverWaitForWidget: Widget '%s' condition not met within timeout"),
				*TargetWidgetName);
		}
		return EBTNodeResult::Failed;
	}
}

FString UBTTask_AutoDriverWaitForWidget::GetStaticDescription() const
{
	FString Action = bWaitForAppear ? TEXT("Appear") : TEXT("Disappear");

	if (bUseBlackboard)
	{
		return FString::Printf(TEXT("Wait For Widget %s from BB Key: %s"), *Action, *WidgetNameKey.SelectedKeyName.ToString());
	}
	else
	{
		return FString::Printf(TEXT("Wait For Widget %s: %s"), *Action, *WidgetName);
	}
}

FString UBTTask_AutoDriverWaitForWidget::GetWidgetName(UBehaviorTreeComponent& OwnerComp) const
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
