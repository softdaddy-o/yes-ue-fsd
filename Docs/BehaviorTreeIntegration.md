// Behavior Tree Integration

## Overview

The Behavior Tree integration enables complex, hierarchical automation behaviors using Unreal's visual scripting system. Create sophisticated AI-like automation with decision trees, conditions, and reusable behavior components.

## Components

### Behavior Tree Tasks

#### 1. BTTask_AutoDriverMove

Moves the pawn to a target location using AutoDriver.

**Properties:**
- **Target Location Key**: Blackboard key with target vector
- **Target Actor Key**: Alternative - blackboard key with target actor
- **Acceptance Radius**: How close to get (default: 50 units)
- **Speed Multiplier**: Movement speed (0.1-5.0x, default: 1.0)
- **Should Sprint**: Enable sprinting
- **Movement Mode**: Navigation, Direct, or InputSimulation
- **Arrival Status Key**: Optional blackboard bool to update on arrival
- **Command Timeout**: Maximum execution time

**Usage:**
```
Sequence
├─ BTTask_AutoDriverMove
│   ├─ Target Location: PatrolPoint
│   ├─ Acceptance Radius: 100
│   └─ Speed Multiplier: 1.5
└─ ... next task
```

#### 2. BTTask_AutoDriverRotate

Rotates the pawn to face a target direction or look at a location/actor.

**Properties:**
- **Target Rotation Key**: Blackboard key with target rotation
- **Look At Location Key**: Alternative - location to look at
- **Look At Actor Key**: Alternative - actor to look at
- **Rotation Speed**: Degrees per second (1-720, default: 180)
- **Acceptance Angle**: Tolerance in degrees (default: 5)
- **Command Timeout**: Maximum execution time

**Usage:**
```
Sequence
├─ BTTask_AutoDriverRotate
│   ├─ Look At Actor: Enemy
│   └─ Rotation Speed: 360
└─ BTTask_AutoDriverInput (Fire)
```

#### 3. BTTask_AutoDriverWait

Waits for a specified duration before continuing.

**Properties:**
- **Wait Duration**: Seconds to wait (default: 1.0)
- **Wait Duration Key**: Optional blackboard float for dynamic duration
- **Random Deviation**: Add randomness ±N seconds

**Usage:**
```
Sequence
├─ BTTask_AutoDriverMove (to cover)
├─ BTTask_AutoDriverWait (2.0 seconds)
└─ BTTask_AutoDriverMove (to next point)
```

#### 4. BTTask_AutoDriverInput

Triggers input actions (button press, hold, etc.).

**Properties:**
- **Input Action Name**: Name of the action to trigger
- **Input Action Key**: Optional blackboard name for dynamic action
- **Input Type**: Press, Hold, or Hold Indefinite
- **Hold Duration**: Seconds to hold (for Hold type)
- **Wait For Completion**: Block until action completes

**Input Types:**
- **Press**: Quick press and release
- **Hold**: Press, hold for duration, release
- **Hold Indefinite**: Press and hold (manual release required)

**Usage:**
```
Sequence
├─ BTTask_AutoDriverRotate (aim at target)
├─ BTTask_AutoDriverInput
│   ├─ Action Name: Fire
│   ├─ Type: Hold
│   └─ Duration: 0.5
└─ BTTask_AutoDriverMove (retreat)
```

### Services

#### BTService_AutoDriverStatus

Continuously monitors AutoDriver status and updates blackboard values.

**Properties:**
- **Is Executing Command Key**: Blackboard bool - updated with execution status
- **Is Location Reachable Key**: Blackboard bool - updated with reachability
- **Target Location Key**: Location to check for reachability
- **Interval**: Update frequency (default: 0.5s)

**Usage:**
```
Selector (with AutoDriverStatus service)
├─ Sequence (if location reachable)
│   ├─ Decorator: IsReachable == true
│   └─ BTTask_AutoDriverMove
└─ BTTask_FindAlternativePath
```

### Decorators

#### BTDecorator_CheckAutoDriver

Conditional decorator for AutoDriver state checks.

**Check Types:**
- **Has AutoDriver**: Verify component exists
- **Is Executing**: Check if command is running
- **Is Reachable**: Check if target location is reachable
- **Within Distance**: Check if within N units of target

**Properties:**
- **Check Type**: Type of condition to evaluate
- **Target Location Key**: Location for distance/reachability checks
- **Target Distance**: Distance threshold for WithinDistance check
- **Invert Condition**: Negate the result

**Usage:**
```
Selector
├─ Sequence
│   ├─ Decorator: Within Distance (100 units)
│   └─ BTTask_AutoDriverInput (Interact)
└─ BTTask_AutoDriverMove (get closer)
```

## Example Behavior Trees

### Example 1: Simple Patrol

```
Sequence (Loop)
├─ BTTask_AutoDriverMove
│   └─ Target: PatrolPoint1
├─ BTTask_AutoDriverWait (2.0s)
├─ BTTask_AutoDriverMove
│   └─ Target: PatrolPoint2
└─ BTTask_AutoDriverWait (2.0s)
```

### Example 2: Combat Behavior

```
Selector
├─ Sequence (Engage enemy)
│   ├─ Decorator: Check AutoDriver
│   │   └─ Within Distance: 1000
│   ├─ BTTask_AutoDriverRotate
│   │   └─ Look At: Enemy
│   ├─ BTTask_AutoDriverInput
│   │   ├─ Action: Fire
│   │   ├─ Type: Hold
│   │   └─ Duration: 1.0
│   └─ BTTask_AutoDriverWait (0.5s)
└─ BTTask_AutoDriverMove (Approach)
    └─ Target: Enemy
```

### Example 3: Resource Gathering

```
Sequence (with AutoDriverStatus service)
├─ BTTask_AutoDriverMove
│   └─ Target: ResourceLocation
├─ Decorator: Check AutoDriver
│   └─ Within Distance: 100
├─ BTTask_AutoDriverInput (Gather)
├─ BTTask_AutoDriverWait (3.0s ±0.5s)
├─ BTTask_AutoDriverMove
│   └─ Target: StorageLocation
├─ BTTask_AutoDriverInput (Deposit)
└─ BTTask_AutoDriverWait (1.0s)
```

### Example 4: Adaptive Navigation

```
Selector
├─ Sequence (Direct path)
│   ├─ Service: AutoDriverStatus
│   ├─ Decorator: IsReachable == true
│   └─ BTTask_AutoDriverMove
│       ├─ Target: Destination
│       └─ Mode: Navigation
└─ Sequence (Alternative route)
    ├─ BTTask_AutoDriverMove
    │   └─ Target: WaypointA
    ├─ BTTask_AutoDriverMove
    │   └─ Target: WaypointB
    └─ BTTask_AutoDriverMove
        └─ Target: Destination
```

## Setup Guide

### 1. Create Behavior Tree Asset

1. Right-click in Content Browser
2. Select **Artificial Intelligence** > **Behavior Tree**
3. Name it (e.g., "BT_AutomationTest")

### 2. Create Blackboard Asset

1. Right-click in Content Browser
2. Select **Artificial Intelligence** > **Blackboard**
3. Name it (e.g., "BB_AutomationTest")
4. Add keys:
   - `TargetLocation` (Vector)
   - `IsExecuting` (Bool)
   - `PatrolIndex` (Int)
   - etc.

### 3. Setup AI Controller

```cpp
// C++ AIController
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

void AMyAIController::BeginPlay()
{
    Super::BeginPlay();

    if (BehaviorTree)
    {
        RunBehaviorTree(BehaviorTree);
    }
}
```

Or in Blueprint:
```
Event BeginPlay
└─ Run Behavior Tree
    └─ BT Asset: BT_AutomationTest
```

### 4. Add AutoDriver Component

Ensure your pawn has an AutoDriverComponent:

```cpp
// In pawn constructor
AutoDriver = CreateDefaultSubobject<UAutoDriverComponent>(TEXT("AutoDriver"));
```

Or add via Blueprint editor.

## Blackboard Integration

### Common Blackboard Keys

Recommended keys for AutoDriver behaviors:

```
# Navigation
Vector  TargetLocation       # Current movement target
Object  TargetActor          # Current target actor
Vector  HomeLocation         # Starting position
Bool    IsReachable          # Is target reachable

# Status
Bool    IsExecuting          # Is command executing
Bool    HasReachedTarget     # Arrived at destination
Float   DistanceToTarget     # Current distance

# Configuration
Float   WaitDuration         # Dynamic wait time
Name    InputAction          # Dynamic input name
Int     PatrolIndex          # Current patrol point
```

### Updating Blackboard from C++

```cpp
UBlackboardComponent* Blackboard = GetBlackboardComponent();

// Set values
Blackboard->SetValueAsVector("TargetLocation", FVector(1000, 0, 0));
Blackboard->SetValueAsBool("IsExecuting", true);
Blackboard->SetValueAsFloat("WaitDuration", 2.5f);

// Get values
FVector Target = Blackboard->GetValueAsVector("TargetLocation");
bool bExecuting = Blackboard->GetValueAsBool("IsExecuting");
```

## Best Practices

### 1. Use Services for Monitoring

Add BTService_AutoDriverStatus to parent nodes that need status updates:

```
Sequence (with AutoDriverStatus service)
├─ Service updates IsExecuting and IsReachable
├─ Selector based on IsReachable
│   ├─ Path A (if reachable)
│   └─ Path B (if blocked)
```

### 2. Combine Decorators

Stack multiple decorators for complex conditions:

```
Sequence
├─ Decorator: Has AutoDriver
├─ Decorator: Within Distance (500)
├─ Decorator: NOT Is Executing
└─ BTTask_AutoDriverInput
```

### 3. Handle Failures

Use Selector nodes for fallback logic:

```
Selector
├─ Preferred action (might fail)
└─ Fallback action (always succeeds)
```

### 4. Optimize Service Intervals

Don't update blackboard too frequently:
- Status monitoring: 0.5s intervals
- Distance checks: 0.2-0.5s
- Expensive queries: 1.0s+

### 5. Use Memory Pooling

For frequently executed BTs, consider:
- Reusing blackboard instances
- Pooling behavior tree components
- Caching expensive lookups

## Performance Considerations

**Task Execution:**
- Tasks with timeouts add timer overhead
- Tick-based tasks update every frame
- Balance tick frequency vs. responsiveness

**Service Updates:**
- Lower intervals = more frequent updates
- Add random deviation to spread CPU load
- Disable services when not needed

**Memory:**
- Each BT instance: ~2-5 KB
- Blackboard: ~100-500 bytes per key
- Task memory: varies by task type

## Debugging

### Visual Debugger

1. Play in PIE
2. Open Behavior Tree editor
3. Select AI controller in World Outliner
4. BT shows active nodes in real-time

### Logging

Enable logging in task properties:
```
bLogExecution = true
```

Logs will show:
```
BTTask_AutoDriverMove: Moving to X=1000 Y=0 Z=100
BTTask_AutoDriverMove: Completed. Distance: 45.2, Success: Yes
```

### Blackboard Debugging

In BT editor, enable "Show Blackboard Values" to see current values during execution.

## Advanced Techniques

### Dynamic Task Parameters

Use blackboard keys for runtime configuration:

```cpp
// Set wait duration dynamically
Blackboard->SetValueAsFloat("WaitDuration", FMath::RandRange(1.0f, 5.0f));

// BT task uses WaitDurationKey instead of fixed value
```

### Composite Behaviors

Create reusable BT sub-trees:

```
BT_Combat.uasset (reusable)
BT_Movement.uasset (reusable)
BT_Main.uasset (runs sub-trees)
```

### Event-Driven Updates

Update blackboard from gameplay events:

```cpp
// On enemy spotted
Blackboard->SetValueAsObject("Enemy", Enemy);
Blackboard->SetValueAsVector("LastKnownLocation", Enemy->GetActorLocation());

// BT reacts automatically
```

## Integration with Recording System

Combine BT with ActionRecorder:

```cpp
// Record BT execution
Recorder->StartRecording("BT_Patrol_Recording");
AIController->RunBehaviorTree(PatrolBT);

// Playback later
Playback->LoadAndPlayTimeline("Saved/Recordings/BT_Patrol_Recording.json");
```

## Troubleshooting

**BT tasks not appearing in editor:**
- Rebuild solution
- Restart Unreal Editor
- Verify module dependencies in Build.cs

**Tasks failing immediately:**
- Check AutoDriver component exists on pawn
- Verify blackboard keys are set
- Enable logging to see error messages

**Infinite loops:**
- Add Wait tasks between actions
- Use decorators to prevent re-execution
- Check sequence/selector logic

**Performance issues:**
- Reduce service update frequency
- Minimize tick-based tasks
- Use event-driven blackboard updates

## Future Enhancements

Planned features:
- Dynamic obstacle avoidance decorators
- Parallel task execution support
- BT sub-tree library
- Visual debugging improvements
- Performance profiling tools
