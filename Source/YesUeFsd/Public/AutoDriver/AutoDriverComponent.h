// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AutoDriver/AutoDriverTypes.h"
#include "AutoDriver/AutoDriverUITypes.h"
#include "AutoDriverComponent.generated.h"

class IAutoDriverCommand;
class AAIController;
class ACharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAutoDriverCommandComplete, bool, bSuccess, const FString&, Message);

/**
 * Auto Driver Component
 *
 * Attach this component to a PlayerController or Character to enable automatic control.
 * Provides high-level automation functions like MoveTo, LookAt, PressButton, etc.
 *
 * Usage:
 *   - Add to PlayerController in Blueprint or C++
 *   - Call MoveToLocation(), RotateToRotation(), etc.
 *   - Monitor command completion via OnCommandComplete delegate
 */
UCLASS(ClassGroup=(AutoDriver), meta=(BlueprintSpawnableComponent))
class YESUEFSD_API UAutoDriverComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAutoDriverComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ========================================
	// Command Execution
	// ========================================

	/**
	 * Execute a custom command
	 * @param Command The command to execute
	 * @return True if command started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver")
	bool ExecuteCommand(TScriptInterface<IAutoDriverCommand> Command);

	/**
	 * Stop the currently executing command
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver")
	void StopCurrentCommand();

	/**
	 * Check if a command is currently executing
	 */
	UFUNCTION(BlueprintPure, Category = "Auto Driver")
	bool IsExecutingCommand() const { return CurrentCommand.IsValid(); }

	// ========================================
	// Movement Commands
	// ========================================

	/**
	 * Move to a target location
	 * @param Params Movement parameters
	 * @return True if movement started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver|Movement")
	bool MoveToLocation(const FAutoDriverMoveParams& Params);

	/**
	 * Move to a target actor
	 * @param TargetActor Actor to move to
	 * @param AcceptanceRadius How close to get
	 * @return True if movement started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver|Movement")
	bool MoveToActor(AActor* TargetActor, float AcceptanceRadius = 50.0f);

	/**
	 * Stop current movement
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver|Movement")
	void StopMovement();

	// ========================================
	// Rotation Commands
	// ========================================

	/**
	 * Rotate to a target rotation
	 * @param Params Rotation parameters
	 * @return True if rotation started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver|Rotation")
	bool RotateToRotation(const FAutoDriverRotateParams& Params);

	/**
	 * Look at a target location
	 * @param TargetLocation World location to look at
	 * @param RotationSpeed Rotation speed in degrees per second
	 * @return True if rotation started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver|Rotation")
	bool LookAtLocation(FVector TargetLocation, float RotationSpeed = 180.0f);

	/**
	 * Look at a target actor
	 * @param TargetActor Actor to look at
	 * @param RotationSpeed Rotation speed in degrees per second
	 * @return True if rotation started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver|Rotation")
	bool LookAtActor(AActor* TargetActor, float RotationSpeed = 180.0f);

	// ========================================
	// Input Commands
	// ========================================

	/**
	 * Simulate a button press
	 * @param ActionName Name of the action to press
	 * @param Duration How long to hold (0 = instant press)
	 * @return True if input started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver|Input")
	bool PressButton(FName ActionName, float Duration = 0.0f);

	/**
	 * Simulate an axis input
	 * @param ActionName Name of the axis action
	 * @param Value Axis value (-1 to 1)
	 * @param Duration How long to hold the value
	 * @return True if input started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver|Input")
	bool SetAxisValue(FName ActionName, float Value, float Duration = 0.0f);

	// ========================================
	// Navigation Queries
	// ========================================

	/**
	 * Check if a location is reachable from current position
	 * @param TargetLocation Target location to check
	 * @return True if location is reachable
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver|Navigation")
	bool IsLocationReachable(FVector TargetLocation);

	/**
	 * Get path length to a target location
	 * @param TargetLocation Target location
	 * @return Path length in units, or -1 if path not found
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver|Navigation")
	float GetPathLengthToLocation(FVector TargetLocation);

	/**
	 * Find a random reachable location within radius
	 * @param Radius Search radius
	 * @param OutLocation Resulting random location
	 * @return True if location found
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver|Navigation")
	bool GetRandomReachableLocation(float Radius, FVector& OutLocation);

	// ========================================
	// UI Commands
	// ========================================

	/**
	 * Click a widget by name
	 * @param WidgetName Name of widget to click
	 * @param ClickParams Click parameters
	 * @return True if widget was found and clicked
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver|UI")
	bool ClickWidget(const FString& WidgetName, const FUIClickParams& ClickParams = FUIClickParams());

	/**
	 * Click a widget by query parameters
	 * @param QueryParams Widget query parameters
	 * @param ClickParams Click parameters
	 * @return True if widget was found and clicked
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver|UI")
	bool ClickWidgetByQuery(const FWidgetQueryParams& QueryParams, const FUIClickParams& ClickParams = FUIClickParams());

	/**
	 * Wait for a widget to appear
	 * @param WidgetName Name of widget to wait for
	 * @param Timeout Maximum time to wait in seconds
	 * @return True if widget appeared within timeout
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver|UI")
	bool WaitForWidget(const FString& WidgetName, float Timeout = 10.0f);

	/**
	 * Wait for a widget to disappear
	 * @param WidgetName Name of widget to wait for disappearance
	 * @param Timeout Maximum time to wait in seconds
	 * @return True if widget disappeared within timeout
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver|UI")
	bool WaitForWidgetToDisappear(const FString& WidgetName, float Timeout = 10.0f);

	/**
	 * Find a widget by name
	 * @param WidgetName Name of widget to find
	 * @return Widget info (bFound will be false if not found)
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver|UI")
	FWidgetInfo FindWidget(const FString& WidgetName);

	/**
	 * Find widgets by query parameters
	 * @param QueryParams Widget query parameters
	 * @return Array of widget info
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver|UI")
	TArray<FWidgetInfo> FindWidgets(const FWidgetQueryParams& QueryParams);

	/**
	 * Get text from a widget
	 * @param WidgetName Name of widget to get text from
	 * @return Text content (empty if widget not found or has no text)
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver|UI")
	FString GetWidgetText(const FString& WidgetName);

	/**
	 * Check if a widget is visible
	 * @param WidgetName Name of widget to check
	 * @return True if widget is visible
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver|UI")
	bool IsWidgetVisible(const FString& WidgetName);

	/**
	 * Get all visible buttons in the UI
	 * @return Array of button widget info
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver|UI")
	TArray<FWidgetInfo> GetAllButtons();

	// ========================================
	// Utility
	// ========================================

	/**
	 * Get the controlled pawn
	 */
	UFUNCTION(BlueprintPure, Category = "Auto Driver")
	APawn* GetControlledPawn() const;

	/**
	 * Get the controlled character (if pawn is a character)
	 */
	UFUNCTION(BlueprintPure, Category = "Auto Driver")
	ACharacter* GetControlledCharacter() const;

	/**
	 * Enable or disable auto driver
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver")
	void SetEnabled(bool bInEnabled);

	/**
	 * Check if auto driver is enabled
	 */
	UFUNCTION(BlueprintPure, Category = "Auto Driver")
	bool IsEnabled() const { return bEnabled; }

	// ========================================
	// Delegates
	// ========================================

	/** Called when a command completes */
	UPROPERTY(BlueprintAssignable, Category = "Auto Driver")
	FOnAutoDriverCommandComplete OnCommandComplete;

protected:
	/** Currently executing command */
	TSharedPtr<IAutoDriverCommand> CurrentCommand;

	/** Is the auto driver enabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Driver")
	bool bEnabled = true;

	/** Debug visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Driver|Debug")
	bool bShowDebugInfo = false;

	/** Cached player controller */
	UPROPERTY()
	APlayerController* CachedPlayerController;

	/** Cached AI controller for navigation */
	UPROPERTY()
	AAIController* CachedAIController;

	/** Use AI controller for navigation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Driver|Navigation")
	bool bUseAIControllerForNavigation = true;

	/** Command completion callback */
	void OnCommandCompleted(const FAutoDriverCommandResult& Result);

	/** Get or create AI controller for navigation */
	AAIController* GetOrCreateAIController();

	/** Release AI controller */
	void ReleaseAIController();
};
