"""
Example Python test script for AutoDriver
Demonstrates basic movement, rotation, and input automation
"""

import unreal
from autodriver_helpers import AutoDriver, wait, find_actor


def test_basic_movement():
    """Test basic movement to a location"""
    print("Test: Basic Movement")

    # Create AutoDriver instance
    driver = AutoDriver(player_index=0)

    # Get current location
    start_pos = driver.location
    print(f"Starting position: {start_pos}")

    # Move to a new location
    target = unreal.Vector(1000.0, 0.0, 100.0)
    print(f"Moving to: {target}")

    success = driver.move_to(target, acceptance_radius=50.0, wait=True, timeout=10.0)

    # Verify movement
    end_pos = driver.location
    distance = unreal.Vector.distance(end_pos, target)

    print(f"End position: {end_pos}")
    print(f"Distance to target: {distance:.2f}")

    assert success, "Movement failed to start"
    assert distance < 100.0, f"Failed to reach target (distance: {distance:.2f})"

    print("✓ Basic movement test passed\n")


def test_rotation():
    """Test rotation to a target"""
    print("Test: Rotation")

    driver = AutoDriver()

    # Rotate to a specific angle
    target_rotation = unreal.Rotator(0.0, 90.0, 0.0)  # Face east
    print(f"Rotating to: {target_rotation}")

    success = driver.rotate_to(target_rotation, wait=True, timeout=5.0)

    current_rotation = driver.rotation
    print(f"Current rotation: {current_rotation}")

    assert success, "Rotation failed"
    print("✓ Rotation test passed\n")


def test_look_at_actor():
    """Test looking at a specific actor"""
    print("Test: Look At Actor")

    driver = AutoDriver()

    # Find a target actor
    target_actor = find_actor("TargetActor")  # Replace with actual actor name

    if target_actor:
        print(f"Found target actor: {target_actor.get_name()}")
        success = driver.look_at_actor(target_actor, wait=True)
        assert success, "Failed to look at actor"
        print("✓ Look at actor test passed\n")
    else:
        print("⚠ Target actor not found, skipping test\n")


def test_navigation():
    """Test navigation queries"""
    print("Test: Navigation")

    driver = AutoDriver()

    # Test reachability
    current_pos = driver.location
    nearby_pos = unreal.Vector(current_pos.x + 500, current_pos.y, current_pos.z)

    is_reachable = driver.is_reachable(nearby_pos)
    print(f"Location {nearby_pos} is {'reachable' if is_reachable else 'not reachable'}")

    # Test path length
    if is_reachable:
        path_length = driver.get_path_length(current_pos, nearby_pos)
        print(f"Path length: {path_length:.2f} units")
        assert path_length > 0, "Invalid path length"

    # Test random location
    random_loc = driver.get_random_location(current_pos, radius=500.0)
    print(f"Random reachable location: {random_loc}")

    print("✓ Navigation test passed\n")


def test_input_actions():
    """Test input actions"""
    print("Test: Input Actions")

    driver = AutoDriver()

    # Press jump button
    print("Pressing Jump button")
    success = driver.press_button("Jump")
    assert success, "Failed to press Jump button"

    wait(0.5)

    # Set movement axis
    print("Setting MoveForward axis")
    driver.set_axis("MoveForward", 1.0)

    wait(1.0)

    driver.set_axis("MoveForward", 0.0)

    print("✓ Input actions test passed\n")


def test_patrol_route():
    """Test following a patrol route"""
    print("Test: Patrol Route")

    driver = AutoDriver()

    # Define patrol points
    start_pos = driver.location
    patrol_points = [
        unreal.Vector(start_pos.x + 500, start_pos.y, start_pos.z),
        unreal.Vector(start_pos.x + 500, start_pos.y + 500, start_pos.z),
        unreal.Vector(start_pos.x, start_pos.y + 500, start_pos.z),
        start_pos
    ]

    print(f"Following patrol route with {len(patrol_points)} points")

    for i, point in enumerate(patrol_points):
        print(f"  Moving to waypoint {i + 1}/{len(patrol_points)}")
        success = driver.move_to(point, acceptance_radius=100.0, wait=True, timeout=10.0)
        assert success, f"Failed to reach waypoint {i + 1}"
        wait(0.5)  # Brief pause at each waypoint

    print("✓ Patrol route test passed\n")


def run_all_tests():
    """Run all test functions"""
    print("=" * 60)
    print("AutoDriver Python Test Suite")
    print("=" * 60 + "\n")

    tests = [
        test_basic_movement,
        test_rotation,
        test_look_at_actor,
        test_navigation,
        test_input_actions,
        test_patrol_route
    ]

    passed = 0
    failed = 0

    for test_func in tests:
        try:
            test_func()
            passed += 1
        except AssertionError as e:
            print(f"✗ Test failed: {e}\n")
            failed += 1
        except Exception as e:
            print(f"✗ Test error: {e}\n")
            failed += 1

    print("=" * 60)
    print(f"Results: {passed} passed, {failed} failed")
    print("=" * 60)


if __name__ == "__main__":
    run_all_tests()
