// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Recording/ActionTimeline.h"
#include "ActionPlayback.generated.h"

class UAutoDriverComponent;
class UActionTimeline;

/**
 * Playback state enum
 */
UENUM(BlueprintType)
enum class EPlaybackState : uint8
{
	Idle,
	Playing,
	Paused,
	Finished
};

/**
 * Playback mode enum
 */
UENUM(BlueprintType)
enum class EPlaybackMode : uint8
{
	/** Play once and stop */
	Once,
	/** Loop continuously */
	Loop,
	/** Loop a specific number of times */
	LoopCount
};

/**
 * Delegate for playback events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlaybackStateChanged, EPlaybackState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionExecuted, const FRecordedAction&, Action);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlaybackFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlaybackLoopCompleted, int32, LoopCount);

/**
 * Component that plays back recorded ActionTimelines
 * Executes actions at their recorded timestamps
 */
UCLASS(ClassGroup=(AutoDriver), meta=(BlueprintSpawnableComponent))
class YESUEFSD_API UActionPlayback : public UActorComponent
{
	GENERATED_BODY()

public:
	UActionPlayback();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Playback Control

	/** Start playing a timeline */
	UFUNCTION(BlueprintCallable, Category = "Action Playback")
	void Play(UActionTimeline* Timeline);

	/** Stop playback */
	UFUNCTION(BlueprintCallable, Category = "Action Playback")
	void Stop();

	/** Pause playback */
	UFUNCTION(BlueprintCallable, Category = "Action Playback")
	void Pause();

	/** Resume playback */
	UFUNCTION(BlueprintCallable, Category = "Action Playback")
	void Resume();

	/** Restart playback from beginning */
	UFUNCTION(BlueprintCallable, Category = "Action Playback")
	void Restart();

	/** Skip to specific time */
	UFUNCTION(BlueprintCallable, Category = "Action Playback")
	void SeekToTime(float Time);

	/** Check if currently playing */
	UFUNCTION(BlueprintCallable, Category = "Action Playback")
	bool IsPlaying() const { return PlaybackState == EPlaybackState::Playing; }

	/** Check if paused */
	UFUNCTION(BlueprintCallable, Category = "Action Playback")
	bool IsPaused() const { return PlaybackState == EPlaybackState::Paused; }

	/** Check if finished */
	UFUNCTION(BlueprintCallable, Category = "Action Playback")
	bool IsFinished() const { return PlaybackState == EPlaybackState::Finished; }

	/** Get current playback state */
	UFUNCTION(BlueprintCallable, Category = "Action Playback")
	EPlaybackState GetPlaybackState() const { return PlaybackState; }

	// Timeline Management

	/** Get the current timeline being played */
	UFUNCTION(BlueprintCallable, Category = "Action Playback")
	UActionTimeline* GetCurrentTimeline() const { return CurrentTimeline; }

	/** Set the timeline to play */
	UFUNCTION(BlueprintCallable, Category = "Action Playback")
	void SetTimeline(UActionTimeline* Timeline);

	/** Load and set timeline from file */
	UFUNCTION(BlueprintCallable, Category = "Action Playback")
	bool LoadAndPlayTimeline(const FString& FilePath);

	// Playback Settings

	/** Set playback speed (1.0 = normal, 2.0 = double speed, 0.5 = half speed) */
	UFUNCTION(BlueprintCallable, Category = "Action Playback")
	void SetPlaybackSpeed(float Speed);

	/** Get current playback speed */
	UFUNCTION(BlueprintCallable, Category = "Action Playback")
	float GetPlaybackSpeed() const { return PlaybackSpeed; }

	/** Set playback mode */
	UFUNCTION(BlueprintCallable, Category = "Action Playback")
	void SetPlaybackMode(EPlaybackMode Mode);

	/** Set number of loops for LoopCount mode */
	UFUNCTION(BlueprintCallable, Category = "Action Playback")
	void SetLoopCount(int32 Count) { DesiredLoopCount = Count; }

	/** Get current playback time */
	UFUNCTION(BlueprintCallable, Category = "Action Playback")
	float GetPlaybackTime() const { return PlaybackTime; }

	/** Get playback progress (0.0 to 1.0) */
	UFUNCTION(BlueprintCallable, Category = "Action Playback")
	float GetPlaybackProgress() const;

	/** Get current loop iteration */
	UFUNCTION(BlueprintCallable, Category = "Action Playback")
	int32 GetCurrentLoop() const { return CurrentLoopCount; }

	// AutoDriver Integration

	/** Set the AutoDriver component to use for playback */
	UFUNCTION(BlueprintCallable, Category = "Action Playback")
	void SetAutoDriver(UAutoDriverComponent* AutoDriver);

	/** Get the AutoDriver component being used */
	UFUNCTION(BlueprintCallable, Category = "Action Playback")
	UAutoDriverComponent* GetAutoDriver() const { return AutoDriverComponent; }

	// Events

	/** Called when playback state changes */
	UPROPERTY(BlueprintAssignable, Category = "Action Playback")
	FOnPlaybackStateChanged OnPlaybackStateChanged;

	/** Called when an action is executed */
	UPROPERTY(BlueprintAssignable, Category = "Action Playback")
	FOnActionExecuted OnActionExecuted;

	/** Called when playback finishes */
	UPROPERTY(BlueprintAssignable, Category = "Action Playback")
	FOnPlaybackFinished OnPlaybackFinished;

	/** Called when a loop completes */
	UPROPERTY(BlueprintAssignable, Category = "Action Playback")
	FOnPlaybackLoopCompleted OnPlaybackLoopCompleted;

protected:
	/** Current playback state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Playback")
	EPlaybackState PlaybackState;

	/** Timeline being played */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Playback")
	TObjectPtr<UActionTimeline> CurrentTimeline;

	/** AutoDriver component for executing actions */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Playback")
	TObjectPtr<UAutoDriverComponent> AutoDriverComponent;

	/** Current playback time */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Playback")
	float PlaybackTime;

	/** Playback speed multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playback Settings")
	float PlaybackSpeed;

	/** Playback mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playback Settings")
	EPlaybackMode PlaybackMode;

	/** Number of times to loop (for LoopCount mode) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playback Settings")
	int32 DesiredLoopCount;

	/** Current loop iteration */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Playback")
	int32 CurrentLoopCount;

	/** Automatically find and use AutoDriver on owner */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playback Settings")
	bool bAutoFindAutoDriver;

	/** Time tolerance for action execution (actions within this tolerance are executed together) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playback Settings")
	float TimeTolerance;

private:
	/** Index of next action to execute */
	int32 NextActionIndex;

	/** Initialize component references */
	void InitializeReferences();

	/** Update playback */
	void UpdatePlayback(float DeltaTime);

	/** Execute actions that should run at current time */
	void ExecutePendingActions();

	/** Execute a single action */
	void ExecuteAction(const FRecordedAction& Action);

	/** Execute movement action */
	void ExecuteMovementAction(const FRecordedAction& Action);

	/** Execute rotation action */
	void ExecuteRotationAction(const FRecordedAction& Action);

	/** Execute input action */
	void ExecuteInputAction(const FRecordedAction& Action);

	/** Execute UI click action */
	void ExecuteUIClickAction(const FRecordedAction& Action);

	/** Handle loop completion */
	void HandleLoopCompletion();

	/** Set playback state and broadcast event */
	void SetPlaybackState(EPlaybackState NewState);
};
