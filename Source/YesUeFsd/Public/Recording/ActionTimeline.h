// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AutoDriver/AutoDriverTypes.h"
#include "ActionTimeline.generated.h"

/**
 * Represents a single recorded action at a specific timestamp
 */
USTRUCT(BlueprintType)
struct YESUEFSD_API FRecordedAction
{
	GENERATED_BODY()

	/** Timestamp in seconds since recording started */
	UPROPERTY(BlueprintReadWrite, Category = "Recording")
	float Timestamp = 0.0f;

	/** Type of action (Movement, Rotation, Input, Custom) */
	UPROPERTY(BlueprintReadWrite, Category = "Recording")
	FString ActionType;

	/** Action name or identifier */
	UPROPERTY(BlueprintReadWrite, Category = "Recording")
	FString ActionName;

	/** Serialized action data in JSON format */
	UPROPERTY(BlueprintReadWrite, Category = "Recording")
	FString ActionData;

	/** Additional metadata for the action */
	UPROPERTY(BlueprintReadWrite, Category = "Recording")
	TMap<FString, FString> Metadata;

	FRecordedAction()
		: Timestamp(0.0f)
	{
	}

	FRecordedAction(float InTimestamp, const FString& InActionType, const FString& InActionName, const FString& InActionData)
		: Timestamp(InTimestamp)
		, ActionType(InActionType)
		, ActionName(InActionName)
		, ActionData(InActionData)
	{
	}
};

/**
 * Metadata about a recording session
 */
USTRUCT(BlueprintType)
struct YESUEFSD_API FRecordingMetadata
{
	GENERATED_BODY()

	/** Name of the recording */
	UPROPERTY(BlueprintReadWrite, Category = "Recording")
	FString RecordingName;

	/** Description of the recording */
	UPROPERTY(BlueprintReadWrite, Category = "Recording")
	FString Description;

	/** Map or level name where this was recorded */
	UPROPERTY(BlueprintReadWrite, Category = "Recording")
	FString MapName;

	/** Date and time when recording was created */
	UPROPERTY(BlueprintReadWrite, Category = "Recording")
	FDateTime CreatedAt;

	/** Total duration of the recording in seconds */
	UPROPERTY(BlueprintReadWrite, Category = "Recording")
	float Duration = 0.0f;

	/** Number of actions in the recording */
	UPROPERTY(BlueprintReadWrite, Category = "Recording")
	int32 ActionCount = 0;

	/** Custom tags for categorization */
	UPROPERTY(BlueprintReadWrite, Category = "Recording")
	TArray<FString> Tags;

	/** Additional custom metadata */
	UPROPERTY(BlueprintReadWrite, Category = "Recording")
	TMap<FString, FString> CustomData;

	FRecordingMetadata()
		: CreatedAt(FDateTime::Now())
		, Duration(0.0f)
		, ActionCount(0)
	{
	}
};

/**
 * Timeline structure that stores timestamped actions
 * Supports serialization to/from JSON for persistence
 */
UCLASS(BlueprintType)
class YESUEFSD_API UActionTimeline : public UObject
{
	GENERATED_BODY()

public:
	UActionTimeline();

	// Action Management

	/** Add an action to the timeline */
	UFUNCTION(BlueprintCallable, Category = "Action Timeline")
	void AddAction(const FRecordedAction& Action);

	/** Add a movement action */
	UFUNCTION(BlueprintCallable, Category = "Action Timeline")
	void AddMovementAction(float Timestamp, const FVector& TargetLocation, const FAutoDriverMoveParams& Params);

	/** Add a rotation action */
	UFUNCTION(BlueprintCallable, Category = "Action Timeline")
	void AddRotationAction(float Timestamp, const FRotator& TargetRotation, const FAutoDriverRotateParams& Params);

	/** Add an input action */
	UFUNCTION(BlueprintCallable, Category = "Action Timeline")
	void AddInputAction(float Timestamp, const FString& ActionName, float Value, float Duration);

	/** Get all actions in the timeline */
	UFUNCTION(BlueprintCallable, Category = "Action Timeline")
	const TArray<FRecordedAction>& GetActions() const { return Actions; }

	/** Get actions within a time range */
	UFUNCTION(BlueprintCallable, Category = "Action Timeline")
	TArray<FRecordedAction> GetActionsInTimeRange(float StartTime, float EndTime) const;

	/** Clear all actions */
	UFUNCTION(BlueprintCallable, Category = "Action Timeline")
	void Clear();

	// Metadata Management

	/** Get recording metadata */
	UFUNCTION(BlueprintCallable, Category = "Action Timeline")
	FRecordingMetadata GetMetadata() const { return Metadata; }

	/** Set recording metadata */
	UFUNCTION(BlueprintCallable, Category = "Action Timeline")
	void SetMetadata(const FRecordingMetadata& InMetadata);

	/** Update metadata name and description */
	UFUNCTION(BlueprintCallable, Category = "Action Timeline")
	void SetRecordingInfo(const FString& Name, const FString& Description);

	/** Add a tag to the recording */
	UFUNCTION(BlueprintCallable, Category = "Action Timeline")
	void AddTag(const FString& Tag);

	// Timeline Properties

	/** Get total duration of the timeline */
	UFUNCTION(BlueprintCallable, Category = "Action Timeline")
	float GetDuration() const;

	/** Get number of actions */
	UFUNCTION(BlueprintCallable, Category = "Action Timeline")
	int32 GetActionCount() const { return Actions.Num(); }

	/** Check if timeline is empty */
	UFUNCTION(BlueprintCallable, Category = "Action Timeline")
	bool IsEmpty() const { return Actions.Num() == 0; }

	// Serialization

	/** Export timeline to JSON string */
	UFUNCTION(BlueprintCallable, Category = "Action Timeline")
	FString ExportToJSON() const;

	/** Import timeline from JSON string */
	UFUNCTION(BlueprintCallable, Category = "Action Timeline")
	bool ImportFromJSON(const FString& JSONString);

	/** Save timeline to file */
	UFUNCTION(BlueprintCallable, Category = "Action Timeline")
	bool SaveToFile(const FString& FilePath);

	/** Load timeline from file */
	UFUNCTION(BlueprintCallable, Category = "Action Timeline")
	bool LoadFromFile(const FString& FilePath);

	// Compression & Optimization

	/** Remove duplicate consecutive actions */
	UFUNCTION(BlueprintCallable, Category = "Action Timeline")
	void OptimizeTimeline();

	/** Compress timeline by removing redundant data */
	UFUNCTION(BlueprintCallable, Category = "Action Timeline")
	void Compress(float TimeTolerance = 0.01f);

protected:
	/** Array of recorded actions sorted by timestamp */
	UPROPERTY()
	TArray<FRecordedAction> Actions;

	/** Recording metadata */
	UPROPERTY()
	FRecordingMetadata Metadata;

	/** Helper to update metadata after modifications */
	void UpdateMetadata();

	/** Sort actions by timestamp */
	void SortActions();
};
