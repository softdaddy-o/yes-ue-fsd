# Screenshot Capture System

**Version**: 1.0
**Date**: 2026-01-30
**Status**: Production Ready

## Overview

The Screenshot Capture System enables automated visual testing and regression detection for the Yes UE FSD plugin. It provides comprehensive functionality for capturing, organizing, and exporting screenshots during test execution.

---

## Table of Contents

1. [Features](#features)
2. [Quick Start](#quick-start)
3. [C++ API](#c-api)
4. [Python API](#python-api)
5. [Blueprint Integration](#blueprint-integration)
6. [Configuration](#configuration)
7. [Metadata Tracking](#metadata-tracking)
8. [Export and Reporting](#export-and-reporting)
9. [CI/CD Integration](#cicd-integration)
10. [Best Practices](#best-practices)

---

## Features

### Core Capabilities

- **Automated Screenshot Capture**: Capture screenshots at any point during test execution
- **Metadata Tracking**: Store comprehensive metadata with each screenshot
  - Test name and phase
  - Timestamp (UTC)
  - Player position and rotation
  - Viewport resolution
  - Custom metadata fields
- **Flexible Storage**: Configurable output directory and naming patterns
- **Export Options**:
  - JSON manifest with all screenshot metadata
  - HTML report with embedded screenshots
  - CI/CD artifact integration
- **Configuration**: Extensive configuration options for capture behavior
- **Multi-Language Support**: C++, Python, and Blueprint APIs

---

## Quick Start

### C++ Usage

```cpp
#include "Testing/ScreenshotHelper.h"

// Capture a basic screenshot
UScreenshotHelper::CaptureScreenshot(TEXT("MyTest"), TEXT("Execution"));

// Capture with custom metadata
TMap<FString, FString> Metadata;
Metadata.Add(TEXT("action"), TEXT("movement"));
Metadata.Add(TEXT("speed"), TEXT("fast"));
UScreenshotHelper::CaptureScreenshot(TEXT("MyTest"), TEXT("Execution"), Metadata);

// Generate reports
UScreenshotHelper::GenerateManifest();
UScreenshotHelper::GenerateHTMLReport();
```

### Python Usage

```python
import screenshot_system

# Capture a screenshot
screenshot_system.capture("MyTest", "Execution")

# Capture with metadata
metadata = {"action": "movement", "speed": "fast"}
screenshot_system.capture("MyTest", "Execution", metadata)

# Generate reports
screenshot_system.generate_manifest()
screenshot_system.generate_html_report()
```

### Blueprint Usage

1. Add "Capture Test Screenshot" node to your test blueprint
2. Set Test Name and Phase inputs
3. Connect to execution flow
4. Use "Generate Manifest" and "Generate HTML Report" nodes after tests

---

## C++ API

### UScreenshotHelper Class

#### Core Functions

##### CaptureScreenshot

```cpp
static bool CaptureScreenshot(
    const FString& TestName,
    const FString& Phase = TEXT("Execution"),
    const TMap<FString, FString>& CustomMetadata = TMap<FString, FString>()
);
```

Captures a screenshot with metadata.

**Parameters**:
- `TestName`: Name of the test
- `Phase`: Test phase (e.g., "Setup", "Execution", "Teardown")
- `CustomMetadata`: Additional metadata key-value pairs

**Returns**: `true` if screenshot was captured successfully

**Example**:
```cpp
TMap<FString, FString> Metadata;
Metadata.Add(TEXT("feature"), TEXT("navigation"));
UScreenshotHelper::CaptureScreenshot(TEXT("NavigationTest"), TEXT("Execution"), Metadata);
```

##### CaptureScreenshotWithMetadata

```cpp
static bool CaptureScreenshotWithMetadata(FScreenshotMetadata& Metadata);
```

Captures a screenshot using a complete metadata structure.

**Parameters**:
- `Metadata`: Screenshot metadata structure (will be filled with capture details)

**Returns**: `true` if screenshot was captured successfully

**Example**:
```cpp
FScreenshotMetadata Metadata;
Metadata.TestName = TEXT("MyTest");
Metadata.TestPhase = TEXT("Setup");
Metadata.CustomMetadata.Add(TEXT("iteration"), TEXT("1"));
UScreenshotHelper::CaptureScreenshotWithMetadata(Metadata);
```

##### CaptureScreenshotOnFailure

```cpp
static bool CaptureScreenshotOnFailure(const FString& TestName, const FString& ErrorMessage);
```

Captures a screenshot when a test fails (called automatically by test framework).

**Parameters**:
- `TestName`: Name of the failed test
- `ErrorMessage`: Error message from the test

**Returns**: `true` if screenshot was captured successfully

#### Export Functions

##### GenerateManifest

```cpp
static bool GenerateManifest(const FString& OutputPath = TEXT(""));
```

Generates a JSON manifest file with all captured screenshots.

**Parameters**:
- `OutputPath`: Path to write the manifest file (defaults to configured output directory)

**Returns**: `true` if manifest was generated successfully

**Manifest Structure**:
```json
{
  "totalScreenshots": 10,
  "generatedAt": "2026-01-30T12:00:00Z",
  "testSummary": {
    "TestA": 5,
    "TestB": 5
  },
  "screenshots": [
    {
      "testName": "TestA",
      "testPhase": "Execution",
      "timestamp": "20260130_120000_000",
      "filePath": "D:/Project/Saved/Screenshots/TestA_20260130_120000_000_Execution.png",
      "width": 1920,
      "height": 1080,
      "playerLocation": { "x": 0, "y": 0, "z": 100 },
      "playerRotation": { "pitch": 0, "yaw": 0, "roll": 0 },
      "customMetadata": {}
    }
  ]
}
```

##### GenerateHTMLReport

```cpp
static bool GenerateHTMLReport(const FString& OutputPath = TEXT(""));
```

Generates an HTML report with embedded screenshots.

**Parameters**:
- `OutputPath`: Path to write the HTML report (defaults to configured output directory)

**Returns**: `true` if report was generated successfully

#### Configuration Functions

##### Configure

```cpp
static void Configure(const FScreenshotCaptureConfig& Config);
```

Configures screenshot capture settings.

**Example**:
```cpp
FScreenshotCaptureConfig Config;
Config.OutputDirectory = TEXT("Saved/Screenshots/MyTests");
Config.NamingPattern = TEXT("{TestName}_{Timestamp}_{Phase}");
Config.bCaptureOnTestFailure = true;
Config.bCaptureOnTestSuccess = false;
Config.MaxScreenshotsPerTest = 10;
UScreenshotHelper::Configure(Config);
```

##### SetOutputDirectory

```cpp
static void SetOutputDirectory(const FString& Directory);
```

Sets the output directory for screenshots.

##### SetNamingPattern

```cpp
static void SetNamingPattern(const FString& Pattern);
```

Sets the naming pattern for screenshot files.

**Supported Placeholders**:
- `{TestName}`: Name of the test
- `{Timestamp}`: Timestamp in format YYYYMMDD_HHMMSS_fff
- `{Phase}`: Test phase (Setup, Execution, Teardown)
- `{Index}`: Screenshot index for the test

**Example**:
```cpp
UScreenshotHelper::SetNamingPattern(TEXT("{TestName}_{Index}_{Phase}"));
```

##### SetEnabled / IsEnabled

```cpp
static void SetEnabled(bool bEnabled);
static bool IsEnabled();
```

Enable or disable screenshot capture globally.

#### Query Functions

##### GetCapturedScreenshots

```cpp
static TArray<FScreenshotMetadata> GetCapturedScreenshots();
```

Gets all captured screenshots for the current session.

##### GetScreenshotsForTest

```cpp
static TArray<FScreenshotMetadata> GetScreenshotsForTest(const FString& TestName);
```

Gets screenshots for a specific test.

##### ClearScreenshots

```cpp
static void ClearScreenshots();
```

Clears all captured screenshots from memory.

---

## Python API

### Module: screenshot_system

#### Functions

##### capture

```python
def capture(test_name: str, phase: str = "Execution", metadata: Optional[Dict[str, str]] = None) -> bool
```

Captures a screenshot with metadata.

**Example**:
```python
import screenshot_system

# Basic capture
screenshot_system.capture("MyTest", "Execution")

# With metadata
metadata = {"feature": "movement", "speed": "fast"}
screenshot_system.capture("MyTest", "Execution", metadata)
```

##### capture_on_failure

```python
def capture_on_failure(test_name: str, error_message: str) -> bool
```

Captures screenshot on test failure.

**Example**:
```python
try:
    # Test code
    assert something
except AssertionError as e:
    screenshot_system.capture_on_failure("MyTest", str(e))
    raise
```

##### configure

```python
def configure(
    output_dir: Optional[str] = None,
    naming_pattern: Optional[str] = None,
    capture_on_failure: Optional[bool] = None,
    capture_on_success: Optional[bool] = None,
    generate_manifest: Optional[bool] = None,
    max_screenshots_per_test: Optional[int] = None
) -> None
```

Configures screenshot system.

**Example**:
```python
screenshot_system.configure(
    output_dir="Saved/Screenshots/Tests",
    capture_on_failure=True,
    max_screenshots_per_test=5
)
```

##### generate_manifest / generate_html_report

```python
def generate_manifest(output_path: str = "") -> bool
def generate_html_report(output_path: str = "") -> bool
```

Generates manifest and HTML report.

**Example**:
```python
screenshot_system.generate_manifest()
screenshot_system.generate_html_report()
```

##### Query Functions

```python
def get_captured_screenshots() -> List
def get_screenshots_for_test(test_name: str) -> List
def is_enabled() -> bool
```

Query screenshot capture state.

---

## Blueprint Integration

### Available Nodes

#### Capture Test Screenshot

**Category**: Testing | Screenshot
**Inputs**:
- Test Name (String)
- Phase (String) - Default: "Execution"
- Custom Metadata (Map<String, String>)

**Outputs**:
- Return Value (Boolean) - Success status

#### Generate Manifest

**Category**: Testing | Screenshot
**Inputs**:
- Output Path (String) - Optional

**Outputs**:
- Return Value (Boolean) - Success status

#### Generate HTML Report

**Category**: Testing | Screenshot
**Inputs**:
- Output Path (String) - Optional

**Outputs**:
- Return Value (Boolean) - Success status

#### Set Output Directory

**Category**: Testing | Screenshot
**Inputs**:
- Directory (String)

#### Set Screenshot Enabled

**Category**: Testing | Screenshot
**Inputs**:
- Enabled (Boolean)

---

## Configuration

### FScreenshotCaptureConfig Structure

```cpp
USTRUCT(BlueprintType)
struct FScreenshotCaptureConfig
{
    UPROPERTY(BlueprintReadWrite, Category = "Screenshot")
    FString OutputDirectory; // Default: "Saved/Screenshots/Tests"

    UPROPERTY(BlueprintReadWrite, Category = "Screenshot")
    FString NamingPattern; // Default: "{TestName}_{Timestamp}_{Phase}"

    UPROPERTY(BlueprintReadWrite, Category = "Screenshot")
    bool bCaptureOnTestFailure; // Default: true

    UPROPERTY(BlueprintReadWrite, Category = "Screenshot")
    bool bCaptureOnTestSuccess; // Default: false

    UPROPERTY(BlueprintReadWrite, Category = "Screenshot")
    bool bGenerateManifest; // Default: true

    UPROPERTY(BlueprintReadWrite, Category = "Screenshot")
    int32 MaxScreenshotsPerTest; // Default: 10
};
```

### Configuration Best Practices

1. **Output Directory**: Use descriptive paths like `Saved/Screenshots/{TestSuite}`
2. **Naming Pattern**: Include timestamp and phase for uniqueness
3. **Capture on Failure**: Always enable for debugging
4. **Max Screenshots**: Set reasonable limits to avoid disk usage issues
5. **Generate Manifest**: Enable for CI/CD integration

---

## Metadata Tracking

### FScreenshotMetadata Structure

```cpp
USTRUCT(BlueprintType)
struct FScreenshotMetadata
{
    UPROPERTY()
    FString TestName; // Name of the test

    UPROPERTY()
    FString TestPhase; // Phase: Setup, Execution, Teardown, Failure

    UPROPERTY()
    FString Timestamp; // UTC timestamp

    UPROPERTY()
    FString FilePath; // Full path to screenshot file

    UPROPERTY()
    int32 Width; // Screenshot width

    UPROPERTY()
    int32 Height; // Screenshot height

    UPROPERTY()
    FVector PlayerLocation; // Player position at capture time

    UPROPERTY()
    FRotator PlayerRotation; // Player rotation at capture time

    UPROPERTY()
    TMap<FString, FString> CustomMetadata; // Custom key-value pairs
};
```

### Automatic Metadata

The system automatically captures:
- Timestamp (UTC)
- Viewport resolution
- Player position and rotation (if available)
- File path

### Custom Metadata

Add custom metadata for your test context:

```cpp
TMap<FString, FString> Metadata;
Metadata.Add(TEXT("test_iteration"), TEXT("5"));
Metadata.Add(TEXT("difficulty"), TEXT("hard"));
Metadata.Add(TEXT("ai_count"), TEXT("10"));
UScreenshotHelper::CaptureScreenshot(TEXT("StressTest"), TEXT("Execution"), Metadata);
```

---

## Export and Reporting

### JSON Manifest

The manifest file contains:
- Summary (total screenshots, generation time)
- Test summary (screenshots per test)
- Complete metadata for all screenshots

**File**: `screenshot_manifest.json`

### HTML Report

The HTML report provides:
- Visual gallery of all screenshots
- Grouped by test name
- Embedded metadata
- Filterable and searchable (when opened in browser)

**File**: `screenshot_report.html`

### Example Workflow

```cpp
// Configure
FScreenshotCaptureConfig Config;
Config.OutputDirectory = TEXT("Saved/Screenshots/SmokeTests");
UScreenshotHelper::Configure(Config);

// Run tests and capture screenshots
UScreenshotHelper::CaptureScreenshot(TEXT("Test1"), TEXT("Setup"));
UScreenshotHelper::CaptureScreenshot(TEXT("Test1"), TEXT("Execution"));
UScreenshotHelper::CaptureScreenshot(TEXT("Test2"), TEXT("Execution"));

// Generate reports
UScreenshotHelper::GenerateManifest();
UScreenshotHelper::GenerateHTMLReport();

// Upload to CI (example with GitHub Actions)
// Artifacts will be in Saved/Screenshots/SmokeTests/
```

---

## CI/CD Integration

### GitHub Actions Example

```yaml
- name: Run Tests with Screenshots
  run: |
    UnrealEditor-Cmd.exe -ExecCmds="Automation RunTests YesUeFsd" -unattended -nopause -NullRHI -log

- name: Generate Screenshot Reports
  if: always()
  run: |
    # Reports are generated automatically if configured

- name: Upload Screenshot Artifacts
  uses: actions/upload-artifact@v4
  if: always()
  with:
    name: test-screenshots
    path: |
      Saved/Screenshots/Tests/**/*.png
      Saved/Screenshots/Tests/screenshot_manifest.json
      Saved/Screenshots/Tests/screenshot_report.html
```

### Integration with Test Framework

```cpp
// In your test
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMyTest, "YesUeFsd.MyTest",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMyTest::RunTest(const FString& Parameters)
{
    // Setup
    UScreenshotHelper::CaptureScreenshot(TEXT("MyTest"), TEXT("Setup"));

    // Test execution
    UScreenshotHelper::CaptureScreenshot(TEXT("MyTest"), TEXT("Execution"));

    // On failure
    if (!TestSomething())
    {
        UScreenshotHelper::CaptureScreenshotOnFailure(TEXT("MyTest"), TEXT("Test failed"));
        return false;
    }

    // Teardown
    UScreenshotHelper::CaptureScreenshot(TEXT("MyTest"), TEXT("Teardown"));

    return true;
}
```

---

## Best Practices

### When to Capture Screenshots

1. **Test Failures**: Always capture on failure for debugging
2. **Visual Changes**: Capture before/after visual modifications
3. **Critical States**: Capture at important test milestones
4. **Regression Testing**: Capture expected UI states for comparison

### Naming Conventions

Use descriptive test names and phases:
- Test Name: `{Feature}_{Scenario}` (e.g., "Navigation_PathBlocked")
- Phase: `Setup`, `Execution`, `Teardown`, `Failure`
- Custom phases: `BeforeAction`, `AfterAction`, etc.

### Storage Management

1. **Clean up old screenshots**: Implement cleanup policies
2. **Limit screenshots per test**: Use `MaxScreenshotsPerTest`
3. **Compress artifacts**: Zip screenshots before uploading to CI
4. **Retention policy**: Keep screenshots for N days

### Performance Considerations

1. **Capture selectively**: Don't capture every frame
2. **Async capture**: Use async operations when possible
3. **Resolution limits**: Consider capturing at lower resolutions for performance
4. **Disable in production**: Screenshot capture should be test-only

### Visual Regression Testing

To implement visual regression testing:

1. Capture baseline screenshots
2. Store baselines in version control or artifact storage
3. Compare new screenshots against baselines
4. Report differences with pixel-diff tools

**Recommended Tools**:
- ImageMagick (compare command)
- Pixelmatch
- Resemble.js

---

## Troubleshooting

### Screenshots Not Captured

**Problem**: `CaptureScreenshot` returns false

**Solutions**:
1. Check if screenshot capture is enabled: `UScreenshotHelper::IsEnabled()`
2. Verify output directory is writable
3. Check if max screenshots limit is reached
4. Ensure viewport is valid (not null)

### No Viewport in Headless Mode

**Problem**: Screenshots fail in headless CI

**Solutions**:
1. Use `-NullRHI` flag for CI tests
2. Screenshots won't work in complete headless mode
3. Consider alternative: Log-based validation

### Large Artifact Sizes

**Problem**: CI artifacts are too large

**Solutions**:
1. Reduce `MaxScreenshotsPerTest`
2. Capture only on failure: `bCaptureOnTestSuccess = false`
3. Use lower resolution captures
4. Compress artifacts before upload

### Missing Metadata

**Problem**: Custom metadata not saved

**Solutions**:
1. Verify metadata map is properly constructed
2. Check JSON serialization in manifest file
3. Ensure strings don't contain invalid JSON characters

---

## Future Enhancements

### Planned Features

- [ ] Screenshot comparison (visual diff)
- [ ] Automatic baseline management
- [ ] Region-of-interest capture
- [ ] Multi-viewport capture
- [ ] Video recording support
- [ ] Cloud storage integration

### Community Requests

Submit feature requests via GitHub Issues with label `feature: screenshot-system`.

---

## API Reference

- [C++ API Documentation](../API.md#screenshot-helper)
- [Python API Documentation](PythonScripting.md#screenshot-system)
- [Blueprint API Documentation](../README.md#blueprint-usage)

---

## Examples

### Complete Test with Screenshots

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNavigationVisualTest,
    "YesUeFsd.Visual.NavigationTest",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNavigationVisualTest::RunTest(const FString& Parameters)
{
    // Configure screenshot system
    FScreenshotCaptureConfig Config;
    Config.OutputDirectory = TEXT("Saved/Screenshots/NavigationTests");
    UScreenshotHelper::Configure(Config);

    // Setup
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    UScreenshotHelper::CaptureScreenshot(TEXT("NavigationVisualTest"), TEXT("Setup"));

    // Create actor and driver
    AActor* TestActor = World->SpawnActor<AActor>();
    UAutoDriverComponent* Driver = NewObject<UAutoDriverComponent>(TestActor);
    Driver->RegisterComponent();

    // Capture initial state
    TMap<FString, FString> InitialMetadata;
    InitialMetadata.Add(TEXT("state"), TEXT("initial"));
    UScreenshotHelper::CaptureScreenshot(TEXT("NavigationVisualTest"), TEXT("InitialState"), InitialMetadata);

    // Execute navigation
    FAutoDriverMoveParams MoveParams;
    MoveParams.TargetLocation = FVector(1000, 0, 100);
    Driver->MoveToLocation(MoveParams);

    // Capture during movement
    UScreenshotHelper::CaptureScreenshot(TEXT("NavigationVisualTest"), TEXT("DuringMovement"));

    // Wait for completion
    // ... wait logic ...

    // Capture final state
    TMap<FString, FString> FinalMetadata;
    FinalMetadata.Add(TEXT("state"), TEXT("final"));
    UScreenshotHelper::CaptureScreenshot(TEXT("NavigationVisualTest"), TEXT("FinalState"), FinalMetadata);

    // Generate reports
    UScreenshotHelper::GenerateManifest();
    UScreenshotHelper::GenerateHTMLReport();

    return true;
}
```

---

**Document Version**: 1.0
**Last Updated**: 2026-01-30
**Status**: Production Ready
