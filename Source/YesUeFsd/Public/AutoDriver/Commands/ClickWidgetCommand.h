// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AutoDriver/Commands/IAutoDriverCommand.h"
#include "AutoDriver/AutoDriverUITypes.h"
#include "ClickWidgetCommand.generated.h"

class UWorld;

/**
 * Click Widget Command
 *
 * Finds and clicks a widget in the UI hierarchy.
 * Supports retry logic with timeout for widgets that appear dynamically.
 */
UCLASS(BlueprintType, Blueprintable)
class YESUEFSD_API UClickWidgetCommand : public UObject, public IAutoDriverCommand
{
	GENERATED_BODY()

public:
	// ========================================
	// Configuration
	// ========================================

	/** Widget query parameters */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	FWidgetQueryParams QueryParams;

	/** Click parameters */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	FUIClickParams ClickParams;

	/** Maximum time to wait for widget to appear (seconds) */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	float Timeout = 5.0f;

	/** Interval between retry attempts (seconds) */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	float RetryInterval = 0.1f;

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
	 * Create a click widget command by name
	 * @param WorldContextObject World context
	 * @param WidgetName Name of widget to click
	 * @param InClickParams Click parameters
	 * @param InTimeout Maximum wait time
	 * @return New command instance
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver|UI", meta = (WorldContext = "WorldContextObject"))
	static UClickWidgetCommand* CreateClickWidgetCommand(
		UObject* WorldContextObject,
		const FString& WidgetName,
		const FUIClickParams& InClickParams = FUIClickParams(),
		float InTimeout = 5.0f);

	/**
	 * Create a click widget command with full query parameters
	 * @param WorldContextObject World context
	 * @param InQueryParams Query parameters
	 * @param InClickParams Click parameters
	 * @param InTimeout Maximum wait time
	 * @return New command instance
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver|UI", meta = (WorldContext = "WorldContextObject"))
	static UClickWidgetCommand* CreateClickWidgetCommandByQuery(
		UObject* WorldContextObject,
		const FWidgetQueryParams& InQueryParams,
		const FUIClickParams& InClickParams = FUIClickParams(),
		float InTimeout = 5.0f);

protected:
	/** World context */
	UPROPERTY()
	TObjectPtr<UWorld> World;

	/** Is the command running */
	bool bIsRunning = false;

	/** Command result */
	FAutoDriverCommandResult Result;

	/** Execution time */
	float ExecutionTime = 0.0f;

	/** Time since last retry */
	float TimeSinceLastRetry = 0.0f;

	/** Has clicked successfully */
	bool bHasClicked = false;

	/** Attempt to find and click the widget */
	bool TryClickWidget();
};
