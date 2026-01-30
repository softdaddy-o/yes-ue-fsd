// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HttpServerModule.h"
#include "IHttpRouter.h"

/**
 * MCP (Model Context Protocol) Server
 *
 * HTTP server for remote control of the Auto Driver via JSON-RPC 2.0 API.
 * Provides endpoints for automation control, status queries, and tool execution.
 *
 * Features:
 * - JSON-RPC 2.0 compliant request/response handling
 * - Pluggable tool system for extensibility
 * - Thread-safe command execution
 * - Automatic lifecycle management via subsystem
 */
class YESUEFSDEDITOR_API FMcpServer
{
public:
	FMcpServer();
	~FMcpServer();

	/**
	 * Start the HTTP server
	 * @param InPort Port to listen on
	 * @param InBindAddress Address to bind to (e.g., "127.0.0.1" or "0.0.0.0")
	 * @return True if server started successfully
	 */
	bool StartServer(uint16 InPort, const FString& InBindAddress = TEXT("127.0.0.1"));

	/**
	 * Stop the HTTP server
	 */
	void StopServer();

	/**
	 * Check if server is running
	 * @return True if server is currently running
	 */
	bool IsRunning() const { return bIsRunning; }

	/**
	 * Get the server port
	 * @return Port number the server is listening on
	 */
	uint16 GetPort() const { return Port; }

	/**
	 * Get the bind address
	 * @return Address the server is bound to
	 */
	FString GetBindAddress() const { return BindAddress; }

	/**
	 * Register a tool for remote execution
	 * @param ToolName Name of the tool (e.g., "autodriver/move_to_location")
	 * @param Handler Function to execute when tool is called
	 */
	void RegisterTool(const FString& ToolName, TFunction<FString(const TSharedPtr<FJsonObject>&)> Handler);

protected:
	/** HTTP server module */
	FHttpServerModule* HttpServerModule;

	/** HTTP router for handling requests */
	TSharedPtr<IHttpRouter> HttpRouter;

	/** Server port */
	uint16 Port;

	/** Bind address */
	FString BindAddress;

	/** Is server running */
	bool bIsRunning;

	/** Registered tools */
	TMap<FString, TFunction<FString(const TSharedPtr<FJsonObject>&)>> RegisteredTools;

	/** Handle JSON-RPC request */
	bool HandleJsonRpcRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	/** Handle tools/list endpoint */
	FString HandleListTools(const TSharedPtr<FJsonObject>& Params);

	/** Handle tools/call endpoint */
	FString HandleToolCall(const TSharedPtr<FJsonObject>& Params);

	/** Build JSON-RPC success response */
	FString BuildSuccessResponse(const TSharedPtr<FJsonValue>& Result, int32 RequestId);

	/** Build JSON-RPC error response */
	FString BuildErrorResponse(int32 Code, const FString& Message, int32 RequestId);

	/** Register default tools */
	void RegisterDefaultTools();
};
