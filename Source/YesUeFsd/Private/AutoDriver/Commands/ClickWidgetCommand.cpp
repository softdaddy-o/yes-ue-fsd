// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoDriver/Commands/ClickWidgetCommand.h"
#include "AutoDriver/WidgetQueryHelper.h"
#include "AutoDriver/UIInteractionHelper.h"
#include "Engine/World.h"

void UClickWidgetCommand::Initialize_Implementation(UObject* InContext)
{
	World = InContext ? InContext->GetWorld() : nullptr;
}

bool UClickWidgetCommand::Execute_Implementation()
{
	if (!World)
	{
		Result.Status = EAutoDriverCommandStatus::Failed;
		Result.Message = TEXT("Invalid world context");
		return false;
	}

	bIsRunning = true;
	ExecutionTime = 0.0f;
	TimeSinceLastRetry = 0.0f;
	bHasClicked = false;

	Result.Status = EAutoDriverCommandStatus::Running;
	Result.Message = TEXT("Searching for widget...");

	// Try to click immediately
	if (TryClickWidget())
	{
		bHasClicked = true;
		bIsRunning = false;
		Result.Status = EAutoDriverCommandStatus::Success;
		Result.Message = TEXT("Widget clicked successfully");
		Result.ExecutionTime = ExecutionTime;
		return true;
	}

	return true;
}

void UClickWidgetCommand::Tick_Implementation(float DeltaTime)
{
	if (!bIsRunning)
	{
		return;
	}

	ExecutionTime += DeltaTime;
	TimeSinceLastRetry += DeltaTime;

	// Check timeout
	if (ExecutionTime >= Timeout)
	{
		bIsRunning = false;
		Result.Status = EAutoDriverCommandStatus::Failed;
		Result.Message = FString::Printf(TEXT("Timeout: Widget not found after %.2f seconds"), Timeout);
		Result.ExecutionTime = ExecutionTime;
		return;
	}

	// Try to click widget at retry interval
	if (TimeSinceLastRetry >= RetryInterval)
	{
		TimeSinceLastRetry = 0.0f;

		if (TryClickWidget())
		{
			bHasClicked = true;
			bIsRunning = false;
			Result.Status = EAutoDriverCommandStatus::Success;
			Result.Message = TEXT("Widget clicked successfully");
			Result.ExecutionTime = ExecutionTime;
		}
	}
}

void UClickWidgetCommand::Cancel_Implementation()
{
	bIsRunning = false;
	Result.Status = EAutoDriverCommandStatus::Cancelled;
	Result.Message = TEXT("Command cancelled");
	Result.ExecutionTime = ExecutionTime;
}

bool UClickWidgetCommand::IsRunning_Implementation() const
{
	return bIsRunning;
}

FAutoDriverCommandResult UClickWidgetCommand::GetResult_Implementation() const
{
	return Result;
}

FString UClickWidgetCommand::GetDescription_Implementation() const
{
	FString Description = TEXT("ClickWidget");

	if (!QueryParams.Name.IsEmpty())
	{
		Description += FString::Printf(TEXT(" (Name: %s)"), *QueryParams.Name);
	}
	else if (!QueryParams.ClassName.IsEmpty())
	{
		Description += FString::Printf(TEXT(" (Class: %s)"), *QueryParams.ClassName);
	}
	else if (!QueryParams.Text.IsEmpty())
	{
		Description += FString::Printf(TEXT(" (Text: %s)"), *QueryParams.Text);
	}

	return Description;
}

UClickWidgetCommand* UClickWidgetCommand::CreateClickWidgetCommand(
	UObject* WorldContextObject,
	const FString& WidgetName,
	const FUIClickParams& InClickParams,
	float InTimeout)
{
	UClickWidgetCommand* Command = NewObject<UClickWidgetCommand>();
	Command->QueryParams = FWidgetQueryParams::ByWidgetName(WidgetName);
	Command->ClickParams = InClickParams;
	Command->Timeout = InTimeout;
	Command->Initialize(WorldContextObject);
	return Command;
}

UClickWidgetCommand* UClickWidgetCommand::CreateClickWidgetCommandByQuery(
	UObject* WorldContextObject,
	const FWidgetQueryParams& InQueryParams,
	const FUIClickParams& InClickParams,
	float InTimeout)
{
	UClickWidgetCommand* Command = NewObject<UClickWidgetCommand>();
	Command->QueryParams = InQueryParams;
	Command->ClickParams = InClickParams;
	Command->Timeout = InTimeout;
	Command->Initialize(WorldContextObject);
	return Command;
}

bool UClickWidgetCommand::TryClickWidget()
{
	if (!World)
	{
		return false;
	}

	// Find the widget
	FWidgetInfo WidgetInfo = UWidgetQueryHelper::FindWidget(World, QueryParams);

	if (!WidgetInfo.IsValid())
	{
		return false;
	}

	// Find the actual widget pointer
	UWidget* Widget = UWidgetQueryHelper::FindWidgetByPredicate(World, [this](UWidget* W)
	{
		if (!W)
		{
			return false;
		}

		switch (QueryParams.QueryType)
		{
			case EWidgetQueryType::ByName:
				return W->GetName().Equals(QueryParams.Name);
			case EWidgetQueryType::ByClass:
				return W->GetClass()->GetName().Contains(QueryParams.ClassName);
			case EWidgetQueryType::ByText:
			{
				FString WidgetText = UWidgetQueryHelper::GetWidgetText(W);
				return WidgetText.Contains(QueryParams.Text);
			}
			default:
				return false;
		}
	});

	if (!Widget)
	{
		return false;
	}

	// Click the widget
	return UUIInteractionHelper::ClickWidget(World, Widget, ClickParams);
}
