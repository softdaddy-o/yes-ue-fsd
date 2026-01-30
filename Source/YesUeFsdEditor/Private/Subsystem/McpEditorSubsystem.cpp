// Copyright Epic Games, Inc. All Rights Reserved.

#include "Subsystem/McpEditorSubsystem.h"
#include "Server/McpServer.h"

void UMcpEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Load settings
	LoadSettings();

	// Create server instance
	McpServer = MakeUnique<FMcpServer>();

	// Auto-start if configured
	if (bAutoStartServer)
	{
		StartServer();
	}

	UE_LOG(LogTemp, Log, TEXT("McpEditorSubsystem: Initialized"));
}

void UMcpEditorSubsystem::Deinitialize()
{
	// Stop server
	StopServer();

	// Cleanup
	McpServer.Reset();

	UE_LOG(LogTemp, Log, TEXT("McpEditorSubsystem: Deinitialized"));

	Super::Deinitialize();
}

bool UMcpEditorSubsystem::StartServer()
{
	if (!McpServer.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("McpEditorSubsystem: Server instance is invalid"));
		return false;
	}

	if (McpServer->IsRunning())
	{
		UE_LOG(LogTemp, Warning, TEXT("McpEditorSubsystem: Server is already running"));
		return true;
	}

	bool bSuccess = McpServer->StartServer(ServerPort, BindAddress);
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("McpEditorSubsystem: Server started at %s"), *GetServerUrl());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("McpEditorSubsystem: Failed to start server"));
	}

	return bSuccess;
}

void UMcpEditorSubsystem::StopServer()
{
	if (McpServer.IsValid() && McpServer->IsRunning())
	{
		McpServer->StopServer();
		UE_LOG(LogTemp, Log, TEXT("McpEditorSubsystem: Server stopped"));
	}
}

bool UMcpEditorSubsystem::IsServerRunning() const
{
	return McpServer.IsValid() && McpServer->IsRunning();
}

int32 UMcpEditorSubsystem::GetServerPort() const
{
	return McpServer.IsValid() ? McpServer->GetPort() : 0;
}

FString UMcpEditorSubsystem::GetServerUrl() const
{
	if (!McpServer.IsValid())
	{
		return TEXT("");
	}

	return FString::Printf(TEXT("http://%s:%d/rpc"),
		*McpServer->GetBindAddress(),
		McpServer->GetPort());
}

void UMcpEditorSubsystem::LoadSettings()
{
	// Load from config file (DefaultYesUeFsd.ini)
	ServerPort = 8081;
	BindAddress = TEXT("127.0.0.1");
	bAutoStartServer = true;

	// Try to read from config
	GConfig->GetInt(
		TEXT("/Script/YesUeFsdEditor.AutoDriverSettings"),
		TEXT("ServerPort"),
		reinterpret_cast<int32&>(ServerPort),
		GEditorIni
	);

	GConfig->GetString(
		TEXT("/Script/YesUeFsdEditor.AutoDriverSettings"),
		TEXT("BindAddress"),
		BindAddress,
		GEditorIni
	);

	GConfig->GetBool(
		TEXT("/Script/YesUeFsdEditor.AutoDriverSettings"),
		TEXT("bAutoStartServer"),
		bAutoStartServer,
		GEditorIni
	);

	UE_LOG(LogTemp, Log, TEXT("McpEditorSubsystem: Loaded settings - Port: %d, Address: %s, AutoStart: %s"),
		ServerPort, *BindAddress, bAutoStartServer ? TEXT("true") : TEXT("false"));
}
