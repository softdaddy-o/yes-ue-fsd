"""
pytest configuration for Yes UE FSD test framework

This module provides fixtures and configuration for pytest-based
automated testing with multiple Unreal Engine instances.
"""

import asyncio
import pytest
from pathlib import Path

from yes_ue_fsd.meta_layer import EditorLauncher, TestRunner, ResultAggregator


# Configure asyncio for pytest
@pytest.fixture(scope="session")
def event_loop():
    """Create event loop for async tests."""
    loop = asyncio.get_event_loop_policy().new_event_loop()
    yield loop
    loop.close()


@pytest.fixture(scope="session")
def project_path():
    """
    Get project path.

    Override this fixture in your conftest.py to set your project path.
    """
    # Default: look for .uproject in parent directories
    current = Path(__file__).parent
    for parent in [current] + list(current.parents):
        uprojects = list(parent.glob("*.uproject"))
        if uprojects:
            return str(uprojects[0])

    raise FileNotFoundError("Could not find .uproject file. Override project_path fixture.")


@pytest.fixture(scope="session")
async def editor_launcher(project_path):
    """
    Provide editor launcher for test session.

    This launcher is shared across all tests in the session.
    """
    launcher = EditorLauncher(project_path=project_path)
    yield launcher
    # Cleanup: shutdown any remaining instances
    await launcher.shutdown_all()


@pytest.fixture
async def single_editor(editor_launcher):
    """
    Provide single editor instance.

    This fixture launches one editor and cleans it up after the test.
    """
    instances = await editor_launcher.launch_instances(count=1)
    yield instances[0]
    await instances[0].stop()


@pytest.fixture
async def multiplayer_editors(editor_launcher):
    """
    Provide two editor instances for multiplayer testing.

    This fixture launches two editors (client roles) and cleans them up.
    """
    instances = await editor_launcher.launch_instances(
        count=2,
        roles=["client", "client"]
    )
    yield instances
    await asyncio.gather(*(instance.stop() for instance in instances))


@pytest.fixture
async def server_and_clients(editor_launcher):
    """
    Provide server and two client instances.

    This fixture launches one server and two clients.
    """
    instances = await editor_launcher.launch_instances(
        count=3,
        roles=["server", "client", "client"]
    )
    yield instances
    await asyncio.gather(*(instance.stop() for instance in instances))


@pytest.fixture
async def test_runner(editor_launcher):
    """
    Provide test runner.

    Use this for complex multi-instance test scenarios.
    """
    return TestRunner(editor_launcher)


@pytest.fixture(scope="session")
def result_aggregator():
    """
    Provide result aggregator for session.

    Use this to collect and report test results.
    """
    return ResultAggregator(output_dir="test_results")


# Custom pytest markers
def pytest_configure(config):
    """Register custom markers."""
    config.addinivalue_line(
        "markers", "singleplayer: mark test as single-player test"
    )
    config.addinivalue_line(
        "markers", "multiplayer: mark test as multiplayer test"
    )
    config.addinivalue_line(
        "markers", "server_required: mark test as requiring a dedicated server"
    )
    config.addinivalue_line(
        "markers", "slow: mark test as slow running"
    )
    config.addinivalue_line(
        "markers", "stress: mark test as stress/performance test"
    )
    config.addinivalue_line(
        "markers", "bdd: mark test as BDD/Gherkin test"
    )
    config.addinivalue_line(
        "markers", "visual: mark test as visual regression test"
    )
    config.addinivalue_line(
        "markers", "performance: mark test as performance profiling test"
    )
    config.addinivalue_line(
        "markers", "flaky: mark test as potentially flaky (will retry on failure)"
    )


# Custom assertions
class AutoDriverAssertions:
    """Custom assertions for AutoDriver tests."""

    @staticmethod
    def assert_instance_running(instance):
        """Assert that an instance is running."""
        assert instance.is_running(), f"Instance {instance.config.instance_id} is not running"

    @staticmethod
    def assert_all_instances_running(instances):
        """Assert that all instances are running."""
        for instance in instances:
            AutoDriverAssertions.assert_instance_running(instance)

    @staticmethod
    def assert_instance_ready(instance):
        """Assert that an instance is ready for automation."""
        assert instance.is_ready(), f"Instance {instance.config.instance_id} is not ready"


@pytest.fixture
def assertions():
    """Provide custom assertions."""
    return AutoDriverAssertions
