# Yes UE FSD - Usage Guide

Comprehensive guide for using the Auto Player Driver plugin.

## Table of Contents

1. [Getting Started](#getting-started)
2. [AutoDriverComponent](#autodrivercomponent)
3. [AutoDriverSubsystem](#autodriversubsystem)
4. [Command System](#command-system)
5. [Input Simulation](#input-simulation)
6. [Common Patterns](#common-patterns)
7. [Troubleshooting](#troubleshooting)

## Getting Started

### Adding Auto Driver to Your Project

#### Method 1: Add to PlayerController (Recommended)

In your custom PlayerController class:

```cpp
// MyPlayerController.h
#include "AutoDriver/AutoDriverComponent.h"

UCLASS()
class AMyPlayerController : public APlayerController
{
    GENERATED_BODY()

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Auto Driver")
    UAutoDriverComponent* AutoDriver;

    virtual void BeginPlay() override;
};

// MyPlayerController.cpp
void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Create and register auto driver
    AutoDriver = NewObject<UAutoDriverComponent>(this);
    AutoDriver->RegisterComponent();
}
```

#### Method 2: Add via Subsystem (Dynamic)

```cpp
// Get or create auto driver at runtime
UAutoDriverSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UAutoDriverSubsystem>();
UAutoDriverComponent* AutoDriver = Subsystem->CreateAutoDriverForController(GetPlayerController());
```

#### Method 3: Blueprint (Designer-Friendly)

1. Open your PlayerController Blueprint
2. Add Component → Search "Auto Driver"
3. Add **AutoDriverComponent**
4. Use Blueprint nodes to control automation

## AutoDriverComponent

The core component that provides automation functionality.

### Basic Movement

#### Move to Location

```cpp
// Simple move
FAutoDriverMoveParams MoveParams;
MoveParams.TargetLocation = FVector(1000, 0, 100);
MoveParams.AcceptanceRadius = 50.0f;
AutoDriver->MoveToLocation(MoveParams);

// Advanced move with options
FAutoDriverMoveParams AdvancedParams;
AdvancedParams.TargetLocation = GetTargetLocation();
AdvancedParams.AcceptanceRadius = 100.0f;
AdvancedParams.SpeedMultiplier = 1.5f;
AdvancedParams.bShouldSprint = true;
AdvancedParams.MovementMode = EAutoDriverMovementMode::Navigation;
AutoDriver->MoveToLocation(AdvancedParams);
```

#### Move to Actor

```cpp
// Move to a specific actor
AActor* TargetActor = FindTargetActor();
AutoDriver->MoveToActor(TargetActor, 100.0f);
```

#### Stop Movement

```cpp
// Cancel current movement
AutoDriver->StopMovement();
```

### Camera Control

#### Rotate to Rotation

```cpp
FAutoDriverRotateParams RotateParams;
RotateParams.TargetRotation = FRotator(0, 90, 0);  // Face east
RotateParams.RotationSpeed = 180.0f;  // 180 degrees per second
RotateParams.AcceptanceAngle = 5.0f;
AutoDriver->RotateToRotation(RotateParams);
```

#### Look at Location

```cpp
// Look at a specific point
FVector TargetPoint = FVector(1000, 500, 200);
AutoDriver->LookAtLocation(TargetPoint, 360.0f);  // Fast rotation
```

#### Look at Actor

```cpp
// Track an actor with the camera
AActor* Enemy = GetNearestEnemy();
AutoDriver->LookAtActor(Enemy, 90.0f);
```

### Input Actions

#### Button Press

```cpp
// Single press
AutoDriver->PressButton("Fire");

// Hold for duration
AutoDriver->PressButton("Crouch", 2.0f);  // Hold crouch for 2 seconds
```

#### Axis Input

```cpp
// Set axis value
AutoDriver->SetAxisValue("Throttle", 0.8f, 5.0f);  // 80% throttle for 5 seconds
```

### Command Management

#### Listen for Completion

```cpp
// Bind to completion delegate
AutoDriver->OnCommandComplete.AddDynamic(this, &AMyController::OnAutoCommandComplete);

void AMyController::OnAutoCommandComplete(bool bSuccess, const FString& Message)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Command completed: %s"), *Message);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Command failed: %s"), *Message);
    }
}
```

#### Check Status

```cpp
if (AutoDriver->IsExecutingCommand())
{
    UE_LOG(LogTemp, Log, TEXT("Automation in progress..."));
}
```

#### Cancel Command

```cpp
AutoDriver->StopCurrentCommand();
```

### Enable/Disable

```cpp
// Disable automation
AutoDriver->SetEnabled(false);

// Re-enable
AutoDriver->SetEnabled(true);

// Check status
if (AutoDriver->IsEnabled())
{
    // Automation is active
}
```

## AutoDriverSubsystem

Global management of all auto drivers in the game instance.

### Getting Auto Drivers

```cpp
UAutoDriverSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UAutoDriverSubsystem>();

// Get driver for first player
UAutoDriverComponent* Driver = Subsystem->GetAutoDriverForPlayer(0);

// Get driver for specific controller
UAutoDriverComponent* Driver = Subsystem->GetAutoDriverForController(MyController);

// Get all drivers
TArray<UAutoDriverComponent*> AllDrivers = Subsystem->GetAllAutoDrivers();
```

### Creating/Removing Drivers

```cpp
// Create driver for controller
UAutoDriverComponent* NewDriver = Subsystem->CreateAutoDriverForController(MyController);

// Remove driver
Subsystem->RemoveAutoDriverFromController(MyController);
```

### Global Control

```cpp
// Disable all automation
Subsystem->SetAllAutoDriversEnabled(false);

// Stop all commands
Subsystem->StopAllCommands();

// Auto-create for new players
Subsystem->SetAutoCreateForNewPlayers(true);
```

### Statistics

```cpp
// Get active count
int32 ActiveCount = Subsystem->GetActiveAutoDriverCount();

// Get total commands executed
int64 TotalCommands = Subsystem->GetTotalCommandsExecuted();
```

## Command System

The command system provides a pluggable architecture for automation actions.

### Built-in Commands

#### MoveToLocationCommand

```cpp
UMoveToLocationCommand* MoveCmd = UMoveToLocationCommand::CreateMoveToLocationCommand(
    this,
    FVector(1000, 0, 100),
    50.0f  // Acceptance radius
);

// Configure
MoveCmd->SpeedMultiplier = 1.5f;
MoveCmd->bShouldSprint = true;
MoveCmd->Timeout = 30.0f;

// Execute (via component)
// AutoDriver->ExecuteCommand(MoveCmd);  // Not yet fully implemented
```

#### RotateToCommand

```cpp
URotateToCommand* RotateCmd = URotateToCommand::CreateRotateToRotation(
    this,
    FRotator(0, 90, 0),
    180.0f  // Rotation speed
);

// Or create look-at command
URotateToCommand* LookAtCmd = URotateToCommand::CreateLookAtLocation(
    this,
    TargetLocation,
    360.0f
);
```

### Creating Custom Commands

Implement `IAutoDriverCommand` interface:

```cpp
// MyCustomCommand.h
UCLASS(BlueprintType)
class UMyCustomCommand : public UObject, public IAutoDriverCommand
{
    GENERATED_BODY()

public:
    // Command parameters
    UPROPERTY(BlueprintReadWrite, Category = "Command")
    float Duration = 5.0f;

    // IAutoDriverCommand interface
    virtual void Initialize_Implementation(UObject* InContext) override;
    virtual bool Execute_Implementation() override;
    virtual void Tick_Implementation(float DeltaTime) override;
    virtual void Cancel_Implementation() override;
    virtual bool IsRunning_Implementation() const override;
    virtual FAutoDriverCommandResult GetResult_Implementation() const override;
    virtual FString GetDescription_Implementation() const override;

protected:
    bool bIsRunning = false;
    float TimeElapsed = 0.0f;
    FAutoDriverCommandResult Result;
};

// MyCustomCommand.cpp
void UMyCustomCommand::Initialize_Implementation(UObject* InContext)
{
    // Store context (usually a PlayerController)
}

bool UMyCustomCommand::Execute_Implementation()
{
    bIsRunning = true;
    TimeElapsed = 0.0f;
    Result = FAutoDriverCommandResult(EAutoDriverCommandStatus::Running, TEXT("Executing"));
    return true;
}

void UMyCustomCommand::Tick_Implementation(float DeltaTime)
{
    if (!bIsRunning) return;

    TimeElapsed += DeltaTime;

    // Your command logic here

    if (TimeElapsed >= Duration)
    {
        Result = FAutoDriverCommandResult(EAutoDriverCommandStatus::Success, TEXT("Completed"));
        bIsRunning = false;
    }
}

void UMyCustomCommand::Cancel_Implementation()
{
    bIsRunning = false;
    Result = FAutoDriverCommandResult(EAutoDriverCommandStatus::Cancelled, TEXT("Cancelled"));
}

bool UMyCustomCommand::IsRunning_Implementation() const
{
    return bIsRunning;
}

FAutoDriverCommandResult UMyCustomCommand::GetResult_Implementation() const
{
    return Result;
}

FString UMyCustomCommand::GetDescription_Implementation() const
{
    return FString::Printf(TEXT("Custom Command (Duration: %.1fs)"), Duration);
}
```

## Input Simulation

Direct input injection for fine-grained control.

### Basic Input

```cpp
// Create simulator
UInputSimulator* InputSim = UInputSimulator::CreateInputSimulator(this, MyController);

// Movement
InputSim->SetMoveForward(1.0f);   // Full forward
InputSim->SetMoveRight(0.5f);     // Half right

// Camera
InputSim->SetLookUp(-0.3f);       // Look up
InputSim->SetLookRight(1.0f);     // Turn right

// Actions
InputSim->Jump();
InputSim->PressButton("Fire");
InputSim->PressAndHoldButton("Aim", 3.0f);  // Hold aim for 3 seconds
```

### Advanced Input

```cpp
// Custom axis
InputSim->SetAxisValue("Throttle", 0.8f);
InputSim->SetAxisValue("Brake", 0.0f);

// 2D axis (e.g., gamepad stick)
InputSim->SetAxis2DValue("MoveStick", FVector2D(0.7f, 0.5f));

// Clear all input
InputSim->ClearAllInput();
```

## Common Patterns

### Patrol Route

```cpp
void AMyController::StartPatrol()
{
    TArray<FVector> PatrolPoints = {
        FVector(1000, 0, 100),
        FVector(1000, 1000, 100),
        FVector(0, 1000, 100),
        FVector(0, 0, 100)
    };

    CurrentPatrolIndex = 0;
    MoveToNextPatrolPoint();

    // Bind to completion
    AutoDriver->OnCommandComplete.AddDynamic(this, &AMyController::OnPatrolPointReached);
}

void AMyController::OnPatrolPointReached(bool bSuccess, const FString& Message)
{
    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    MoveToNextPatrolPoint();
}

void AMyController::MoveToNextPatrolPoint()
{
    FAutoDriverMoveParams Params;
    Params.TargetLocation = PatrolPoints[CurrentPatrolIndex];
    Params.AcceptanceRadius = 100.0f;
    AutoDriver->MoveToLocation(Params);
}
```

### Sequence of Actions

```cpp
void AMyController::ExecuteSequence()
{
    SequenceIndex = 0;
    ExecuteNextAction();

    AutoDriver->OnCommandComplete.AddDynamic(this, &AMyController::OnSequenceStepComplete);
}

void AMyController::OnSequenceStepComplete(bool bSuccess, const FString& Message)
{
    if (bSuccess)
    {
        SequenceIndex++;
        ExecuteNextAction();
    }
}

void AMyController::ExecuteNextAction()
{
    switch (SequenceIndex)
    {
        case 0:
            AutoDriver->MoveToLocation(GetMoveParams(FVector(1000, 0, 100)));
            break;
        case 1:
            AutoDriver->LookAtLocation(FVector(2000, 500, 200));
            break;
        case 2:
            AutoDriver->PressButton("Interact", 1.0f);
            break;
        default:
            UE_LOG(LogTemp, Log, TEXT("Sequence complete!"));
            break;
    }
}
```

### Automated Combat Test

```cpp
void ATestController::RunCombatTest()
{
    // Find enemies
    TArray<AActor*> Enemies = FindAllEnemies();

    for (AActor* Enemy : Enemies)
    {
        // Move to enemy
        AutoDriver->MoveToActor(Enemy, 500.0f);

        // Look at enemy
        AutoDriver->LookAtActor(Enemy);

        // Fire
        AutoDriver->PressButton("Fire", 2.0f);

        // Wait for completion before next enemy
    }
}
```

## Troubleshooting

### Command Not Executing

**Problem:** Command starts but doesn't complete

**Solutions:**
- Check `AcceptanceRadius` isn't too small
- Verify target location is reachable
- Check for `Timeout` expiration
- Enable debug visualization: `AutoDriver->bShowDebugInfo = true`

### Input Not Working

**Problem:** Input simulation has no effect

**Solutions:**
- Ensure `InputSimulator` is initialized with valid controller
- Verify player controller has a valid pawn
- Check input action names match your input configuration
- Note: Enhanced Input System integration is not yet fully implemented

### Component Not Found

**Problem:** `GetAutoDriverForPlayer()` returns nullptr

**Solutions:**
- Ensure component was created and registered
- Try `CreateAutoDriverForController()` instead
- Check component is attached to correct actor (controller or pawn)

### Performance Issues

**Problem:** Automation causes frame rate drops

**Solutions:**
- Reduce tick frequency of custom commands
- Use navigation system instead of direct movement
- Limit number of concurrent auto drivers
- Profile with Unreal Insights to identify bottlenecks

---

**Need Help?** File an issue on the GitHub repository or check the API reference in README.md
