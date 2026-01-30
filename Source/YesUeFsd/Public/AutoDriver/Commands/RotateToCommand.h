// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AutoDriver/Commands/IAutoDriverCommand.h"
#include "RotateToCommand.generated.h"

class APlayerController;
class APawn;

/**
 * Rotate To Command
 *
 * Rotates the controlled pawn to a target rotation smoothly.
 */
UCLASS(BlueprintType, Blueprintable)
class YESUEFSD_API URotateToCommand : public UObject, public IAutoDriverCommand
{
	GENERATED_BODY()

public:
	// ========================================
	// Configuration
	// ========================================

	/** Target rotation */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver")
	FRotator TargetRotation = FRotator::ZeroRotator;

	/** Rotation speed (degrees per second) */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver")
	float RotationSpeed = 180.0f;

	/** Acceptable angle difference in degrees */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver")
	float AcceptanceAngle = 5.0f;

	/** Maximum time to attempt rotation (0 = no timeout) */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver")
	float Timeout = 10.0f;

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
	// Factory Methods
	// ========================================

	/**
	 * Create a rotate to rotation command
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver", meta = (WorldContext = "WorldContextObject"))
	static URotateToCommand* CreateRotateToRotation(
		UObject* WorldContextObject,
		FRotator InTargetRotation,
		float InRotationSpeed = 180.0f);

	/**
	 * Create a look at location command
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver", meta = (WorldContext = "WorldContextObject"))
	static URotateToCommand* CreateLookAtLocation(
		UObject* WorldContextObject,
		FVector TargetLocation,
		float InRotationSpeed = 180.0f);

protected:
	/** Cached player controller */
	UPROPERTY()
	TObjectPtr<APlayerController> PlayerController;

	/** Cached pawn */
	UPROPERTY()
	TObjectPtr<APawn> Pawn;

	/** Is the command running */
	bool bIsRunning = false;

	/** Command result */
	FAutoDriverCommandResult Result;

	/** Execution time */
	float ExecutionTime = 0.0f;

	/** Has rotation been completed */
	bool HasReachedRotation() const;

	/** Get angle difference from target */
	float GetAngleDifferenceFromTarget() const;
};
