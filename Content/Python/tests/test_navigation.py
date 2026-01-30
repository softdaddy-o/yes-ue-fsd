"""
Pytest tests for AutoDriver navigation functionality
"""

import pytest
import unreal
from autodriver_helpers import AutoDriver
from pytest_helpers import assertions


@pytest.mark.navigation
def test_is_location_reachable(autodriver):
    """Test reachability checking"""
    current = autodriver.location

    # Nearby location should be reachable
    nearby = unreal.Vector(current.x + 100, current.y, current.z)
    assert autodriver.is_reachable(nearby), "Nearby location should be reachable"

    # Very far location might not be reachable
    far_away = unreal.Vector(current.x + 100000, current.y, current.z)
    # Just verify the query doesn't crash
    _ = autodriver.is_reachable(far_away)


@pytest.mark.navigation
def test_get_path_length(autodriver):
    """Test path length calculation"""
    current = autodriver.location
    target = unreal.Vector(current.x + 500, current.y, current.z)

    path_length = autodriver.get_path_length(current, target)

    # Path should be at least as long as straight-line distance
    straight_distance = unreal.Vector.distance(current, target)

    if path_length > 0:  # If path exists
        assert path_length >= straight_distance * 0.8, \
            f"Path length {path_length} seems too short for straight distance {straight_distance}"


@pytest.mark.navigation
def test_get_random_location(autodriver):
    """Test getting random reachable locations"""
    current = autodriver.location
    radius = 500.0

    # Get multiple random locations
    locations = []
    for _ in range(5):
        loc = autodriver.get_random_location(current, radius)
        locations.append(loc)

        # Verify it's within radius
        distance = unreal.Vector.distance(current, loc)
        assert distance <= radius * 1.1, \
            f"Random location {distance:.2f} units away, exceeds radius {radius}"

    # Verify we got different locations (randomness)
    unique_locations = len(set((l.x, l.y, l.z) for l in locations))
    assert unique_locations > 1, "Random locations should vary"


@pytest.mark.navigation
def test_path_to_reachable_location(autodriver):
    """Test moving to a verified reachable location"""
    current = autodriver.location
    target = unreal.Vector(current.x + 300, current.y + 300, current.z)

    # First verify it's reachable
    assertions.assert_location_reachable(autodriver, target)

    # Then move to it
    success = autodriver.move_to(target, wait=True, timeout=10.0)

    assert success, "Failed to move to reachable location"
    assertions.assert_reached_location(autodriver, target)
