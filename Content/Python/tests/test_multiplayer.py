"""
Multiplayer test examples

These tests demonstrate multi-instance testing scenarios.
"""

import pytest
from yes_ue_fsd.meta_layer import TestScenario


@pytest.mark.multiplayer
@pytest.mark.asyncio
async def test_two_players_launch(multiplayer_editors, assertions):
    """Test that two editor instances launch successfully."""
    assert len(multiplayer_editors) == 2
    assertions.assert_all_instances_running(multiplayer_editors)


@pytest.mark.multiplayer
@pytest.mark.asyncio
async def test_two_player_interaction(test_runner, editor_launcher):
    """Test two players interacting in a shared world."""

    # Define scenario
    scenario = TestScenario(
        name="two_player_interaction",
        instances=2,
        roles={0: "player_a", 1: "player_b"},
        timeout=120.0
    )

    # Create player A script
    player_a_script = """
from yes_ue_fsd import AutoDriver, instance

print(f"Player A starting on {instance}")

driver = AutoDriver(player_index=0)

# Wait at spawn point
driver.wait(5.0)

# Check if player B is nearby
# (In real scenario, would use UE networking to detect other players)

print("Player A: Test complete")

import unreal
unreal.SystemLibrary.quit_editor()
"""

    # Create player B script
    player_b_script = """
from yes_ue_fsd import AutoDriver, instance

print(f"Player B starting on {instance}")

driver = AutoDriver(player_index=0)

# Move to player A
# (In real scenario, would find player A's location via networking)
driver.move_to(location=(500, 500, 100))

# Interact
driver.press_button("Interact")

print("Player B: Test complete")

import unreal
unreal.SystemLibrary.quit_editor()
"""

    # Write scripts to temp files
    import tempfile
    with tempfile.NamedTemporaryFile(mode='w', suffix='.py', delete=False) as f:
        f.write(player_a_script)
        player_a_path = f.name

    with tempfile.NamedTemporaryFile(mode='w', suffix='.py', delete=False) as f:
        f.write(player_b_script)
        player_b_path = f.name

    # Assign scripts to roles
    scenario.set_script("player_a", player_a_path)
    scenario.set_script("player_b", player_b_path)

    # Run scenario
    result = await test_runner.run_scenario(scenario)

    # Verify results
    assert result.success, f"Test failed: {result.errors}"
    assert len(result.instance_results) == 2
    assert all(r.success for r in result.instance_results)


@pytest.mark.server_required
@pytest.mark.asyncio
async def test_server_and_clients(test_runner, editor_launcher):
    """Test dedicated server with multiple clients."""

    scenario = TestScenario(
        name="server_and_clients",
        instances=3,
        roles={0: "server", 1: "client", 2: "client"},
        timeout=180.0
    )

    # Shared client script
    client_script = """
from yes_ue_fsd import AutoDriver, instance

print(f"Client starting on {instance}")

driver = AutoDriver(player_index=0)

# Connect to server (handled by UE networking)
driver.wait(10.0)

# Perform client actions
driver.move_to(location=(1000, 1000, 100))
driver.press_button("Jump")

print(f"Client {instance.instance_id}: Test complete")

import unreal
unreal.SystemLibrary.quit_editor()
"""

    # Server script
    server_script = """
from yes_ue_fsd import instance

print(f"Server starting on {instance}")

# Server just waits for clients
import time
time.sleep(15.0)

print("Server: Test complete")

import unreal
unreal.SystemLibrary.quit_editor()
"""

    # Write scripts
    import tempfile
    with tempfile.NamedTemporaryFile(mode='w', suffix='.py', delete=False) as f:
        f.write(server_script)
        server_path = f.name

    with tempfile.NamedTemporaryFile(mode='w', suffix='.py', delete=False) as f:
        f.write(client_script)
        client_path = f.name

    scenario.set_script("server", server_path)
    scenario.set_script("client", client_path)

    # Run scenario
    result = await test_runner.run_scenario(scenario)

    # Verify
    assert result.success
    assert len(result.instance_results) == 3


@pytest.mark.stress
@pytest.mark.slow
@pytest.mark.asyncio
async def test_many_players(test_runner, editor_launcher):
    """Stress test with many simultaneous players."""

    scenario = TestScenario(
        name="many_players_stress",
        instances=5,  # Can be increased for real stress testing
        timeout=300.0
    )

    # All players run the same script
    player_script = """
from yes_ue_fsd import AutoDriver, instance
import random

print(f"Player starting on {instance}")

driver = AutoDriver(player_index=0)

# Random movement
for i in range(5):
    x = random.randint(-1000, 1000)
    y = random.randint(-1000, 1000)
    driver.move_to(location=(x, y, 100))
    driver.wait(1.0)

print(f"Player {instance.instance_id}: Test complete")

import unreal
unreal.SystemLibrary.quit_editor()
"""

    import tempfile
    with tempfile.NamedTemporaryFile(mode='w', suffix='.py', delete=False) as f:
        f.write(player_script)
        script_path = f.name

    scenario.set_script_for_all(script_path)

    # Run scenario
    result = await test_runner.run_scenario(scenario)

    # Verify all completed
    assert result.success
    assert len(result.instance_results) == 5

    # Check performance metrics
    for instance_result in result.instance_results:
        print(f"Instance {instance_result.instance_id} metrics: {instance_result.metrics}")
