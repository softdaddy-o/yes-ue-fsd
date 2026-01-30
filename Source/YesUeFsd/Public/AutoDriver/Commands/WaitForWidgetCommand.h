// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AutoDriver/Commands/IAutoDriverCommand.h"
#include "AutoDriver/AutoDriverUITypes.h"
#include "WaitForWidgetCommand.generated.h"

class UWorld;

/**
 * Wait For Widget Command
 *
 * Waits for a widget to appear or disappear in the UI hierarchy.
 * Useful for synchronizing with dynamic UI updates.
 */
UCLASS(BlueprintType, Blueprintable)
class YESUEFSD_API UWaitForWidgetCommand : public UObject, public IAutoDriverCommand
{
	GENERATED_BODY()

public:
	// ========================================
	// Configuration
	// ========================================

	/** Widget query parameters */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	FWidgetQueryParams QueryParams;

	/** Wait for widget to appear (true) or disappear (false) */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	bool bWaitForAppear = true;

	/** Maximum time to wait (seconds) */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	float Timeout = 10.0f;

	/** Interval between checks (seconds) */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	float PollInterval = 0.1f;

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
	 * Create a wait for widget command (wait for appear)
	 * @param WorldContextObject World context
	 * @param WidgetName Name of widget to wait for
	 * @param InTimeout Maximum wait time
	 * @return New command instance
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver|UI", meta = (WorldContext = "WorldContextObject"))
	static UWaitForWidgetCommand* CreateWaitForWidgetCommand(
		UObject* WorldContextObject,
		const FString& WidgetName,
		float InTimeout = 10.0f);

	/**
	 * Create a wait for widget to disappear command
	 * @param WorldContextObject World context
	 * @param WidgetName Name of widget to wait for disappearance
	 * @param InTimeout Maximum wait time
	 * @return New command instance
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver|UI", meta = (WorldContext = "WorldContextObject"))
	static UWaitForWidgetCommand* CreateWaitForWidgetToDisappearCommand(
		UObject* WorldContextObject,
		const FString& WidgetName,
		float InTimeout = 10.0f);

	/**
	 * Create a wait for widget command with full query parameters
	 * @param WorldContextObject World context
	 * @param InQueryParams Query parameters
	 * @param bInWaitForAppear Wait for appear (true) or disappear (false)
	 * @param InTimeout Maximum wait time
	 * @return New command instance
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver|UI", meta = (WorldContext = "WorldContextObject"))
	static UWaitForWidgetCommand* CreateWaitForWidgetCommandByQuery(
		UObject* WorldContextObject,
		const FWidgetQueryParams& InQueryParams,
		bool bInWaitForAppear = true,
		float InTimeout = 10.0f);

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

	/** Time since last poll */
	float TimeSinceLastPoll = 0.0f;

	/** Check if condition is met */
	bool CheckCondition();
};
