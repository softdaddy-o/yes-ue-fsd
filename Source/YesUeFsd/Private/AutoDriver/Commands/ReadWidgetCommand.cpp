// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoDriver/Commands/ReadWidgetCommand.h"
#include "AutoDriver/WidgetQueryHelper.h"
#include "Engine/World.h"

void UReadWidgetCommand::Initialize_Implementation(UObject* InContext)
{
	World = InContext ? InContext->GetWorld() : nullptr;
}

bool UReadWidgetCommand::Execute_Implementation()
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
	bHasFound = false;

	Result.Status = EAutoDriverCommandStatus::Running;
	Result.Message = TEXT("Reading widget...");

	// Try to read immediately
	if (TryReadWidget())
	{
		bHasFound = true;
		bIsRunning = false;
		Result.Status = EAutoDriverCommandStatus::Success;
		Result.Message = TEXT("Widget read successfully");
		Result.ExecutionTime = ExecutionTime;
		return true;
	}

	return true;
}

void UReadWidgetCommand::Tick_Implementation(float DeltaTime)
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

	// Try to read widget at retry interval
	if (TimeSinceLastRetry >= RetryInterval)
	{
		TimeSinceLastRetry = 0.0f;

		if (TryReadWidget())
		{
			bHasFound = true;
			bIsRunning = false;
			Result.Status = EAutoDriverCommandStatus::Success;
			Result.Message = TEXT("Widget read successfully");
			Result.ExecutionTime = ExecutionTime;
		}
	}
}

void UReadWidgetCommand::Cancel_Implementation()
{
	bIsRunning = false;
	Result.Status = EAutoDriverCommandStatus::Cancelled;
	Result.Message = TEXT("Command cancelled");
	Result.ExecutionTime = ExecutionTime;
}

bool UReadWidgetCommand::IsRunning_Implementation() const
{
	return bIsRunning;
}

FAutoDriverCommandResult UReadWidgetCommand::GetResult_Implementation() const
{
	return Result;
}

FString UReadWidgetCommand::GetDescription_Implementation() const
{
	FString Description = TEXT("ReadWidget");

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

UReadWidgetCommand* UReadWidgetCommand::CreateReadWidgetCommand(
	UObject* WorldContextObject,
	const FString& WidgetName,
	float InTimeout)
{
	UReadWidgetCommand* Command = NewObject<UReadWidgetCommand>();
	Command->QueryParams = FWidgetQueryParams::ByWidgetName(WidgetName);
	Command->Timeout = InTimeout;
	Command->Initialize(WorldContextObject);
	return Command;
}

UReadWidgetCommand* UReadWidgetCommand::CreateReadWidgetCommandByQuery(
	UObject* WorldContextObject,
	const FWidgetQueryParams& InQueryParams,
	float InTimeout)
{
	UReadWidgetCommand* Command = NewObject<UReadWidgetCommand>();
	Command->QueryParams = InQueryParams;
	Command->Timeout = InTimeout;
	Command->Initialize(WorldContextObject);
	return Command;
}

bool UReadWidgetCommand::TryReadWidget()
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

	// Store the results
	FoundWidgetInfo = WidgetInfo;
	FoundText = WidgetInfo.TextContent;

	return true;
}
