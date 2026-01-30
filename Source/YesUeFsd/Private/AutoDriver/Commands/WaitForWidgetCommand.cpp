// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoDriver/Commands/WaitForWidgetCommand.h"
#include "AutoDriver/WidgetQueryHelper.h"
#include "Engine/World.h"

void UWaitForWidgetCommand::Initialize_Implementation(UObject* InContext)
{
	World = InContext ? InContext->GetWorld() : nullptr;
}

bool UWaitForWidgetCommand::Execute_Implementation()
{
	if (!World)
	{
		Result.Status = EAutoDriverCommandStatus::Failed;
		Result.Message = TEXT("Invalid world context");
		return false;
	}

	bIsRunning = true;
	ExecutionTime = 0.0f;
	TimeSinceLastPoll = 0.0f;

	Result.Status = EAutoDriverCommandStatus::Running;
	Result.Message = bWaitForAppear ? TEXT("Waiting for widget to appear...") : TEXT("Waiting for widget to disappear...");

	// Check immediately
	if (CheckCondition())
	{
		bIsRunning = false;
		Result.Status = EAutoDriverCommandStatus::Success;
		Result.Message = bWaitForAppear ? TEXT("Widget appeared") : TEXT("Widget disappeared");
		Result.ExecutionTime = ExecutionTime;
		return true;
	}

	return true;
}

void UWaitForWidgetCommand::Tick_Implementation(float DeltaTime)
{
	if (!bIsRunning)
	{
		return;
	}

	ExecutionTime += DeltaTime;
	TimeSinceLastPoll += DeltaTime;

	// Check timeout
	if (ExecutionTime >= Timeout)
	{
		bIsRunning = false;
		Result.Status = EAutoDriverCommandStatus::Failed;
		Result.Message = FString::Printf(TEXT("Timeout: Widget %s after %.2f seconds"),
			bWaitForAppear ? TEXT("did not appear") : TEXT("did not disappear"), Timeout);
		Result.ExecutionTime = ExecutionTime;
		return;
	}

	// Check condition at poll interval
	if (TimeSinceLastPoll >= PollInterval)
	{
		TimeSinceLastPoll = 0.0f;

		if (CheckCondition())
		{
			bIsRunning = false;
			Result.Status = EAutoDriverCommandStatus::Success;
			Result.Message = bWaitForAppear ? TEXT("Widget appeared") : TEXT("Widget disappeared");
			Result.ExecutionTime = ExecutionTime;
		}
	}
}

void UWaitForWidgetCommand::Cancel_Implementation()
{
	bIsRunning = false;
	Result.Status = EAutoDriverCommandStatus::Cancelled;
	Result.Message = TEXT("Command cancelled");
	Result.ExecutionTime = ExecutionTime;
}

bool UWaitForWidgetCommand::IsRunning_Implementation() const
{
	return bIsRunning;
}

FAutoDriverCommandResult UWaitForWidgetCommand::GetResult_Implementation() const
{
	return Result;
}

FString UWaitForWidgetCommand::GetDescription_Implementation() const
{
	FString Description = bWaitForAppear ? TEXT("WaitForWidget") : TEXT("WaitForWidgetToDisappear");

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

UWaitForWidgetCommand* UWaitForWidgetCommand::CreateWaitForWidgetCommand(
	UObject* WorldContextObject,
	const FString& WidgetName,
	float InTimeout)
{
	UWaitForWidgetCommand* Command = NewObject<UWaitForWidgetCommand>();
	Command->QueryParams = FWidgetQueryParams::ByWidgetName(WidgetName);
	Command->bWaitForAppear = true;
	Command->Timeout = InTimeout;
	Command->Initialize(WorldContextObject);
	return Command;
}

UWaitForWidgetCommand* UWaitForWidgetCommand::CreateWaitForWidgetToDisappearCommand(
	UObject* WorldContextObject,
	const FString& WidgetName,
	float InTimeout)
{
	UWaitForWidgetCommand* Command = NewObject<UWaitForWidgetCommand>();
	Command->QueryParams = FWidgetQueryParams::ByWidgetName(WidgetName);
	Command->bWaitForAppear = false;
	Command->Timeout = InTimeout;
	Command->Initialize(WorldContextObject);
	return Command;
}

UWaitForWidgetCommand* UWaitForWidgetCommand::CreateWaitForWidgetCommandByQuery(
	UObject* WorldContextObject,
	const FWidgetQueryParams& InQueryParams,
	bool bInWaitForAppear,
	float InTimeout)
{
	UWaitForWidgetCommand* Command = NewObject<UWaitForWidgetCommand>();
	Command->QueryParams = InQueryParams;
	Command->bWaitForAppear = bInWaitForAppear;
	Command->Timeout = InTimeout;
	Command->Initialize(WorldContextObject);
	return Command;
}

bool UWaitForWidgetCommand::CheckCondition()
{
	if (!World)
	{
		return false;
	}

	// Find the widget
	FWidgetInfo WidgetInfo = UWidgetQueryHelper::FindWidget(World, QueryParams);

	// If waiting for appear, check if widget is found
	// If waiting for disappear, check if widget is NOT found
	return bWaitForAppear ? WidgetInfo.IsValid() : !WidgetInfo.IsValid();
}
