// Copyright Epic Games, Inc. All Rights Reserved.

#include "BehaviorTree/BTTask_AutoDriverClickWidget.h"
#include "AutoDriver/AutoDriverComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTTask_AutoDriverClickWidget::UBTTask_AutoDriverClickWidget()
{
	NodeName = "Click Widget";
	bUseBlackboard = false;
	ClickType = EUIClickType::Left;
	ClickCount = 1;
	RetryAttempts = 3;
	RetryDelay = 0.5f;
	CommandTimeout = 5.0f;
}

EBTNodeResult::Type UBTTask_AutoDriverClickWidget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverComponent(OwnerComp);
	if (!AutoDriver)
	{
		if (bLogExecution)
		{
			UE_LOG(LogTemp, Warning, TEXT("BTTask_AutoDriverClickWidget: No AutoDriver component found"));
		}
		return EBTNodeResult::Failed;
	}

	FString TargetWidgetName = GetWidgetName(OwnerComp);
	if (TargetWidgetName.IsEmpty())
	{
		if (bLogExecution)
		{
			UE_LOG(LogTemp, Warning, TEXT("BTTask_AutoDriverClickWidget: Widget name is empty"));
		}
		return EBTNodeResult::Failed;
	}

	// Setup click parameters
	FUIClickParams ClickParams;
	ClickParams.ClickType = ClickType;
	ClickParams.ClickCount = ClickCount;

	// Try to click with retries
	for (int32 i = 0; i <= RetryAttempts; ++i)
	{
		bool Success = AutoDriver->ClickWidget(TargetWidgetName, ClickParams);

		if (Success)
		{
			if (bLogExecution)
			{
				UE_LOG(LogTemp, Log, TEXT("BTTask_AutoDriverClickWidget: Successfully clicked widget '%s' on attempt %d"),
					*TargetWidgetName, i + 1);
			}
			return EBTNodeResult::Succeeded;
		}

		// Wait before retry (except on last attempt)
		if (i < RetryAttempts)
		{
			if (bLogExecution)
			{
				UE_LOG(LogTemp, Warning, TEXT("BTTask_AutoDriverClickWidget: Widget '%s' not found, retrying in %.1fs..."),
					*TargetWidgetName, RetryDelay);
			}
			FPlatformProcess::Sleep(RetryDelay);
		}
	}

	if (bLogExecution)
	{
		UE_LOG(LogTemp, Error, TEXT("BTTask_AutoDriverClickWidget: Failed to click widget '%s' after %d attempts"),
			*TargetWidgetName, RetryAttempts + 1);
	}

	return EBTNodeResult::Failed;
}

FString UBTTask_AutoDriverClickWidget::GetStaticDescription() const
{
	if (bUseBlackboard)
	{
		return FString::Printf(TEXT("Click Widget from BB Key: %s"), *WidgetNameKey.SelectedKeyName.ToString());
	}
	else
	{
		return FString::Printf(TEXT("Click Widget: %s"), *WidgetName);
	}
}

FString UBTTask_AutoDriverClickWidget::GetWidgetName(UBehaviorTreeComponent& OwnerComp) const
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
