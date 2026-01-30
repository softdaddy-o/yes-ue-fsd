// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InputActionValue.h"
#include "EnhancedInputAdapter.generated.h"

class UInputAction;
class UInputMappingContext;
class UEnhancedInputComponent;
class UEnhancedInputLocalPlayerSubsystem;
class APlayerController;

/**
 * Enhanced Input action mapping
 * Maps legacy action names to Enhanced Input actions
 */
USTRUCT(BlueprintType)
struct FEnhancedInputActionMapping
{
	GENERATED_BODY()

	/** Legacy action name for backwards compatibility */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FName ActionName;

	/** Enhanced Input Action asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> InputAction;

	/** Optional context for this action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputMappingContext> MappingContext;

	/** Priority for the mapping context (higher = higher priority) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	int32 Priority = 0;
};

/**
 * Input action recording entry
 * Used for recording and playback of Enhanced Input events
 */
USTRUCT(BlueprintType)
struct FInputActionRecord
{
	GENERATED_BODY()

	/** Timestamp of the input event */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recording")
	float Timestamp = 0.0f;

	/** Action that was triggered */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recording")
	TObjectPtr<UInputAction> InputAction;

	/** Action name for reference */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recording")
	FName ActionName;

	/** Value of the action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recording")
	FInputActionValue ActionValue;

	/** Whether this is a trigger event */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recording")
	bool bTriggered = false;

	/** Whether this is a started event */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recording")
	bool bStarted = false;

	/** Whether this is a completed event */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recording")
	bool bCompleted = false;

	/** Mapping context that was active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recording")
	TObjectPtr<UInputMappingContext> ActiveContext;
};

/**
 * Enhanced Input Adapter
 *
 * Bridges the gap between the automation system and UE5's Enhanced Input System.
 * Provides:
 * - Mapping from legacy action names to Enhanced Input actions
 * - Input action injection and simulation
 * - Input Mapping Context management
 * - Recording and playback support
 * - Trigger and modifier handling
 */
UCLASS(BlueprintType)
class YESUEFSD_API UEnhancedInputAdapter : public UObject
{
	GENERATED_BODY()

public:
	// ========================================
	// Initialization
	// ========================================

	/**
	 * Initialize the adapter for a player controller
	 * @param InPlayerController Player controller to adapt input for
	 */
	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Adapter")
	void Initialize(APlayerController* InPlayerController);

	/**
	 * Check if adapter is initialized
	 */
	UFUNCTION(BlueprintPure, Category = "Enhanced Input Adapter")
	bool IsInitialized() const { return PlayerController != nullptr; }

	// ========================================
	// Action Mapping
	// ========================================

	/**
	 * Register an action mapping
	 * @param Mapping Enhanced Input action mapping to register
	 */
	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Adapter")
	void RegisterActionMapping(const FEnhancedInputActionMapping& Mapping);

	/**
	 * Register multiple action mappings
	 * @param Mappings Array of mappings to register
	 */
	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Adapter")
	void RegisterActionMappings(const TArray<FEnhancedInputActionMapping>& Mappings);

	/**
	 * Find input action by legacy action name
	 * @param ActionName Legacy action name
	 * @return Input action if found, nullptr otherwise
	 */
	UFUNCTION(BlueprintPure, Category = "Enhanced Input Adapter")
	UInputAction* FindInputAction(FName ActionName) const;

	/**
	 * Get all registered action mappings
	 */
	UFUNCTION(BlueprintPure, Category = "Enhanced Input Adapter")
	const TArray<FEnhancedInputActionMapping>& GetActionMappings() const { return ActionMappings; }

	// ========================================
	// Context Management
	// ========================================

	/**
	 * Add an Input Mapping Context
	 * @param MappingContext Context to add
	 * @param Priority Priority for the context (higher = higher priority)
	 */
	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Adapter")
	void AddMappingContext(UInputMappingContext* MappingContext, int32 Priority = 0);

	/**
	 * Remove an Input Mapping Context
	 * @param MappingContext Context to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Adapter")
	void RemoveMappingContext(UInputMappingContext* MappingContext);

	/**
	 * Clear all mapping contexts
	 */
	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Adapter")
	void ClearAllMappingContexts();

	/**
	 * Get currently active mapping contexts
	 */
	UFUNCTION(BlueprintPure, Category = "Enhanced Input Adapter")
	TArray<UInputMappingContext*> GetActiveMappingContexts() const;

	// ========================================
	// Input Injection
	// ========================================

	/**
	 * Inject an input action by name
	 * @param ActionName Legacy action name
	 * @param ActionValue Value to inject
	 * @return True if action was found and injected
	 */
	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Adapter")
	bool InjectInputAction(FName ActionName, FInputActionValue ActionValue);

	/**
	 * Inject a button press
	 * @param ActionName Action name
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Adapter")
	bool InjectButtonPress(FName ActionName);

	/**
	 * Inject a button release
	 * @param ActionName Action name
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Adapter")
	bool InjectButtonRelease(FName ActionName);

	/**
	 * Inject an axis value
	 * @param ActionName Action name
	 * @param Value Axis value
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Adapter")
	bool InjectAxisValue(FName ActionName, float Value);

	/**
	 * Inject a 2D axis value
	 * @param ActionName Action name
	 * @param Value 2D axis value
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Adapter")
	bool InjectAxis2DValue(FName ActionName, FVector2D Value);

	/**
	 * Inject a 3D axis value
	 * @param ActionName Action name
	 * @param Value 3D axis value
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Adapter")
	bool InjectAxis3DValue(FName ActionName, FVector Value);

	// ========================================
	// Recording and Playback
	// ========================================

	/**
	 * Start recording input actions
	 */
	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Adapter")
	void StartRecording();

	/**
	 * Stop recording input actions
	 */
	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Adapter")
	void StopRecording();

	/**
	 * Check if currently recording
	 */
	UFUNCTION(BlueprintPure, Category = "Enhanced Input Adapter")
	bool IsRecording() const { return bIsRecording; }

	/**
	 * Get recorded input actions
	 */
	UFUNCTION(BlueprintPure, Category = "Enhanced Input Adapter")
	const TArray<FInputActionRecord>& GetRecordedActions() const { return RecordedActions; }

	/**
	 * Clear recorded actions
	 */
	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Adapter")
	void ClearRecordedActions();

	/**
	 * Export recorded actions to JSON string
	 */
	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Adapter")
	FString ExportRecordingToJSON() const;

	/**
	 * Import recorded actions from JSON string
	 */
	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Adapter")
	bool ImportRecordingFromJSON(const FString& JSONString);

	// ========================================
	// Factory
	// ========================================

	/**
	 * Create an Enhanced Input adapter for a player controller
	 */
	UFUNCTION(BlueprintCallable, Category = "Enhanced Input Adapter", meta = (WorldContext = "WorldContextObject"))
	static UEnhancedInputAdapter* CreateEnhancedInputAdapter(UObject* WorldContextObject, APlayerController* InPlayerController);

protected:
	/** Player controller this adapter is for */
	UPROPERTY()
	TObjectPtr<APlayerController> PlayerController;

	/** Enhanced Input Component */
	UPROPERTY()
	TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent;

	/** Enhanced Input Subsystem */
	UPROPERTY()
	TObjectPtr<UEnhancedInputLocalPlayerSubsystem> InputSubsystem;

	/** Registered action mappings */
	UPROPERTY()
	TArray<FEnhancedInputActionMapping> ActionMappings;

	/** Active mapping contexts */
	UPROPERTY()
	TMap<TObjectPtr<UInputMappingContext>, int32> ActiveContexts;

	/** Recording state */
	UPROPERTY()
	bool bIsRecording = false;

	/** Recorded actions */
	UPROPERTY()
	TArray<FInputActionRecord> RecordedActions;

	/** Recording start time */
	float RecordingStartTime = 0.0f;

	// ========================================
	// Internal Methods
	// ========================================

	/** Get or cache Enhanced Input Component */
	UEnhancedInputComponent* GetEnhancedInputComponent();

	/** Get or cache Enhanced Input Subsystem */
	UEnhancedInputLocalPlayerSubsystem* GetInputSubsystem();

	/** Record an input action event */
	void RecordInputAction(UInputAction* InputAction, FName ActionName, const FInputActionValue& Value, bool bTriggered, bool bStarted, bool bCompleted);

	/** Setup input bindings for recording */
	void SetupRecordingBindings();

	/** Clear recording bindings */
	void ClearRecordingBindings();

	/** Stored binding handles for cleanup */
	TArray<uint32> RecordingBindingHandles;
};
