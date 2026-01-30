"""
Pytest integration helpers for AutoDriver testing
Provides fixtures and utilities for test automation
"""

import pytest
import unreal
from autodriver_helpers import AutoDriver, ActionRecorder, ActionPlayer


@pytest.fixture
def autodriver():
    """Fixture providing AutoDriver instance

    Yields:
        AutoDriver instance for player 0
    """
    driver = AutoDriver(player_index=0)
    yield driver
    # Cleanup - stop any running commands
    driver.stop()


@pytest.fixture
def recorder():
    """Fixture providing ActionRecorder instance

    Yields:
        ActionRecorder instance
    """
    rec = ActionRecorder(player_index=0)
    yield rec
    # Cleanup - stop recording if active
    if rec.recorder and rec.recorder.is_recording():
        rec.stop()


@pytest.fixture
def player():
    """Fixture providing ActionPlayer instance

    Yields:
        ActionPlayer instance
    """
    play = ActionPlayer(player_index=0)
    yield play
    # Cleanup - stop playback if active
    if play.is_playing:
        play.stop()


@pytest.fixture
def starting_position(autodriver):
    """Fixture storing starting position

    Args:
        autodriver: AutoDriver fixture

    Returns:
        Starting position vector
    """
    return autodriver.location


class AutoDriverAssertions:
    """Custom assertions for AutoDriver testing"""

    @staticmethod
    def assert_reached_location(driver: AutoDriver,
                               target: unreal.Vector,
                               tolerance: float = 100.0):
        """Assert that driver reached target location

        Args:
            driver: AutoDriver instance
            target: Target location
            tolerance: Acceptable distance

        Raises:
            AssertionError: If not within tolerance
        """
        current = driver.location
        distance = unreal.Vector.distance(current, target)

        assert distance <= tolerance, \
            f"Failed to reach target. Distance: {distance:.2f}, Tolerance: {tolerance:.2f}"

    @staticmethod
    def assert_facing_direction(driver: AutoDriver,
                               target_yaw: float,
                               tolerance: float = 5.0):
        """Assert that driver is facing a direction

        Args:
            driver: AutoDriver instance
            target_yaw: Target yaw angle
            tolerance: Acceptable angle difference

        Raises:
            AssertionError: If not within tolerance
        """
        current_yaw = driver.rotation.yaw
        diff = abs(current_yaw - target_yaw)

        # Handle 360-degree wrap
        if diff > 180:
            diff = 360 - diff

        assert diff <= tolerance, \
            f"Not facing target direction. Yaw: {current_yaw:.1f}°, Target: {target_yaw:.1f}°, Diff: {diff:.1f}°"

    @staticmethod
    def assert_location_reachable(driver: AutoDriver,
                                  location: unreal.Vector):
        """Assert that a location is reachable

        Args:
            driver: AutoDriver instance
            location: Location to check

        Raises:
            AssertionError: If not reachable
        """
        reachable = driver.is_reachable(location)
        assert reachable, f"Location {location} is not reachable"

    @staticmethod
    def assert_command_executing(driver: AutoDriver,
                                 should_be_executing: bool = True):
        """Assert command execution state

        Args:
            driver: AutoDriver instance
            should_be_executing: Expected state

        Raises:
            AssertionError: If state doesn't match
        """
        is_executing = driver.is_executing()

        if should_be_executing:
            assert is_executing, "Command should be executing but isn't"
        else:
            assert not is_executing, "Command should not be executing but is"


# Export assertions for easy access
assertions = AutoDriverAssertions()


def pytest_configure(config):
    """Pytest configuration hook

    Args:
        config: Pytest config object
    """
    # Register custom markers
    config.addinivalue_line(
        "markers", "movement: marks tests that involve movement"
    )
    config.addinivalue_line(
        "markers", "rotation: marks tests that involve rotation"
    )
    config.addinivalue_line(
        "markers", "input: marks tests that involve input simulation"
    )
    config.addinivalue_line(
        "markers", "navigation: marks tests that involve navigation queries"
    )
    config.addinivalue_line(
        "markers", "recording: marks tests that involve recording/playback"
    )
    config.addinivalue_line(
        "markers", "slow: marks tests that take longer to execute"
    )


def pytest_collection_modifyitems(config, items):
    """Modify test collection

    Args:
        config: Pytest config
        items: Test items
    """
    # Add skip marker for tests requiring specific setup
    for item in items:
        if "requires_actor" in item.keywords:
            skip_marker = pytest.mark.skip(reason="Requires specific actor in level")
            item.add_marker(skip_marker)
