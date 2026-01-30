# Two-Layer Test Framework Architecture
**Yes UE FSD - Automated Testing Framework Design**

**Version**: 2.0
**Date**: 2026-01-31
**Status**: Design Specification

---

## Table of Contents

1. [Executive Summary](#executive-summary)
2. [Architecture Overview](#architecture-overview)
3. [Layer 1: Meta Layer (Test Orchestration)](#layer-1-meta-layer-test-orchestration)
4. [Layer 2: Local Control Layer (In-Game Automation)](#layer-2-local-control-layer-in-game-automation)
5. [Communication Protocol](#communication-protocol)
6. [Migration from MCP Architecture](#migration-from-mcp-architecture)
7. [Implementation Plan](#implementation-plan)
8. [Example Usage](#example-usage)
9. [Comparison with Gauntlet](#comparison-with-gauntlet)

---

## Executive Summary

### Problem Statement

The current Yes UE FSD plugin implements an **MCP-style HTTP server** for remote control, which:
- ❌ Requires external HTTP clients to coordinate multiple editors
- ❌ Doesn't provide native multi-instance orchestration
- ❌ Lacks pytest-first testing workflow
- ❌ Couples automation API with server protocol
- ❌ Not optimized for typical QA testing workflows

### Solution: Two-Layer Architecture

Transform Yes UE FSD into a **comprehensive automated test framework** with:

**Layer 1 - Meta Layer (Python):**
- Launch and coordinate multiple UE editor instances
- pytest-based test orchestration
- Multi-player/networked test scenarios
- Result aggregation and reporting

**Layer 2 - Local Control Layer (C++ Plugin + Python API):**
- In-game player automation (existing AutoDriver)
- Python scripting within each editor instance
- Rich automation capabilities (movement, UI, recording, etc.)
- No HTTP server required

### Benefits

- ✅ **Simpler**: No HTTP server complexity
- ✅ **Faster**: Direct Python API, no network overhead
- ✅ **pytest Integration**: Standard Python testing workflow
- ✅ **Multi-Instance**: Native support for networked testing
- ✅ **Flexible**: Same or different scripts per player
- ✅ **Type-Safe**: Full Python type hints and IDE autocomplete

---

## Architecture Overview

```
┌────────────────────────────────────────────────────────────────┐
│                    META LAYER (Python)                         │
│                 Test Orchestration Framework                    │
│                                                                 │
│  ┌──────────────┐  ┌─────────────┐  ┌──────────────────┐     │
│  │ pytest       │  │ UE Editor   │  │ Result           │     │
│  │ Test Runner  │─→│ Launcher    │→│ Aggregation      │     │
│  └──────────────┘  └─────────────┘  └──────────────────┘     │
│         │                  │                   │               │
│         └──────────────────┼───────────────────┘               │
│                            ↓                                   │
└────────────────────────────────────────────────────────────────┘
                             │
            ┌────────────────┼────────────────┐
            ↓                ↓                ↓
   ┌─────────────┐  ┌─────────────┐  ┌─────────────┐
   │  Editor 1   │  │  Editor 2   │  │  Editor N   │
   │  (Player A) │  │  (Player B) │  │  (Server)   │
   └─────────────┘  └─────────────┘  └─────────────┘
            │                │                │
            ↓                ↓                ↓
┌────────────────────────────────────────────────────────────────┐
│              LOCAL CONTROL LAYER (C++ Plugin + Python)         │
│                    In-Game Automation                           │
│                                                                 │
│  ┌──────────────────┐  ┌──────────────────┐  ┌──────────────┐│
│  │ AutoDriver       │  │ Python API       │  │ Recording    ││
│  │ Component        │←→│ Bridge           │←→│ & Playback   ││
│  └──────────────────┘  └──────────────────┘  └──────────────┘│
│           │                      │                    │        │
│           ↓                      ↓                    ↓        │
│  ┌──────────────────┐  ┌──────────────────┐  ┌──────────────┐│
│  │ Navigation       │  │ UI Interaction   │  │ Input        ││
│  │ System           │  │ System           │  │ Simulation   ││
│  └──────────────────┘  └──────────────────┘  └──────────────┘│
└────────────────────────────────────────────────────────────────┘
```

---

## Layer 1: Meta Layer (Test Orchestration)

### Purpose

Coordinate test execution across multiple Unreal Engine editor instances.

### Core Components

#### 1. Editor Launcher (`ue_launcher.py`)

**Responsibilities:**
- Launch UE editor instances with specific configurations
- Manage editor process lifecycle
- Monitor editor health and handle crashes
- Collect editor output and logs

**API:**
```python
class EditorInstance:
    """Represents a running UE editor instance."""

    def __init__(
        self,
        project_path: str,
        editor_path: Optional[str] = None,
        role: str = "client",
        instance_id: int = 0,
        port: Optional[int] = None,
        extra_args: List[str] = None
    ):
        """Initialize editor instance configuration."""

    async def start(self) -> bool:
        """Start the editor instance."""

    async def stop(self, timeout: float = 10.0) -> bool:
        """Stop the editor instance gracefully."""

    async def wait_for_ready(self, timeout: float = 60.0) -> bool:
        """Wait for editor to be ready for automation."""

    def is_running(self) -> bool:
        """Check if editor is still running."""

    def get_logs(self) -> List[str]:
        """Get editor output logs."""

    def execute_python(self, script: str) -> Any:
        """Execute Python script in the editor."""

    @property
    def pid(self) -> int:
        """Process ID of the editor."""


class EditorLauncher:
    """Manages multiple editor instances."""

    def __init__(self, project_path: str, editor_path: Optional[str] = None):
        """Initialize launcher with project configuration."""

    async def launch_instances(
        self,
        count: int,
        roles: Optional[List[str]] = None,
        base_port: int = 7777
    ) -> List[EditorInstance]:
        """Launch multiple editor instances."""

    async def shutdown_all(self, timeout: float = 10.0):
        """Shutdown all running instances."""

    def get_instances(self) -> List[EditorInstance]:
        """Get all managed instances."""
```

#### 2. Test Runner (`test_runner.py`)

**Responsibilities:**
- pytest integration
- Test scenario definition
- Script distribution to editors
- Result collection and aggregation

**API:**
```python
class TestScenario:
    """Defines a multi-instance test scenario."""

    def __init__(
        self,
        name: str,
        instances: int = 1,
        roles: Optional[Dict[int, str]] = None
    ):
        """Initialize test scenario."""
        self.name = name
        self.instances = instances
        self.roles = roles or {}
        self.scripts: Dict[str, str] = {}

    def set_script(self, role: str, script_path: str):
        """Assign a script to a role."""

    def set_script_for_all(self, script_path: str):
        """Use same script for all instances."""


class TestRunner:
    """Orchestrates test execution."""

    def __init__(self, launcher: EditorLauncher):
        """Initialize with editor launcher."""

    async def run_scenario(
        self,
        scenario: TestScenario,
        timeout: float = 300.0
    ) -> TestResult:
        """Run a test scenario."""

    async def run_scenarios(
        self,
        scenarios: List[TestScenario]
    ) -> List[TestResult]:
        """Run multiple scenarios sequentially."""


class TestResult:
    """Test execution result."""

    scenario_name: str
    success: bool
    duration: float
    instance_results: List[InstanceResult]
    errors: List[str]
    screenshots: List[str]
    logs: Dict[int, List[str]]


class InstanceResult:
    """Result from a single instance."""

    instance_id: int
    role: str
    success: bool
    errors: List[str]
    metrics: Dict[str, Any]
```

#### 3. pytest Integration (`conftest.py`)

**Responsibilities:**
- pytest fixtures for editor management
- Test markers for categorization
- Custom assertions
- Report generation

**API:**
```python
import pytest
from yes_ue_fsd.test_framework import EditorLauncher, TestRunner, TestScenario

@pytest.fixture(scope="session")
async def editor_launcher():
    """Provide editor launcher for test session."""
    launcher = EditorLauncher(project_path="path/to/project.uproject")
    yield launcher
    await launcher.shutdown_all()


@pytest.fixture
async def single_editor(editor_launcher):
    """Provide single editor instance."""
    instances = await editor_launcher.launch_instances(count=1)
    yield instances[0]
    await instances[0].stop()


@pytest.fixture
async def multiplayer_editors(editor_launcher):
    """Provide multiple editor instances for multiplayer testing."""
    instances = await editor_launcher.launch_instances(
        count=2,
        roles=["client", "client"]
    )
    yield instances
    for instance in instances:
        await instance.stop()


# Custom markers
pytest.mark.multiplayer = pytest.mark.multiplayer
pytest.mark.singleplayer = pytest.mark.singleplayer
pytest.mark.server_required = pytest.mark.server_required
```

#### 4. Result Aggregator (`result_aggregator.py`)

**Responsibilities:**
- Collect results from all instances
- Generate test reports (JUnit XML, HTML)
- Screenshot aggregation
- Performance metrics

**API:**
```python
class ResultAggregator:
    """Aggregates results from multiple instances."""

    def __init__(self, output_dir: str):
        """Initialize with output directory."""

    def add_result(self, result: TestResult):
        """Add a test result."""

    def generate_junit_xml(self, output_path: str):
        """Generate JUnit XML report."""

    def generate_html_report(self, output_path: str):
        """Generate HTML report with screenshots."""

    def get_summary(self) -> Dict[str, Any]:
        """Get test summary statistics."""
```

---

## Layer 2: Local Control Layer (In-Game Automation)

### Purpose

Provide Python API for controlling player actions within each editor instance.

### Core Components (Existing - Preserved)

#### 1. AutoDriver Component (C++)

**Preserved from existing implementation:**
- `UAutoDriverComponent` - Player control
- Command system (movement, rotation, input)
- Navigation integration
- UI interaction
- Recording and playback

#### 2. Python Bridge (C++)

**Preserved and enhanced:**
- `UAutoDriverPythonBridge` - Existing bridge
- All existing Python-exposed functions
- **New**: Direct Python module import (no HTTP)

#### 3. Python API (Python)

**Enhanced from existing `autodriver_helpers.py`:**

```python
from yes_ue_fsd import AutoDriver, ActionRecorder, ActionPlayer

# Simple, direct API (no HTTP)
driver = AutoDriver(player_index=0)
driver.move_to(location=(100, 200, 50))
driver.click_widget("StartButton")
driver.press_button("Jump")

# Recording
recorder = ActionRecorder(player_index=0)
recorder.start("test_sequence")
# ... perform actions ...
timeline = recorder.stop()
timeline.save("saved_test.json")

# Playback
player = ActionPlayer(player_index=0)
player.load_and_play("saved_test.json")
```

### New: Instance Context Manager

**New component for managing instance identity:**

```python
class InstanceContext:
    """Manages local instance configuration."""

    def __init__(self):
        self.instance_id: int = 0
        self.role: str = "client"
        self.port: int = 7777

    @classmethod
    def from_environment(cls) -> 'InstanceContext':
        """Load context from environment variables."""
        ctx = cls()
        ctx.instance_id = int(os.getenv("UE_INSTANCE_ID", "0"))
        ctx.role = os.getenv("UE_INSTANCE_ROLE", "client")
        ctx.port = int(os.getenv("UE_INSTANCE_PORT", "7777"))
        return ctx


# Global instance context
instance = InstanceContext.from_environment()
```

---

## Communication Protocol

### Between Layers

**Meta Layer → Local Control Layer:**
- **Mechanism**: Editor command-line arguments + environment variables
- **Data**: Instance configuration (ID, role, port)
- **Script Execution**: Python scripts executed via UE's Python plugin

```python
# Meta layer launches editor with:
subprocess.Popen([
    "UnrealEditor.exe",
    "MyProject.uproject",
    f"-ExecCmds=py {script_path}",
    f"-UE_INSTANCE_ID={instance_id}",
    f"-UE_INSTANCE_ROLE={role}",
    f"-UE_INSTANCE_PORT={port}"
])
```

**Local Control Layer → Meta Layer:**
- **Mechanism**: Shared file system or named pipes
- **Data**: Test results, logs, screenshots
- **Format**: JSON files in shared directory

```python
# Local layer writes results:
{
    "instance_id": 0,
    "role": "client",
    "success": true,
    "duration": 45.2,
    "errors": [],
    "screenshots": ["shot1.png", "shot2.png"],
    "metrics": {
        "frames_rendered": 2700,
        "avg_fps": 60.0
    }
}
```

### Between Instances (Networked Testing)

**Standard UE Networking:**
- Unreal Engine's built-in replication system
- Servers and clients connect via standard ports
- No custom protocol needed - UE handles it

---

## Migration from MCP Architecture

### What Gets Removed

1. **HTTP Server** (`McpServer.h/cpp`) ❌
   - No longer needed
   - Direct Python API replaces HTTP calls

2. **Editor Subsystem** (`McpEditorSubsystem.h/cpp`) ❌
   - Server lifecycle management not needed
   - Replaced by meta layer launcher

3. **JSON-RPC Protocol** ❌
   - No network protocol overhead
   - Direct function calls via Python bridge

### What Gets Preserved

1. **AutoDriver Components** ✅
   - All automation logic stays
   - Core engine unchanged

2. **Python Bridge** ✅
   - Enhanced for direct import
   - No HTTP wrapper needed

3. **Test Infrastructure** ✅
   - C++ automation tests
   - Python test files

### What Gets Added

1. **Meta Layer Framework** (NEW)
   - Editor launcher
   - Test runner
   - Result aggregator
   - pytest integration

2. **Instance Context** (NEW)
   - Environment-based configuration
   - Role and ID management

3. **Multi-Instance Support** (NEW)
   - Parallel editor coordination
   - Result collection from multiple sources

### Migration Steps

```
Phase 1: Add Meta Layer
├─ Implement EditorLauncher
├─ Implement TestRunner
├─ Create pytest fixtures
└─ Test single-instance scenarios

Phase 2: Remove HTTP Server
├─ Delete McpServer files
├─ Delete McpEditorSubsystem
├─ Remove HTTP server tests
└─ Clean up dependencies

Phase 3: Enhance Python API
├─ Add direct module import
├─ Remove HTTP client code
├─ Update documentation
└─ Create migration guide

Phase 4: Multi-Instance Testing
├─ Add instance context management
├─ Implement result aggregation
├─ Test networked scenarios
└─ Performance optimization
```

---

## Implementation Plan

### Milestone 1: Meta Layer Foundation (Week 1)

**Tasks:**
- [ ] Create `meta_layer/` directory structure
- [ ] Implement `EditorLauncher` class
- [ ] Implement `EditorInstance` class
- [ ] Add process management and monitoring
- [ ] Write unit tests for launcher

**Deliverables:**
- `meta_layer/ue_launcher.py`
- `meta_layer/tests/test_launcher.py`
- Single-editor launch working

### Milestone 2: Test Runner & pytest Integration (Week 1-2)

**Tasks:**
- [ ] Implement `TestScenario` class
- [ ] Implement `TestRunner` class
- [ ] Create pytest fixtures in `conftest.py`
- [ ] Add custom markers
- [ ] Implement result collection

**Deliverables:**
- `meta_layer/test_runner.py`
- `meta_layer/conftest.py`
- Basic pytest tests working

### Milestone 3: Multi-Instance Support (Week 2)

**Tasks:**
- [ ] Implement instance context management
- [ ] Add environment variable passing
- [ ] Test 2-editor scenarios
- [ ] Test N-editor scenarios
- [ ] Add role-based script assignment

**Deliverables:**
- `local_layer/instance_context.py`
- Multi-editor tests passing
- Example multiplayer test

### Milestone 4: Remove MCP Server (Week 2-3)

**Tasks:**
- [ ] Remove `McpServer.h/cpp`
- [ ] Remove `McpEditorSubsystem.h/cpp`
- [ ] Update Python API (remove HTTP client)
- [ ] Update all tests
- [ ] Update documentation

**Deliverables:**
- Cleaned codebase
- Updated tests passing
- Migration guide document

### Milestone 5: Result Aggregation & Reporting (Week 3)

**Tasks:**
- [ ] Implement `ResultAggregator` class
- [ ] Add JUnit XML generation
- [ ] Add HTML report generation
- [ ] Integrate screenshot system
- [ ] Add performance metrics

**Deliverables:**
- `meta_layer/result_aggregator.py`
- Report examples
- CI/CD integration guide

### Milestone 6: Documentation & Examples (Week 3-4)

**Tasks:**
- [ ] Write comprehensive user guide
- [ ] Create example test scenarios
- [ ] Document API reference
- [ ] Create video tutorials
- [ ] Write migration guide

**Deliverables:**
- `Docs/UserGuide.md`
- `Examples/` directory with 5+ examples
- `Docs/MigrationGuide.md`

---

## Example Usage

### Example 1: Single-Player Test

```python
# tests/test_movement.py
import pytest
from yes_ue_fsd import AutoDriver

@pytest.mark.singleplayer
async def test_player_movement(single_editor):
    """Test basic player movement."""

    # Execute test script in editor
    result = single_editor.execute_python("""
from yes_ue_fsd import AutoDriver

driver = AutoDriver(player_index=0)

# Move to target location
success = driver.move_to(location=(1000, 2000, 100))
assert success, "Failed to reach target location"

# Verify position
pos = driver.location
assert abs(pos.x - 1000) < 50, "X position incorrect"
assert abs(pos.y - 2000) < 50, "Y position incorrect"
    """)

    assert result.success, "Test failed in editor"
```

### Example 2: Multiplayer Test

```python
# tests/test_multiplayer.py
import pytest
from yes_ue_fsd.test_framework import TestScenario

@pytest.mark.multiplayer
async def test_two_player_interaction(editor_launcher):
    """Test two players interacting."""

    # Define scenario
    scenario = TestScenario(
        name="two_player_interaction",
        instances=2,
        roles={0: "player_a", 1: "player_b"}
    )

    # Assign scripts
    scenario.set_script("player_a", "tests/scripts/player_a.py")
    scenario.set_script("player_b", "tests/scripts/player_b.py")

    # Run scenario
    runner = TestRunner(editor_launcher)
    result = await runner.run_scenario(scenario, timeout=120.0)

    # Verify results
    assert result.success, f"Test failed: {result.errors}"
    assert len(result.instance_results) == 2
    assert all(r.success for r in result.instance_results)
```

### Example 3: Role-Based Scripts

```python
# tests/scripts/player_a.py (Server)
from yes_ue_fsd import AutoDriver, instance

driver = AutoDriver(player_index=0)

if instance.role == "server":
    # Server waits at spawn
    print("Server: Waiting for client...")
    driver.wait(10.0)

    # Check if client connected
    # (Use UE's networking to query connected clients)
    print("Server: Test complete")


# tests/scripts/player_b.py (Client)
from yes_ue_fsd import AutoDriver, instance

driver = AutoDriver(player_index=0)

if instance.role == "client":
    # Client moves to server
    print("Client: Moving to server...")
    success = driver.move_to_actor("ServerPlayer")
    assert success, "Failed to reach server"

    # Interact with server
    driver.press_button("Interact")
    print("Client: Test complete")
```

### Example 4: Same Script for All Instances

```python
# tests/test_stress.py
import pytest
from yes_ue_fsd.test_framework import TestScenario

@pytest.mark.multiplayer
async def test_10_player_stress(editor_launcher):
    """Stress test with 10 players."""

    scenario = TestScenario(
        name="10_player_stress",
        instances=10
    )

    # All instances run the same script
    scenario.set_script_for_all("tests/scripts/random_movement.py")

    runner = TestRunner(editor_launcher)
    result = await runner.run_scenario(scenario, timeout=300.0)

    assert result.success

    # Check performance metrics
    for instance_result in result.instance_results:
        assert instance_result.metrics["avg_fps"] > 30, \
            f"Instance {instance_result.instance_id} had low FPS"
```

---

## Comparison with Gauntlet

### What Gauntlet Provides

| Feature | Gauntlet | Yes UE FSD |
|---------|----------|------------|
| **Multi-instance launch** | ✅ Packaged builds | ✅ PIE/Editor |
| **Platform support** | ✅ PC, console, mobile | ✅ PC (editor-focused) |
| **Language** | C# via UAT | Python via pytest |
| **CI/CD integration** | ✅ Full | ✅ Full |
| **Player control API** | ❌ Manual | ✅ AutoDriver |
| **Input simulation** | ❌ Plugin needed | ✅ Built-in |
| **Recording/playback** | ❌ No | ✅ Built-in |
| **Python scripting** | ❌ No | ✅ Full API |
| **In-editor testing** | ❌ Packaged only | ✅ PIE support |

### Complementary Relationship

**Use Gauntlet for:**
- Packaged build testing
- Multi-platform testing (console, mobile)
- Large-scale CI/CD deployments
- Performance profiling across platforms

**Use Yes UE FSD for:**
- Rapid iteration in-editor
- Python-driven test automation
- Complex player behavior testing
- Visual testing and recording
- QA team workflows

**Use Both Together:**
- Gauntlet launches packaged builds
- Yes UE FSD provides automation inside each instance
- Best of both worlds for enterprise testing

---

## Technical Notes

### Performance Considerations

**Editor Launch Time:**
- Cold start: ~30-60 seconds per editor
- Warm start (same project): ~15-30 seconds
- Mitigation: Launch editors in parallel, reuse instances

**Memory Usage:**
- ~2-4 GB per editor instance
- Test on machines with sufficient RAM
- Recommended: 8 GB RAM + 4 GB per additional instance

**CPU Usage:**
- Multi-instance testing is CPU-intensive
- Recommended: 8+ cores for 4+ instances
- Consider cloud testing infrastructure

### Security

**Local Execution Only:**
- No network exposure (no HTTP server)
- All communication via local filesystem or pipes
- Safe for development machines

### Extensibility

**Custom Test Fixtures:**
```python
# conftest.py
@pytest.fixture
async def custom_scenario(editor_launcher):
    """Your custom test scenario."""
    # ... setup ...
    yield instances
    # ... teardown ...
```

**Custom Assertions:**
```python
# assertions.py
def assert_networked(instances):
    """Assert all instances are networked together."""
    # ... implementation ...
```

---

## Conclusion

The **two-layer architecture** transforms Yes UE FSD from an HTTP-based remote control system into a **comprehensive automated test framework**. This design:

- ✅ Eliminates HTTP server complexity
- ✅ Provides native multi-instance orchestration
- ✅ Integrates seamlessly with pytest
- ✅ Preserves all existing automation capabilities
- ✅ Enables flexible role-based testing
- ✅ Supports both single-player and multiplayer scenarios

The migration path is clear, preserving the excellent AutoDriver engine while modernizing the architecture for QA workflows.
