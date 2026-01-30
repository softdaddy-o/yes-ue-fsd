# Example Content Guide

This document provides instructions for creating and using example content with the Yes UE FSD plugin.

## Table of Contents

1. [Overview](#overview)
2. [Example Maps](#example-maps)
3. [Example Blueprints](#example-blueprints)
4. [Waypoint System](#waypoint-system)
5. [Demo Scenarios](#demo-scenarios)
6. [Setup Instructions](#setup-instructions)

---

## Overview

The Yes UE FSD plugin includes templates and guidance for creating demonstration content that showcases the automation capabilities. While the actual map files (`.umap`) and Blueprint assets (`.uasset`) must be created in the Unreal Editor, this guide provides step-by-step instructions.

---

## Example Maps

### 1. AutoDriver_Basics Map

**Purpose**: Simple environment for testing basic automation features.

**Setup Instructions**:

1. Create a new level in Unreal Editor
2. Add basic geometry:
   - Floor: 4000x4000 units
   - Simple obstacles (boxes, ramps)
   - Nav Mesh Bounds Volume (covers entire floor)
3. Place PlayerStart
4. Rebake Navigation Mesh
5. Save as `Content/Maps/AutoDriver_Basics.umap`

**What to Include**:
- Open flat area for movement testing
- Simple obstacles for navigation testing
- Waypoint markers (see Waypoint System below)
- Testing widgets (UI automation)

**Recommended Testing**:
```python
# Python test script for AutoDriver_Basics
from autodriver_helpers import AutoDriver

driver = AutoDriver()

# Test basic movement
driver.move_to_location(500, 0, 0)
driver.wait(3.0)

# Test rotation
driver.rotate_to(0, 180, 0)
driver.wait(2.0)

# Test input
driver.press_button("Jump")
```

---

### 2. AutoDriver_Navigation Map

**Purpose**: Complex navigation scenarios with multiple path types.

**Setup Instructions**:

1. Create a new level
2. Add varied terrain:
   - Multi-level platforms (different heights)
   - Stairs and ramps
   - Narrow corridors
   - Open areas
   - Dead ends
   - Obstacles requiring pathfinding
3. Add Nav Mesh Bounds Volume
4. Configure NavMesh generation:
   - Agent Radius: 34 units
   - Agent Height: 144 units
   - Max Slope: 45 degrees
5. Rebake Navigation Mesh
6. Save as `Content/Maps/AutoDriver_Navigation.umap`

**What to Include**:
- Long-distance navigation routes
- Multi-path options (test pathfinding choices)
- Unreachable areas (for negative testing)
- Teleport locations
- Patrol routes with 5+ waypoints

**Testing Scenarios**:
- Can agent find path to distant location?
- Does agent choose optimal path?
- How does agent handle blocked paths?
- Can agent navigate multi-level structures?

---

### 3. AutoDriver_Combat Map

**Purpose**: Test automation in combat scenarios.

**Setup Instructions**:

1. Create arena-style level
2. Add:
   - Cover positions (walls, boxes)
   - Vantage points (elevated positions)
   - Spawn points for AI enemies
   - Health/ammo pickups
3. Configure combat areas with Nav Mesh
4. Add trigger volumes for event zones
5. Save as `Content/Maps/AutoDriver_Combat.umap`

**What to Include**:
- BP_EnemyBot actors (simple AI opponents)
- Cover positions marked with waypoints
- Weapon spawns
- Health regen zones

**Automation Examples**:
```python
# Combat automation example
driver = AutoDriver()

# Move to cover
driver.move_to_location(1000, 500, 0)
driver.wait(2.0)

# Look at enemy
enemy_pos = (1500, -300, 100)
driver.look_at_location(*enemy_pos)

# Fire weapon
driver.press_button("Fire")
driver.wait(0.5)
driver.press_button("Fire")  # Shoot again
```

---

### 4. AutoDriver_Recording Map

**Purpose**: Demonstrate recording and playback functionality.

**Setup Instructions**:

1. Create simple level with clear movement paths
2. Add visual markers (numbered waypoints 1-10)
3. Add UI elements for playback controls
4. Save as `Content/Maps/AutoDriver_Recording.umap`

**What to Include**:
- Clear visual path markers
- Playback control UI (Play, Pause, Stop, Speed)
- Recording indicator UI
- Timeline scrubber widget

**Recording Workflow**:
1. Start recording
2. Perform actions (move, rotate, jump, interact)
3. Stop recording
4. Save timeline to file
5. Load and play back recording
6. Modify playback speed

**Example Recording Script**:
```python
from autodriver_helpers import ActionRecorder, ActionPlayer

# Record
recorder = ActionRecorder()
recorder.start_recording(player_actor)

# Perform actions
driver.move_to_location(500, 0, 0)
driver.wait(2.0)
driver.rotate_to(0, 90, 0)
driver.wait(1.0)
driver.press_button("Jump")
driver.wait(1.0)

# Stop and save
timeline = recorder.stop_recording()
recorder.save_timeline("my_recording.json")

# Playback
player = ActionPlayer()
player.load_and_play("my_recording.json", speed=1.5)
```

---

## Example Blueprints

### 1. BP_AutoDriver_Example (Player Controller)

**Parent Class**: PlayerController

**Components**:
- AutoDriver Component (added in Components panel)
- Enhanced Input Component

**Setup**:

1. Create new Blueprint Class → Player Controller
2. Name it `BP_AutoDriver_Example`
3. Open Blueprint editor
4. Add Components:
   - AutoDriver Component
   - Set "Enabled" to true by default
5. Event Graph setup:
   ```
   Event BeginPlay
   ↓
   Get AutoDriver Component
   ↓
   Set Enabled (true)
   ```

6. Add example automation logic:
   ```
   Custom Event: Test_Basic_Movement
   ↓
   AutoDriver->Move To Location (Target: 1000,0,0)
   ```

**Variables**:
- `TestWaypoints`: Array of Vectors (waypoint positions)
- `CurrentWaypointIndex`: Integer
- `AutomationEnabled`: Boolean

**Functions**:
- `StartAutomatedPatrol()`: Begin patrol route
- `StopAutomation()`: Stop all automation
- `RecordActions()`: Start recording
- `PlaybackActions()`: Start playback

---

### 2. BP_AutoDriver_TestBot (AI Character)

**Parent Class**: Character

**Components**:
- AutoDriver Component
- Behavior Tree Component
- Blackboard Component

**Setup**:

1. Create new Blueprint Class → Character
2. Name it `BP_AutoDriver_TestBot`
3. Add AutoDriver Component
4. Configure AI:
   - Create Behavior Tree: `BT_AutoDriver_Test`
   - Create Blackboard: `BB_AutoDriver_Test`
5. Set AI Controller Class to `AIController`

**Behavior Tree Tasks** (use provided BT nodes):
- `BTTask_AutoDriverMove`: Move to target location
- `BTTask_AutoDriverRotate`: Rotate to face direction
- `BTTask_AutoDriverWait`: Wait for duration
- `BTService_AutoDriverStatus`: Monitor automation status

**Example BT Structure**:
```
Root
├── Sequence
│   ├── BTTask_AutoDriverMove (Target: Waypoint1)
│   ├── BTTask_AutoDriverWait (Duration: 2.0)
│   ├── BTTask_AutoDriverRotate (Target: Waypoint2)
│   ├── BTTask_AutoDriverMove (Target: Waypoint2)
│   └── Loop back to start
```

---

### 3. BP_Waypoint (Actor)

**Parent Class**: Actor

**Components**:
- Static Mesh Component (sphere or marker mesh)
- Text Render Component (waypoint number/name)
- Billboard Component (editor visibility)

**Setup**:

1. Create new Blueprint Class → Actor
2. Name it `BP_Waypoint`
3. Add components:
   - Static Mesh: sphere mesh
   - Material: emissive material (color-coded)
   - Text Render: waypoint label
4. Make visible in-game and in-editor

**Variables**:
- `WaypointName`: String
- `WaypointIndex`: Integer
- `WaypointColor`: LinearColor
- `AutoProceed`: Boolean (auto-move to next waypoint)

---

### 4. BP_AutoDriver_DemoSequencer (Level Blueprint Usage)

**Purpose**: Orchestrate demo sequences automatically.

**Event Graph**:

```
Event BeginPlay
↓
Delay (2.0 seconds)
↓
Custom Event: Run_Demo_Sequence

Run_Demo_Sequence:
├── Phase 1: Move to Waypoint 1
│   ├── AutoDriver->Move To Location
│   ├── Delay (3.0)
│   └── Continue
├── Phase 2: Rotate to Face Target
│   ├── AutoDriver->Rotate To
│   ├── Delay (2.0)
│   └── Continue
├── Phase 3: Input Simulation
│   ├── AutoDriver->Press Button (Jump)
│   ├── Delay (1.0)
│   └── Continue
└── Phase 4: Navigation Query
    ├── AutoDriver->Get Path Length
    ├── Print String (show result)
    └── End
```

---

## Waypoint System

The waypoint system allows defining patrol routes and navigation targets.

### C++ Waypoint Component

Create `WaypointComponent.h` in `Source/YesUeFsd/Public/Examples/`:

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "WaypointComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class YESUEFSD_API UWaypointComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UWaypointComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    FString WaypointName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    int32 WaypointIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    FLinearColor WaypointColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    bool bAutoProceedToNext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    UWaypointComponent* NextWaypoint;

    UFUNCTION(BlueprintCallable, Category = "Waypoint")
    FVector GetWaypointLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Waypoint")
    UWaypointComponent* GetNextWaypoint() const;
};
```

### Using Waypoints in Blueprints

```
1. Place BP_Waypoint actors in your level
2. Number them sequentially (0, 1, 2, ...)
3. In BP_AutoDriver_Example or Level Blueprint:

Event BeginPlay
↓
Get All Actors Of Class (BP_Waypoint)
↓
Sort By Waypoint Index
↓
Store in Array

Custom Event: Patrol_Waypoints
↓
For Each Loop (Waypoints Array)
│   ├── Get Waypoint Location
│   ├── AutoDriver->Move To Location
│   ├── Wait For Arrival (use Timer)
│   └── Next waypoint
└── Loop back to start
```

---

## Demo Scenarios

### Scenario 1: Automated Patrol

**Objective**: Demonstrate autonomous patrol behavior.

**Setup**:
1. Place 5-10 waypoints in a loop
2. Spawn BP_AutoDriver_TestBot
3. Run patrol behavior tree

**Expected Behavior**:
- Bot moves to each waypoint in sequence
- Pauses briefly at each waypoint
- Loops continuously
- Avoids obstacles using nav mesh

---

### Scenario 2: UI Automation

**Objective**: Demonstrate automated UI interaction.

**Setup**:
1. Create UMG widget with buttons
2. Display widget on screen
3. Use AutoDriver to click buttons

**Script**:
```python
driver = AutoDriver()

# Click button by name
driver.click_widget("StartButton")
driver.wait(0.5)

# Wait for widget to appear
driver.wait_for_widget("ResultsPanel", timeout=5.0)

# Read widget text
text = driver.read_widget_text("ScoreLabel")
print(f"Score: {text}")
```

---

### Scenario 3: Recording and Playback

**Objective**: Record player actions and replay them.

**Setup**:
1. Load AutoDriver_Recording map
2. Start recording
3. Perform complex movement sequence
4. Save recording
5. Play back at different speeds

---

### Scenario 4: Navigation Stress Test

**Objective**: Test navigation under heavy load.

**Setup**:
1. Spawn 50+ BP_AutoDriver_TestBot actors
2. Each bot navigates to random location
3. Monitor performance stats

**Metrics to Track**:
- Frame rate (should stay > 60 FPS)
- Nav query time (< 10ms)
- Cache hit rate (> 60%)
- Memory usage (< 500 MB for all bots)

---

## Setup Instructions

### Quick Start

1. **Open your Unreal Engine project**
2. **Enable Yes UE FSD plugin** (if not already enabled)
3. **Create Content folders**:
   ```
   Content/
   ├── Maps/
   ├── Blueprints/
   │   ├── Characters/
   │   ├── PlayerControllers/
   │   └── Waypoints/
   └── UI/
   ```

4. **Create your first example map**:
   - File → New Level → Basic
   - Add PlayerStart
   - Add Nav Mesh Bounds Volume (scale to cover play area)
   - Place some static meshes for obstacles
   - Rebake navigation (right-click NavMeshBoundsVolume → Rebuild Navigation)
   - Save as `Content/Maps/AutoDriver_Basics.umap`

5. **Create example player controller**:
   - Right-click in Content Browser → Blueprint Class → Player Controller
   - Name: `BP_AutoDriver_Example`
   - Open blueprint → Add Component → AutoDriver
   - Set Enabled to true
   - Compile and save

6. **Test in-game**:
   - Open `AutoDriver_Basics` map
   - World Settings → Game Mode → Player Controller Class → `BP_AutoDriver_Example`
   - Play in Editor (PIE)
   - Open console (`~` key)
   - Type: `py` and enter Python commands

### Advanced Setup

For full example content, follow the detailed instructions in each map section above.

---

## Python Testing Scripts

Place these in `Content/Python/examples/`:

### `demo_basic.py`
```python
"""Basic automation demo."""
from autodriver_helpers import AutoDriver

def run_basic_demo():
    driver = AutoDriver()

    print("Moving forward...")
    driver.move_to_location(1000, 0, 0)
    driver.wait(3.0)

    print("Rotating...")
    driver.rotate_to(0, 180, 0)
    driver.wait(2.0)

    print("Jumping...")
    driver.press_button("Jump")
    driver.wait(1.0)

    print("Demo complete!")

if __name__ == "__main__":
    run_basic_demo()
```

### `demo_navigation.py`
```python
"""Navigation demo."""
from autodriver_helpers import AutoDriver

def run_navigation_demo():
    driver = AutoDriver()

    waypoints = [
        (500, 0, 0),
        (500, 500, 0),
        (0, 500, 0),
        (0, 0, 0)
    ]

    for i, waypoint in enumerate(waypoints):
        print(f"Moving to waypoint {i+1}...")

        # Check if reachable
        if driver.is_location_reachable(*waypoint):
            path_length = driver.get_path_length(*waypoint)
            print(f"  Path length: {path_length:.1f} units")

            driver.move_to_location(*waypoint)
            driver.wait(3.0)
        else:
            print(f"  Waypoint {i+1} is not reachable!")

    print("Navigation demo complete!")

if __name__ == "__main__":
    run_navigation_demo()
```

### `demo_recording.py`
```python
"""Recording and playback demo."""
from autodriver_helpers import AutoDriver, ActionRecorder, ActionPlayer

def run_recording_demo():
    driver = AutoDriver()
    recorder = ActionRecorder()

    print("Starting recording...")
    recorder.start_recording()

    # Perform actions
    driver.move_to_location(500, 0, 0)
    driver.wait(2.0)
    driver.rotate_to(0, 90, 0)
    driver.wait(1.0)
    driver.press_button("Jump")
    driver.wait(1.0)

    # Stop recording
    timeline = recorder.stop_recording()
    recorder.save_timeline("demo_recording.json")
    print("Recording saved!")

    # Wait a moment
    driver.wait(2.0)

    # Playback
    print("Playing back recording...")
    player = ActionPlayer()
    player.load_and_play("demo_recording.json", speed=1.0)

    print("Demo complete!")

if __name__ == "__main__":
    run_recording_demo()
```

---

## Troubleshooting Example Content

### Map Issues

**Navigation mesh not building:**
- Check Nav Mesh Bounds Volume covers the area
- Verify Agent Radius/Height settings
- Ensure geometry has collision
- Right-click NavMeshBoundsVolume → Rebuild Navigation
- Press `P` in viewport to visualize nav mesh

**Player not spawning:**
- Ensure PlayerStart is placed
- Check Game Mode settings
- Verify Player Controller is set correctly

### Blueprint Issues

**AutoDriver not working:**
- Check component is enabled
- Verify plugin is loaded
- Check output log for errors

**Behavior Tree not running:**
- Ensure AI Controller is set
- Check Blackboard is assigned
- Verify BT is set to run on controller

### Python Script Issues

**Cannot import autodriver_helpers:**
- Ensure `Content/Python` is in Python path
- Check plugin is loaded and MCP server is running
- Verify server port is 8081 (default)

**Commands not executing:**
- Check if AutoDriver component exists on actor
- Ensure actor is spawned and active
- Check console output for errors

---

## Next Steps

1. Create your first example map following the AutoDriver_Basics guide
2. Set up example player controller blueprint
3. Run the `demo_basic.py` script to verify setup
4. Expand to more complex scenarios
5. Create custom automation workflows

For more information, see:
- [USAGE.md](../USAGE.md) - General usage guide
- [API.md](../API.md) - API reference
- [Testing.md](./Testing.md) - Testing guide
