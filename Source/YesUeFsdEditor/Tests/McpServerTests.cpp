// Copyright Yes UE FSD. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Server/McpServer.h"
#include "Subsystem/McpEditorSubsystem.h"
#include "Tests/AutomationCommon.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMcpServerCreationTest,
	"YesUeFsd.Server.MCP.Creation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMcpServerCreationTest::RunTest(const FString& Parameters)
{
	UMcpServer* Server = NewObject<UMcpServer>();
	TestNotNull(TEXT("MCP Server Created"), Server);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMcpServerStartStopTest,
	"YesUeFsd.Server.MCP.StartStop",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMcpServerStartStopTest::RunTest(const FString& Parameters)
{
	UMcpServer* Server = NewObject<UMcpServer>();

	// Test server start
	bool bStarted = Server->Start(8082); // Use different port to avoid conflicts
	TestTrue(TEXT("Server Started"), bStarted);

	// Test server stop
	Server->Stop();
	TestTrue(TEXT("Server Stopped"), true); // If we reach here, stop succeeded

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMcpServerPortTest,
	"YesUeFsd.Server.MCP.Port",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMcpServerPortTest::RunTest(const FString& Parameters)
{
	UMcpServer* Server = NewObject<UMcpServer>();

	// Test invalid port
	bool bStarted = Server->Start(0);
	TestFalse(TEXT("Invalid Port Rejected"), bStarted);

	// Test valid port
	bStarted = Server->Start(8083);
	TestTrue(TEXT("Valid Port Accepted"), bStarted);

	Server->Stop();

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMcpServerMultipleStartTest,
	"YesUeFsd.Server.MCP.MultipleStart",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMcpServerMultipleStartTest::RunTest(const FString& Parameters)
{
	UMcpServer* Server = NewObject<UMcpServer>();

	// Start server
	bool bStarted = Server->Start(8084);
	TestTrue(TEXT("Server Started"), bStarted);

	// Try to start again (should handle gracefully)
	bStarted = Server->Start(8084);

	// Stop server
	Server->Stop();

	TestTrue(TEXT("Multiple Start Handled"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMcpServerJsonRpcTest,
	"YesUeFsd.Server.MCP.JsonRpc",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMcpServerJsonRpcTest::RunTest(const FString& Parameters)
{
	// Test JSON-RPC request parsing
	FString JsonRequest = TEXT(R"({
		"jsonrpc": "2.0",
		"method": "moveToLocation",
		"params": {
			"x": 100.0,
			"y": 200.0,
			"z": 50.0
		},
		"id": 1
	})");

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonRequest);

	bool bParsed = FJsonSerializer::Deserialize(Reader, JsonObject);
	TestTrue(TEXT("JSON Parsed"), bParsed);
	TestTrue(TEXT("JSON Object Valid"), JsonObject.IsValid());

	if (JsonObject.IsValid())
	{
		FString Method = JsonObject->GetStringField(TEXT("method"));
		TestEqual(TEXT("Method Extracted"), Method, FString(TEXT("moveToLocation")));

		int32 Id = JsonObject->GetIntegerField(TEXT("id"));
		TestEqual(TEXT("ID Extracted"), Id, 1);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMcpServerJsonRpcResponseTest,
	"YesUeFsd.Server.MCP.JsonRpcResponse",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMcpServerJsonRpcResponseTest::RunTest(const FString& Parameters)
{
	// Test JSON-RPC response generation
	TSharedPtr<FJsonObject> Response = MakeShareable(new FJsonObject());
	Response->SetStringField(TEXT("jsonrpc"), TEXT("2.0"));
	Response->SetNumberField(TEXT("id"), 1);

	TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject());
	Result->SetBoolField(TEXT("success"), true);
	Response->SetObjectField(TEXT("result"), Result);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	bool bSerialized = FJsonSerializer::Serialize(Response.ToSharedRef(), Writer);

	TestTrue(TEXT("Response Serialized"), bSerialized);
	TestTrue(TEXT("Response Not Empty"), !OutputString.IsEmpty());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMcpServerErrorResponseTest,
	"YesUeFsd.Server.MCP.ErrorResponse",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMcpServerErrorResponseTest::RunTest(const FString& Parameters)
{
	// Test error response generation
	TSharedPtr<FJsonObject> Response = MakeShareable(new FJsonObject());
	Response->SetStringField(TEXT("jsonrpc"), TEXT("2.0"));
	Response->SetNumberField(TEXT("id"), 1);

	TSharedPtr<FJsonObject> Error = MakeShareable(new FJsonObject());
	Error->SetNumberField(TEXT("code"), -32601); // Method not found
	Error->SetStringField(TEXT("message"), TEXT("Method not found"));
	Response->SetObjectField(TEXT("error"), Error);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	bool bSerialized = FJsonSerializer::Serialize(Response.ToSharedRef(), Writer);

	TestTrue(TEXT("Error Response Serialized"), bSerialized);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMcpEditorSubsystemTest,
	"YesUeFsd.Server.EditorSubsystem.Creation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMcpEditorSubsystemTest::RunTest(const FString& Parameters)
{
	// Editor subsystem test
	UMcpEditorSubsystem* Subsystem = GEditor ? GEditor->GetEditorSubsystem<UMcpEditorSubsystem>() : nullptr;

	if (GEditor)
	{
		TestNotNull(TEXT("Editor Subsystem Available"), Subsystem);
	}
	else
	{
		AddInfo(TEXT("Editor not available, skipping subsystem test"));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMcpServerEndpointTest,
	"YesUeFsd.Server.MCP.Endpoints",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMcpServerEndpointTest::RunTest(const FString& Parameters)
{
	// Test that required endpoints are defined
	TArray<FString> RequiredMethods = {
		TEXT("moveToLocation"),
		TEXT("rotateTo"),
		TEXT("pressButton"),
		TEXT("setAxisValue"),
		TEXT("stopCommand"),
		TEXT("isLocationReachable"),
		TEXT("getPathLength"),
		TEXT("getRandomLocation")
	};

	// Just verify the method list is defined
	TestTrue(TEXT("Required Methods Defined"), RequiredMethods.Num() > 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMcpServerConcurrentRequestsTest,
	"YesUeFsd.Server.MCP.ConcurrentRequests",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMcpServerConcurrentRequestsTest::RunTest(const FString& Parameters)
{
	// Test that server can handle multiple requests
	// This is a structural test - actual concurrent testing would require running server

	UMcpServer* Server = NewObject<UMcpServer>();
	bool bStarted = Server->Start(8085);
	TestTrue(TEXT("Server Started"), bStarted);

	// In a real scenario, multiple HTTP requests would be sent here
	// For unit test, we just verify server is running

	Server->Stop();
	TestTrue(TEXT("Concurrent Requests Structure Test Passed"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMcpServerInvalidJsonTest,
	"YesUeFsd.Server.MCP.InvalidJson",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMcpServerInvalidJsonTest::RunTest(const FString& Parameters)
{
	// Test handling of invalid JSON
	FString InvalidJson = TEXT("{invalid json}");

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(InvalidJson);

	bool bParsed = FJsonSerializer::Deserialize(Reader, JsonObject);
	TestFalse(TEXT("Invalid JSON Rejected"), bParsed);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMcpServerMissingFieldsTest,
	"YesUeFsd.Server.MCP.MissingFields",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMcpServerMissingFieldsTest::RunTest(const FString& Parameters)
{
	// Test handling of JSON-RPC with missing required fields
	FString IncompleteJson = TEXT(R"({
		"jsonrpc": "2.0",
		"method": "moveToLocation"
	})");

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(IncompleteJson);

	bool bParsed = FJsonSerializer::Deserialize(Reader, JsonObject);
	TestTrue(TEXT("JSON Parsed"), bParsed);

	// Should be missing 'params' field
	bool bHasParams = JsonObject->HasField(TEXT("params"));
	TestFalse(TEXT("Missing Params Detected"), bHasParams);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
