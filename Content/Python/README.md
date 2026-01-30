# Yes UE FSD - Two-Layer Test Framework

**Version 2.0** - Automated Testing Framework for Unreal Engine

## Overview

Yes UE FSD provides a **two-layer architecture** for automated testing of Unreal Engine projects:

- **Layer 1 (Meta Layer)**: Test orchestration and multi-instance management
- **Layer 2 (Local Control Layer)**: In-game player automation

This architecture enables:
- ✅ Single-player testing
- ✅ Multiplayer testing with multiple editor instances
- ✅ Role-based test scenarios (server, client, player A/B)
- ✅ pytest integration for standard Python testing workflow
- ✅ Result aggregation and reporting (JUnit XML, HTML, JSON)

## Quick Start

### Installation

```bash
# Install dependencies
pip install -r requirements.txt

# Add to Python path (or install as package)
export PYTHONPATH="${PYTHONPATH}:/path/to/Content/Python"
```

### Single-Player Test

```python
# tests/test_basic.py
import pytest
from yes_ue_fsd import AutoDriver

@pytest.mark.singleplayer
@pytest.mark.asyncio
async def test_movement(single_editor):
    """Test basic player movement."""
    # Test script runs inside the editor
    # AutoDriver controls the player
    pass
```

### Multiplayer Test

```python
# tests/test_multiplayer.py
import pytest
from yes_ue_fsd.meta_layer import TestScenario

@pytest.mark.multiplayer
@pytest.mark.asyncio
async def test_two_players(test_runner, editor_launcher):
    """Test two players interacting."""

    scenario = TestScenario(
        name="two_player_test",
        instances=2,
        roles={0: "player_a", 1: "player_b"}
    )

    scenario.set_script("player_a", "scripts/player_a.py")
    scenario.set_script("player_b", "scripts/player_b.py")

    result = await test_runner.run_scenario(scenario)
    assert result.success
```

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│          Meta Layer (Test Orchestration)                 │
│  ┌────────────┐  ┌───────────┐  ┌─────────────────┐   │
│  │ pytest     │──│ Editor    │──│ Result          │   │
│  │ Tests      │  │ Launcher  │  │ Aggregator      │   │
│  └────────────┘  └───────────┘  └─────────────────┘   │
└────────────────────────┬────────────────────────────────┘
                         │
        ┌────────────────┼────────────────┐
        ↓                ↓                ↓
 ┌────────────┐  ┌────────────┐  ┌────────────┐
 │ Editor 1   │  │ Editor 2   │  │ Editor N   │
 └────────────┘  └────────────┘  └────────────┘
        │                │                │
        ↓                ↓                ↓
┌──────────────────────────────────────────────────────────┐
│        Local Control Layer (In-Game Automation)          │
│  ┌────────────┐  ┌────────────┐  ┌────────────┐        │
│  │ AutoDriver │  │ Recording  │  │ Navigation │        │
│  │ Component  │  │ & Playback │  │ & UI       │        │
│  └────────────┘  └────────────┘  └────────────┘        │
└──────────────────────────────────────────────────────────┘
```

## Meta Layer API

### EditorLauncher

Launch and manage multiple editor instances:

```python
from yes_ue_fsd.meta_layer import EditorLauncher

launcher = EditorLauncher(project_path="MyProject.uproject")

# Launch 2 instances
instances = await launcher.launch_instances(
    count=2,
    roles=["server", "client"]
)

# Shutdown all
await launcher.shutdown_all()
```

### TestRunner

Execute test scenarios:

```python
from yes_ue_fsd.meta_layer import TestRunner, TestScenario

scenario = TestScenario(
    name="my_test",
    instances=2,
    timeout=120.0
)

runner = TestRunner(launcher)
result = await runner.run_scenario(scenario)

print(f"Success: {result.success}")
print(f"Duration: {result.duration}s")
```

### ResultAggregator

Collect and report results:

```python
from yes_ue_fsd.meta_layer import ResultAggregator

aggregator = ResultAggregator(output_dir="test_results")

# Add results
aggregator.add_result(result1)
aggregator.add_result(result2)

# Generate reports
aggregator.generate_junit_xml()
aggregator.generate_html_report()
aggregator.generate_json_report()
```

## Local Control Layer API

### AutoDriver

Control player actions (inside editor):

```python
from yes_ue_fsd import AutoDriver

driver = AutoDriver(player_index=0)

# Movement
driver.move_to(location=(1000, 2000, 100))
driver.rotate_to(rotation=(0, 90, 0))

# Input
driver.press_button("Jump")
driver.set_axis("MoveForward", 1.0)

# UI
driver.click_widget("StartButton")
driver.wait_for_widget("LoadingScreen", timeout=10.0)

# Status
position = driver.location
is_moving = driver.is_executing()
```

### Instance Context

Access instance information:

```python
from yes_ue_fsd import instance

print(f"Instance ID: {instance.instance_id}")
print(f"Role: {instance.role}")
print(f"Port: {instance.port}")

if instance.is_server():
    # Server-specific logic
    pass
elif instance.is_client():
    # Client-specific logic
    pass
```

### Recording & Playback

Record and play back actions:

```python
from yes_ue_fsd import ActionRecorder, ActionPlayer

# Record
recorder = ActionRecorder(player_index=0)
recorder.start("test_sequence")
# ... perform actions ...
timeline = recorder.stop()
timeline.save("recording.json")

# Playback
player = ActionPlayer(player_index=0)
player.load_and_play("recording.json")
```

## pytest Configuration

### Fixtures

Available fixtures in `conftest.py`:

- `editor_launcher`: EditorLauncher instance (session-scoped)
- `single_editor`: Single editor instance
- `multiplayer_editors`: Two editor instances
- `server_and_clients`: Server + two clients
- `test_runner`: TestRunner instance
- `result_aggregator`: ResultAggregator instance

### Markers

Custom pytest markers:

- `@pytest.mark.singleplayer`: Single-player test
- `@pytest.mark.multiplayer`: Multiplayer test
- `@pytest.mark.server_required`: Requires dedicated server
- `@pytest.mark.slow`: Slow-running test
- `@pytest.mark.stress`: Stress/performance test

### Usage

```bash
# Run all tests
pytest tests/

# Run only singleplayer tests
pytest -m singleplayer tests/

# Run only multiplayer tests
pytest -m multiplayer tests/

# Generate JUnit XML report
pytest --junit-xml=results.xml tests/

# Verbose output
pytest -v tests/

# Stop on first failure
pytest -x tests/
```

## Example Test Scenarios

### Example 1: Basic Movement Test

```python
@pytest.mark.singleplayer
@pytest.mark.asyncio
async def test_movement(single_editor):
    """Test player can move to target location."""
    # Script executed in editor would test movement
    assert single_editor.is_running()
```

### Example 2: Two-Player Interaction

```python
@pytest.mark.multiplayer
@pytest.mark.asyncio
async def test_interaction(test_runner):
    """Test two players can interact."""

    scenario = TestScenario(name="interaction", instances=2)
    scenario.set_script_for_all("scripts/interaction.py")

    result = await test_runner.run_scenario(scenario)
    assert result.success
```

### Example 3: Server Stress Test

```python
@pytest.mark.stress
@pytest.mark.asyncio
async def test_many_clients(test_runner):
    """Stress test with 10 clients."""

    scenario = TestScenario(name="stress", instances=10, timeout=600.0)
    scenario.set_script_for_all("scripts/stress_client.py")

    result = await test_runner.run_scenario(scenario)
    assert result.success

    # Check performance
    for ir in result.instance_results:
        assert ir.metrics.get("avg_fps", 0) > 30
```

## Directory Structure

```
Content/Python/
├── yes_ue_fsd/
│   ├── __init__.py
│   ├── meta_layer/              # Test orchestration
│   │   ├── __init__.py
│   │   ├── ue_launcher.py       # Editor launching
│   │   ├── test_runner.py       # Test execution
│   │   └── result_aggregator.py # Result collection
│   └── local_layer/              # In-game automation
│       ├── __init__.py
│       ├── autodriver.py         # Player control
│       ├── recording.py          # Record/playback
│       └── instance_context.py   # Instance info
├── tests/
│   ├── conftest.py               # pytest configuration
│   ├── test_single_player.py     # Single-player tests
│   └── test_multiplayer.py       # Multiplayer tests
├── autodriver_helpers.py         # Legacy helpers (preserved)
├── pytest_helpers.py             # pytest utilities (preserved)
└── screenshot_system.py          # Screenshot capture (preserved)
```

## Migration from v1.x (MCP Server)

If you were using the previous MCP-based architecture:

1. **Remove HTTP client code** - No longer needed
2. **Use direct Python API** - Import `from yes_ue_fsd import AutoDriver`
3. **Update tests to pytest** - Use fixtures instead of HTTP requests
4. **Multi-instance coordination** - Use TestRunner and TestScenario

See `Docs/MigrationGuide.md` for detailed migration instructions.

## Requirements

- Python 3.7+
- Unreal Engine 5.4+ (with Python plugin enabled)
- pytest 7.0+
- psutil 5.9+

```bash
pip install pytest pytest-asyncio psutil
```

## Advantages Over MCP Architecture

| Feature | v1.x (MCP) | v2.0 (Two-Layer) |
|---------|------------|------------------|
| **API Access** | HTTP requests | Direct Python |
| **Type Safety** | JSON strings | Python types |
| **IDE Support** | None | Full autocomplete |
| **Multi-Instance** | Manual coordination | Native support |
| **pytest Integration** | External | Built-in |
| **Network Overhead** | High | None |
| **Setup Complexity** | Server + client | Just pytest |
| **Debugging** | Difficult | Standard Python |

## Troubleshooting

### Editor Won't Launch

- Check `UE_ROOT` environment variable or specify `editor_path`
- Verify project path is correct (.uproject file)
- Check editor logs in output

### Tests Timeout

- Increase `timeout` parameter in `TestScenario`
- Check editor startup time (first launch is slow)
- Monitor system resources (RAM, CPU)

### Import Errors

- Ensure `Content/Python` is in `PYTHONPATH`
- Check Python version (3.7+)
- Install requirements: `pip install -r requirements.txt`

## Documentation

- **Architecture**: `Docs/TwoLayerArchitecture.md`
- **Migration Guide**: `Docs/MigrationGuide.md`
- **API Reference**: `Docs/API.md`
- **Example Tests**: `Content/Python/tests/`

## License

[Your License Here]

## Contributing

[Contributing Guidelines]
