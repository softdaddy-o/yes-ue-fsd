// Copyright Epic Games, Inc. All Rights Reserved.

#include "Python/AutoDriverPythonBridge.h"
#include "AutoDriver/AutoDriverComponent.h"
#include "AutoDriver/AutoDriverSubsystem.h"
#include "AutoDriver/AutoDriverUITypes.h"
#include "AutoDriver/WidgetQueryHelper.h"
#include "AutoDriver/UIInteractionHelper.h"
#include "Recording/ActionTimeline.h"
#include "Recording/ActionRecorder.h"
#include "Recording/ActionPlayback.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

UAutoDriverComponent* UAutoDriverPythonBridge::GetAutoDriverForPlayer(int32 PlayerIndex)
{
	UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return nullptr;
	}

	UAutoDriverSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UAutoDriverSubsystem>();
	if (!Subsystem)
	{
		return nullptr;
	}

	return Subsystem->GetAutoDriverForPlayer(PlayerIndex);
}

UAutoDriverSubsystem* UAutoDriverPythonBridge::GetAutoDriverSubsystem()
{
	UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return nullptr;
	}

	return World->GetGameInstance()->GetSubsystem<UAutoDriverSubsystem>();
}

bool UAutoDriverPythonBridge::MoveToLocation(FVector Location, float AcceptanceRadius, float SpeedMultiplier, int32 PlayerIndex)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverForPlayer(PlayerIndex);
	if (!AutoDriver)
	{
		UE_LOG(LogTemp, Error, TEXT("Python: No AutoDriver found for player %d"), PlayerIndex);
		return false;
	}

	FAutoDriverMoveParams Params;
	Params.TargetLocation = Location;
	Params.AcceptanceRadius = AcceptanceRadius;
	Params.SpeedMultiplier = SpeedMultiplier;
	return AutoDriver->MoveToLocation(Params);
}

bool UAutoDriverPythonBridge::MoveToActor(AActor* TargetActor, float AcceptanceRadius, float SpeedMultiplier, int32 PlayerIndex)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverForPlayer(PlayerIndex);
	if (!AutoDriver || !TargetActor)
	{
		return false;
	}

	return AutoDriver->MoveToActor(TargetActor, AcceptanceRadius);
}

void UAutoDriverPythonBridge::StopMovement(int32 PlayerIndex)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverForPlayer(PlayerIndex);
	if (AutoDriver)
	{
		AutoDriver->StopMovement();
	}
}

bool UAutoDriverPythonBridge::RotateToRotation(FRotator Rotation, int32 PlayerIndex)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverForPlayer(PlayerIndex);
	if (!AutoDriver)
	{
		return false;
	}

	FAutoDriverRotateParams Params;
	Params.TargetRotation = Rotation;
	return AutoDriver->RotateToRotation(Params);
}

bool UAutoDriverPythonBridge::LookAtLocation(FVector Location, int32 PlayerIndex)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverForPlayer(PlayerIndex);
	if (!AutoDriver)
	{
		return false;
	}

	return AutoDriver->LookAtLocation(Location);
}

bool UAutoDriverPythonBridge::LookAtActor(AActor* Actor, int32 PlayerIndex)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverForPlayer(PlayerIndex);
	if (!AutoDriver || !Actor)
	{
		return false;
	}

	return AutoDriver->LookAtActor(Actor);
}

bool UAutoDriverPythonBridge::PressButton(const FString& ActionName, int32 PlayerIndex)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverForPlayer(PlayerIndex);
	if (!AutoDriver)
	{
		return false;
	}

	return AutoDriver->PressButton(FName(*ActionName));
}

bool UAutoDriverPythonBridge::SetAxisValue(const FString& AxisName, float Value, int32 PlayerIndex)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverForPlayer(PlayerIndex);
	if (!AutoDriver)
	{
		return false;
	}

	return AutoDriver->SetAxisValue(FName(*AxisName), Value);
}

bool UAutoDriverPythonBridge::IsLocationReachable(FVector Location, int32 PlayerIndex)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverForPlayer(PlayerIndex);
	if (!AutoDriver)
	{
		return false;
	}

	return AutoDriver->IsLocationReachable(Location);
}

float UAutoDriverPythonBridge::GetPathLength(FVector From, FVector To, int32 PlayerIndex)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverForPlayer(PlayerIndex);
	if (!AutoDriver)
	{
		return -1.0f;
	}

	return AutoDriver->GetPathLengthToLocation(To);
}

FVector UAutoDriverPythonBridge::GetRandomReachableLocation(FVector Origin, float Radius, int32 PlayerIndex)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverForPlayer(PlayerIndex);
	if (!AutoDriver)
	{
		return FVector::ZeroVector;
	}

	FVector OutLocation;
	if (AutoDriver->GetRandomReachableLocation(Radius, OutLocation))
	{
		return OutLocation;
	}
	return FVector::ZeroVector;
}

bool UAutoDriverPythonBridge::IsExecutingCommand(int32 PlayerIndex)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverForPlayer(PlayerIndex);
	if (!AutoDriver)
	{
		return false;
	}

	return AutoDriver->IsExecutingCommand();
}

void UAutoDriverPythonBridge::StopCurrentCommand(int32 PlayerIndex)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverForPlayer(PlayerIndex);
	if (AutoDriver)
	{
		AutoDriver->StopCurrentCommand();
	}
}

UActionRecorder* UAutoDriverPythonBridge::CreateRecorder(int32 PlayerIndex)
{
	UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return nullptr;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, PlayerIndex);
	if (!PC)
	{
		return nullptr;
	}

	UActionRecorder* Recorder = NewObject<UActionRecorder>(PC);
	Recorder->RegisterComponent();
	return Recorder;
}

UActionPlayback* UAutoDriverPythonBridge::CreatePlayback(int32 PlayerIndex)
{
	UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return nullptr;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, PlayerIndex);
	if (!PC)
	{
		return nullptr;
	}

	UActionPlayback* Playback = NewObject<UActionPlayback>(PC);
	Playback->RegisterComponent();

	// Auto-set AutoDriver
	UAutoDriverComponent* AutoDriver = GetAutoDriverForPlayer(PlayerIndex);
	if (AutoDriver)
	{
		Playback->SetAutoDriver(AutoDriver);
	}

	return Playback;
}

UActionTimeline* UAutoDriverPythonBridge::LoadTimeline(const FString& FilePath)
{
	UActionTimeline* Timeline = NewObject<UActionTimeline>();
	if (Timeline->LoadFromFile(FilePath))
	{
		return Timeline;
	}

	return nullptr;
}

bool UAutoDriverPythonBridge::SaveTimeline(UActionTimeline* Timeline, const FString& FilePath)
{
	if (!Timeline)
	{
		return false;
	}

	return Timeline->SaveToFile(FilePath);
}

void UAutoDriverPythonBridge::WaitForCommandCompletion(float Timeout, int32 PlayerIndex)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverForPlayer(PlayerIndex);
	if (!AutoDriver)
	{
		return;
	}

	float ElapsedTime = 0.0f;
	const float PollInterval = 0.1f;

	while (AutoDriver->IsExecutingCommand() && ElapsedTime < Timeout)
	{
		FPlatformProcess::Sleep(PollInterval);
		ElapsedTime += PollInterval;
	}

	if (ElapsedTime >= Timeout)
	{
		UE_LOG(LogTemp, Warning, TEXT("Python: WaitForCommandCompletion timed out after %.2fs"), Timeout);
	}
}

FVector UAutoDriverPythonBridge::GetPlayerLocation(int32 PlayerIndex)
{
	UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return FVector::ZeroVector;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, PlayerIndex);
	if (!PC || !PC->GetPawn())
	{
		return FVector::ZeroVector;
	}

	return PC->GetPawn()->GetActorLocation();
}

FRotator UAutoDriverPythonBridge::GetPlayerRotation(int32 PlayerIndex)
{
	UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return FRotator::ZeroRotator;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, PlayerIndex);
	if (!PC || !PC->GetPawn())
	{
		return FRotator::ZeroRotator;
	}

	return PC->GetPawn()->GetActorRotation();
}

void UAutoDriverPythonBridge::ExecuteConsoleCommand(const FString& Command)
{
	UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::ReturnNull);
	if (World)
	{
		GEngine->Exec(World, *Command);
	}
}

TArray<AActor*> UAutoDriverPythonBridge::GetAllActorsOfClass(UClass* ActorClass)
{
	TArray<AActor*> OutActors;

	UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::ReturnNull);
	if (World && ActorClass)
	{
		UGameplayStatics::GetAllActorsOfClass(World, ActorClass, OutActors);
	}

	return OutActors;
}

AActor* UAutoDriverPythonBridge::FindActorByName(const FString& ActorName)
{
	UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (It->GetName() == ActorName)
		{
			return *It;
		}
	}

	return nullptr;
}

// ========================================
// UI Methods
// ========================================

bool UAutoDriverPythonBridge::ClickWidget(const FString& WidgetName, int32 PlayerIndex)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverForPlayer(PlayerIndex);
	if (!AutoDriver)
	{
		UE_LOG(LogTemp, Error, TEXT("Python: No AutoDriver found for player %d"), PlayerIndex);
		return false;
	}

	return AutoDriver->ClickWidget(WidgetName);
}

bool UAutoDriverPythonBridge::ClickWidgetWithParams(const FString& WidgetName, const FString& ClickType, int32 ClickCount, int32 PlayerIndex)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverForPlayer(PlayerIndex);
	if (!AutoDriver)
	{
		UE_LOG(LogTemp, Error, TEXT("Python: No AutoDriver found for player %d"), PlayerIndex);
		return false;
	}

	FUIClickParams ClickParams;
	ClickParams.ClickCount = ClickCount;
	ClickParams.ClickType = FUIClickParams::StringToClickType(ClickType);

	return AutoDriver->ClickWidget(WidgetName, ClickParams);
}

bool UAutoDriverPythonBridge::WaitForWidget(const FString& WidgetName, float Timeout, int32 PlayerIndex)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverForPlayer(PlayerIndex);
	if (!AutoDriver)
	{
		UE_LOG(LogTemp, Error, TEXT("Python: No AutoDriver found for player %d"), PlayerIndex);
		return false;
	}

	return AutoDriver->WaitForWidget(WidgetName, Timeout);
}

bool UAutoDriverPythonBridge::WaitForWidgetGone(const FString& WidgetName, float Timeout, int32 PlayerIndex)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverForPlayer(PlayerIndex);
	if (!AutoDriver)
	{
		UE_LOG(LogTemp, Error, TEXT("Python: No AutoDriver found for player %d"), PlayerIndex);
		return false;
	}

	return AutoDriver->WaitForWidgetToDisappear(WidgetName, Timeout);
}

FString UAutoDriverPythonBridge::FindWidgetByName(const FString& WidgetName, int32 PlayerIndex)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverForPlayer(PlayerIndex);
	if (!AutoDriver)
	{
		return TEXT("{}");
	}

	FWidgetInfo WidgetInfo = AutoDriver->FindWidget(WidgetName);

	// Convert to JSON
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetBoolField(TEXT("found"), WidgetInfo.bFound);
	JsonObject->SetStringField(TEXT("name"), WidgetInfo.Name);
	JsonObject->SetStringField(TEXT("className"), WidgetInfo.ClassName);
	JsonObject->SetStringField(TEXT("path"), WidgetInfo.Path);
	JsonObject->SetNumberField(TEXT("positionX"), WidgetInfo.Position.X);
	JsonObject->SetNumberField(TEXT("positionY"), WidgetInfo.Position.Y);
	JsonObject->SetNumberField(TEXT("sizeX"), WidgetInfo.Size.X);
	JsonObject->SetNumberField(TEXT("sizeY"), WidgetInfo.Size.Y);
	JsonObject->SetBoolField(TEXT("visible"), WidgetInfo.bIsVisible);
	JsonObject->SetBoolField(TEXT("enabled"), WidgetInfo.bIsEnabled);
	JsonObject->SetStringField(TEXT("text"), WidgetInfo.TextContent);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	return OutputString;
}

FString UAutoDriverPythonBridge::FindWidgets(const FString& QueryJson, int32 PlayerIndex)
{
	// Parse JSON query (simplified - expects {"name": "...", "className": "...", etc.})
	// For now, return empty array
	return TEXT("[]");
}

FString UAutoDriverPythonBridge::GetWidgetText(const FString& WidgetName, int32 PlayerIndex)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverForPlayer(PlayerIndex);
	if (!AutoDriver)
	{
		return FString();
	}

	return AutoDriver->GetWidgetText(WidgetName);
}

bool UAutoDriverPythonBridge::IsWidgetVisible(const FString& WidgetName, int32 PlayerIndex)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverForPlayer(PlayerIndex);
	if (!AutoDriver)
	{
		return false;
	}

	return AutoDriver->IsWidgetVisible(WidgetName);
}

FString UAutoDriverPythonBridge::GetAllButtons(int32 PlayerIndex)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverForPlayer(PlayerIndex);
	if (!AutoDriver)
	{
		return TEXT("[]");
	}

	TArray<FWidgetInfo> Buttons = AutoDriver->GetAllButtons();

	// Convert to JSON array
	TArray<TSharedPtr<FJsonValue>> JsonArray;

	for (const FWidgetInfo& Button : Buttons)
	{
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
		JsonObject->SetStringField(TEXT("name"), Button.Name);
		JsonObject->SetStringField(TEXT("className"), Button.ClassName);
		JsonObject->SetStringField(TEXT("path"), Button.Path);
		JsonObject->SetNumberField(TEXT("positionX"), Button.Position.X);
		JsonObject->SetNumberField(TEXT("positionY"), Button.Position.Y);
		JsonObject->SetNumberField(TEXT("sizeX"), Button.Size.X);
		JsonObject->SetNumberField(TEXT("sizeY"), Button.Size.Y);
		JsonObject->SetBoolField(TEXT("visible"), Button.bIsVisible);
		JsonObject->SetStringField(TEXT("text"), Button.TextContent);

		JsonArray.Add(MakeShareable(new FJsonValueObject(JsonObject)));
	}

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonArray, Writer);

	return OutputString;
}

bool UAutoDriverPythonBridge::ClickAtPosition(float X, float Y, int32 PlayerIndex)
{
	UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return false;
	}

	return UUIInteractionHelper::ClickAtScreenPosition(World, FVector2D(X, Y));
}
