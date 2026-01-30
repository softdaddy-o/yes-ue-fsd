// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InputSimulator.generated.h"

class APlayerController;
class UEnhancedInputAdapter;

/**
 * Input action type
 */
UENUM(BlueprintType)
enum class EInputActionType : uint8
{
	/** Button press (digital input) */
	Button,

	/** Axis input (analog input) */
	Axis,

	/** 2D axis input (e.g., joystick) */
	Axis2D
};

/**
 * Input mode for the simulator
 */
UENUM(BlueprintType)
enum class EInputSimulatorMode : uint8
{
	/** Use legacy input system */
	Legacy,

	/** Use Enhanced Input System */
	EnhancedInput,

	/** Automatically detect and use available system */
	Auto
};

/**
 * Input Simulator
 *
 * Simulates player input for automated control.
 * Supports keyboard, mouse, and gamepad input simulation.
 * Integrates with both legacy input system and UE5's Enhanced Input System.
 */
UCLASS(BlueprintType)
class YESUEFSD_API UInputSimulator : public UObject
{
	GENERATED_BODY()

public:
	// ========================================
	// Initialization
	// ========================================

	/**
	 * Initialize the input simulator
	 * @param InPlayerController Player controller to simulate input for
	 * @param Mode Input mode to use (Auto will detect available system)
	 */
	UFUNCTION(BlueprintCallable, Category = "Input Simulator")
	void Initialize(APlayerController* InPlayerController, EInputSimulatorMode Mode = EInputSimulatorMode::Auto);

	// ========================================
	// Button Input
	// ========================================

	/**
	 * Press a button
	 * @param ActionName Name of the action
	 */
	UFUNCTION(BlueprintCallable, Category = "Input Simulator")
	void PressButton(FName ActionName);

	/**
	 * Release a button
	 * @param ActionName Name of the action
	 */
	UFUNCTION(BlueprintCallable, Category = "Input Simulator")
	void ReleaseButton(FName ActionName);

	/**
	 * Press and hold a button for a duration
	 * @param ActionName Name of the action
	 * @param Duration How long to hold in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "Input Simulator")
	void PressAndHoldButton(FName ActionName, float Duration);

	// ========================================
	// Axis Input
	// ========================================

	/**
	 * Set axis value
	 * @param AxisName Name of the axis
	 * @param Value Axis value (-1 to 1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Input Simulator")
	void SetAxisValue(FName AxisName, float Value);

	/**
	 * Set 2D axis value (e.g., joystick)
	 * @param AxisName Name of the axis
	 * @param Value 2D axis value
	 */
	UFUNCTION(BlueprintCallable, Category = "Input Simulator")
	void SetAxis2DValue(FName AxisName, FVector2D Value);

	/**
	 * Clear axis value (set to 0)
	 * @param AxisName Name of the axis
	 */
	UFUNCTION(BlueprintCallable, Category = "Input Simulator")
	void ClearAxisValue(FName AxisName);

	// ========================================
	// Movement Shortcuts
	// ========================================

	/**
	 * Simulate forward/backward movement
	 * @param Value -1 to 1 (-1 = backward, 1 = forward)
	 */
	UFUNCTION(BlueprintCallable, Category = "Input Simulator")
	void SetMoveForward(float Value);

	/**
	 * Simulate right/left movement
	 * @param Value -1 to 1 (-1 = left, 1 = right)
	 */
	UFUNCTION(BlueprintCallable, Category = "Input Simulator")
	void SetMoveRight(float Value);

	/**
	 * Simulate look up/down
	 * @param Value Pitch delta
	 */
	UFUNCTION(BlueprintCallable, Category = "Input Simulator")
	void SetLookUp(float Value);

	/**
	 * Simulate look left/right
	 * @param Value Yaw delta
	 */
	UFUNCTION(BlueprintCallable, Category = "Input Simulator")
	void SetLookRight(float Value);

	/**
	 * Simulate jump
	 */
	UFUNCTION(BlueprintCallable, Category = "Input Simulator")
	void Jump();

	/**
	 * Stop jumping
	 */
	UFUNCTION(BlueprintCallable, Category = "Input Simulator")
	void StopJumping();

	// ========================================
	// Utility
	// ========================================

	/**
	 * Clear all input
	 */
	UFUNCTION(BlueprintCallable, Category = "Input Simulator")
	void ClearAllInput();

	/**
	 * Check if input simulator is initialized
	 */
	UFUNCTION(BlueprintPure, Category = "Input Simulator")
	bool IsInitialized() const { return PlayerController != nullptr; }

	/**
	 * Get current input mode
	 */
	UFUNCTION(BlueprintPure, Category = "Input Simulator")
	EInputSimulatorMode GetInputMode() const { return CurrentMode; }

	/**
	 * Check if using Enhanced Input
	 */
	UFUNCTION(BlueprintPure, Category = "Input Simulator")
	bool IsUsingEnhancedInput() const { return CurrentMode == EInputSimulatorMode::EnhancedInput; }

	/**
	 * Get Enhanced Input Adapter (if using Enhanced Input)
	 */
	UFUNCTION(BlueprintPure, Category = "Input Simulator")
	UEnhancedInputAdapter* GetEnhancedInputAdapter() const { return EnhancedInputAdapter; }

	// ========================================
	// Factory
	// ========================================

	/**
	 * Create an input simulator for a player controller
	 */
	UFUNCTION(BlueprintCallable, Category = "Input Simulator", meta = (WorldContext = "WorldContextObject"))
	static UInputSimulator* CreateInputSimulator(UObject* WorldContextObject, APlayerController* InPlayerController);

protected:
	/** Player controller to simulate input for */
	UPROPERTY()
	TObjectPtr<APlayerController> PlayerController;

	/** Enhanced Input Adapter (when using Enhanced Input mode) */
	UPROPERTY()
	TObjectPtr<UEnhancedInputAdapter> EnhancedInputAdapter;

	/** Current input mode */
	UPROPERTY()
	EInputSimulatorMode CurrentMode = EInputSimulatorMode::Legacy;

	/** Active button presses */
	UPROPERTY()
	TSet<FName> ActiveButtons;

	/** Active axis values */
	UPROPERTY()
	TMap<FName, float> ActiveAxes;

	/** Timed button releases */
	struct FTimedButtonRelease
	{
		FName ActionName;
		float TimeRemaining;
	};

	TArray<FTimedButtonRelease> TimedReleases;

	/** Update timed releases */
	void TickTimedReleases(float DeltaTime);

	/** World tick handle */
	FDelegateHandle TickHandle;

	/** Tick function */
	void Tick(float DeltaTime);

	/** Determine input mode to use */
	EInputSimulatorMode DetermineInputMode(EInputSimulatorMode RequestedMode);
};
