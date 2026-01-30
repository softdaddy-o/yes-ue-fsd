// Copyright Epic Games, Inc. All Rights Reserved.

#include "Recording/ActionPlayback.h"
#include "AutoDriver/AutoDriverComponent.h"
#include "AutoDriver/AutoDriverUITypes.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

UActionPlayback::UActionPlayback()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	PlaybackState = EPlaybackState::Idle;
	PlaybackTime = 0.0f;
	PlaybackSpeed = 1.0f;
	PlaybackMode = EPlaybackMode::Once;
	DesiredLoopCount = 1;
	CurrentLoopCount = 0;
	bAutoFindAutoDriver = true;
	TimeTolerance = 0.05f;  // 50ms tolerance
	NextActionIndex = 0;
}

void UActionPlayback::BeginPlay()
{
	Super::BeginPlay();
	InitializeReferences();
}

void UActionPlayback::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Stop();
	Super::EndPlay(EndPlayReason);
}

void UActionPlayback::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (PlaybackState == EPlaybackState::Playing)
	{
		UpdatePlayback(DeltaTime);
	}
}

void UActionPlayback::Play(UActionTimeline* Timeline)
{
	if (!Timeline || Timeline->IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot play empty or null timeline"));
		return;
	}

	SetTimeline(Timeline);

	// Reset playback state
	PlaybackTime = 0.0f;
	NextActionIndex = 0;
	CurrentLoopCount = 0;

	SetPlaybackState(EPlaybackState::Playing);
	UE_LOG(LogTemp, Log, TEXT("Started playback of timeline: %s"), *Timeline->GetMetadata().RecordingName);
}

void UActionPlayback::Stop()
{
	if (PlaybackState == EPlaybackState::Idle)
	{
		return;
	}

	SetPlaybackState(EPlaybackState::Idle);
	PlaybackTime = 0.0f;
	NextActionIndex = 0;

	UE_LOG(LogTemp, Log, TEXT("Stopped playback"));
}

void UActionPlayback::Pause()
{
	if (PlaybackState == EPlaybackState::Playing)
	{
		SetPlaybackState(EPlaybackState::Paused);
		UE_LOG(LogTemp, Log, TEXT("Paused playback"));
	}
}

void UActionPlayback::Resume()
{
	if (PlaybackState == EPlaybackState::Paused)
	{
		SetPlaybackState(EPlaybackState::Playing);
		UE_LOG(LogTemp, Log, TEXT("Resumed playback"));
	}
}

void UActionPlayback::Restart()
{
	if (CurrentTimeline)
	{
		Play(CurrentTimeline);
	}
}

void UActionPlayback::SeekToTime(float Time)
{
	if (!CurrentTimeline)
	{
		return;
	}

	PlaybackTime = FMath::Clamp(Time, 0.0f, CurrentTimeline->GetDuration());

	// Find next action to execute
	const TArray<FRecordedAction>& Actions = CurrentTimeline->GetActions();
	NextActionIndex = 0;

	for (int32 i = 0; i < Actions.Num(); ++i)
	{
		if (Actions[i].Timestamp > PlaybackTime)
		{
			NextActionIndex = i;
			break;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Seeked to time: %.2f"), PlaybackTime);
}

void UActionPlayback::SetTimeline(UActionTimeline* Timeline)
{
	if (Timeline)
	{
		CurrentTimeline = Timeline;
	}
}

bool UActionPlayback::LoadAndPlayTimeline(const FString& FilePath)
{
	UActionTimeline* Timeline = NewObject<UActionTimeline>(this);
	if (Timeline->LoadFromFile(FilePath))
	{
		Play(Timeline);
		return true;
	}

	UE_LOG(LogTemp, Error, TEXT("Failed to load timeline from: %s"), *FilePath);
	return false;
}

void UActionPlayback::SetPlaybackSpeed(float Speed)
{
	PlaybackSpeed = FMath::Max(0.0f, Speed);
	UE_LOG(LogTemp, Log, TEXT("Set playback speed to: %.2fx"), PlaybackSpeed);
}

void UActionPlayback::SetPlaybackMode(EPlaybackMode Mode)
{
	PlaybackMode = Mode;
}

float UActionPlayback::GetPlaybackProgress() const
{
	if (!CurrentTimeline || CurrentTimeline->GetDuration() <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(PlaybackTime / CurrentTimeline->GetDuration(), 0.0f, 1.0f);
}

void UActionPlayback::SetAutoDriver(UAutoDriverComponent* AutoDriver)
{
	AutoDriverComponent = AutoDriver;
}

void UActionPlayback::InitializeReferences()
{
	if (bAutoFindAutoDriver && !AutoDriverComponent)
	{
		// Try to find AutoDriver on owner
		if (AActor* Owner = GetOwner())
		{
			AutoDriverComponent = Owner->FindComponentByClass<UAutoDriverComponent>();
		}
	}
}

void UActionPlayback::UpdatePlayback(float DeltaTime)
{
	if (!CurrentTimeline || !AutoDriverComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot play: Missing timeline or AutoDriver"));
		Stop();
		return;
	}

	// Update playback time
	PlaybackTime += DeltaTime * PlaybackSpeed;

	// Execute pending actions
	ExecutePendingActions();

	// Check if playback finished
	if (PlaybackTime >= CurrentTimeline->GetDuration())
	{
		HandleLoopCompletion();
	}
}

void UActionPlayback::ExecutePendingActions()
{
	if (!CurrentTimeline)
	{
		return;
	}

	const TArray<FRecordedAction>& Actions = CurrentTimeline->GetActions();

	// Execute all actions that should run at or before current time
	while (NextActionIndex < Actions.Num())
	{
		const FRecordedAction& Action = Actions[NextActionIndex];

		// Check if action should be executed
		if (Action.Timestamp <= PlaybackTime + TimeTolerance)
		{
			ExecuteAction(Action);
			OnActionExecuted.Broadcast(Action);
			NextActionIndex++;
		}
		else
		{
			break;
		}
	}
}

void UActionPlayback::ExecuteAction(const FRecordedAction& Action)
{
	if (Action.ActionType == TEXT("Movement"))
	{
		ExecuteMovementAction(Action);
	}
	else if (Action.ActionType == TEXT("Rotation"))
	{
		ExecuteRotationAction(Action);
	}
	else if (Action.ActionType == TEXT("Input"))
	{
		ExecuteInputAction(Action);
	}
	else if (Action.ActionType == TEXT("UIClick"))
	{
		ExecuteUIClickAction(Action);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Unknown action type: %s"), *Action.ActionType);
	}
}

void UActionPlayback::ExecuteMovementAction(const FRecordedAction& Action)
{
	// Parse JSON data
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Action.ActionData);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse movement action data"));
		return;
	}

	// Extract movement parameters
	FVector TargetLocation;
	TargetLocation.X = JsonObject->GetNumberField(TEXT("X"));
	TargetLocation.Y = JsonObject->GetNumberField(TEXT("Y"));
	TargetLocation.Z = JsonObject->GetNumberField(TEXT("Z"));

	FAutoDriverMoveParams Params;
	Params.TargetLocation = TargetLocation;
	Params.AcceptanceRadius = JsonObject->GetNumberField(TEXT("AcceptanceRadius"));
	Params.SpeedMultiplier = JsonObject->GetNumberField(TEXT("SpeedMultiplier"));
	Params.bShouldSprint = JsonObject->GetBoolField(TEXT("ShouldSprint"));
	Params.MovementMode = static_cast<EAutoDriverMovementMode>(
		static_cast<int32>(JsonObject->GetNumberField(TEXT("MovementMode"))));

	// Execute movement command
	AutoDriverComponent->MoveToLocation(TargetLocation, Params.AcceptanceRadius, Params.SpeedMultiplier, Params.bShouldSprint);

	UE_LOG(LogTemp, Verbose, TEXT("Executed movement to: %s"), *TargetLocation.ToString());
}

void UActionPlayback::ExecuteRotationAction(const FRecordedAction& Action)
{
	// Parse JSON data
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Action.ActionData);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse rotation action data"));
		return;
	}

	// Extract rotation parameters
	FRotator TargetRotation;
	TargetRotation.Pitch = JsonObject->GetNumberField(TEXT("Pitch"));
	TargetRotation.Yaw = JsonObject->GetNumberField(TEXT("Yaw"));
	TargetRotation.Roll = JsonObject->GetNumberField(TEXT("Roll"));

	FAutoDriverRotateParams Params;
	Params.TargetRotation = TargetRotation;
	Params.RotationSpeed = JsonObject->GetNumberField(TEXT("RotationSpeed"));
	Params.AcceptanceAngle = JsonObject->GetNumberField(TEXT("AcceptanceAngle"));

	// Execute rotation command
	AutoDriverComponent->RotateToRotation(TargetRotation);

	UE_LOG(LogTemp, Verbose, TEXT("Executed rotation to: %s"), *TargetRotation.ToString());
}

void UActionPlayback::ExecuteInputAction(const FRecordedAction& Action)
{
	// Parse JSON data
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Action.ActionData);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse input action data"));
		return;
	}

	// Extract input parameters
	FString ActionName = JsonObject->GetStringField(TEXT("ActionName"));
	float Value = JsonObject->GetNumberField(TEXT("Value"));
	float Duration = JsonObject->GetNumberField(TEXT("Duration"));

	// Execute input command
	if (Value > 0.0f)
	{
		AutoDriverComponent->PressButton(FName(*ActionName));
	}

	UE_LOG(LogTemp, Verbose, TEXT("Executed input: %s (Value: %.2f)"), *ActionName, Value);
}

void UActionPlayback::ExecuteUIClickAction(const FRecordedAction& Action)
{
	// Parse JSON data
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Action.ActionData);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse UI click action data"));
		return;
	}

	// Extract click parameters
	FString ClickTypeStr = JsonObject->GetStringField(TEXT("ClickType"));
	int32 ClickCount = JsonObject->GetIntegerField(TEXT("ClickCount"));

	// Build click params
	FUIClickParams ClickParams;
	ClickParams.ClickCount = ClickCount;

	if (ClickTypeStr == TEXT("Right"))
	{
		ClickParams.ClickType = EUIClickType::Right;
	}
	else if (ClickTypeStr == TEXT("Middle"))
	{
		ClickParams.ClickType = EUIClickType::Middle;
	}
	else
	{
		ClickParams.ClickType = EUIClickType::Left;
	}

	// Execute UI click command
	FString WidgetName = Action.ActionName;
	AutoDriverComponent->ClickWidget(WidgetName, ClickParams);

	UE_LOG(LogTemp, Verbose, TEXT("Executed UI click: %s (Type: %s, Count: %d)"),
		*WidgetName, *ClickTypeStr, ClickCount);
}

void UActionPlayback::HandleLoopCompletion()
{
	CurrentLoopCount++;
	OnPlaybackLoopCompleted.Broadcast(CurrentLoopCount);

	bool bShouldContinue = false;

	switch (PlaybackMode)
	{
	case EPlaybackMode::Once:
		bShouldContinue = false;
		break;

	case EPlaybackMode::Loop:
		bShouldContinue = true;
		break;

	case EPlaybackMode::LoopCount:
		bShouldContinue = (CurrentLoopCount < DesiredLoopCount);
		break;
	}

	if (bShouldContinue)
	{
		// Restart playback
		PlaybackTime = 0.0f;
		NextActionIndex = 0;
		UE_LOG(LogTemp, Log, TEXT("Loop %d completed, restarting playback"), CurrentLoopCount);
	}
	else
	{
		// Finish playback
		SetPlaybackState(EPlaybackState::Finished);
		OnPlaybackFinished.Broadcast();
		UE_LOG(LogTemp, Log, TEXT("Playback finished after %d loop(s)"), CurrentLoopCount);
	}
}

void UActionPlayback::SetPlaybackState(EPlaybackState NewState)
{
	if (PlaybackState != NewState)
	{
		PlaybackState = NewState;
		OnPlaybackStateChanged.Broadcast(NewState);
	}
}
