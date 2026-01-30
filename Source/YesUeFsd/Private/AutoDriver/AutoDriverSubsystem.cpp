// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoDriver/AutoDriverSubsystem.h"
#include "AutoDriver/AutoDriverComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"

void UAutoDriverSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("AutoDriverSubsystem: Initialized"));
}

void UAutoDriverSubsystem::Deinitialize()
{
	// Clean up all auto drivers
	AutoDrivers.Empty();

	UE_LOG(LogTemp, Log, TEXT("AutoDriverSubsystem: Deinitialized"));

	Super::Deinitialize();
}

UAutoDriverComponent* UAutoDriverSubsystem::GetAutoDriverForPlayer(int32 PlayerIndex)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController)
	{
		return nullptr;
	}

	// For player index > 0, we'd need to iterate through all player controllers
	// For now, just support player 0
	if (PlayerIndex != 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("AutoDriverSubsystem: Multi-player support not yet implemented"));
		return nullptr;
	}

	return GetAutoDriverForController(PlayerController);
}

UAutoDriverComponent* UAutoDriverSubsystem::GetAutoDriverForController(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return nullptr;
	}

	// First, check if controller already has a component
	UAutoDriverComponent* ExistingComponent = PlayerController->FindComponentByClass<UAutoDriverComponent>();
	if (ExistingComponent)
	{
		return ExistingComponent;
	}

	// Check if pawn has a component
	if (APawn* Pawn = PlayerController->GetPawn())
	{
		ExistingComponent = Pawn->FindComponentByClass<UAutoDriverComponent>();
		if (ExistingComponent)
		{
			return ExistingComponent;
		}
	}

	return nullptr;
}

UAutoDriverComponent* UAutoDriverSubsystem::CreateAutoDriverForController(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("AutoDriverSubsystem: Cannot create auto driver - invalid player controller"));
		return nullptr;
	}

	// Check if one already exists
	UAutoDriverComponent* ExistingComponent = GetAutoDriverForController(PlayerController);
	if (ExistingComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("AutoDriverSubsystem: Auto driver already exists for this controller"));
		return ExistingComponent;
	}

	// Create new component on the player controller
	UAutoDriverComponent* NewComponent = NewObject<UAutoDriverComponent>(PlayerController, NAME_None, RF_Transient);
	if (NewComponent)
	{
		NewComponent->RegisterComponent();
		AutoDrivers.Add(NewComponent);

		UE_LOG(LogTemp, Log, TEXT("AutoDriverSubsystem: Created auto driver for player controller: %s"),
			*PlayerController->GetName());
	}

	return NewComponent;
}

void UAutoDriverSubsystem::RemoveAutoDriverFromController(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	UAutoDriverComponent* Component = GetAutoDriverForController(PlayerController);
	if (Component)
	{
		AutoDrivers.Remove(Component);
		Component->DestroyComponent();

		UE_LOG(LogTemp, Log, TEXT("AutoDriverSubsystem: Removed auto driver from controller: %s"),
			*PlayerController->GetName());
	}
}

TArray<UAutoDriverComponent*> UAutoDriverSubsystem::GetAllAutoDrivers()
{
	// Clean up any destroyed components first
	CleanupDestroyedAutoDrivers();

	TArray<UAutoDriverComponent*> Result;
	for (UAutoDriverComponent* Driver : AutoDrivers)
	{
		if (Driver && IsValid(Driver))
		{
			Result.Add(Driver);
		}
	}

	return Result;
}

void UAutoDriverSubsystem::SetAllAutoDriversEnabled(bool bEnabled)
{
	TArray<UAutoDriverComponent*> Drivers = GetAllAutoDrivers();
	for (UAutoDriverComponent* Driver : Drivers)
	{
		Driver->SetEnabled(bEnabled);
	}

	UE_LOG(LogTemp, Log, TEXT("AutoDriverSubsystem: %s all auto drivers (%d total)"),
		bEnabled ? TEXT("Enabled") : TEXT("Disabled"), Drivers.Num());
}

void UAutoDriverSubsystem::StopAllCommands()
{
	TArray<UAutoDriverComponent*> Drivers = GetAllAutoDrivers();
	for (UAutoDriverComponent* Driver : Drivers)
	{
		Driver->StopCurrentCommand();
	}

	UE_LOG(LogTemp, Log, TEXT("AutoDriverSubsystem: Stopped all commands on %d auto drivers"), Drivers.Num());
}

void UAutoDriverSubsystem::SetAutoCreateForNewPlayers(bool bEnabled)
{
	bAutoCreateForNewPlayers = bEnabled;
	UE_LOG(LogTemp, Log, TEXT("AutoDriverSubsystem: Auto-create for new players: %s"),
		bEnabled ? TEXT("Enabled") : TEXT("Disabled"));
}

int32 UAutoDriverSubsystem::GetActiveAutoDriverCount() const
{
	int32 Count = 0;
	for (const UAutoDriverComponent* Driver : AutoDrivers)
	{
		if (Driver && IsValid(Driver) && Driver->IsEnabled())
		{
			Count++;
		}
	}
	return Count;
}

void UAutoDriverSubsystem::OnPostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer)
{
	if (bAutoCreateForNewPlayers && NewPlayer)
	{
		CreateAutoDriverForController(NewPlayer);
	}
}

void UAutoDriverSubsystem::CleanupDestroyedAutoDrivers()
{
	AutoDrivers.RemoveAll([](const UAutoDriverComponent* Driver)
	{
		return !Driver || !IsValid(Driver);
	});
}
