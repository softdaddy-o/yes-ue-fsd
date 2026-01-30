// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/GameViewportClient.h"
#include "ScreenshotHelper.generated.h"

/**
 * Metadata for a captured screenshot
 */
USTRUCT(BlueprintType)
struct FScreenshotMetadata
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Screenshot")
	FString TestName;

	UPROPERTY(BlueprintReadWrite, Category = "Screenshot")
	FString TestPhase;

	UPROPERTY(BlueprintReadWrite, Category = "Screenshot")
	FString Timestamp;

	UPROPERTY(BlueprintReadWrite, Category = "Screenshot")
	FString FilePath;

	UPROPERTY(BlueprintReadWrite, Category = "Screenshot")
	int32 Width;

	UPROPERTY(BlueprintReadWrite, Category = "Screenshot")
	int32 Height;

	UPROPERTY(BlueprintReadWrite, Category = "Screenshot")
	FVector PlayerLocation;

	UPROPERTY(BlueprintReadWrite, Category = "Screenshot")
	FRotator PlayerRotation;

	UPROPERTY(BlueprintReadWrite, Category = "Screenshot")
	TMap<FString, FString> CustomMetadata;

	FScreenshotMetadata()
		: Width(0)
		, Height(0)
		, PlayerLocation(FVector::ZeroVector)
		, PlayerRotation(FRotator::ZeroRotator)
	{
	}
};

/**
 * Configuration for screenshot capture
 */
USTRUCT(BlueprintType)
struct FScreenshotCaptureConfig
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Screenshot")
	FString OutputDirectory;

	UPROPERTY(BlueprintReadWrite, Category = "Screenshot")
	FString NamingPattern;

	UPROPERTY(BlueprintReadWrite, Category = "Screenshot")
	bool bCaptureOnTestFailure;

	UPROPERTY(BlueprintReadWrite, Category = "Screenshot")
	bool bCaptureOnTestSuccess;

	UPROPERTY(BlueprintReadWrite, Category = "Screenshot")
	bool bGenerateManifest;

	UPROPERTY(BlueprintReadWrite, Category = "Screenshot")
	int32 MaxScreenshotsPerTest;

	FScreenshotCaptureConfig()
		: OutputDirectory(TEXT("Saved/Screenshots/Tests"))
		, NamingPattern(TEXT("{TestName}_{Timestamp}_{Phase}"))
		, bCaptureOnTestFailure(true)
		, bCaptureOnTestSuccess(false)
		, bGenerateManifest(true)
		, MaxScreenshotsPerTest(10)
	{
	}
};

/**
 * Helper class for capturing and managing test screenshots
 */
UCLASS(BlueprintType)
class YESUEFSD_API UScreenshotHelper : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Capture a screenshot with metadata
	 * @param TestName Name of the test
	 * @param Phase Test phase (e.g., "Setup", "Execution", "Teardown")
	 * @param CustomMetadata Additional metadata to store
	 * @return True if screenshot was captured successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Testing|Screenshot")
	static bool CaptureScreenshot(
		const FString& TestName,
		const FString& Phase = TEXT("Execution"),
		const TMap<FString, FString>& CustomMetadata = TMap<FString, FString>()
	);

	/**
	 * Capture a screenshot with full metadata structure
	 * @param Metadata Screenshot metadata structure
	 * @return True if screenshot was captured successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Testing|Screenshot")
	static bool CaptureScreenshotWithMetadata(FScreenshotMetadata& Metadata);

	/**
	 * Capture screenshot on test failure (called automatically by test framework)
	 * @param TestName Name of the failed test
	 * @param ErrorMessage Error message from the test
	 * @return True if screenshot was captured successfully
	 */
	static bool CaptureScreenshotOnFailure(const FString& TestName, const FString& ErrorMessage);

	/**
	 * Generate manifest file with all captured screenshots
	 * @param OutputPath Path to write the manifest file
	 * @return True if manifest was generated successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Testing|Screenshot")
	static bool GenerateManifest(const FString& OutputPath = TEXT(""));

	/**
	 * Generate HTML report with embedded screenshots
	 * @param OutputPath Path to write the HTML report
	 * @return True if report was generated successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Testing|Screenshot")
	static bool GenerateHTMLReport(const FString& OutputPath = TEXT(""));

	/**
	 * Configure screenshot capture settings
	 * @param Config Configuration structure
	 */
	UFUNCTION(BlueprintCallable, Category = "Testing|Screenshot")
	static void Configure(const FScreenshotCaptureConfig& Config);

	/**
	 * Get current configuration
	 * @return Current screenshot capture configuration
	 */
	UFUNCTION(BlueprintPure, Category = "Testing|Screenshot")
	static FScreenshotCaptureConfig GetConfiguration();

	/**
	 * Set output directory for screenshots
	 * @param Directory Path to output directory
	 */
	UFUNCTION(BlueprintCallable, Category = "Testing|Screenshot")
	static void SetOutputDirectory(const FString& Directory);

	/**
	 * Set naming pattern for screenshot files
	 * Supports placeholders: {TestName}, {Timestamp}, {Phase}, {Index}
	 * @param Pattern Naming pattern string
	 */
	UFUNCTION(BlueprintCallable, Category = "Testing|Screenshot")
	static void SetNamingPattern(const FString& Pattern);

	/**
	 * Clear all captured screenshots for current session
	 */
	UFUNCTION(BlueprintCallable, Category = "Testing|Screenshot")
	static void ClearScreenshots();

	/**
	 * Get list of all captured screenshots
	 * @return Array of screenshot metadata
	 */
	UFUNCTION(BlueprintPure, Category = "Testing|Screenshot")
	static TArray<FScreenshotMetadata> GetCapturedScreenshots();

	/**
	 * Get screenshots for a specific test
	 * @param TestName Name of the test
	 * @return Array of screenshot metadata for the test
	 */
	UFUNCTION(BlueprintPure, Category = "Testing|Screenshot")
	static TArray<FScreenshotMetadata> GetScreenshotsForTest(const FString& TestName);

	/**
	 * Enable or disable screenshot capture
	 * @param bEnabled True to enable, false to disable
	 */
	UFUNCTION(BlueprintCallable, Category = "Testing|Screenshot")
	static void SetEnabled(bool bEnabled);

	/**
	 * Check if screenshot capture is enabled
	 * @return True if enabled, false otherwise
	 */
	UFUNCTION(BlueprintPure, Category = "Testing|Screenshot")
	static bool IsEnabled();

private:
	// Capture screenshot implementation
	static bool CaptureScreenshotInternal(FScreenshotMetadata& Metadata);

	// Generate filename from pattern
	static FString GenerateFilename(const FScreenshotMetadata& Metadata, int32 Index = 0);

	// Ensure output directory exists
	static bool EnsureOutputDirectory(const FString& Directory);

	// Get player context for metadata
	static void GetPlayerContext(FVector& OutLocation, FRotator& OutRotation);

	// Serialize metadata to JSON
	static FString SerializeMetadataToJSON(const FScreenshotMetadata& Metadata);

	// Serialize all screenshots to JSON
	static FString SerializeAllMetadataToJSON();

	// Generate HTML report content
	static FString GenerateHTMLContent();

	// Static configuration
	static FScreenshotCaptureConfig Config;

	// Array of captured screenshots
	static TArray<FScreenshotMetadata> CapturedScreenshots;

	// Screenshot counter per test
	static TMap<FString, int32> ScreenshotCounters;

	// Is screenshot capture enabled
	static bool bEnabled;

	// Screenshot request queue
	static TArray<FScreenshotMetadata> PendingScreenshots;
};
