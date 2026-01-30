// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AutoDriverTypes.generated.h"

/**
 * Result status for auto driver commands
 */
UENUM(BlueprintType)
enum class EAutoDriverCommandStatus : uint8
{
	Success,
	Running,
	Failed,
	Cancelled
};

/**
 * Movement mode for auto driver
 */
UENUM(BlueprintType)
enum class EAutoDriverMovementMode : uint8
{
	/** Direct character movement */
	Direct,

	/** Navigation system pathfinding */
	Navigation,

	/** Manual input simulation */
	InputSimulation
};

/**
 * Command execution result
 */
USTRUCT(BlueprintType)
struct YESUEFSD_API FAutoDriverCommandResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver")
	EAutoDriverCommandStatus Status = EAutoDriverCommandStatus::Success;

	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver")
	FString Message;

	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver")
	float ExecutionTime = 0.0f;

	FAutoDriverCommandResult() = default;

	FAutoDriverCommandResult(EAutoDriverCommandStatus InStatus, const FString& InMessage = TEXT(""))
		: Status(InStatus), Message(InMessage)
	{
	}

	bool IsSuccess() const { return Status == EAutoDriverCommandStatus::Success; }
	bool IsRunning() const { return Status == EAutoDriverCommandStatus::Running; }
	bool IsFailed() const { return Status == EAutoDriverCommandStatus::Failed; }
};

/**
 * Movement command parameters
 */
USTRUCT(BlueprintType)
struct YESUEFSD_API FAutoDriverMoveParams
{
	GENERATED_BODY()

	/** Target location in world space */
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

	/** Movement mode to use */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver")
	EAutoDriverMovementMode MovementMode = EAutoDriverMovementMode::Navigation;
};

/**
 * Rotation command parameters
 */
USTRUCT(BlueprintType)
struct YESUEFSD_API FAutoDriverRotateParams
{
	GENERATED_BODY()

	/** Target rotation */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver")
	FRotator TargetRotation = FRotator::ZeroRotator;

	/** Rotation speed (degrees per second) */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver")
	float RotationSpeed = 180.0f;

	/** Acceptable angle difference in degrees */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver")
	float AcceptanceAngle = 5.0f;
};

/**
 * Input action parameters
 */
USTRUCT(BlueprintType)
struct YESUEFSD_API FAutoDriverInputParams
{
	GENERATED_BODY()

	/** Input action name */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver")
	FName ActionName;

	/** Input value (for axis inputs) */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver")
	float Value = 1.0f;

	/** Duration to hold the input (0 = single press) */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver")
	float Duration = 0.0f;
};
