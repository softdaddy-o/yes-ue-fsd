// Copyright Epic Games, Inc. All Rights Reserved.

#include "Recording/ActionTimeline.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

UActionTimeline::UActionTimeline()
{
	Metadata.RecordingName = TEXT("Untitled Recording");
	Metadata.CreatedAt = FDateTime::Now();
}

void UActionTimeline::AddAction(const FRecordedAction& Action)
{
	Actions.Add(Action);
	SortActions();
	UpdateMetadata();
}

void UActionTimeline::AddMovementAction(float Timestamp, const FVector& TargetLocation, const FAutoDriverMoveParams& Params)
{
	// Serialize movement params to JSON
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetNumberField(TEXT("X"), TargetLocation.X);
	JsonObject->SetNumberField(TEXT("Y"), TargetLocation.Y);
	JsonObject->SetNumberField(TEXT("Z"), TargetLocation.Z);
	JsonObject->SetNumberField(TEXT("AcceptanceRadius"), Params.AcceptanceRadius);
	JsonObject->SetNumberField(TEXT("SpeedMultiplier"), Params.SpeedMultiplier);
	JsonObject->SetBoolField(TEXT("ShouldSprint"), Params.bShouldSprint);
	JsonObject->SetNumberField(TEXT("MovementMode"), static_cast<int32>(Params.MovementMode));

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	FRecordedAction Action(Timestamp, TEXT("Movement"), TEXT("MoveToLocation"), JsonString);
	AddAction(Action);
}

void UActionTimeline::AddRotationAction(float Timestamp, const FRotator& TargetRotation, const FAutoDriverRotateParams& Params)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetNumberField(TEXT("Pitch"), TargetRotation.Pitch);
	JsonObject->SetNumberField(TEXT("Yaw"), TargetRotation.Yaw);
	JsonObject->SetNumberField(TEXT("Roll"), TargetRotation.Roll);
	JsonObject->SetNumberField(TEXT("RotationSpeed"), Params.RotationSpeed);
	JsonObject->SetNumberField(TEXT("AcceptanceAngle"), Params.AcceptanceAngle);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	FRecordedAction Action(Timestamp, TEXT("Rotation"), TEXT("RotateTo"), JsonString);
	AddAction(Action);
}

void UActionTimeline::AddInputAction(float Timestamp, const FString& ActionName, float Value, float Duration)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("ActionName"), ActionName);
	JsonObject->SetNumberField(TEXT("Value"), Value);
	JsonObject->SetNumberField(TEXT("Duration"), Duration);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	FRecordedAction Action(Timestamp, TEXT("Input"), ActionName, JsonString);
	AddAction(Action);
}

TArray<FRecordedAction> UActionTimeline::GetActionsInTimeRange(float StartTime, float EndTime) const
{
	TArray<FRecordedAction> Result;
	for (const FRecordedAction& Action : Actions)
	{
		if (Action.Timestamp >= StartTime && Action.Timestamp <= EndTime)
		{
			Result.Add(Action);
		}
	}
	return Result;
}

void UActionTimeline::Clear()
{
	Actions.Empty();
	UpdateMetadata();
}

void UActionTimeline::SetMetadata(const FRecordingMetadata& InMetadata)
{
	Metadata = InMetadata;
}

void UActionTimeline::SetRecordingInfo(const FString& Name, const FString& Description)
{
	Metadata.RecordingName = Name;
	Metadata.Description = Description;
}

void UActionTimeline::AddTag(const FString& Tag)
{
	if (!Metadata.Tags.Contains(Tag))
	{
		Metadata.Tags.Add(Tag);
	}
}

float UActionTimeline::GetDuration() const
{
	if (Actions.Num() == 0)
	{
		return 0.0f;
	}

	float MaxTime = 0.0f;
	for (const FRecordedAction& Action : Actions)
	{
		MaxTime = FMath::Max(MaxTime, Action.Timestamp);
	}
	return MaxTime;
}

FString UActionTimeline::ExportToJSON() const
{
	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);

	// Metadata
	TSharedPtr<FJsonObject> MetadataObject = MakeShareable(new FJsonObject);
	MetadataObject->SetStringField(TEXT("RecordingName"), Metadata.RecordingName);
	MetadataObject->SetStringField(TEXT("Description"), Metadata.Description);
	MetadataObject->SetStringField(TEXT("MapName"), Metadata.MapName);
	MetadataObject->SetStringField(TEXT("CreatedAt"), Metadata.CreatedAt.ToString());
	MetadataObject->SetNumberField(TEXT("Duration"), Metadata.Duration);
	MetadataObject->SetNumberField(TEXT("ActionCount"), Metadata.ActionCount);

	TArray<TSharedPtr<FJsonValue>> TagsArray;
	for (const FString& Tag : Metadata.Tags)
	{
		TagsArray.Add(MakeShareable(new FJsonValueString(Tag)));
	}
	MetadataObject->SetArrayField(TEXT("Tags"), TagsArray);

	RootObject->SetObjectField(TEXT("Metadata"), MetadataObject);

	// Actions
	TArray<TSharedPtr<FJsonValue>> ActionsArray;
	for (const FRecordedAction& Action : Actions)
	{
		TSharedPtr<FJsonObject> ActionObject = MakeShareable(new FJsonObject);
		ActionObject->SetNumberField(TEXT("Timestamp"), Action.Timestamp);
		ActionObject->SetStringField(TEXT("ActionType"), Action.ActionType);
		ActionObject->SetStringField(TEXT("ActionName"), Action.ActionName);
		ActionObject->SetStringField(TEXT("ActionData"), Action.ActionData);

		ActionsArray.Add(MakeShareable(new FJsonValueObject(ActionObject)));
	}
	RootObject->SetArrayField(TEXT("Actions"), ActionsArray);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	return OutputString;
}

bool UActionTimeline::ImportFromJSON(const FString& JSONString)
{
	TSharedPtr<FJsonObject> RootObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JSONString);

	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		return false;
	}

	// Clear existing data
	Actions.Empty();

	// Parse metadata
	if (RootObject->HasTypedField<EJson::Object>(TEXT("Metadata")))
	{
		TSharedPtr<FJsonObject> MetadataObject = RootObject->GetObjectField(TEXT("Metadata"));
		Metadata.RecordingName = MetadataObject->GetStringField(TEXT("RecordingName"));
		Metadata.Description = MetadataObject->GetStringField(TEXT("Description"));
		Metadata.MapName = MetadataObject->GetStringField(TEXT("MapName"));
		Metadata.Duration = MetadataObject->GetNumberField(TEXT("Duration"));
		Metadata.ActionCount = MetadataObject->GetIntegerField(TEXT("ActionCount"));

		FDateTime::Parse(MetadataObject->GetStringField(TEXT("CreatedAt")), Metadata.CreatedAt);

		const TArray<TSharedPtr<FJsonValue>>* TagsArray;
		if (MetadataObject->TryGetArrayField(TEXT("Tags"), TagsArray))
		{
			for (const TSharedPtr<FJsonValue>& TagValue : *TagsArray)
			{
				Metadata.Tags.Add(TagValue->AsString());
			}
		}
	}

	// Parse actions
	const TArray<TSharedPtr<FJsonValue>>* ActionsArray;
	if (RootObject->TryGetArrayField(TEXT("Actions"), ActionsArray))
	{
		for (const TSharedPtr<FJsonValue>& ActionValue : *ActionsArray)
		{
			TSharedPtr<FJsonObject> ActionObject = ActionValue->AsObject();

			FRecordedAction Action;
			Action.Timestamp = ActionObject->GetNumberField(TEXT("Timestamp"));
			Action.ActionType = ActionObject->GetStringField(TEXT("ActionType"));
			Action.ActionName = ActionObject->GetStringField(TEXT("ActionName"));
			Action.ActionData = ActionObject->GetStringField(TEXT("ActionData"));

			Actions.Add(Action);
		}
	}

	SortActions();
	return true;
}

bool UActionTimeline::SaveToFile(const FString& FilePath)
{
	FString JSONString = ExportToJSON();
	return FFileHelper::SaveStringToFile(JSONString, *FilePath);
}

bool UActionTimeline::LoadFromFile(const FString& FilePath)
{
	FString JSONString;
	if (!FFileHelper::LoadFileToString(JSONString, *FilePath))
	{
		return false;
	}

	return ImportFromJSON(JSONString);
}

void UActionTimeline::OptimizeTimeline()
{
	if (Actions.Num() < 2)
	{
		return;
	}

	TArray<FRecordedAction> OptimizedActions;
	OptimizedActions.Add(Actions[0]);

	for (int32 i = 1; i < Actions.Num(); ++i)
	{
		const FRecordedAction& Current = Actions[i];
		const FRecordedAction& Previous = OptimizedActions.Last();

		// Skip if it's a duplicate of the previous action
		if (Current.ActionType == Previous.ActionType &&
			Current.ActionName == Previous.ActionName &&
			Current.ActionData == Previous.ActionData)
		{
			continue;
		}

		OptimizedActions.Add(Current);
	}

	Actions = OptimizedActions;
	UpdateMetadata();
}

void UActionTimeline::Compress(float TimeTolerance)
{
	OptimizeTimeline();

	// Additional compression could be implemented here
	// For example, reducing precision of timestamps or data values
	if (TimeTolerance > 0.0f)
	{
		for (FRecordedAction& Action : Actions)
		{
			// Round timestamp to tolerance
			Action.Timestamp = FMath::RoundToFloat(Action.Timestamp / TimeTolerance) * TimeTolerance;
		}
		SortActions();
	}
}

void UActionTimeline::UpdateMetadata()
{
	Metadata.ActionCount = Actions.Num();
	Metadata.Duration = GetDuration();
}

void UActionTimeline::SortActions()
{
	Actions.Sort([](const FRecordedAction& A, const FRecordedAction& B)
	{
		return A.Timestamp < B.Timestamp;
	});
}
