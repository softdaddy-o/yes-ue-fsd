# Migration Guide: v1.x (MCP) → v2.0 (Two-Layer Architecture)

**Date**: 2026-01-31
**Status**: Migration Guide

---

## Overview

This guide helps you migrate from the v1.x MCP-based architecture to the new v2.0 two-layer test framework.

## Key Changes

### v1.x (MCP Server)

```
External Test Script
    ↓ (HTTP POST)
MCP Server (port 8081)
    ↓ (JSON-RPC)
AutoDriver Component
```

### v2.0 (Two-Layer)

```
pytest Tests (Meta Layer)
    ↓ (Launch)
Multiple Editor Instances
    ↓ (Direct Python API)
AutoDriver Component (Local Layer)
```

---

## What Changed

| Aspect | v1.x | v2.0 |
|--------|------|------|
| **API Access** | HTTP requests to port 8081 | Direct Python imports |
| **Protocol** | JSON-RPC 2.0 | Native Python |
| **Server** | McpServer (HTTP) | No server needed |
| **Multi-Instance** | Manual setup | EditorLauncher |
| **Testing** | External scripts | pytest framework |
| **Type Safety** | JSON strings | Python types with hints |

---

## Step-by-Step Migration

### Step 1: Install New Dependencies

```bash
# Install pytest and dependencies
pip install -r Content/Python/requirements.txt
```

### Step 2: Update Imports

#### Before (v1.x)

```python
import requests
import json

# Make HTTP request to MCP server
response = requests.post(
    "http://localhost:8081/rpc",
    json={
        "jsonrpc": "2.0",
        "method": "tools/call",
        "params": {
            "name": "autodriver/move_to_location",
            "arguments": {
                "location": {"x": 100, "y": 200, "z": 50},
                "player_index": 0
            }
        },
        "id": 1
    }
)
result = response.json()
```

#### After (v2.0)

```python
from yes_ue_fsd import AutoDriver

# Direct Python API
driver = AutoDriver(player_index=0)
driver.move_to(location=(100, 200, 50))
```

### Step 3: Convert Tests to pytest

#### Before (v1.x)

```python
# test_movement.py
import requests

def test_movement():
    """Test player movement via HTTP API."""

    # Start MCP server manually
    # ...

    # Make HTTP request
    response = requests.post(
        "http://localhost:8081/rpc",
        json={
            "jsonrpc": "2.0",
            "method": "tools/call",
            "params": {
                "name": "autodriver/move_to_location",
                "arguments": {"location": {"x": 1000, "y": 2000, "z": 100}}
            },
            "id": 1
        }
    )

    assert response.status_code == 200
    result = response.json()
    assert "result" in result

# Run manually: python test_movement.py
```

#### After (v2.0)

```python
# tests/test_movement.py
import pytest
from yes_ue_fsd import AutoDriver

@pytest.mark.singleplayer
@pytest.mark.asyncio
async def test_movement(single_editor):
    """Test player movement with direct API."""

    # Script runs inside editor
    script = """
from yes_ue_fsd import AutoDriver

driver = AutoDriver(player_index=0)
success = driver.move_to(location=(1000, 2000, 100))
assert success

import unreal
unreal.SystemLibrary.quit_editor()
"""

    # Editor instance is managed by pytest fixture
    assert single_editor.is_running()

# Run with: pytest tests/test_movement.py
```

### Step 4: Multi-Instance Tests

#### Before (v1.x)

```python
# Manually launch multiple editors
# Manually configure network ports
# Make HTTP requests to each editor's MCP server (different ports)
# Manually coordinate between instances
# Manually collect results
```

#### After (v2.0)

```python
# tests/test_multiplayer.py
import pytest
from yes_ue_fsd.meta_layer import TestScenario

@pytest.mark.multiplayer
@pytest.mark.asyncio
async def test_multiplayer(test_runner):
    """Test with multiple instances."""

    # Define scenario
    scenario = TestScenario(
        name="multiplayer_test",
        instances=2,
        roles={0: "player_a", 1: "player_b"}
    )

    # Assign scripts
    scenario.set_script("player_a", "scripts/player_a.py")
    scenario.set_script("player_b", "scripts/player_b.py")

    # Run (launcher handles everything)
    result = await test_runner.run_scenario(scenario)

    # Automatic result collection
    assert result.success
    assert len(result.instance_results) == 2
```

### Step 5: Remove HTTP Client Code

Delete all HTTP client code:

```python
# DELETE THIS
import requests
response = requests.post("http://localhost:8081/rpc", ...)

# REPLACE WITH THIS
from yes_ue_fsd import AutoDriver
driver = AutoDriver()
driver.method()
```

### Step 6: Update Configuration

#### Before (v1.x)

```ini
# Config/DefaultYesUeFsd.ini
[/Script/YesUeFsdEditor.AutoDriverSettings]
ServerPort=8081
bAutoStartServer=true
BindAddress=127.0.0.1
```

#### After (v2.0)

```python
# tests/conftest.py
@pytest.fixture(scope="session")
def project_path():
    """Set your project path."""
    return "D:/MyProject/MyProject.uproject"
```

No server configuration needed!

---

## API Mapping

### Movement Commands

| v1.x (HTTP) | v2.0 (Python) |
|-------------|---------------|
| `POST /rpc` → `autodriver/move_to_location` | `driver.move_to(location)` |
| `POST /rpc` → `autodriver/rotate_to` | `driver.rotate_to(rotation)` |
| `POST /rpc` → `autodriver/look_at` | `driver.look_at(location)` |

### UI Commands

| v1.x (HTTP) | v2.0 (Python) |
|-------------|---------------|
| `POST /rpc` → `autodriver/click_widget` | `driver.click_widget(name)` |
| `POST /rpc` → `autodriver/wait_for_widget` | `driver.wait_for_widget(name, timeout)` |
| `POST /rpc` → `autodriver/find_widget` | `driver.find_widget(name)` |

### Status Queries

| v1.x (HTTP) | v2.0 (Python) |
|-------------|---------------|
| `POST /rpc` → `autodriver/query_status` | `driver.is_executing()` |
| `POST /rpc` → `autodriver/get_location` | `driver.location` |
| `POST /rpc` → `autodriver/get_rotation` | `driver.rotation` |

---

## Breaking Changes

### 1. No HTTP Server

**Before:**
```python
# McpServer automatically started in editor
# Access via http://localhost:8081/rpc
```

**After:**
```python
# No server - direct Python API only
# Access via: from yes_ue_fsd import AutoDriver
```

**Migration:** Remove all HTTP client code.

### 2. JSON-RPC Removed

**Before:**
```python
{
    "jsonrpc": "2.0",
    "method": "tools/call",
    "params": {
        "name": "autodriver/move_to_location",
        "arguments": {...}
    },
    "id": 1
}
```

**After:**
```python
driver.move_to(location=(100, 200, 50))
```

**Migration:** Replace JSON-RPC calls with direct Python method calls.

### 3. Test Execution Model Changed

**Before:**
- Manual editor startup
- External test scripts making HTTP requests
- Manual result collection

**After:**
- pytest-managed editor instances
- Scripts run inside editors
- Automatic result collection

**Migration:** Convert tests to pytest format with fixtures.

### 4. Configuration Files

**Before:**
- `Config/DefaultYesUeFsd.ini` for server settings

**After:**
- `tests/conftest.py` for pytest configuration
- No server configuration needed

**Migration:** Move settings to pytest fixtures.

---

## Compatibility Layer (Temporary)

If you need to maintain v1.x compatibility temporarily:

```python
# compatibility.py - Provides HTTP-like interface for legacy code
import warnings
from yes_ue_fsd import AutoDriver

class LegacyMcpClient:
    """Compatibility layer for v1.x HTTP API."""

    def __init__(self):
        warnings.warn(
            "LegacyMcpClient is deprecated. Use direct Python API.",
            DeprecationWarning
        )
        self.driver = AutoDriver()

    def call_tool(self, name: str, arguments: dict):
        """Emulate HTTP tool call."""
        if name == "autodriver/move_to_location":
            loc = arguments["location"]
            return self.driver.move_to(location=(loc["x"], loc["y"], loc["z"]))
        elif name == "autodriver/click_widget":
            return self.driver.click_widget(arguments["widget_name"])
        # ... add more mappings as needed

# Usage (temporary)
client = LegacyMcpClient()
client.call_tool("autodriver/move_to_location", {"location": {"x": 100, "y": 200, "z": 50}})
```

---

## Removed Components

These files will be deleted in the migration:

- `Source/YesUeFsdEditor/Public/Server/McpServer.h`
- `Source/YesUeFsdEditor/Private/Server/McpServer.cpp`
- `Source/YesUeFsdEditor/Public/Subsystem/McpEditorSubsystem.h`
- `Source/YesUeFsdEditor/Private/Subsystem/McpEditorSubsystem.cpp`
- `Source/YesUeFsdEditor/Tests/McpServerTests.cpp`

**Migration:** No action needed - these are internal components.

---

## Preserved Components

These files are **preserved and enhanced**:

- `Source/YesUeFsd/Public/AutoDriver/**` - All AutoDriver logic
- `Source/YesUeFsd/Private/AutoDriver/**` - Implementation
- `Source/YesUeFsd/Tests/**` - C++ automation tests
- `Content/Python/autodriver_helpers.py` - Python helpers (now wrapped)
- `Content/Python/screenshot_system.py` - Screenshot capture

**Migration:** These continue to work as before!

---

## Testing Checklist

After migration, verify:

- [ ] All dependencies installed (`pip install -r requirements.txt`)
- [ ] Project path configured in `conftest.py`
- [ ] Single-player tests pass (`pytest -m singleplayer`)
- [ ] Multiplayer tests pass (`pytest -m multiplayer`)
- [ ] Reports generated (`pytest --junit-xml=results.xml`)
- [ ] No HTTP client code remaining
- [ ] All imports updated to `from yes_ue_fsd import ...`

---

## Getting Help

If you encounter issues during migration:

1. Check the examples in `Content/Python/tests/`
2. Read `Docs/TwoLayerArchitecture.md`
3. Review API documentation in `Docs/API.md`
4. Open an issue on GitHub

---

## Rollback Plan

If you need to rollback to v1.x:

```bash
# Checkout previous version
git checkout v1.x

# Reinstall dependencies
pip install requests

# Restart MCP server
# Resume HTTP-based testing
```

We recommend migrating incrementally - convert one test at a time while keeping the old system running.
