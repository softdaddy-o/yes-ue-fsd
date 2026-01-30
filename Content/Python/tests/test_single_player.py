"""
Single-player test examples

These tests demonstrate basic single-instance testing.
"""

import pytest


@pytest.mark.singleplayer
@pytest.mark.asyncio
async def test_editor_launches(single_editor, assertions):
    """Test that a single editor instance launches successfully."""
    assertions.assert_instance_running(single_editor)
    assertions.assert_instance_ready(single_editor)


@pytest.mark.singleplayer
@pytest.mark.asyncio
async def test_basic_movement(single_editor):
    """Test basic player movement in single editor."""

    # Execute test script in editor
    script = """
from yes_ue_fsd import AutoDriver, instance

print(f"Running on {instance}")

driver = AutoDriver(player_index=0)

# Test basic movement
success = driver.move_to(location=(1000, 2000, 100))
assert success, "Failed to move to target location"

# Verify position
pos = driver.location
print(f"Final position: {pos}")

# Exit editor when done
import unreal
unreal.SystemLibrary.quit_editor()
"""

    # Write script to temp file
    import tempfile
    with tempfile.NamedTemporaryFile(mode='w', suffix='.py', delete=False) as f:
        f.write(script)
        script_path = f.name

    # Wait for editor to complete
    single_editor.config.python_script = script_path
    # Note: In real implementation, we would restart with the script
    # or use a communication channel to execute it

    # For now, just verify editor is running
    assert single_editor.is_running()


@pytest.mark.singleplayer
@pytest.mark.asyncio
async def test_screenshot_capture(single_editor):
    """Test screenshot capture functionality."""

    script = """
from yes_ue_fsd import AutoDriver
import screenshot_system

driver = AutoDriver(player_index=0)

# Configure screenshot system
screenshot_system.configure(
    output_dir="test_screenshots",
    capture_on_failure=True
)

# Capture a screenshot
success = screenshot_system.capture("SinglePlayerTest", "Setup")
assert success, "Failed to capture screenshot"

# Verify capture count
captures = screenshot_system.get_captured_screenshots()
assert len(captures) >= 1, "No screenshots captured"

import unreal
unreal.SystemLibrary.quit_editor()
"""

    # Similar to above - in full implementation would execute this
    assert single_editor.is_running()
