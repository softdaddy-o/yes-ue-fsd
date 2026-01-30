// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AutoDriver/Commands/IAutoDriverCommand.h"
#include "AutoDriver/AutoDriverUITypes.h"
#include "ReadWidgetCommand.generated.h"

class UWorld;

/**
 * Read Widget Command
 *
 * Reads information from a widget in the UI hierarchy.
 * Returns widget info and text content.
 */
UCLASS(BlueprintType, Blueprintable)
class YESUEFSD_API UReadWidgetCommand : public UObject, public IAutoDriverCommand
{
	GENERATED_BODY()

public:
	// ========================================
	// Configuration
	// ========================================

	/** Widget query parameters */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	FWidgetQueryParams QueryParams;

	/** Maximum time to wait for widget to appear (seconds) */
	UPROPERTY(BlueprintReadWrite, Category = "Auto Driver|UI")
	float Timeout = 5.0f;

	// ========================================
	// Results
	// ========================================

	/** Found widget info */
	UPROPERTY(BlueprintReadOnly, Category = "Auto Driver|UI")
	FWidgetInfo FoundWidgetInfo;

	/** Found text content */
	UPROPERTY(BlueprintReadOnly, Category = "Auto Driver|UI")
	FString FoundText;

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
	 * Create a read widget command by name
	 * @param WorldContextObject World context
	 * @param WidgetName Name of widget to read
	 * @param InTimeout Maximum wait time
	 * @return New command instance
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver|UI", meta = (WorldContext = "WorldContextObject"))
	static UReadWidgetCommand* CreateReadWidgetCommand(
		UObject* WorldContextObject,
		const FString& WidgetName,
		float InTimeout = 5.0f);

	/**
	 * Create a read widget command with full query parameters
	 * @param WorldContextObject World context
	 * @param InQueryParams Query parameters
	 * @param InTimeout Maximum wait time
	 * @return New command instance
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Driver|UI", meta = (WorldContext = "WorldContextObject"))
	static UReadWidgetCommand* CreateReadWidgetCommandByQuery(
		UObject* WorldContextObject,
		const FWidgetQueryParams& InQueryParams,
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

	/** Retry interval */
	float RetryInterval = 0.1f;

	/** Has found widget */
	bool bHasFound = false;

	/** Attempt to find and read the widget */
	bool TryReadWidget();
};
