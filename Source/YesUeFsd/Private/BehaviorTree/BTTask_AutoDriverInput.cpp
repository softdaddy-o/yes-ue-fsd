// Copyright Epic Games, Inc. All Rights Reserved.

#include "BehaviorTree/BTTask_AutoDriverInput.h"
#include "AutoDriver/AutoDriverComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_AutoDriverInput::UBTTask_AutoDriverInput()
{
	NodeName = "AutoDriver Input";
	InputActionName = NAME_None;
	InputType = EAutoDriverInputType::Press;
	HoldDuration = 1.0f;
	bWaitForCompletion = false;
	bNotifyTick = true;

	InputActionKey.AddNameFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_AutoDriverInput, InputActionKey));
}

EBTNodeResult::Type UBTTask_AutoDriverInput::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverComponent(OwnerComp);
	if (!AutoDriver)
	{
		UE_LOG(LogTemp, Error, TEXT("BTTask_AutoDriverInput: No AutoDriver component found"));
		return EBTNodeResult::Failed;
	}

	FName ActionName = InputActionName;

	// Try to get action name from blackboard if key is set
	if (InputActionKey.SelectedKeyName != NAME_None)
	{
		UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
		if (BlackboardComp)
		{
			ActionName = BlackboardComp->GetValueAsName(InputActionKey.SelectedKeyName);
		}
	}

	if (ActionName == NAME_None)
	{
		UE_LOG(LogTemp, Error, TEXT("BTTask_AutoDriverInput: No input action name specified"));
		return EBTNodeResult::Failed;
	}

	if (bLogExecution)
	{
		UE_LOG(LogTemp, Log, TEXT("BTTask_AutoDriverInput: Triggering input '%s'"), *ActionName.ToString());
	}

	bool bSuccess = false;

	switch (InputType)
	{
	case EAutoDriverInputType::Press:
		bSuccess = AutoDriver->PressButton(ActionName);
		break;

	case EAutoDriverInputType::Hold:
		{
			FBTInputTaskMemory* Memory = reinterpret_cast<FBTInputTaskMemory*>(NodeMemory);
			Memory->RemainingHoldTime = HoldDuration;
			bSuccess = AutoDriver->PressButton(ActionName);
		}
		break;

	case EAutoDriverInputType::HoldIndefinite:
		bSuccess = AutoDriver->PressButton(ActionName);
		break;
	}

	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_AutoDriverInput: Failed to trigger input"));
		return EBTNodeResult::Failed;
	}

	// If we need to wait for completion or hold, return in progress
	if (bWaitForCompletion || InputType == EAutoDriverInputType::Hold)
	{
		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Succeeded;
}

void UBTTask_AutoDriverInput::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (InputType == EAutoDriverInputType::Hold)
	{
		FBTInputTaskMemory* Memory = reinterpret_cast<FBTInputTaskMemory*>(NodeMemory);
		Memory->RemainingHoldTime -= DeltaSeconds;

		if (Memory->RemainingHoldTime <= 0.0f)
		{
			// Release the button
			FName ActionName = InputActionName;
			if (InputActionKey.SelectedKeyName != NAME_None)
			{
				UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
				if (BlackboardComp)
				{
					ActionName = BlackboardComp->GetValueAsName(InputActionKey.SelectedKeyName);
				}
			}

			// Note: AutoDriver would need a ReleaseButton method - for now we just finish
			if (bLogExecution)
			{
				UE_LOG(LogTemp, Log, TEXT("BTTask_AutoDriverInput: Hold completed"));
			}

			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
	}
	else if (bWaitForCompletion)
	{
		UAutoDriverComponent* AutoDriver = GetAutoDriverComponent(OwnerComp);
		if (AutoDriver && !AutoDriver->IsExecutingCommand())
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
	}
}

FString UBTTask_AutoDriverInput::GetStaticDescription() const
{
	FString ActionNameStr;

	if (InputActionKey.SelectedKeyName != NAME_None)
	{
		ActionNameStr = InputActionKey.SelectedKeyName.ToString();
	}
	else if (InputActionName != NAME_None)
	{
		ActionNameStr = InputActionName.ToString();
	}
	else
	{
		ActionNameStr = TEXT("(Not Set)");
	}

	FString Description = FString::Printf(TEXT("Input: %s"), *ActionNameStr);

	switch (InputType)
	{
	case EAutoDriverInputType::Press:
		Description += TEXT(" (Press)");
		break;
	case EAutoDriverInputType::Hold:
		Description += FString::Printf(TEXT(" (Hold %.1fs)"), HoldDuration);
		break;
	case EAutoDriverInputType::HoldIndefinite:
		Description += TEXT(" (Hold)");
		break;
	}

	return Description;
}

uint16 UBTTask_AutoDriverInput::GetInstanceMemorySize() const
{
	return sizeof(FBTInputTaskMemory);
}
