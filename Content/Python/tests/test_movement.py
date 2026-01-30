"""
Pytest tests for AutoDriver movement functionality
"""

import pytest
import unreal
from autodriver_helpers import AutoDriver
from pytest_helpers import assertions


@pytest.mark.movement
def test_move_to_location(autodriver):
    """Test moving to a specific location"""
    target = unreal.Vector(1000.0, 0.0, 100.0)
    success = autodriver.move_to(target, wait=True, timeout=10.0)

    assert success, "Movement command failed"
    assertions.assert_reached_location(autodriver, target, tolerance=100.0)


@pytest.mark.movement
def test_move_to_unreachable_location(autodriver):
    """Test moving to an unreachable location fails gracefully"""
    # Very far away location that's likely unreachable
    unreachable = unreal.Vector(999999.0, 999999.0, 999999.0)

    # Should either fail to start or timeout
    success = autodriver.move_to(unreachable, wait=True, timeout=2.0)
    # Test should complete without crashing


@pytest.mark.movement
@pytest.mark.parametrize("speed", [0.5, 1.0, 1.5, 2.0])
def test_move_with_different_speeds(autodriver, starting_position, speed):
    """Test movement with different speed multipliers"""
    target = unreal.Vector(
        starting_position.x + 500,
        starting_position.y,
        starting_position.z
    )

    success = autodriver.move_to(target, speed=speed, wait=True, timeout=15.0)

    assert success, f"Movement with speed {speed}x failed"
    assertions.assert_reached_location(autodriver, target)


@pytest.mark.movement
def test_stop_movement(autodriver):
    """Test stopping movement mid-execution"""
    far_target = unreal.Vector(5000.0, 5000.0, 100.0)

    # Start movement but don't wait
    autodriver.move_to(far_target, wait=False)

    # Verify it started
    import time
    time.sleep(0.5)
    assertions.assert_command_executing(autodriver, should_be_executing=True)

    # Stop it
    autodriver.stop()

    # Verify it stopped
    time.sleep(0.2)
    assertions.assert_command_executing(autodriver, should_be_executing=False)


@pytest.mark.movement
@pytest.mark.slow
def test_patrol_route(autodriver, starting_position):
    """Test following a multi-point patrol route"""
    waypoints = [
        unreal.Vector(starting_position.x + 300, starting_position.y, starting_position.z),
        unreal.Vector(starting_position.x + 300, starting_position.y + 300, starting_position.z),
        unreal.Vector(starting_position.x, starting_position.y + 300, starting_position.z),
        starting_position
    ]

    for i, waypoint in enumerate(waypoints):
        success = autodriver.move_to(waypoint, wait=True, timeout=10.0)
        assert success, f"Failed to reach waypoint {i + 1}"
        assertions.assert_reached_location(autodriver, waypoint, tolerance=150.0)
