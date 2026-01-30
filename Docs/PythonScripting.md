// Python Scripting Support

## Overview

Python scripting support enables test automation and external tool integration via Unreal's PythonScriptPlugin. Write automated tests in Python, integrate with CI/CD pipelines, and create custom tooling.

## Setup

### 1. Enable Python Plugin

The Python plugin is already configured in `YesUeFsd.uplugin`:

```json
{
  "Plugins": [
    { "Name": "PythonScriptPlugin", "Enabled": true, "Optional": true }
  ]
}
```

### 2. Configure Python Path

Add the Python helper modules to your Python path:

**Windows:**
```batch
set PYTHONPATH=%PYTHONPATH%;D:\srcp\yes-ue-fsd\Content\Python
```

**Linux/Mac:**
```bash
export PYTHONPATH=$PYTHONPATH:/path/to/yes-ue-fsd/Content/Python
```

### 3. Install pytest (Optional)

For automated testing:

```bash
pip install pytest pytest-timeout
```

## Python API

### AutoDriver Class

Main interface for automation:

```python
from autodriver_helpers import AutoDriver

# Create instance for player 0
driver = AutoDriver(player_index=0)

# Movement
driver.move_to(unreal.Vector(1000, 0, 100), wait=True)
driver.move_to_actor(target_actor, acceptance_radius=50.0)

# Rotation
driver.rotate_to(unreal.Rotator(0, 90, 0))
driver.look_at(unreal.Vector(500, 500, 100))
driver.look_at_actor(enemy_actor)

# Input
driver.press_button("Jump")
driver.set_axis("MoveForward", 1.0)

# Navigation
reachable = driver.is_reachable(location)
path_length = driver.get_path_length(from_loc, to_loc)
random_loc = driver.get_random_location(origin, radius=500)

# Properties
current_pos = driver.location
current_rot = driver.rotation

# Control
driver.stop()
driver.wait_for_completion(timeout=10.0)
is_busy = driver.is_executing()
```

### Recording & Playback

```python
from autodriver_helpers import ActionRecorder, ActionPlayer

# Recording
recorder = ActionRecorder(player_index=0)
recorder.start("My Test Recording")
# ... perform actions ...
recorder.stop()
recorder.save("Saved/Recordings/test.json")

# Playback
player = ActionPlayer(player_index=0)
player.load_and_play("Saved/Recordings/test.json")
player.set_speed(2.0)  # 2x speed
player.pause()
player.resume()
player.stop()
```

### Utility Functions

```python
from autodriver_helpers import find_actor, get_actors_of_class, execute_command, wait

# Find actors
actor = find_actor("MyActor")
actors = get_actors_of_class(unreal.StaticMeshActor)

# Console commands
execute_command("stat fps")

# Wait
wait(2.5)  # Wait 2.5 seconds
```

## Example Scripts

### Basic Movement Test

```python
import unreal
from autodriver_helpers import AutoDriver

def test_movement():
    driver = AutoDriver()

    # Move to several locations
    waypoints = [
        unreal.Vector(1000, 0, 100),
        unreal.Vector(1000, 1000, 100),
        unreal.Vector(0, 1000, 100),
    ]

    for waypoint in waypoints:
        print(f"Moving to {waypoint}")
        driver.move_to(waypoint, wait=True, timeout=10.0)
        print(f"Reached {driver.location}")

test_movement()
```

### Combat Simulation

```python
from autodriver_helpers import AutoDriver, find_actor

def simulate_combat():
    driver = AutoDriver()

    # Find enemy
    enemy = find_actor("Enemy")
    if not enemy:
        print("No enemy found")
        return

    # Approach enemy
    driver.move_to_actor(enemy, acceptance_radius=300.0, wait=True)

    # Look at enemy
    driver.look_at_actor(enemy, wait=True)

    # Fire
    for _ in range(5):
        driver.press_button("Fire")
        wait(0.5)

    # Retreat
    retreat_pos = unreal.Vector(
        driver.location.x - 500,
        driver.location.y,
        driver.location.z
    )
    driver.move_to(retreat_pos, wait=True)

simulate_combat()
```

### Navigation Stress Test

```python
from autodriver_helpers import AutoDriver
import random

def navigation_stress_test():
    driver = AutoDriver()

    start_pos = driver.location

    # Visit 20 random locations
    for i in range(20):
        target = driver.get_random_location(start_pos, radius=2000.0)

        print(f"Waypoint {i + 1}/20: {target}")

        if driver.is_reachable(target):
            success = driver.move_to(target, wait=True, timeout=15.0)
            print(f"  Result: {'Success' if success else 'Failed'}")
        else:
            print(f"  Skipped (unreachable)")

navigation_stress_test()
```

## Pytest Integration

### Basic Test Structure

```python
# test_autodriver.py
import pytest
import unreal
from autodriver_helpers import AutoDriver
from pytest_helpers import assertions

@pytest.mark.movement
def test_basic_movement(autodriver):
    """Test basic movement functionality"""
    target = unreal.Vector(1000, 0, 100)

    success = autodriver.move_to(target, wait=True)

    assert success
    assertions.assert_reached_location(autodriver, target, tolerance=100.0)
```

### Fixtures

Available fixtures from `pytest_helpers`:

```python
def test_with_fixtures(autodriver, recorder, player, starting_position):
    """
    autodriver: AutoDriver instance
    recorder: ActionRecorder instance
    player: ActionPlayer instance
    starting_position: Initial position vector
    """
    pass
```

### Custom Assertions

```python
from pytest_helpers import assertions

# Assert reached location
assertions.assert_reached_location(driver, target, tolerance=100.0)

# Assert facing direction
assertions.assert_facing_direction(driver, target_yaw=90.0, tolerance=5.0)

# Assert location reachable
assertions.assert_location_reachable(driver, location)

# Assert execution state
assertions.assert_command_executing(driver, should_be_executing=True)
```

### Markers

```python
@pytest.mark.movement      # Movement tests
@pytest.mark.rotation      # Rotation tests
@pytest.mark.input         # Input tests
@pytest.mark.navigation    # Navigation tests
@pytest.mark.recording     # Recording/playback tests
@pytest.mark.slow          # Long-running tests
```

### Running Tests

```bash
# Run all tests
pytest Content/Python/tests/

# Run specific test file
pytest Content/Python/tests/test_movement.py

# Run tests with specific marker
pytest -m movement

# Run with verbose output
pytest -v

# Run with timeout
pytest --timeout=30
```

### Parametrized Tests

```python
@pytest.mark.parametrize("speed", [0.5, 1.0, 1.5, 2.0])
def test_movement_speeds(autodriver, speed):
    """Test movement at different speeds"""
    target = unreal.Vector(1000, 0, 100)
    success = autodriver.move_to(target, speed=speed, wait=True)
    assert success
```

## CI/CD Integration

### GitHub Actions Example

```yaml
name: AutoDriver Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: windows-latest

    steps:
    - uses: actions/checkout@v2

    - name: Setup Python
      uses: actions/setup-python@v2
      with:
        python-version: '3.9'

    - name: Install dependencies
      run: |
        pip install pytest pytest-timeout

    - name: Run Unreal Editor (headless)
      run: |
        UnrealEditor.exe YourProject.uproject -ExecCmds="py Content/Python/tests/test_movement.py" -unattended -nullrhi

    - name: Run pytest
      run: |
        pytest Content/Python/tests/ --junit-xml=test-results.xml

    - name: Upload results
      uses: actions/upload-artifact@v2
      with:
        name: test-results
        path: test-results.xml
```

### Jenkins Pipeline

```groovy
pipeline {
    agent any

    stages {
        stage('Setup') {
            steps {
                bat 'pip install pytest pytest-timeout'
            }
        }

        stage('Test') {
            steps {
                bat '''
                    set PYTHONPATH=%PYTHONPATH%;Content/Python
                    pytest Content/Python/tests/ --junit-xml=results.xml
                '''
            }
        }

        stage('Report') {
            steps {
                junit 'results.xml'
            }
        }
    }
}
```

## Advanced Usage

### Async Testing

```python
import asyncio
from autodriver_helpers import AutoDriver

async def async_test():
    driver = AutoDriver()

    # Start movement (non-blocking)
    driver.move_to(unreal.Vector(1000, 0, 100), wait=False)

    # Do other work while moving
    while driver.is_executing():
        print(f"Current position: {driver.location}")
        await asyncio.sleep(0.1)

    print("Movement complete")

asyncio.run(async_test())
```

### Multi-Player Testing

```python
from autodriver_helpers import AutoDriver

def multi_player_test():
    # Control multiple players
    player1 = AutoDriver(player_index=0)
    player2 = AutoDriver(player_index=1)

    # Move both simultaneously
    target1 = unreal.Vector(1000, 0, 100)
    target2 = unreal.Vector(-1000, 0, 100)

    player1.move_to(target1, wait=False)
    player2.move_to(target2, wait=False)

    # Wait for both
    player1.wait_for_completion()
    player2.wait_for_completion()
```

### Recording Replay

```python
from autodriver_helpers import AutoDriver, ActionRecorder, ActionPlayer

def record_and_replay():
    driver = AutoDriver()

    # Record gameplay
    recorder = ActionRecorder()
    recorder.start("Gameplay Session")

    # Perform actions
    driver.move_to(unreal.Vector(1000, 0, 100), wait=True)
    driver.press_button("Interact")

    # Stop and save
    timeline = recorder.stop()
    recorder.save("Saved/Recordings/session.json")

    # Later, replay
    player = ActionPlayer()
    player.load_and_play("Saved/Recordings/session.json")
    player.set_speed(2.0)  # Fast replay
```

## Headless Mode

Run tests without graphics for CI/CD:

```bash
UnrealEditor.exe YourProject.uproject \
    -ExecCmds="py run_tests.py" \
    -unattended \
    -nullrhi \
    -nosplash \
    -log
```

## Debugging

### Enable Logging

```python
import unreal

# Enable AutoDriver logging
unreal.AutoDriverPythonBridge.execute_console_command("log LogTemp Verbose")

# Your test code
driver = AutoDriver()
driver.move_to(unreal.Vector(1000, 0, 100))
```

### Print Debugging

```python
def debug_movement():
    driver = AutoDriver()
    target = unreal.Vector(1000, 0, 100)

    print(f"Starting at: {driver.location}")
    print(f"Target: {target}")
    print(f"Is reachable: {driver.is_reachable(target)}")

    driver.move_to(target, wait=False)

    while driver.is_executing():
        print(f"Current: {driver.location}")
        wait(0.5)

    print(f"Final: {driver.location}")
```

### Pytest Debugging

```bash
# Run with print output
pytest -s

# Run single test with verbose
pytest -v test_file.py::test_function

# Drop to debugger on failure
pytest --pdb
```

## Best Practices

1. **Use Fixtures**: Leverage pytest fixtures for setup/teardown
2. **Timeouts**: Always set timeouts to prevent hanging tests
3. **Assertions**: Use custom assertions for clearer error messages
4. **Markers**: Tag tests for selective execution
5. **Cleanup**: Stop commands and recordings in teardown
6. **Headless**: Run CI tests in headless mode
7. **Logging**: Enable verbose logging for debugging
8. **Isolation**: Each test should be independent

## Troubleshooting

**Import errors:**
- Verify PYTHONPATH includes Content/Python
- Check PythonScriptPlugin is enabled

**Tests hang:**
- Always use timeouts
- Check for infinite waits
- Stop commands in cleanup

**Intermittent failures:**
- Increase tolerances for location/rotation checks
- Add wait times between actions
- Check for timing-dependent logic

**No AutoDriver found:**
- Ensure AutoDriverComponent exists on player pawn
- Verify player index is correct
- Check subsystem initialization

## Performance Considerations

**Test Execution:**
- Headless mode: ~100-200ms per simple test
- With rendering: ~500-1000ms per test
- Network overhead: minimal

**Recommendations:**
- Group related tests
- Use markers to filter tests
- Run slow tests separately
- Parallelize when possible

## Future Enhancements

Planned features:
- Async/await support
- Multi-threaded test execution
- Visual test reports
- Performance profiling
- Coverage analysis
- Test recording/replay
