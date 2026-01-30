// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AutoDriver/AutoDriverTypes.h"
#include "IAutoDriverCommand.generated.h"

/**
 * Interface for auto driver commands (UObject version)
 * This is the UInterface wrapper required by Unreal's reflection system
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UAutoDriverCommand : public UInterface
{
	GENERATED_BODY()
};

/**
 * Auto Driver Command Interface
 *
 * Implement this interface to create custom auto driver commands.
 * Commands can be written in C++ or Blueprint.
 */
class YESUEFSD_API IAutoDriverCommand
{
	GENERATED_BODY()

public:
	/**
	 * Initialize the command
	 * Called once when the command is about to execute
	 * @param InContext Execution context (player controller, world, etc.)
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Auto Driver")
	void Initialize(UObject* InContext);
	virtual void Initialize_Implementation(UObject* InContext) {}

	/**
	 * Execute the command
	 * Called once at the start of command execution
	 * @return True if command started successfully
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Auto Driver")
	bool Execute();
	virtual bool Execute_Implementation() { return false; }

	/**
	 * Tick the command
	 * Called every frame while the command is running
	 * @param DeltaTime Time since last tick
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Auto Driver")
	void Tick(float DeltaTime);
	virtual void Tick_Implementation(float DeltaTime) {}

	/**
	 * Cancel the command
	 * Called when the command needs to be stopped prematurely
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Auto Driver")
	void Cancel();
	virtual void Cancel_Implementation() {}

	/**
	 * Check if command is still running
	 * @return True if command is still executing
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Auto Driver")
	bool IsRunning() const;
	virtual bool IsRunning_Implementation() const { return false; }

	/**
	 * Get the command result
	 * @return Command execution result
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Auto Driver")
	FAutoDriverCommandResult GetResult() const;
	virtual FAutoDriverCommandResult GetResult_Implementation() const
	{
		return FAutoDriverCommandResult(EAutoDriverCommandStatus::Failed, TEXT("Not implemented"));
	}

	/**
	 * Get command description (for logging/debugging)
	 * @return Human-readable command description
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Auto Driver")
	FString GetDescription() const;
	virtual FString GetDescription_Implementation() const { return TEXT("Unknown Command"); }
};

/**
 * Base class for C++ auto driver commands
 * Inherit from this for easier C++ command implementation
 */
class YESUEFSD_API FAutoDriverCommandBase
{
public:
	virtual ~FAutoDriverCommandBase() = default;

	/** Initialize the command */
	virtual void Initialize(UObject* InContext) { Context = InContext; }

	/** Execute the command */
	virtual bool Execute() { return false; }

	/** Tick the command */
	virtual void Tick(float DeltaTime) {}

	/** Cancel the command */
	virtual void Cancel()
	{
		bIsRunning = false;
		Result.Status = EAutoDriverCommandStatus::Cancelled;
		Result.Message = TEXT("Command cancelled");
	}

	/** Check if running */
	virtual bool IsRunning() const { return bIsRunning; }

	/** Get result */
	virtual FAutoDriverCommandResult GetResult() const { return Result; }

	/** Get description */
	virtual FString GetDescription() const { return TEXT("Base Command"); }

protected:
	/** Mark command as complete */
	void Complete(bool bSuccess, const FString& Message = TEXT(""))
	{
		bIsRunning = false;
		Result.Status = bSuccess ? EAutoDriverCommandStatus::Success : EAutoDriverCommandStatus::Failed;
		Result.Message = Message;
		Result.ExecutionTime = TotalExecutionTime;
	}

	/** Execution context */
	UObject* Context = nullptr;

	/** Is the command running */
	bool bIsRunning = false;

	/** Command result */
	FAutoDriverCommandResult Result;

	/** Total execution time */
	float TotalExecutionTime = 0.0f;
};
