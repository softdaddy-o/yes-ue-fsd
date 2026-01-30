// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Recording/ActionTimeline.h"
#include "AutoDriver/AutoDriverTypes.h"
#include "ActionRecorder.generated.h"

class UAutoDriverComponent;
class APawn;
class APlayerController;

/**
 * Recording state enum
 */
UENUM(BlueprintType)
enum class ERecordingState : uint8
{
	Idle,
	Recording,
	Paused
};

/**
 * Delegate for recording events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecordingStateChanged, ERecordingState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionRecorded, const FRecordedAction&, Action);

/**
 * Component that records player actions to an ActionTimeline
 * Can be attached to PlayerController or Pawn to record gameplay
 */
UCLASS(ClassGroup=(AutoDriver), meta=(BlueprintSpawnableComponent))
class YESUEFSD_API UActionRecorder : public UActorComponent
{
	GENERATED_BODY()

public:
	UActionRecorder();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Recording Control

	/** Start recording actions */
	UFUNCTION(BlueprintCallable, Category = "Action Recorder")
	void StartRecording(const FString& RecordingName = TEXT("New Recording"));

	/** Stop recording and finalize the timeline */
	UFUNCTION(BlueprintCallable, Category = "Action Recorder")
	void StopRecording();

	/** Pause recording without finalizing */
	UFUNCTION(BlueprintCallable, Category = "Action Recorder")
	void PauseRecording();

	/** Resume recording after pause */
	UFUNCTION(BlueprintCallable, Category = "Action Recorder")
	void ResumeRecording();

	/** Check if currently recording */
	UFUNCTION(BlueprintCallable, Category = "Action Recorder")
	bool IsRecording() const { return RecordingState == ERecordingState::Recording; }

	/** Check if paused */
	UFUNCTION(BlueprintCallable, Category = "Action Recorder")
	bool IsPaused() const { return RecordingState == ERecordingState::Paused; }

	/** Get current recording state */
	UFUNCTION(BlueprintCallable, Category = "Action Recorder")
	ERecordingState GetRecordingState() const { return RecordingState; }

	// Timeline Management

	/** Get the current timeline being recorded */
	UFUNCTION(BlueprintCallable, Category = "Action Recorder")
	UActionTimeline* GetCurrentTimeline() const { return CurrentTimeline; }

	/** Create a new timeline */
	UFUNCTION(BlueprintCallable, Category = "Action Recorder")
	UActionTimeline* CreateNewTimeline();

	/** Set the timeline to record to */
	UFUNCTION(BlueprintCallable, Category = "Action Recorder")
	void SetTimeline(UActionTimeline* Timeline);

	/** Save current recording to file */
	UFUNCTION(BlueprintCallable, Category = "Action Recorder")
	bool SaveRecording(const FString& FilePath);

	/** Get elapsed recording time */
	UFUNCTION(BlueprintCallable, Category = "Action Recorder")
	float GetRecordingTime() const { return RecordingTime; }

	// Recording Settings

	/** Set maximum recording duration in seconds (0 = unlimited) */
	UFUNCTION(BlueprintCallable, Category = "Action Recorder")
	void SetMaxDuration(float Duration) { MaxRecordingDuration = Duration; }

	/** Set recording buffer size (max number of actions to keep in memory) */
	UFUNCTION(BlueprintCallable, Category = "Action Recorder")
	void SetBufferSize(int32 Size) { RecordingBufferSize = Size; }

	/** Enable/disable automatic recording on BeginPlay */
	UFUNCTION(BlueprintCallable, Category = "Action Recorder")
	void SetAutoStartRecording(bool bEnabled) { bAutoStartRecording = bEnabled; }

	/** Set minimum time between recorded actions */
	UFUNCTION(BlueprintCallable, Category = "Action Recorder")
	void SetRecordingInterval(float Interval) { RecordingInterval = Interval; }

	// Action Recording

	/** Manually record a movement action */
	UFUNCTION(BlueprintCallable, Category = "Action Recorder")
	void RecordMovementAction(const FVector& TargetLocation, const FAutoDriverMoveParams& Params);

	/** Manually record a rotation action */
	UFUNCTION(BlueprintCallable, Category = "Action Recorder")
	void RecordRotationAction(const FRotator& TargetRotation, const FAutoDriverRotateParams& Params);

	/** Manually record an input action */
	UFUNCTION(BlueprintCallable, Category = "Action Recorder")
	void RecordInputAction(const FString& ActionName, float Value, float Duration);

	/** Manually record a custom action */
	UFUNCTION(BlueprintCallable, Category = "Action Recorder")
	void RecordCustomAction(const FString& ActionType, const FString& ActionName, const FString& ActionData);

	// Events

	/** Called when recording state changes */
	UPROPERTY(BlueprintAssignable, Category = "Action Recorder")
	FOnRecordingStateChanged OnRecordingStateChanged;

	/** Called when an action is recorded */
	UPROPERTY(BlueprintAssignable, Category = "Action Recorder")
	FOnActionRecorded OnActionRecorded;

protected:
	/** Current recording state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Recorder")
	ERecordingState RecordingState;

	/** Timeline being recorded to */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Recorder")
	TObjectPtr<UActionTimeline> CurrentTimeline;

	/** Elapsed time since recording started */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Recorder")
	float RecordingTime;

	/** Maximum recording duration in seconds (0 = unlimited) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recording Settings")
	float MaxRecordingDuration;

	/** Maximum number of actions to keep in buffer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recording Settings")
	int32 RecordingBufferSize;

	/** Automatically start recording on BeginPlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recording Settings")
	bool bAutoStartRecording;

	/** Minimum time between recorded actions (for automatic recording) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recording Settings")
	float RecordingInterval;

	/** Record movement changes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recording Settings")
	bool bRecordMovement;

	/** Record rotation changes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recording Settings")
	bool bRecordRotation;

	/** Record input events */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recording Settings")
	bool bRecordInput;

	/** Movement threshold - minimum distance to record movement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recording Settings")
	float MovementThreshold;

	/** Rotation threshold - minimum angle to record rotation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recording Settings")
	float RotationThreshold;

private:
	/** Cached pawn reference */
	UPROPERTY()
	TObjectPtr<APawn> CachedPawn;

	/** Cached player controller */
	UPROPERTY()
	TObjectPtr<APlayerController> CachedController;

	/** Last recorded position */
	FVector LastRecordedPosition;

	/** Last recorded rotation */
	FRotator LastRecordedRotation;

	/** Time since last action was recorded */
	float TimeSinceLastAction;

	/** Initialize component references */
	void InitializeReferences();

	/** Update automatic recording */
	void UpdateRecording(float DeltaTime);

	/** Check and record movement changes */
	void CheckMovementChanges();

	/** Check and record rotation changes */
	void CheckRotationChanges();

	/** Apply buffer size limit */
	void EnforceBufferLimit();

	/** Set recording state and broadcast event */
	void SetRecordingState(ERecordingState NewState);
};
