// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AutoDriverPythonBridge.generated.h"

class UAutoDriverComponent;
class UAutoDriverSubsystem;
class UActionTimeline;
class UActionRecorder;
class UActionPlayback;

/**
 * Python bridge class for AutoDriver functionality
 * Exposes AutoDriver to Python scripts via Unreal's PythonScriptPlugin
 */
UCLASS(BlueprintType)
class YESUEFSDEDITOR_API UAutoDriverPythonBridge : public UObject
{
	GENERATED_BODY()

public:
	// AutoDriver Component Access

	/** Get AutoDriver component for player index */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver")
	static UAutoDriverComponent* GetAutoDriverForPlayer(int32 PlayerIndex = 0);

	/** Get AutoDriver subsystem */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver")
	static UAutoDriverSubsystem* GetAutoDriverSubsystem();

	// Movement Commands

	/** Move to a location */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver")
	static bool MoveToLocation(FVector Location, float AcceptanceRadius = 50.0f, float SpeedMultiplier = 1.0f, int32 PlayerIndex = 0);

	/** Move to an actor */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver")
	static bool MoveToActor(AActor* TargetActor, float AcceptanceRadius = 50.0f, float SpeedMultiplier = 1.0f, int32 PlayerIndex = 0);

	/** Stop current movement */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver")
	static void StopMovement(int32 PlayerIndex = 0);

	// Rotation Commands

	/** Rotate to a rotation */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver")
	static bool RotateToRotation(FRotator Rotation, int32 PlayerIndex = 0);

	/** Look at a location */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver")
	static bool LookAtLocation(FVector Location, int32 PlayerIndex = 0);

	/** Look at an actor */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver")
	static bool LookAtActor(AActor* Actor, int32 PlayerIndex = 0);

	// Input Commands

	/** Press a button */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver")
	static bool PressButton(const FString& ActionName, int32 PlayerIndex = 0);

	/** Set axis value */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver")
	static bool SetAxisValue(const FString& AxisName, float Value, int32 PlayerIndex = 0);

	// Navigation Queries

	/** Check if location is reachable */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver")
	static bool IsLocationReachable(FVector Location, int32 PlayerIndex = 0);

	/** Get path length to location */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver")
	static float GetPathLength(FVector From, FVector To, int32 PlayerIndex = 0);

	/** Get random reachable location */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver")
	static FVector GetRandomReachableLocation(FVector Origin, float Radius, int32 PlayerIndex = 0);

	// Status Queries

	/** Check if executing a command */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver")
	static bool IsExecutingCommand(int32 PlayerIndex = 0);

	/** Stop current command */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver")
	static void StopCurrentCommand(int32 PlayerIndex = 0);

	// UI Commands

	/** Click a widget by name */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver|UI")
	static bool ClickWidget(const FString& WidgetName, int32 PlayerIndex = 0);

	/** Click a widget with specific parameters */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver|UI")
	static bool ClickWidgetWithParams(const FString& WidgetName, const FString& ClickType, int32 ClickCount, int32 PlayerIndex = 0);

	/** Wait for a widget to appear */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver|UI")
	static bool WaitForWidget(const FString& WidgetName, float Timeout, int32 PlayerIndex = 0);

	/** Wait for a widget to disappear */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver|UI")
	static bool WaitForWidgetGone(const FString& WidgetName, float Timeout, int32 PlayerIndex = 0);

	/** Find a widget by name and return JSON info */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver|UI")
	static FString FindWidgetByName(const FString& WidgetName, int32 PlayerIndex = 0);

	/** Find widgets by query and return JSON array */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver|UI")
	static FString FindWidgets(const FString& QueryJson, int32 PlayerIndex = 0);

	/** Get text content from a widget */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver|UI")
	static FString GetWidgetText(const FString& WidgetName, int32 PlayerIndex = 0);

	/** Check if a widget is visible */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver|UI")
	static bool IsWidgetVisible(const FString& WidgetName, int32 PlayerIndex = 0);

	/** Get all buttons and return JSON array */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver|UI")
	static FString GetAllButtons(int32 PlayerIndex = 0);

	/** Click at screen position */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver|UI")
	static bool ClickAtPosition(float X, float Y, int32 PlayerIndex = 0);

	// Recording & Playback

	/** Create a new action recorder */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver")
	static UActionRecorder* CreateRecorder(int32 PlayerIndex = 0);

	/** Create a new action playback */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver")
	static UActionPlayback* CreatePlayback(int32 PlayerIndex = 0);

	/** Load timeline from file */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver")
	static UActionTimeline* LoadTimeline(const FString& FilePath);

	/** Save timeline to file */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver")
	static bool SaveTimeline(UActionTimeline* Timeline, const FString& FilePath);

	// Utility Functions

	/** Wait for command completion (blocking) */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver")
	static void WaitForCommandCompletion(float Timeout = 10.0f, int32 PlayerIndex = 0);

	/** Get current player location */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver")
	static FVector GetPlayerLocation(int32 PlayerIndex = 0);

	/** Get current player rotation */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver")
	static FRotator GetPlayerRotation(int32 PlayerIndex = 0);

	/** Execute console command */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver")
	static void ExecuteConsoleCommand(const FString& Command);

	/** Get all actors of class */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver")
	static TArray<AActor*> GetAllActorsOfClass(UClass* ActorClass);

	/** Find actor by name */
	UFUNCTION(BlueprintCallable, Category = "Python|AutoDriver")
	static AActor* FindActorByName(const FString& ActorName);
};
