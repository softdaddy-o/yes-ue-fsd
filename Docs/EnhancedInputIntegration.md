# Enhanced Input System Integration

## Overview

The AutoDriver system now supports Unreal Engine 5's Enhanced Input System, providing modern input handling, better flexibility, and comprehensive recording/playback capabilities.

## Features

- **Dual Input Support**: Works with both legacy input and Enhanced Input System
- **Automatic Detection**: Automatically detects and uses the appropriate input system
- **Action Mapping**: Maps legacy action names to Enhanced Input actions
- **Context Management**: Full support for Input Mapping Contexts with priority handling
- **Recording & Playback**: Record Enhanced Input events and export to JSON
- **Trigger Support**: Works with all Enhanced Input trigger types
- **Modifier Support**: Compatible with Enhanced Input modifiers

## Architecture

### Components

1. **UEnhancedInputAdapter**: Core adapter that bridges automation and Enhanced Input
2. **UInputSimulator**: Updated to support both legacy and Enhanced Input modes
3. **FEnhancedInputActionMapping**: Maps legacy action names to UInputAction assets
4. **FInputActionRecord**: Records input events for playback

## Usage

### Basic Setup

```cpp
// Get your player controller
APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

// Create input simulator (auto-detects Enhanced Input)
UInputSimulator* Simulator = UInputSimulator::CreateInputSimulator(this, PlayerController);

// Check if using Enhanced Input
if (Simulator->IsUsingEnhancedInput())
{
    UE_LOG(LogTemp, Log, TEXT("Using Enhanced Input System"));
}
```

### Registering Actions

```cpp
// Get the Enhanced Input Adapter
UEnhancedInputAdapter* Adapter = Simulator->GetEnhancedInputAdapter();

// Register action mappings
FEnhancedInputActionMapping JumpMapping;
JumpMapping.ActionName = "Jump";
JumpMapping.InputAction = JumpInputAction; // Your UInputAction asset
JumpMapping.MappingContext = DefaultMappingContext;
JumpMapping.Priority = 0;

Adapter->RegisterActionMapping(JumpMapping);

// Or register multiple at once
TArray<FEnhancedInputActionMapping> Mappings;
// ... populate mappings
Adapter->RegisterActionMappings(Mappings);
```

### Injecting Input

```cpp
// Press a button
Simulator->PressButton("Jump");

// Release a button
Simulator->ReleaseButton("Jump");

// Press and hold
Simulator->PressAndHoldButton("Jump", 0.5f); // Hold for 0.5 seconds

// Set axis value
Simulator->SetAxisValue("MoveForward", 1.0f);

// Set 2D axis (e.g., movement)
Simulator->SetAxis2DValue("Move", FVector2D(1.0f, 0.5f));
```

### Managing Input Contexts

```cpp
UEnhancedInputAdapter* Adapter = Simulator->GetEnhancedInputAdapter();

// Add a mapping context
Adapter->AddMappingContext(GameplayContext, 0);

// Add with higher priority
Adapter->AddMappingContext(UIContext, 10);

// Remove a context
Adapter->RemoveMappingContext(GameplayContext);

// Clear all contexts
Adapter->ClearAllMappingContexts();

// Get active contexts
TArray<UInputMappingContext*> ActiveContexts = Adapter->GetActiveMappingContexts();
```

### Recording Input

```cpp
UEnhancedInputAdapter* Adapter = Simulator->GetEnhancedInputAdapter();

// Start recording
Adapter->StartRecording();

// ... perform actions ...

// Stop recording
Adapter->StopRecording();

// Get recorded actions
const TArray<FInputActionRecord>& RecordedActions = Adapter->GetRecordedActions();

// Export to JSON
FString JSONString = Adapter->ExportRecordingToJSON();

// Save to file
FFileHelper::SaveStringToFile(JSONString, TEXT("RecordedInput.json"));

// Import from JSON
FString LoadedJSON;
FFileHelper::LoadFileToString(LoadedJSON, TEXT("RecordedInput.json"));
Adapter->ImportRecordingFromJSON(LoadedJSON);
```

### Blueprint Usage

All functionality is exposed to Blueprints:

```
Create Input Simulator
  -> Get Enhanced Input Adapter
    -> Register Action Mapping
    -> Add Mapping Context
    -> Start Recording
    -> Inject Button Press
    -> Stop Recording
    -> Export Recording to JSON
```

## Advanced Features

### Custom Input Mode

```cpp
// Force legacy mode
Simulator->Initialize(PlayerController, EInputSimulatorMode::Legacy);

// Force Enhanced Input mode
Simulator->Initialize(PlayerController, EInputSimulatorMode::EnhancedInput);

// Auto-detect (default)
Simulator->Initialize(PlayerController, EInputSimulatorMode::Auto);
```

### Direct Enhanced Input Injection

```cpp
UEnhancedInputAdapter* Adapter = Simulator->GetEnhancedInputAdapter();

// Inject with custom value
FInputActionValue Value(1.0f);
Adapter->InjectInputAction("MyAction", Value);

// Inject 2D value
Adapter->InjectAxis2DValue("Move", FVector2D(0.5f, 0.8f));

// Inject 3D value (e.g., for 3D motion)
Adapter->InjectAxis3DValue("Motion3D", FVector(1.0f, 0.5f, 0.2f));
```

### Recording Analysis

```cpp
const TArray<FInputActionRecord>& Records = Adapter->GetRecordedActions();

for (const FInputActionRecord& Record : Records)
{
    UE_LOG(LogTemp, Log, TEXT("Time: %.2f, Action: %s, Triggered: %d"),
        Record.Timestamp,
        *Record.ActionName.ToString(),
        Record.bTriggered);

    // Check value type
    switch (Record.ActionValue.GetValueType())
    {
    case EInputActionValueType::Boolean:
        bool BoolValue = Record.ActionValue.Get<bool>();
        break;
    case EInputActionValueType::Axis1D:
        float FloatValue = Record.ActionValue.Get<float>();
        break;
    case EInputActionValueType::Axis2D:
        FVector2D Vec2D = Record.ActionValue.Get<FVector2D>();
        break;
    case EInputActionValueType::Axis3D:
        FVector Vec3D = Record.ActionValue.Get<FVector>();
        break;
    }
}
```

## Integration with Existing Code

The Enhanced Input integration is backward compatible:

### Before (Legacy)
```cpp
UInputSimulator* Simulator = UInputSimulator::CreateInputSimulator(this, PC);
Simulator->PressButton("Jump");
```

### After (Enhanced Input, same API)
```cpp
UInputSimulator* Simulator = UInputSimulator::CreateInputSimulator(this, PC);
Simulator->PressButton("Jump"); // Works with Enhanced Input automatically!
```

The simulator automatically:
1. Detects if Enhanced Input is available
2. Creates the adapter if needed
3. Falls back to legacy behavior if Enhanced Input injection fails

## Configuration

### Setting Up Input Actions

1. Create Input Action assets in your Content Browser
2. Create Input Mapping Contexts
3. Register them with the adapter:

```cpp
void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Get simulator from AutoDriver
    UAutoDriverComponent* AutoDriver = FindComponentByClass<UAutoDriverComponent>();
    UInputSimulator* Simulator = AutoDriver->GetInputSimulator();

    if (UEnhancedInputAdapter* Adapter = Simulator->GetEnhancedInputAdapter())
    {
        // Register common actions
        RegisterCommonActions(Adapter);
    }
}

void AMyPlayerController::RegisterCommonActions(UEnhancedInputAdapter* Adapter)
{
    TArray<FEnhancedInputActionMapping> Mappings;

    // Jump
    FEnhancedInputActionMapping& Jump = Mappings.AddDefaulted_GetRef();
    Jump.ActionName = "Jump";
    Jump.InputAction = IA_Jump;
    Jump.MappingContext = IMC_Default;

    // Move
    FEnhancedInputActionMapping& Move = Mappings.AddDefaulted_GetRef();
    Move.ActionName = "Move";
    Move.InputAction = IA_Move;
    Move.MappingContext = IMC_Default;

    // Look
    FEnhancedInputActionMapping& Look = Mappings.AddDefaulted_GetRef();
    Look.ActionName = "Look";
    Look.InputAction = IA_Look;
    Look.MappingContext = IMC_Default;

    Adapter->RegisterActionMappings(Mappings);
}
```

## Testing

### Manual Testing

```cpp
void TestEnhancedInput()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    UInputSimulator* Simulator = UInputSimulator::CreateInputSimulator(this, PC);

    // Verify Enhanced Input is active
    check(Simulator->IsUsingEnhancedInput());

    UEnhancedInputAdapter* Adapter = Simulator->GetEnhancedInputAdapter();
    check(Adapter->IsInitialized());

    // Test button press
    Simulator->PressButton("Jump");
    // Verify character jumped

    // Test axis
    Simulator->SetAxisValue("MoveForward", 1.0f);
    // Verify character moves forward
}
```

### Recording Testing

```cpp
void TestRecording()
{
    UEnhancedInputAdapter* Adapter = GetAdapter();

    Adapter->StartRecording();

    // Perform test actions
    SimulateTestSequence();

    Adapter->StopRecording();

    // Export and verify
    FString JSON = Adapter->ExportRecordingToJSON();
    check(!JSON.IsEmpty());

    // Clear and reimport
    Adapter->ClearRecordedActions();
    check(Adapter->ImportRecordingFromJSON(JSON));
}
```

## Troubleshooting

### Enhanced Input Not Detected

**Problem**: Simulator uses legacy mode even though Enhanced Input is set up.

**Solution**: Ensure your PlayerController has UEnhancedInputComponent:

```cpp
void AMyPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // Make sure we're using Enhanced Input Component
    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
    {
        // Setup your bindings
    }
}
```

### Action Not Found

**Problem**: `InjectInputAction` returns false.

**Solution**: Register the action mapping first:

```cpp
UEnhancedInputAdapter* Adapter = Simulator->GetEnhancedInputAdapter();

FEnhancedInputActionMapping Mapping;
Mapping.ActionName = "MyAction";
Mapping.InputAction = MyInputAction; // Must not be null!
Adapter->RegisterActionMapping(Mapping);
```

### Recording Not Working

**Problem**: Recording captures no events.

**Solution**:
1. Ensure actions are registered before starting recording
2. Verify Enhanced Input Component is available
3. Check that mapping contexts are active

```cpp
// Register actions first
Adapter->RegisterActionMappings(MyMappings);

// Add mapping context
Adapter->AddMappingContext(MyContext, 0);

// Then start recording
Adapter->StartRecording();
```

## Performance Considerations

- **Input Injection**: Minimal overhead, uses UE's native injection
- **Recording**: Stores events in memory, consider clearing periodically
- **Context Switching**: Low cost, managed by Enhanced Input subsystem
- **JSON Export**: Can be expensive for large recordings, do asynchronously

## Future Enhancements

Potential additions:
- Playback system for recorded input
- Timeline editing for recorded sequences
- Compression for large recordings
- Network replication support
- Visual debugging tools

## API Reference

### UEnhancedInputAdapter

| Method | Description |
|--------|-------------|
| `Initialize(PlayerController)` | Initialize adapter for a player |
| `RegisterActionMapping(Mapping)` | Register single action mapping |
| `RegisterActionMappings(Mappings)` | Register multiple mappings |
| `FindInputAction(ActionName)` | Find UInputAction by name |
| `AddMappingContext(Context, Priority)` | Add Input Mapping Context |
| `RemoveMappingContext(Context)` | Remove context |
| `ClearAllMappingContexts()` | Remove all contexts |
| `InjectInputAction(Name, Value)` | Inject input action |
| `InjectButtonPress(Name)` | Inject button press |
| `InjectButtonRelease(Name)` | Inject button release |
| `InjectAxisValue(Name, Value)` | Inject 1D axis |
| `InjectAxis2DValue(Name, Value)` | Inject 2D axis |
| `InjectAxis3DValue(Name, Value)` | Inject 3D axis |
| `StartRecording()` | Start recording input |
| `StopRecording()` | Stop recording |
| `GetRecordedActions()` | Get recorded events |
| `ExportRecordingToJSON()` | Export to JSON string |
| `ImportRecordingFromJSON(JSON)` | Import from JSON |

### UInputSimulator (Enhanced)

| Method | Description |
|--------|-------------|
| `Initialize(PC, Mode)` | Initialize with input mode |
| `GetInputMode()` | Get current input mode |
| `IsUsingEnhancedInput()` | Check if using Enhanced Input |
| `GetEnhancedInputAdapter()` | Get adapter instance |

## Examples

See the following files for complete examples:
- `Content/Examples/BP_EnhancedInputExample.uasset`
- `Source/Examples/EnhancedInputExample.cpp`
- `Source/Examples/EnhancedInputExample.h`

## See Also

- [AutoDriver Documentation](AutoDriver.md)
- [Input Simulator Documentation](InputSimulator.md)
- [Unreal Engine Enhanced Input Documentation](https://docs.unrealengine.com/5.0/en-US/enhanced-input-in-unreal-engine/)
