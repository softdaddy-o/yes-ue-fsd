// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnhancedInputExample.generated.h"

class UInputSimulator;
class UEnhancedInputAdapter;
class UInputAction;
class UInputMappingContext;

/**
 * Enhanced Input System Integration Example
 *
 * Demonstrates how to use the Enhanced Input System with AutoDriver.
 * Shows registration, injection, context management, and recording.
 */
UCLASS()
class YESUEFSD_API AEnhancedInputExample : public AActor
{
	GENERATED_BODY()

public:
	AEnhancedInputExample();

protected:
	virtual void BeginPlay() override;

public:
	// ========================================
	// Example: Basic Setup
	// ========================================

	/** Example 1: Initialize Enhanced Input Simulator */
	UFUNCTION(BlueprintCallable, Category = "Examples|Enhanced Input")
	void Example_BasicSetup();

	// ========================================
	// Example: Action Registration
	// ========================================

	/** Example 2: Register input actions */
	UFUNCTION(BlueprintCallable, Category = "Examples|Enhanced Input")
	void Example_RegisterActions();

	// ========================================
	// Example: Input Injection
	// ========================================

	/** Example 3: Inject button presses */
	UFUNCTION(BlueprintCallable, Category = "Examples|Enhanced Input")
	void Example_InjectButtons();

	/** Example 4: Inject axis values */
	UFUNCTION(BlueprintCallable, Category = "Examples|Enhanced Input")
	void Example_InjectAxis();

	/** Example 5: Inject 2D axis (movement) */
	UFUNCTION(BlueprintCallable, Category = "Examples|Enhanced Input")
	void Example_InjectMovement();

	// ========================================
	// Example: Context Management
	// ========================================

	/** Example 6: Manage input contexts */
	UFUNCTION(BlueprintCallable, Category = "Examples|Enhanced Input")
	void Example_ManageContexts();

	/** Example 7: Context priority and switching */
	UFUNCTION(BlueprintCallable, Category = "Examples|Enhanced Input")
	void Example_ContextSwitching();

	// ========================================
	// Example: Recording
	// ========================================

	/** Example 8: Record and export input */
	UFUNCTION(BlueprintCallable, Category = "Examples|Enhanced Input")
	void Example_RecordInput();

	/** Example 9: Import and analyze recording */
	UFUNCTION(BlueprintCallable, Category = "Examples|Enhanced Input")
	void Example_ImportRecording();

	// ========================================
	// Example: Advanced Usage
	// ========================================

	/** Example 10: Automated sequence with timing */
	UFUNCTION(BlueprintCallable, Category = "Examples|Enhanced Input")
	void Example_AutomatedSequence();

	/** Example 11: Complex movement pattern */
	UFUNCTION(BlueprintCallable, Category = "Examples|Enhanced Input")
	void Example_ComplexMovement();

	// ========================================
	// Configuration
	// ========================================

	/** Input Action for Jump */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input|Actions")
	TObjectPtr<UInputAction> IA_Jump;

	/** Input Action for Move */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input|Actions")
	TObjectPtr<UInputAction> IA_Move;

	/** Input Action for Look */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input|Actions")
	TObjectPtr<UInputAction> IA_Look;

	/** Input Action for Interact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input|Actions")
	TObjectPtr<UInputAction> IA_Interact;

	/** Default Mapping Context */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input|Contexts")
	TObjectPtr<UInputMappingContext> IMC_Default;

	/** UI Mapping Context */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input|Contexts")
	TObjectPtr<UInputMappingContext> IMC_UI;

	/** Combat Mapping Context */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input|Contexts")
	TObjectPtr<UInputMappingContext> IMC_Combat;

protected:
	/** Input Simulator instance */
	UPROPERTY()
	TObjectPtr<UInputSimulator> InputSimulator;

	/** Enhanced Input Adapter */
	UPROPERTY()
	TObjectPtr<UEnhancedInputAdapter> EnhancedInputAdapter;

	/** Recording file path */
	FString RecordingFilePath;

	/** Timer handle for sequences */
	FTimerHandle SequenceTimerHandle;

	// ========================================
	// Helpers
	// ========================================

	/** Get player controller */
	APlayerController* GetPlayerController() const;

	/** Setup common action mappings */
	void SetupCommonMappings();

	/** Log recording statistics */
	void LogRecordingStats() const;
};
