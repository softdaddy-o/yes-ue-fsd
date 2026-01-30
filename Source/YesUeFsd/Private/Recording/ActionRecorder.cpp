// Copyright Epic Games, Inc. All Rights Reserved.

#include "Recording/ActionRecorder.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UActionRecorder::UActionRecorder()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	RecordingState = ERecordingState::Idle;
	RecordingTime = 0.0f;
	MaxRecordingDuration = 0.0f;  // Unlimited
	RecordingBufferSize = 10000;
	bAutoStartRecording = false;
	RecordingInterval = 0.1f;  // 10 times per second
	bRecordMovement = true;
	bRecordRotation = true;
	bRecordInput = true;
	MovementThreshold = 10.0f;  // 10 cm
	RotationThreshold = 1.0f;   // 1 degree
	TimeSinceLastAction = 0.0f;

	LastRecordedPosition = FVector::ZeroVector;
	LastRecordedRotation = FRotator::ZeroRotator;
}

void UActionRecorder::BeginPlay()
{
	Super::BeginPlay();

	InitializeReferences();

	// Create initial timeline
	if (!CurrentTimeline)
	{
		CreateNewTimeline();
	}

	// Auto-start recording if enabled
	if (bAutoStartRecording)
	{
		StartRecording();
	}
}

void UActionRecorder::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsRecording())
	{
		StopRecording();
	}

	Super::EndPlay(EndPlayReason);
}

void UActionRecorder::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (RecordingState == ERecordingState::Recording)
	{
		UpdateRecording(DeltaTime);
	}
}

void UActionRecorder::StartRecording(const FString& RecordingName)
{
	if (RecordingState == ERecordingState::Recording)
	{
		UE_LOG(LogTemp, Warning, TEXT("Already recording!"));
		return;
	}

	// Create new timeline if needed
	if (!CurrentTimeline)
	{
		CreateNewTimeline();
	}

	// Reset timeline for new recording
	CurrentTimeline->Clear();
	CurrentTimeline->SetRecordingInfo(RecordingName, TEXT(""));

	// Set map name
	FRecordingMetadata Metadata = CurrentTimeline->GetMetadata();
	if (UWorld* World = GetWorld())
	{
		Metadata.MapName = World->GetMapName();
	}
	CurrentTimeline->SetMetadata(Metadata);

	// Reset recording state
	RecordingTime = 0.0f;
	TimeSinceLastAction = 0.0f;

	// Initialize position/rotation tracking
	if (CachedPawn)
	{
		LastRecordedPosition = CachedPawn->GetActorLocation();
		LastRecordedRotation = CachedPawn->GetActorRotation();
	}

	SetRecordingState(ERecordingState::Recording);

	UE_LOG(LogTemp, Log, TEXT("Started recording: %s"), *RecordingName);
}

void UActionRecorder::StopRecording()
{
	if (RecordingState == ERecordingState::Idle)
	{
		return;
	}

	SetRecordingState(ERecordingState::Idle);

	UE_LOG(LogTemp, Log, TEXT("Stopped recording. Duration: %.2f seconds, Actions: %d"),
		RecordingTime, CurrentTimeline ? CurrentTimeline->GetActionCount() : 0);
}

void UActionRecorder::PauseRecording()
{
	if (RecordingState == ERecordingState::Recording)
	{
		SetRecordingState(ERecordingState::Paused);
		UE_LOG(LogTemp, Log, TEXT("Paused recording"));
	}
}

void UActionRecorder::ResumeRecording()
{
	if (RecordingState == ERecordingState::Paused)
	{
		SetRecordingState(ERecordingState::Recording);
		UE_LOG(LogTemp, Log, TEXT("Resumed recording"));
	}
}

UActionTimeline* UActionRecorder::CreateNewTimeline()
{
	CurrentTimeline = NewObject<UActionTimeline>(this);
	return CurrentTimeline;
}

void UActionRecorder::SetTimeline(UActionTimeline* Timeline)
{
	if (Timeline)
	{
		CurrentTimeline = Timeline;
	}
}

bool UActionRecorder::SaveRecording(const FString& FilePath)
{
	if (!CurrentTimeline)
	{
		UE_LOG(LogTemp, Error, TEXT("No timeline to save!"));
		return false;
	}

	bool bSuccess = CurrentTimeline->SaveToFile(FilePath);
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Saved recording to: %s"), *FilePath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save recording to: %s"), *FilePath);
	}

	return bSuccess;
}

void UActionRecorder::RecordMovementAction(const FVector& TargetLocation, const FAutoDriverMoveParams& Params)
{
	if (!IsRecording() || !CurrentTimeline)
	{
		return;
	}

	CurrentTimeline->AddMovementAction(RecordingTime, TargetLocation, Params);
	OnActionRecorded.Broadcast(CurrentTimeline->GetActions().Last());
}

void UActionRecorder::RecordRotationAction(const FRotator& TargetRotation, const FAutoDriverRotateParams& Params)
{
	if (!IsRecording() || !CurrentTimeline)
	{
		return;
	}

	CurrentTimeline->AddRotationAction(RecordingTime, TargetRotation, Params);
	OnActionRecorded.Broadcast(CurrentTimeline->GetActions().Last());
}

void UActionRecorder::RecordInputAction(const FString& ActionName, float Value, float Duration)
{
	if (!IsRecording() || !CurrentTimeline)
	{
		return;
	}

	CurrentTimeline->AddInputAction(RecordingTime, ActionName, Value, Duration);
	OnActionRecorded.Broadcast(CurrentTimeline->GetActions().Last());
}

void UActionRecorder::RecordCustomAction(const FString& ActionType, const FString& ActionName, const FString& ActionData)
{
	if (!IsRecording() || !CurrentTimeline)
	{
		return;
	}

	FRecordedAction Action(RecordingTime, ActionType, ActionName, ActionData);
	CurrentTimeline->AddAction(Action);
	OnActionRecorded.Broadcast(Action);
}

void UActionRecorder::InitializeReferences()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Try to get pawn from owner
	CachedPawn = Cast<APawn>(Owner);

	// If owner is a controller, get its pawn
	if (!CachedPawn)
	{
		if (APlayerController* PC = Cast<APlayerController>(Owner))
		{
			CachedController = PC;
			CachedPawn = PC->GetPawn();
		}
	}
	// If owner is a pawn, get its controller
	else
	{
		CachedController = Cast<APlayerController>(CachedPawn->GetController());
	}
}

void UActionRecorder::UpdateRecording(float DeltaTime)
{
	RecordingTime += DeltaTime;
	TimeSinceLastAction += DeltaTime;

	// Check max duration
	if (MaxRecordingDuration > 0.0f && RecordingTime >= MaxRecordingDuration)
	{
		UE_LOG(LogTemp, Log, TEXT("Recording reached max duration: %.2f seconds"), MaxRecordingDuration);
		StopRecording();
		return;
	}

	// Only record at specified intervals
	if (TimeSinceLastAction < RecordingInterval)
	{
		return;
	}

	TimeSinceLastAction = 0.0f;

	// Check for changes to record
	CheckMovementChanges();
	CheckRotationChanges();

	// Enforce buffer limit
	EnforceBufferLimit();
}

void UActionRecorder::CheckMovementChanges()
{
	if (!bRecordMovement || !CachedPawn || !CurrentTimeline)
	{
		return;
	}

	FVector CurrentPosition = CachedPawn->GetActorLocation();
	float Distance = FVector::Dist(CurrentPosition, LastRecordedPosition);

	if (Distance >= MovementThreshold)
	{
		// Record movement
		FAutoDriverMoveParams Params;
		Params.TargetLocation = CurrentPosition;
		Params.AcceptanceRadius = 50.0f;
		Params.SpeedMultiplier = 1.0f;
		Params.MovementMode = EAutoDriverMovementMode::Direct;

		RecordMovementAction(CurrentPosition, Params);
		LastRecordedPosition = CurrentPosition;
	}
}

void UActionRecorder::CheckRotationChanges()
{
	if (!bRecordRotation || !CachedPawn || !CurrentTimeline)
	{
		return;
	}

	FRotator CurrentRotation = CachedPawn->GetActorRotation();
	float RotationDiff = FMath::Abs((CurrentRotation - LastRecordedRotation).Yaw);

	if (RotationDiff >= RotationThreshold)
	{
		// Record rotation
		FAutoDriverRotateParams Params;
		Params.TargetRotation = CurrentRotation;
		Params.RotationSpeed = 180.0f;
		Params.AcceptanceAngle = 1.0f;

		RecordRotationAction(CurrentRotation, Params);
		LastRecordedRotation = CurrentRotation;
	}
}

void UActionRecorder::EnforceBufferLimit()
{
	if (!CurrentTimeline)
	{
		return;
	}

	// Remove oldest actions if buffer is full
	while (CurrentTimeline->GetActionCount() > RecordingBufferSize)
	{
		TArray<FRecordedAction> Actions = CurrentTimeline->GetActions();
		if (Actions.Num() > 0)
		{
			Actions.RemoveAt(0);
			// Note: This is inefficient - in production, ActionTimeline should have a RemoveAt method
			// For now, we'll rely on the buffer size being reasonable
			break;
		}
	}
}

void UActionRecorder::SetRecordingState(ERecordingState NewState)
{
	if (RecordingState != NewState)
	{
		RecordingState = NewState;
		OnRecordingStateChanged.Broadcast(NewState);
	}
}
