// Copyright Epic Games, Inc. All Rights Reserved.

#include "Server/McpServer.h"
#include "HttpServerResponse.h"
#include "HttpPath.h"
#include "Json.h"
#include "JsonObjectConverter.h"
#include "AutoDriver/AutoDriverComponent.h"
#include "AutoDriver/AutoDriverSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Editor.h"

FMcpServer::FMcpServer()
	: HttpServerModule(nullptr)
	, Port(8081)
	, bIsRunning(false)
{
}

FMcpServer::~FMcpServer()
{
	StopServer();
}

bool FMcpServer::StartServer(uint16 InPort, const FString& InBindAddress)
{
	if (bIsRunning)
	{
		UE_LOG(LogTemp, Warning, TEXT("McpServer: Server is already running"));
		return false;
	}

	Port = InPort;
	BindAddress = InBindAddress;

	// Get HTTP server module
	HttpServerModule = &FHttpServerModule::Get();
	if (!HttpServerModule)
	{
		UE_LOG(LogTemp, Error, TEXT("McpServer: Failed to get HttpServerModule"));
		return false;
	}

	// Create router
	HttpRouter = HttpServerModule->GetHttpRouter(Port);
	if (!HttpRouter.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("McpServer: Failed to create HTTP router"));
		return false;
	}

	// Register default tools
	RegisterDefaultTools();

	// Register JSON-RPC endpoint
	HttpRouter->BindRoute(
		FHttpPath(TEXT("/rpc")),
		EHttpServerRequestVerbs::VERB_POST,
		[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
		{
			return HandleJsonRpcRequest(Request, OnComplete);
		}
	);

	// Start HTTP server
	HttpServerModule->StartAllListeners();

	bIsRunning = true;
	UE_LOG(LogTemp, Log, TEXT("McpServer: Started on %s:%d"), *BindAddress, Port);

	return true;
}

void FMcpServer::StopServer()
{
	if (!bIsRunning)
	{
		return;
	}

	if (HttpServerModule)
	{
		HttpServerModule->StopAllListeners();
	}

	HttpRouter.Reset();
	RegisteredTools.Empty();

	bIsRunning = false;
	UE_LOG(LogTemp, Log, TEXT("McpServer: Stopped"));
}

void FMcpServer::RegisterTool(const FString& ToolName, TFunction<FString(const TSharedPtr<FJsonObject>&)> Handler)
{
	RegisteredTools.Add(ToolName, Handler);
	UE_LOG(LogTemp, Verbose, TEXT("McpServer: Registered tool '%s'"), *ToolName);
}

bool FMcpServer::HandleJsonRpcRequest(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	// Parse request body as JSON
	FString RequestBody = Request.Body;

	TSharedPtr<FJsonObject> JsonRequest;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RequestBody);

	if (!FJsonSerializer::Deserialize(Reader, JsonRequest) || !JsonRequest.IsValid())
	{
		FString ErrorResponse = BuildErrorResponse(-32700, TEXT("Parse error"), 0);
		auto Response = FHttpServerResponse::Create(ErrorResponse, TEXT("application/json"));
		OnComplete(MoveTemp(Response));
		return true;
	}

	// Extract JSON-RPC fields
	FString JsonRpcVersion = JsonRequest->GetStringField(TEXT("jsonrpc"));
	FString Method = JsonRequest->GetStringField(TEXT("method"));
	int32 Id = (int32)JsonRequest->GetNumberField(TEXT("id"));
	TSharedPtr<FJsonObject> Params = JsonRequest->GetObjectField(TEXT("params"));

	// Validate JSON-RPC version
	if (JsonRpcVersion != TEXT("2.0"))
	{
		FString ErrorResponse = BuildErrorResponse(-32600, TEXT("Invalid Request: jsonrpc must be 2.0"), Id);
		auto Response = FHttpServerResponse::Create(ErrorResponse, TEXT("application/json"));
		OnComplete(MoveTemp(Response));
		return true;
	}

	// Route to method handler
	FString ResponseBody;
	if (Method == TEXT("tools/list"))
	{
		ResponseBody = HandleListTools(Params);
	}
	else if (Method == TEXT("tools/call"))
	{
		ResponseBody = HandleToolCall(Params);
	}
	else
	{
		ResponseBody = BuildErrorResponse(-32601, TEXT("Method not found"), Id);
	}

	// Send response
	auto Response = FHttpServerResponse::Create(ResponseBody, TEXT("application/json"));
	Response->Code = EHttpServerResponseCodes::Ok;
	OnComplete(MoveTemp(Response));

	return true;
}

FString FMcpServer::HandleListTools(const TSharedPtr<FJsonObject>& Params)
{
	TArray<TSharedPtr<FJsonValue>> ToolsArray;

	for (const auto& Tool : RegisteredTools)
	{
		TSharedPtr<FJsonObject> ToolObj = MakeShareable(new FJsonObject());
		ToolObj->SetStringField(TEXT("name"), Tool.Key);
		ToolObj->SetStringField(TEXT("description"), FString::Printf(TEXT("Tool: %s"), *Tool.Key));

		TSharedPtr<FJsonObject> InputSchema = MakeShareable(new FJsonObject());
		InputSchema->SetStringField(TEXT("type"), TEXT("object"));
		ToolObj->SetObjectField(TEXT("inputSchema"), InputSchema);

		ToolsArray.Add(MakeShareable(new FJsonValueObject(ToolObj)));
	}

	TSharedPtr<FJsonObject> ResultObj = MakeShareable(new FJsonObject());
	ResultObj->SetArrayField(TEXT("tools"), ToolsArray);

	return BuildSuccessResponse(MakeShareable(new FJsonValueObject(ResultObj)), 1);
}

FString FMcpServer::HandleToolCall(const TSharedPtr<FJsonObject>& Params)
{
	if (!Params.IsValid())
	{
		return BuildErrorResponse(-32602, TEXT("Invalid params"), 1);
	}

	FString ToolName = Params->GetStringField(TEXT("name"));
	TSharedPtr<FJsonObject> Arguments = Params->GetObjectField(TEXT("arguments"));

	// Find tool handler
	if (!RegisteredTools.Contains(ToolName))
	{
		return BuildErrorResponse(-32601, FString::Printf(TEXT("Tool not found: %s"), *ToolName), 1);
	}

	// Execute tool
	FString ToolResult = RegisteredTools[ToolName](Arguments);

	// Build result
	TSharedPtr<FJsonObject> ResultObj = MakeShareable(new FJsonObject());

	TArray<TSharedPtr<FJsonValue>> ContentArray;
	TSharedPtr<FJsonObject> ContentObj = MakeShareable(new FJsonObject());
	ContentObj->SetStringField(TEXT("type"), TEXT("text"));
	ContentObj->SetStringField(TEXT("text"), ToolResult);
	ContentArray.Add(MakeShareable(new FJsonValueObject(ContentObj)));

	ResultObj->SetArrayField(TEXT("content"), ContentArray);
	ResultObj->SetBoolField(TEXT("isError"), false);

	return BuildSuccessResponse(MakeShareable(new FJsonValueObject(ResultObj)), 1);
}

FString FMcpServer::BuildSuccessResponse(const TSharedPtr<FJsonValue>& Result, int32 RequestId)
{
	TSharedPtr<FJsonObject> Response = MakeShareable(new FJsonObject());
	Response->SetStringField(TEXT("jsonrpc"), TEXT("2.0"));
	Response->SetField(TEXT("result"), Result);
	Response->SetNumberField(TEXT("id"), RequestId);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(Response.ToSharedRef(), Writer);

	return OutputString;
}

FString FMcpServer::BuildErrorResponse(int32 Code, const FString& Message, int32 RequestId)
{
	TSharedPtr<FJsonObject> ErrorObj = MakeShareable(new FJsonObject());
	ErrorObj->SetNumberField(TEXT("code"), Code);
	ErrorObj->SetStringField(TEXT("message"), Message);

	TSharedPtr<FJsonObject> Response = MakeShareable(new FJsonObject());
	Response->SetStringField(TEXT("jsonrpc"), TEXT("2.0"));
	Response->SetObjectField(TEXT("error"), ErrorObj);
	Response->SetNumberField(TEXT("id"), RequestId);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(Response.ToSharedRef(), Writer);

	return OutputString;
}

void FMcpServer::RegisterDefaultTools()
{
	// Register move_to_location tool
	RegisterTool(TEXT("autodriver/move_to_location"),
		[](const TSharedPtr<FJsonObject>& Args) -> FString
		{
			// Extract parameters
			TSharedPtr<FJsonObject> LocationObj = Args->GetObjectField(TEXT("location"));
			float X = LocationObj->GetNumberField(TEXT("x"));
			float Y = LocationObj->GetNumberField(TEXT("y"));
			float Z = LocationObj->GetNumberField(TEXT("z"));
			FVector TargetLocation(X, Y, Z);

			float AcceptanceRadius = Args->HasField(TEXT("acceptanceRadius"))
				? Args->GetNumberField(TEXT("acceptanceRadius"))
				: 50.0f;

			// TODO: Get auto driver and execute command
			// For now, return success message
			return FString::Printf(TEXT("Moving to location (%.1f, %.1f, %.1f) with radius %.1f"),
				X, Y, Z, AcceptanceRadius);
		}
	);

	// Register query_status tool
	RegisterTool(TEXT("autodriver/query_status"),
		[](const TSharedPtr<FJsonObject>& Args) -> FString
		{
			// TODO: Get actual status from auto driver
			return TEXT("Status: Ready");
		}
	);

	// Register stop_command tool
	RegisterTool(TEXT("autodriver/stop_command"),
		[](const TSharedPtr<FJsonObject>& Args) -> FString
		{
			// TODO: Stop current command
			return TEXT("Command stopped");
		}
	);
}
