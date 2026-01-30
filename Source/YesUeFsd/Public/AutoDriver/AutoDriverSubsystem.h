// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AutoDriver/AutoDriverTypes.h"
#include "AutoDriverSubsystem.generated.h"

class UAutoDriverComponent;

/**
 * Auto Driver Subsystem
 *
 * Global subsystem for managing automatic player control.
 * Provides centralized access to auto driver functionality and manages
 * multiple auto driver components across the game instance.
 *
 * Usage:
 *   UAutoDriverSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UAutoDriverSubsystem>();
 *   UAutoDriverComponent* Driver = Subsystem->GetAutoDriverForPlayer(0);
 */
UCLASS()
class YESUEFSD_API UAutoDriverSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ========================================
	// Subsystem Interface
	// ========================================

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================
	// Auto Driver Management
	// ========================================

	/**
	 * Get auto driver component for a specific player
	 * @param PlayerIndex Player index (0 for first player)
	 * @return Auto driver component, or nullptr if not found
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver")
	UAutoDriverComponent* GetAutoDriverForPlayer(int32 PlayerIndex = 0);

	/**
	 * Get auto driver component for a specific player controller
	 * @param PlayerController The player controller
	 * @return Auto driver component, or nullptr if not found
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver")
	UAutoDriverComponent* GetAutoDriverForController(APlayerController* PlayerController);

	/**
	 * Create and attach auto driver to a player controller
	 * @param PlayerController The player controller to attach to
	 * @return The created auto driver component
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver")
	UAutoDriverComponent* CreateAutoDriverForController(APlayerController* PlayerController);

	/**
	 * Remove auto driver from a player controller
	 * @param PlayerController The player controller
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver")
	void RemoveAutoDriverFromController(APlayerController* PlayerController);

	/**
	 * Get all active auto driver components
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver")
	TArray<UAutoDriverComponent*> GetAllAutoDrivers();

	/**
	 * Enable or disable all auto drivers
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver")
	void SetAllAutoDriversEnabled(bool bEnabled);

	/**
	 * Stop all active commands on all auto drivers
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver")
	void StopAllCommands();

	// ========================================
	// Settings
	// ========================================

	/**
	 * Get whether auto drivers should be created automatically for new players
	 */
	UFUNCTION(BlueprintPure, Category = "Auto Driver")
	bool GetAutoCreateForNewPlayers() const { return bAutoCreateForNewPlayers; }

	/**
	 * Set whether auto drivers should be created automatically for new players
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver")
	void SetAutoCreateForNewPlayers(bool bEnabled);

	// ========================================
	// Statistics
	// ========================================

	/**
	 * Get the number of active auto drivers
	 */
	UFUNCTION(BlueprintPure, Category = "Auto Driver")
	int32 GetActiveAutoDriverCount() const;

	/**
	 * Get the total number of commands executed
	 */
	UFUNCTION(BlueprintPure, Category = "Auto Driver")
	int64 GetTotalCommandsExecuted() const { return TotalCommandsExecuted; }

protected:
	/** Tracked auto driver components */
	UPROPERTY()
	TArray<TObjectPtr<UAutoDriverComponent>> AutoDrivers;

	/** Should auto drivers be created automatically for new players */
	UPROPERTY()
	bool bAutoCreateForNewPlayers = false;

	/** Total commands executed (for statistics) */
	int64 TotalCommandsExecuted = 0;

	/** Handle player controller creation */
	void OnPostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer);

	/** Clean up destroyed auto drivers */
	void CleanupDestroyedAutoDrivers();
};
