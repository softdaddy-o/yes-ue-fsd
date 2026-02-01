// Copyright Epic Games, Inc. All Rights Reserved.

#include "Testing/ScreenshotHelper.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/DateTime.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Dom/JsonObject.h"

// Initialize static members
FScreenshotCaptureConfig UScreenshotHelper::Config = FScreenshotCaptureConfig();
TArray<FScreenshotMetadata> UScreenshotHelper::CapturedScreenshots;
TMap<FString, int32> UScreenshotHelper::ScreenshotCounters;
bool UScreenshotHelper::bEnabled = true;
TArray<FScreenshotMetadata> UScreenshotHelper::PendingScreenshots;

bool UScreenshotHelper::CaptureScreenshot(
	const FString& TestName,
	const FString& Phase)
{
	if (!bEnabled)
	{
		UE_LOG(LogTemp, Warning, TEXT("Screenshot capture is disabled"));
		return false;
	}

	FScreenshotMetadata Metadata;
	Metadata.TestName = TestName;
	Metadata.TestPhase = Phase;

	return CaptureScreenshotInternal(Metadata);
}

bool UScreenshotHelper::CaptureScreenshotWithCustomMetadata(
	const FString& TestName,
	const FString& Phase,
	const TMap<FString, FString>& CustomMetadata)
{
	if (!bEnabled)
	{
		UE_LOG(LogTemp, Warning, TEXT("Screenshot capture is disabled"));
		return false;
	}

	FScreenshotMetadata Metadata;
	Metadata.TestName = TestName;
	Metadata.TestPhase = Phase;
	Metadata.CustomMetadata = CustomMetadata;

	return CaptureScreenshotInternal(Metadata);
}

bool UScreenshotHelper::CaptureScreenshotWithMetadata(FScreenshotMetadata& Metadata)
{
	if (!bEnabled)
	{
		UE_LOG(LogTemp, Warning, TEXT("Screenshot capture is disabled"));
		return false;
	}

	return CaptureScreenshotInternal(Metadata);
}

bool UScreenshotHelper::CaptureScreenshotOnFailure(const FString& TestName, const FString& ErrorMessage)
{
	if (!Config.bCaptureOnTestFailure)
	{
		return false;
	}

	TMap<FString, FString> CustomMetadata;
	CustomMetadata.Add(TEXT("ErrorMessage"), ErrorMessage);
	CustomMetadata.Add(TEXT("FailureType"), TEXT("TestFailure"));

	return CaptureScreenshotWithCustomMetadata(TestName, TEXT("Failure"), CustomMetadata);
}

bool UScreenshotHelper::CaptureScreenshotInternal(FScreenshotMetadata& Metadata)
{
	// Check if we've reached the limit for this test
	int32& Counter = ScreenshotCounters.FindOrAdd(Metadata.TestName, 0);
	if (Counter >= Config.MaxScreenshotsPerTest)
	{
		UE_LOG(LogTemp, Warning, TEXT("Max screenshots reached for test: %s"), *Metadata.TestName);
		return false;
	}
	Counter++;

	// Generate timestamp
	FDateTime Now = FDateTime::UtcNow();
	Metadata.Timestamp = Now.ToString(TEXT("%Y%m%d_%H%M%S_%f"));

	// Get player context
	GetPlayerContext(Metadata.PlayerLocation, Metadata.PlayerRotation);

	// Generate filename
	FString Filename = GenerateFilename(Metadata, Counter);
	FString OutputDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / Config.OutputDirectory);
	FString FullPath = OutputDir / Filename;

	// Ensure directory exists
	if (!EnsureOutputDirectory(OutputDir))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create output directory: %s"), *OutputDir);
		return false;
	}

	// Store file path in metadata
	Metadata.FilePath = FullPath;

	// Request screenshot from viewport
	if (GEngine && GEngine->GameViewport)
	{
		UGameViewportClient* ViewportClient = GEngine->GameViewport;
		FViewport* Viewport = ViewportClient->Viewport;

		if (Viewport)
		{
			// Get viewport size
			FIntPoint Size = Viewport->GetSizeXY();
			Metadata.Width = Size.X;
			Metadata.Height = Size.Y;

			// Read pixels from viewport
			TArray<FColor> Bitmap;
			if (Viewport->ReadPixels(Bitmap))
			{
				// Save as PNG
				IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
				TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

				if (ImageWrapper.IsValid() && ImageWrapper->SetRaw(Bitmap.GetData(), Bitmap.Num() * sizeof(FColor), Size.X, Size.Y, ERGBFormat::BGRA, 8))
				{
					const TArray64<uint8>& CompressedData = ImageWrapper->GetCompressed();
					if (FFileHelper::SaveArrayToFile(CompressedData, *FullPath))
					{
						UE_LOG(LogTemp, Log, TEXT("Screenshot captured: %s"), *FullPath);

						// Add to captured screenshots list
						CapturedScreenshots.Add(Metadata);

						return true;
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("Failed to save screenshot: %s"), *FullPath);
					}
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to read pixels from viewport"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Viewport is null"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GEngine or GameViewport is null"));
	}

	return false;
}

bool UScreenshotHelper::GenerateManifest(const FString& OutputPath)
{
	FString ManifestPath = OutputPath;
	if (ManifestPath.IsEmpty())
	{
		FString OutputDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / Config.OutputDirectory);
		ManifestPath = OutputDir / TEXT("screenshot_manifest.json");
	}

	FString JSONContent = SerializeAllMetadataToJSON();

	if (FFileHelper::SaveStringToFile(JSONContent, *ManifestPath))
	{
		UE_LOG(LogTemp, Log, TEXT("Screenshot manifest generated: %s"), *ManifestPath);
		return true;
	}

	UE_LOG(LogTemp, Error, TEXT("Failed to save manifest: %s"), *ManifestPath);
	return false;
}

bool UScreenshotHelper::GenerateHTMLReport(const FString& OutputPath)
{
	FString ReportPath = OutputPath;
	if (ReportPath.IsEmpty())
	{
		FString OutputDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / Config.OutputDirectory);
		ReportPath = OutputDir / TEXT("screenshot_report.html");
	}

	FString HTMLContent = GenerateHTMLContent();

	if (FFileHelper::SaveStringToFile(HTMLContent, *ReportPath))
	{
		UE_LOG(LogTemp, Log, TEXT("Screenshot HTML report generated: %s"), *ReportPath);
		return true;
	}

	UE_LOG(LogTemp, Error, TEXT("Failed to save HTML report: %s"), *ReportPath);
	return false;
}

void UScreenshotHelper::Configure(const FScreenshotCaptureConfig& NewConfig)
{
	Config = NewConfig;
	UE_LOG(LogTemp, Log, TEXT("Screenshot helper configured with output directory: %s"), *Config.OutputDirectory);
}

FScreenshotCaptureConfig UScreenshotHelper::GetConfiguration()
{
	return Config;
}

void UScreenshotHelper::SetOutputDirectory(const FString& Directory)
{
	Config.OutputDirectory = Directory;
}

void UScreenshotHelper::SetNamingPattern(const FString& Pattern)
{
	Config.NamingPattern = Pattern;
}

void UScreenshotHelper::ClearScreenshots()
{
	CapturedScreenshots.Empty();
	ScreenshotCounters.Empty();
	UE_LOG(LogTemp, Log, TEXT("Screenshot cache cleared"));
}

TArray<FScreenshotMetadata> UScreenshotHelper::GetCapturedScreenshots()
{
	return CapturedScreenshots;
}

TArray<FScreenshotMetadata> UScreenshotHelper::GetScreenshotsForTest(const FString& TestName)
{
	TArray<FScreenshotMetadata> TestScreenshots;
	for (const FScreenshotMetadata& Metadata : CapturedScreenshots)
	{
		if (Metadata.TestName == TestName)
		{
			TestScreenshots.Add(Metadata);
		}
	}
	return TestScreenshots;
}

void UScreenshotHelper::SetEnabled(bool bNewEnabled)
{
	bEnabled = bNewEnabled;
	UE_LOG(LogTemp, Log, TEXT("Screenshot capture %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

bool UScreenshotHelper::IsEnabled()
{
	return bEnabled;
}

FString UScreenshotHelper::GenerateFilename(const FScreenshotMetadata& Metadata, int32 Index)
{
	FString Filename = Config.NamingPattern;

	// Replace placeholders
	Filename = Filename.Replace(TEXT("{TestName}"), *Metadata.TestName);
	Filename = Filename.Replace(TEXT("{Timestamp}"), *Metadata.Timestamp);
	Filename = Filename.Replace(TEXT("{Phase}"), *Metadata.TestPhase);
	Filename = Filename.Replace(TEXT("{Index}"), *FString::FromInt(Index));

	// Sanitize filename (remove invalid characters)
	Filename = Filename.Replace(TEXT(":"), TEXT("-"));
	Filename = Filename.Replace(TEXT("/"), TEXT("-"));
	Filename = Filename.Replace(TEXT("\\"), TEXT("-"));
	Filename = Filename.Replace(TEXT("*"), TEXT("-"));
	Filename = Filename.Replace(TEXT("?"), TEXT("-"));
	Filename = Filename.Replace(TEXT("\""), TEXT("-"));
	Filename = Filename.Replace(TEXT("<"), TEXT("-"));
	Filename = Filename.Replace(TEXT(">"), TEXT("-"));
	Filename = Filename.Replace(TEXT("|"), TEXT("-"));

	// Add .png extension if not present
	if (!Filename.EndsWith(TEXT(".png")))
	{
		Filename += TEXT(".png");
	}

	return Filename;
}

bool UScreenshotHelper::EnsureOutputDirectory(const FString& Directory)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (!PlatformFile.DirectoryExists(*Directory))
	{
		return PlatformFile.CreateDirectoryTree(*Directory);
	}

	return true;
}

void UScreenshotHelper::GetPlayerContext(FVector& OutLocation, FRotator& OutRotation)
{
	OutLocation = FVector::ZeroVector;
	OutRotation = FRotator::ZeroRotator;

	if (GEngine && GEngine->GameViewport)
	{
		UWorld* World = GEngine->GameViewport->GetWorld();
		if (World)
		{
			APlayerController* PC = World->GetFirstPlayerController();
			if (PC && PC->GetPawn())
			{
				OutLocation = PC->GetPawn()->GetActorLocation();
				OutRotation = PC->GetControlRotation();
			}
		}
	}
}

FString UScreenshotHelper::SerializeMetadataToJSON(const FScreenshotMetadata& Metadata)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	JsonObject->SetStringField(TEXT("testName"), Metadata.TestName);
	JsonObject->SetStringField(TEXT("testPhase"), Metadata.TestPhase);
	JsonObject->SetStringField(TEXT("timestamp"), Metadata.Timestamp);
	JsonObject->SetStringField(TEXT("filePath"), Metadata.FilePath);
	JsonObject->SetNumberField(TEXT("width"), Metadata.Width);
	JsonObject->SetNumberField(TEXT("height"), Metadata.Height);

	// Player location
	TSharedPtr<FJsonObject> LocationObject = MakeShareable(new FJsonObject);
	LocationObject->SetNumberField(TEXT("x"), Metadata.PlayerLocation.X);
	LocationObject->SetNumberField(TEXT("y"), Metadata.PlayerLocation.Y);
	LocationObject->SetNumberField(TEXT("z"), Metadata.PlayerLocation.Z);
	JsonObject->SetObjectField(TEXT("playerLocation"), LocationObject);

	// Player rotation
	TSharedPtr<FJsonObject> RotationObject = MakeShareable(new FJsonObject);
	RotationObject->SetNumberField(TEXT("pitch"), Metadata.PlayerRotation.Pitch);
	RotationObject->SetNumberField(TEXT("yaw"), Metadata.PlayerRotation.Yaw);
	RotationObject->SetNumberField(TEXT("roll"), Metadata.PlayerRotation.Roll);
	JsonObject->SetObjectField(TEXT("playerRotation"), RotationObject);

	// Custom metadata
	TSharedPtr<FJsonObject> CustomMetadataObject = MakeShareable(new FJsonObject);
	for (const auto& Pair : Metadata.CustomMetadata)
	{
		CustomMetadataObject->SetStringField(Pair.Key, Pair.Value);
	}
	JsonObject->SetObjectField(TEXT("customMetadata"), CustomMetadataObject);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	return OutputString;
}

FString UScreenshotHelper::SerializeAllMetadataToJSON()
{
	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);

	// Add summary
	RootObject->SetNumberField(TEXT("totalScreenshots"), CapturedScreenshots.Num());
	RootObject->SetStringField(TEXT("generatedAt"), FDateTime::UtcNow().ToString());

	// Group by test name
	TMap<FString, int32> TestCounts;
	for (const FScreenshotMetadata& Metadata : CapturedScreenshots)
	{
		int32& Count = TestCounts.FindOrAdd(Metadata.TestName, 0);
		Count++;
	}

	TSharedPtr<FJsonObject> TestSummaryObject = MakeShareable(new FJsonObject);
	for (const auto& Pair : TestCounts)
	{
		TestSummaryObject->SetNumberField(Pair.Key, Pair.Value);
	}
	RootObject->SetObjectField(TEXT("testSummary"), TestSummaryObject);

	// Add all screenshots
	TArray<TSharedPtr<FJsonValue>> ScreenshotArray;
	for (const FScreenshotMetadata& Metadata : CapturedScreenshots)
	{
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

		JsonObject->SetStringField(TEXT("testName"), Metadata.TestName);
		JsonObject->SetStringField(TEXT("testPhase"), Metadata.TestPhase);
		JsonObject->SetStringField(TEXT("timestamp"), Metadata.Timestamp);
		JsonObject->SetStringField(TEXT("filePath"), Metadata.FilePath);
		JsonObject->SetNumberField(TEXT("width"), Metadata.Width);
		JsonObject->SetNumberField(TEXT("height"), Metadata.Height);

		// Player location
		TSharedPtr<FJsonObject> LocationObject = MakeShareable(new FJsonObject);
		LocationObject->SetNumberField(TEXT("x"), Metadata.PlayerLocation.X);
		LocationObject->SetNumberField(TEXT("y"), Metadata.PlayerLocation.Y);
		LocationObject->SetNumberField(TEXT("z"), Metadata.PlayerLocation.Z);
		JsonObject->SetObjectField(TEXT("playerLocation"), LocationObject);

		// Player rotation
		TSharedPtr<FJsonObject> RotationObject = MakeShareable(new FJsonObject);
		RotationObject->SetNumberField(TEXT("pitch"), Metadata.PlayerRotation.Pitch);
		RotationObject->SetNumberField(TEXT("yaw"), Metadata.PlayerRotation.Yaw);
		RotationObject->SetNumberField(TEXT("roll"), Metadata.PlayerRotation.Roll);
		JsonObject->SetObjectField(TEXT("playerRotation"), RotationObject);

		// Custom metadata
		TSharedPtr<FJsonObject> CustomMetadataObject = MakeShareable(new FJsonObject);
		for (const auto& Pair : Metadata.CustomMetadata)
		{
			CustomMetadataObject->SetStringField(Pair.Key, Pair.Value);
		}
		JsonObject->SetObjectField(TEXT("customMetadata"), CustomMetadataObject);

		ScreenshotArray.Add(MakeShareable(new FJsonValueObject(JsonObject)));
	}
	RootObject->SetArrayField(TEXT("screenshots"), ScreenshotArray);

	FString OutputString;
	TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> Writer =
		TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&OutputString);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	return OutputString;
}

FString UScreenshotHelper::GenerateHTMLContent()
{
	FString HTML = TEXT("<!DOCTYPE html>\n<html>\n<head>\n");
	HTML += TEXT("<meta charset=\"utf-8\">\n");
	HTML += TEXT("<title>Screenshot Test Report</title>\n");
	HTML += TEXT("<style>\n");
	HTML += TEXT("body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }\n");
	HTML += TEXT("h1 { color: #333; }\n");
	HTML += TEXT(".summary { background: white; padding: 20px; margin-bottom: 20px; border-radius: 5px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }\n");
	HTML += TEXT(".test-group { background: white; padding: 20px; margin-bottom: 20px; border-radius: 5px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }\n");
	HTML += TEXT(".screenshot { margin: 10px 0; padding: 10px; border: 1px solid #ddd; background: #fafafa; }\n");
	HTML += TEXT(".screenshot img { max-width: 800px; border: 1px solid #ccc; }\n");
	HTML += TEXT(".metadata { font-size: 12px; color: #666; margin-top: 5px; }\n");
	HTML += TEXT(".metadata-key { font-weight: bold; }\n");
	HTML += TEXT("</style>\n");
	HTML += TEXT("</head>\n<body>\n");

	HTML += TEXT("<h1>Screenshot Test Report</h1>\n");

	// Summary section
	HTML += TEXT("<div class=\"summary\">\n");
	HTML += FString::Printf(TEXT("<p><strong>Total Screenshots:</strong> %d</p>\n"), CapturedScreenshots.Num());
	HTML += FString::Printf(TEXT("<p><strong>Generated At:</strong> %s</p>\n"), *FDateTime::UtcNow().ToString());
	HTML += TEXT("</div>\n");

	// Group screenshots by test name
	TMap<FString, TArray<FScreenshotMetadata>> GroupedScreenshots;
	for (const FScreenshotMetadata& Metadata : CapturedScreenshots)
	{
		TArray<FScreenshotMetadata>& TestGroup = GroupedScreenshots.FindOrAdd(Metadata.TestName);
		TestGroup.Add(Metadata);
	}

	// Generate HTML for each test group
	for (const auto& Pair : GroupedScreenshots)
	{
		HTML += FString::Printf(TEXT("<div class=\"test-group\">\n<h2>%s</h2>\n"), *Pair.Key);

		for (const FScreenshotMetadata& Metadata : Pair.Value)
		{
			HTML += TEXT("<div class=\"screenshot\">\n");
			HTML += FString::Printf(TEXT("<h3>%s</h3>\n"), *Metadata.TestPhase);
			HTML += FString::Printf(TEXT("<img src=\"file:///%s\" alt=\"Screenshot\">\n"), *Metadata.FilePath);
			HTML += TEXT("<div class=\"metadata\">\n");
			HTML += FString::Printf(TEXT("<p><span class=\"metadata-key\">Timestamp:</span> %s</p>\n"), *Metadata.Timestamp);
			HTML += FString::Printf(TEXT("<p><span class=\"metadata-key\">Resolution:</span> %dx%d</p>\n"), Metadata.Width, Metadata.Height);
			HTML += FString::Printf(TEXT("<p><span class=\"metadata-key\">Player Location:</span> (%.2f, %.2f, %.2f)</p>\n"),
				Metadata.PlayerLocation.X, Metadata.PlayerLocation.Y, Metadata.PlayerLocation.Z);

			if (Metadata.CustomMetadata.Num() > 0)
			{
				HTML += TEXT("<p><span class=\"metadata-key\">Custom Metadata:</span></p>\n");
				HTML += TEXT("<ul>\n");
				for (const auto& CustomPair : Metadata.CustomMetadata)
				{
					HTML += FString::Printf(TEXT("<li>%s: %s</li>\n"), *CustomPair.Key, *CustomPair.Value);
				}
				HTML += TEXT("</ul>\n");
			}

			HTML += TEXT("</div>\n"); // metadata
			HTML += TEXT("</div>\n"); // screenshot
		}

		HTML += TEXT("</div>\n"); // test-group
	}

	HTML += TEXT("</body>\n</html>");

	return HTML;
}
