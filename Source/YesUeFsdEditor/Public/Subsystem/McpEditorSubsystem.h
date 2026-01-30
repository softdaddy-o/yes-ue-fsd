// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "McpEditorSubsystem.generated.h"

class FMcpServer;

/**
 * MCP Editor Subsystem
 *
 * Manages the lifecycle of the MCP HTTP server.
 * Handles auto-start on editor launch and shutdown cleanup.
 */
UCLASS()
class YESUEFSDEDITOR_API UMcpEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem implementation
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Start the MCP server
	 * @return True if server started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "MCP Server")
	bool StartServer();

	/**
	 * Stop the MCP server
	 */
	UFUNCTION(BlueprintCallable, Category = "MCP Server")
	void StopServer();

	/**
	 * Check if server is running
	 * @return True if server is currently running
	 */
	UFUNCTION(BlueprintPure, Category = "MCP Server")
	bool IsServerRunning() const;

	/**
	 * Get the server port
	 * @return Port number
	 */
	UFUNCTION(BlueprintPure, Category = "MCP Server")
	int32 GetServerPort() const;

	/**
	 * Get the server URL
	 * @return Full server URL
	 */
	UFUNCTION(BlueprintPure, Category = "MCP Server")
	FString GetServerUrl() const;

protected:
	/** MCP Server instance */
	TUniquePtr<FMcpServer> McpServer;

	/** Load settings from config */
	void LoadSettings();

	/** Server port from config */
	uint16 ServerPort;

	/** Bind address from config */
	FString BindAddress;

	/** Auto-start server on editor launch */
	bool bAutoStartServer;
};
