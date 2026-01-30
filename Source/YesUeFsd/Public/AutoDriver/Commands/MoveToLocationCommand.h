// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AutoDriver/Commands/IAutoDriverCommand.h"
#include "MoveToLocationCommand.generated.h"

class APlayerController;
class ACharacter;

/**
 * Move To Location Command
 *
 * Moves the controlled character to a target location using navigation.
 * Supports different movement modes (direct, navigation, input simulation).
 */
UCLASS(BlueprintType, Blueprintable)
class YESUEFSD_API UMoveToLocationCommand : public UObject, public IAutoDriverCommand
{
	GENERATED_BODY()

public:
	// ========================================
	// Configuration
	// ========================================

	/** Target location to move to */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver")
	FVector TargetLocation = FVector::ZeroVector;

	/** Acceptable distance from target */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver")
	float AcceptanceRadius = 50.0f;

	/** Movement speed multiplier */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver")
	float SpeedMultiplier = 1.0f;

	/** Should sprint while moving */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver")
	bool bShouldSprint = false;

	/** Movement mode */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver")
	EAutoDriverMovementMode MovementMode = EAutoDriverMovementMode::Navigation;

	/** Maximum time to attempt movement (0 = no timeout) */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver")
	float Timeout = 30.0f;

	// ========================================
	// IAutoDriverCommand Interface
	// ========================================

	virtual void Initialize_Implementation(UObject* InContext) override;
	virtual bool Execute_Implementation() override;
	virtual void Tick_Implementation(float DeltaTime) override;
	virtual void Cancel_Implementation() override;
	virtual bool IsRunning_Implementation() const override;
	virtual FAutoDriverCommandResult GetResult_Implementation() const override;
	virtual FString GetDescription_Implementation() const override;

	// ========================================
	// Factory Method
	// ========================================

	/**
	 * Create a move to location command
	 * @param InTargetLocation Target location
	 * @param InAcceptanceRadius Acceptable distance from target
	 * @return New command instance
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver", meta = (WorldContext = "WorldContextObject"))
	static UMoveToLocationCommand* CreateMoveToLocationCommand(
		UObject* WorldContextObject,
		FVector InTargetLocation,
		float InAcceptanceRadius = 50.0f);

protected:
	/** Cached player controller */
	UPROPERTY()
	TObjectPtr<APlayerController> PlayerController;

	/** Cached character */
	UPROPERTY()
	TObjectPtr<ACharacter> Character;

	/** Is the command running */
	bool bIsRunning = false;

	/** Command result */
	FAutoDriverCommandResult Result;

	/** Execution time */
	float ExecutionTime = 0.0f;

	/** Execute movement using navigation */
	bool ExecuteNavigationMovement();

	/** Execute movement using direct control */
	bool ExecuteDirectMovement();

	/** Execute movement using input simulation */
	bool ExecuteInputSimulation();

	/** Check if we've reached the target */
	bool HasReachedTarget() const;

	/** Get distance to target */
	float GetDistanceToTarget() const;
};
