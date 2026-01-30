# Yes UE FSD - Auto Player Driver Plugin

**Version:** 0.1.0
**Target Engine:** Unreal Engine 5.6+
**Status:** Early Development (Beta)

## Overview

**Yes UE FSD** (Full Self-Driving) is an Unreal Engine plugin that enables automatic player control and gameplay automation. This plugin is designed for automated testing, AI-driven gameplay, procedural navigation, and autonomous player behavior.

## Features

### Current Features (v0.1.0)

- ✅ Basic plugin structure with runtime and editor modules
- ✅ HTTP server for remote control (default port: 8081)
- ✅ Configuration system via `DefaultYesUeFsd.ini`
- ✅ Modular architecture based on yes-ue-mcp reference
- ✅ **AutoDriverComponent** - Attach to player controllers for automation
- ✅ **AutoDriverSubsystem** - Global automation management
- ✅ **Command System** - Pluggable command architecture
- ✅ **Movement Commands** - MoveToLocation, RotateTo, LookAt
- ✅ **Input Simulation** - Basic input injection system

### Planned Features

- 🔲 **Automatic Navigation**: AI-driven pathfinding and movement
- 🔲 **Input Simulation**: Programmatic control of player inputs (keyboard, mouse, gamepad)
- 🔲 **Action Recording**: Record player actions for playback
- 🔲 **Action Playback**: Replay recorded action sequences
- 🔲 **Remote Control API**: HTTP/JSON-RPC interface for external control
- 🔲 **Behavior Trees Integration**: Use UE's behavior tree system for complex behaviors
- 🔲 **PIE Integration**: Seamless Play-In-Editor automation
- 🔲 **Python Scripting**: Optional Python API for test automation

## Use Cases

- **Automated Testing**: Run gameplay tests without manual input
- **AI Demonstrations**: Showcase AI-driven player behavior
- **Procedural Gameplay**: Create autonomous player experiences
- **Quality Assurance**: Stress test levels and gameplay systems
- **Game Balancing**: Gather automated gameplay data for analysis

## Installation

### As a Plugin

1. Copy the `YesUeFsd` folder to your project's `Plugins/` directory
2. Regenerate project files (right-click `.uproject` → Generate Visual Studio project files)
3. Rebuild your project
4. Enable the plugin in Edit → Plugins → Automation → Yes UE FSD

### As an Engine Plugin

1. Copy the `YesUeFsd` folder to `[EngineDir]/Engine/Plugins/`
2. Regenerate engine project files
3. Rebuild the engine
4. Plugin will be available in all projects

## Configuration

Edit `Config/DefaultYesUeFsd.ini` to customize settings:

```ini
[/Script/YesUeFsdEditor.AutoDriverSettings]
ServerPort=8081                  ; HTTP server port
bAutoStartServer=true            ; Auto-start server on editor launch
BindAddress=127.0.0.1           ; Localhost for security
LogLevel=Log                     ; Logging verbosity
```

## Project Structure

```
YesUeFsd/
├── Config/                      # Plugin configuration
│   └── DefaultYesUeFsd.ini
├── Resources/                   # Icons and assets
├── Source/
│   ├── YesUeFsd/               # Runtime module (core logic)
│   │   ├── Public/
│   │   │   └── YesUeFsd.h
│   │   ├── Private/
│   │   │   └── YesUeFsd.cpp
│   │   └── YesUeFsd.Build.cs
│   └── YesUeFsdEditor/         # Editor module (UI, HTTP server)
│       ├── Public/
│       │   └── YesUeFsdEditor.h
│       ├── Private/
│       │   └── YesUeFsdEditor.cpp
│       └── YesUeFsdEditor.Build.cs
├── YesUeFsd.uplugin            # Plugin manifest
└── README.md
```

## Development

### Building the Plugin

1. Open your UE project in Visual Studio
2. Build in Development Editor configuration
3. Launch the editor
4. Check the Output Log for "YesUeFsd Runtime Module Started"

### Adding Features

The plugin is designed with modularity in mind:

- **Runtime Module (`YesUeFsd`)**: Add gameplay-accessible features here
- **Editor Module (`YesUeFsdEditor`)**: Add editor-only tools and UI here

### Dependencies

**Runtime Module:**
- Core, CoreUObject, Engine
- InputCore, EnhancedInput
- AIModule, NavigationSystem

**Editor Module:**
- HTTP, HTTPServer (for remote control)
- UnrealEd, EditorSubsystem
- ToolMenus, Slate, SlateCore

## Quick Start

### C++ Usage

```cpp
// Add AutoDriverComponent to your PlayerController
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Auto Driver")
UAutoDriverComponent* AutoDriver;

void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Create auto driver component
    AutoDriver = NewObject<UAutoDriverComponent>(this);
    AutoDriver->RegisterComponent();

    // Move to a location
    FAutoDriverMoveParams MoveParams;
    MoveParams.TargetLocation = FVector(1000, 0, 100);
    MoveParams.AcceptanceRadius = 50.0f;
    AutoDriver->MoveToLocation(MoveParams);

    // Look at a target
    AutoDriver->LookAtLocation(FVector(2000, 500, 200));
}
```

### Blueprint Usage

1. Add **AutoDriverComponent** to your PlayerController Blueprint
2. Use the following nodes:
   - `Move To Location` - Move to a target position
   - `Move To Actor` - Move to a target actor
   - `Look At Location` - Rotate camera to look at a position
   - `Look At Actor` - Look at a target actor
   - `Press Button` - Simulate button press
   - `Set Axis Value` - Simulate axis input

### Using the Subsystem

```cpp
// Get the subsystem
UAutoDriverSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UAutoDriverSubsystem>();

// Get auto driver for first player
UAutoDriverComponent* Driver = Subsystem->GetAutoDriverForPlayer(0);

// Or create one for a specific controller
Driver = Subsystem->CreateAutoDriverForController(MyPlayerController);

// Enable/disable all auto drivers
Subsystem->SetAllAutoDriversEnabled(false);
```

### Input Simulation

```cpp
// Create input simulator
UInputSimulator* InputSim = UInputSimulator::CreateInputSimulator(this, MyPlayerController);

// Simulate movement
InputSim->SetMoveForward(1.0f);  // Move forward
InputSim->SetMoveRight(0.5f);    // Strafe right

// Simulate looking
InputSim->SetLookUp(-0.5f);      // Look up
InputSim->SetLookRight(1.0f);    // Turn right

// Simulate jump
InputSim->Jump();

// Press and hold a button
InputSim->PressAndHoldButton("Fire", 2.0f);  // Hold for 2 seconds

// Clear all input
InputSim->ClearAllInput();
```

## API Reference

### AutoDriverComponent

Main component for controlling automation. Attach to PlayerController or Pawn.

**Key Methods:**
- `MoveToLocation(FAutoDriverMoveParams)` - Move to target location
- `MoveToActor(AActor*, float)` - Move to target actor
- `RotateToRotation(FAutoDriverRotateParams)` - Rotate to target rotation
- `LookAtLocation(FVector, float)` - Look at target location
- `LookAtActor(AActor*, float)` - Look at target actor
- `PressButton(FName, float)` - Simulate button press
- `SetAxisValue(FName, float, float)` - Simulate axis input
- `StopCurrentCommand()` - Cancel current command
- `SetEnabled(bool)` - Enable/disable automation

**Delegates:**
- `OnCommandComplete` - Fired when a command completes

### AutoDriverSubsystem

Global subsystem for managing all auto drivers.

**Key Methods:**
- `GetAutoDriverForPlayer(int32)` - Get driver for player index
- `GetAutoDriverForController(APlayerController*)` - Get driver for controller
- `CreateAutoDriverForController(APlayerController*)` - Create new driver
- `RemoveAutoDriverFromController(APlayerController*)` - Remove driver
- `GetAllAutoDrivers()` - Get all active drivers
- `SetAllAutoDriversEnabled(bool)` - Enable/disable all drivers
- `StopAllCommands()` - Stop all active commands
- `SetAutoCreateForNewPlayers(bool)` - Auto-create for new players
- `GetActiveAutoDriverCount()` - Get count of active drivers

### Command System

Create custom commands by implementing `IAutoDriverCommand`.

**Built-in Commands:**
- `UMoveToLocationCommand` - Navigate to a target location
- `URotateToCommand` - Rotate to a target rotation

**Creating Custom Commands:**

```cpp
UCLASS()
class UMyCustomCommand : public UObject, public IAutoDriverCommand
{
    GENERATED_BODY()

    virtual bool Execute_Implementation() override
    {
        // Your command logic
        return true;
    }

    virtual void Tick_Implementation(float DeltaTime) override
    {
        // Update command state
    }

    virtual bool IsRunning_Implementation() const override
    {
        return bIsRunning;
    }
};
```

### InputSimulator

Simulates player input programmatically.

**Key Methods:**
- `Initialize(APlayerController*)` - Initialize simulator
- `PressButton(FName)` - Press a button
- `ReleaseButton(FName)` - Release a button
- `PressAndHoldButton(FName, float)` - Press and hold
- `SetAxisValue(FName, float)` - Set axis value
- `SetMoveForward(float)` - Simulate forward movement
- `SetMoveRight(float)` - Simulate right movement
- `SetLookUp(float)` - Simulate look up/down
- `SetLookRight(float)` - Simulate look left/right
- `Jump()` - Simulate jump
- `ClearAllInput()` - Clear all input

## Roadmap

### Phase 1: Foundation (Current)
- ✅ Basic plugin structure
- ✅ Configuration system
- ✅ HTTP server setup

### Phase 2: Core Automation (In Progress)
- ✅ Input simulation system
- ✅ Basic movement commands
- 🔲 Advanced navigation integration (AI pathfinding)
- 🔲 Action recording/playback
- 🔲 Enhanced Input System integration

### Phase 3: Advanced Features
- 🔲 Behavior tree integration
- 🔲 Remote control API
- 🔲 Python scripting support

### Phase 4: Production Ready
- 🔲 Comprehensive testing
- 🔲 Performance optimization
- 🔲 Documentation and examples

## Contributing

This is an early-stage project. Contributions, feedback, and suggestions are welcome!

## License

*License information to be added*

## Credits

Based on the architectural patterns from [yes-ue-mcp](https://github.com/softdaddy-o/yes-ue-mcp).

---

**Status:** 🚧 Early Development - Not production ready
**Last Updated:** 2026-01-30
