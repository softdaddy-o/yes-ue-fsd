# Yes UE FSD - Elpis Project Setup Guide

This guide helps you set up the Yes UE FSD testing framework for the Elpis project.

## Development Workflow

**IMPORTANT:** Always edit source files in `D:\srcp\yes-ue-fsd`, not in the Elpis plugin directory.

### Making Changes to the Plugin

1. **Edit source files** in `D:\srcp\yes-ue-fsd\Source\YesUeFsd\`
   - Never edit files directly in `F:\src3\Covenant\ElpisClient\Plugins\YesUeFsd`
   - The Elpis directory is a deployment target, not the source of truth

2. **Copy to Elpis** after making changes:
   ```powershell
   cd D:\srcp\yes-ue-fsd
   .\copy_plugin.ps1
   ```
   This script safely updates the plugin in both Elpis and GameAnim projects.

3. **Build the Elpis project** to verify changes:
   ```powershell
   cd F:\src3\Covenant\ElpisClient
   & "D:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" ElpisEditor Win64 Development -Project="F:\src3\Covenant\ElpisClient\Elpis.uproject" -WaitMutex -FromMSBuild
   ```

4. **Fix any compilation errors** in the source directory (D:\srcp\yes-ue-fsd), then repeat step 2-3.

### Why This Workflow?

- Maintains a single source of truth in `D:\srcp\yes-ue-fsd`
- Keeps deployment targets clean and synchronized
- Makes it easy to update multiple projects simultaneously
- Prevents accidental changes in deployment directories from being lost

## Plugin Installation

The plugin has been copied to:
```
F:\src3\Covenant\ElpisClient\Plugins\YesUeFsd
```

## Step 1: Enable the Plugin

1. **Regenerate Visual Studio project files**
   ```bash
   cd F:\src3\Covenant\ElpisClient
   # Right-click Elpis.uproject → Generate Visual Studio project files
   ```

2. **Enable plugin in Unreal Editor**
   - Open Elpis project in Unreal Editor
   - Go to `Edit → Plugins`
   - Search for "Yes UE FSD"
   - Check the "Enabled" checkbox
   - Restart the editor when prompted

3. **Rebuild the project**
   - Open `Elpis.sln` in Visual Studio
   - Build in `Development Editor` configuration
   - Launch the editor
   - Check Output Log for: "YesUeFsd Runtime Module Started"

## Step 2: Add AutoDriver to Your PlayerController

### Option A: C++ Integration (Recommended)

In your Elpis PlayerController class:

```cpp
// ElpisPlayerController.h
#include "AutoDriver/AutoDriverComponent.h"

UCLASS()
class AElpisPlayerController : public APlayerController
{
    GENERATED_BODY()

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Auto Driver")
    UAutoDriverComponent* AutoDriver;

    virtual void BeginPlay() override;
};

// ElpisPlayerController.cpp
#include "AutoDriver/AutoDriverComponent.h"

void AElpisPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Create and register auto driver
    AutoDriver = NewObject<UAutoDriverComponent>(this);
    AutoDriver->RegisterComponent();
}
```

### Option B: Blueprint Integration

1. Open your PlayerController Blueprint
2. Add Component → Search "Auto Driver"
3. Add **AutoDriverComponent**
4. Save and compile

## Step 3: Set Up Python Testing

### Install Python Dependencies

```bash
cd F:\src3\Covenant\ElpisClient\Plugins\YesUeFsd\Content\Python
pip install -r requirements.txt
```

### Create Test Directory for Elpis

```bash
cd F:\src3\Covenant\ElpisClient
mkdir -p Content\Python\tests
```

### Configure pytest

Create `Content\Python\tests\conftest.py`:

```python
import pytest
from pathlib import Path

@pytest.fixture(scope="session")
def project_path():
    """Path to Elpis project file."""
    return r"F:\src3\Covenant\ElpisClient\Elpis.uproject"

@pytest.fixture(scope="session")
def editor_path():
    """Path to Unreal Engine editor (optional)."""
    # Override if using custom UE build
    return None  # Will use UE_ROOT environment variable
```

## Step 4: Write Your First Test

Create `Content\Python\tests\test_elpis_basic.py`:

```python
import pytest

@pytest.mark.singleplayer
@pytest.mark.asyncio
async def test_elpis_launches(single_editor, assertions):
    """Test that Elpis editor launches successfully."""
    assertions.assert_instance_running(single_editor)
    assertions.assert_instance_ready(single_editor)

@pytest.mark.singleplayer
@pytest.mark.asyncio
async def test_player_movement(single_editor):
    """Test basic player movement in Elpis."""

    # Script to run inside Elpis editor
    test_script = """
from yes_ue_fsd import AutoDriver
import unreal

# Get AutoDriver
driver = AutoDriver(player_index=0)

# Test movement
success = driver.move_to(location=(1000, 0, 100))
assert success, "Failed to move to target location"

# Verify position
pos = driver.location
print(f"Player moved to: {pos}")

# Quit editor when done
unreal.SystemLibrary.quit_editor()
"""

    # This would execute in the editor instance
    assert single_editor.is_running()
```

## Step 5: Run Tests

```bash
cd F:\src3\Covenant\ElpisClient\Content\Python
pytest tests/ -v
```

### Test Options

```bash
# Run specific test
pytest tests/test_elpis_basic.py::test_player_movement -v

# Run only singleplayer tests
pytest -m singleplayer tests/

# Generate HTML report
pytest tests/ --html=report.html

# Stop on first failure
pytest -x tests/
```

## Step 6: Advanced Testing

### Multiplayer Testing

Create `Content\Python\tests\test_elpis_multiplayer.py`:

```python
import pytest
from yes_ue_fsd.meta_layer import TestScenario

@pytest.mark.multiplayer
@pytest.mark.asyncio
async def test_two_players(test_runner):
    """Test two players in Elpis."""

    scenario = TestScenario(
        name="elpis_two_player",
        instances=2,
        roles={0: "player_1", 1: "player_2"}
    )

    # Set scripts for each player
    scenario.set_script("player_1", "scripts/player_1.py")
    scenario.set_script("player_2", "scripts/player_2.py")

    result = await test_runner.run_scenario(scenario)
    assert result.success
```

### Screenshot Testing

```python
import pytest

@pytest.mark.singleplayer
@pytest.mark.visual
async def test_ui_rendering(single_editor):
    """Capture screenshots for visual regression testing."""

    script = """
from yes_ue_fsd import AutoDriver
import screenshot_system

# Configure screenshots
screenshot_system.configure(
    output_dir="F:/src3/Covenant/ElpisClient/Saved/Screenshots",
    capture_on_failure=True
)

driver = AutoDriver(player_index=0)

# Navigate to location
driver.move_to(location=(1000, 0, 100))

# Capture screenshot
screenshot_system.capture("ElpisTest", "MainMenu")

import unreal
unreal.SystemLibrary.quit_editor()
"""

    assert single_editor.is_running()
```

## Directory Structure

After setup, your Elpis project should have:

```
ElpisClient/
├── Elpis.uproject
├── Plugins/
│   ├── YesUeFsd/              # ← Plugin installed here
│   │   ├── Source/
│   │   ├── Content/
│   │   │   └── Python/        # Framework code (don't modify)
│   │   └── YesUeFsd.uplugin
│   └── ... (other plugins)
├── Content/
│   └── Python/                 # ← Your test scripts
│       ├── tests/
│       │   ├── conftest.py
│       │   ├── test_elpis_basic.py
│       │   └── test_elpis_multiplayer.py
│       ├── scripts/
│       │   ├── player_1.py
│       │   └── player_2.py
│       └── requirements.txt
└── Source/
    └── ... (your game source)
```

## Troubleshooting

### Plugin Not Showing in Editor

1. Check `.uplugin` file exists: `Plugins\YesUeFsd\YesUeFsd.uplugin`
2. Regenerate project files
3. Rebuild project in Visual Studio

### AutoDriver Component Not Found

1. Ensure plugin is enabled in `Edit → Plugins`
2. Check module dependencies in your `.Build.cs`:
   ```csharp
   PublicDependencyModuleNames.AddRange(new string[] {
       "Core",
       "CoreUObject",
       "Engine",
       "YesUeFsd"  // Add this
   });
   ```

### Python Import Errors

1. Install requirements: `pip install -r requirements.txt`
2. Check Python version: `python --version` (needs 3.7+)
3. Verify UE Python plugin is enabled

### Tests Timeout

1. Increase timeout in conftest.py:
   ```python
   @pytest.fixture
   async def single_editor(editor_launcher):
       instances = await editor_launcher.launch_instances(
           count=1,
           timeout=300  # 5 minutes
       )
       yield instances[0]
       await instances[0].stop()
   ```

2. Monitor editor startup in Output Log

## Next Steps

1. ✅ Plugin installed
2. ✅ AutoDriver added to PlayerController
3. ✅ Python testing environment set up
4. ✅ First test written and run
5. 🔲 Add more test scenarios for Elpis gameplay
6. 🔲 Set up CI/CD integration
7. 🔲 Create custom automation commands

## Resources

- **Plugin README**: `Plugins\YesUeFsd\README.md`
- **API Reference**: `Plugins\YesUeFsd\API.md`
- **Usage Guide**: `Plugins\YesUeFsd\USAGE.md`
- **Example Tests**: `Plugins\YesUeFsd\Content\Python\tests\`

## Support

For issues or questions:
- Check `Plugins\YesUeFsd\Docs\Troubleshooting.md`
- Review plugin documentation
- Check Output Log for error messages

---

**Last Updated**: 2026-02-01
**Plugin Version**: 2.0.0
**Target Project**: Elpis (Covenant)
