// Copyright Epic Games, Inc. All Rights Reserved.

#include "Python/AutoDriverPythonBridge.h"
#include "AutoDriver/AutoDriverComponent.h"
#include "AutoDriver/AutoDriverSubsystem.h"
#include "Recording/ActionTimeline.h"
#include "Recording/ActionRecorder.h"
#include "Recording/ActionPlayback.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

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

	return AutoDriver->MoveToLocation(Location, AcceptanceRadius, SpeedMultiplier);
}

bool UAutoDriverPythonBridge::MoveToActor(AActor* TargetActor, float AcceptanceRadius, float SpeedMultiplier, int32 PlayerIndex)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverForPlayer(PlayerIndex);
	if (!AutoDriver || !TargetActor)
	{
		return false;
	}

	return AutoDriver->MoveToActor(TargetActor, AcceptanceRadius, SpeedMultiplier);
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

	return AutoDriver->RotateToRotation(Rotation);
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

	return AutoDriver->GetPathLengthToLocation(From, To);
}

FVector UAutoDriverPythonBridge::GetRandomReachableLocation(FVector Origin, float Radius, int32 PlayerIndex)
{
	UAutoDriverComponent* AutoDriver = GetAutoDriverForPlayer(PlayerIndex);
	if (!AutoDriver)
	{
		return FVector::ZeroVector;
	}

	return AutoDriver->GetRandomReachableLocation(Origin, Radius);
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
