"""World-related test fixtures."""
import pytest

@pytest.fixture
def clean_world(single_editor):
    """Provide clean world state."""
    # Skeleton implementation
    yield
    # Cleanup after test

@pytest.fixture
def world_with_enemies(single_editor):
    """Provide world with spawned enemies."""
    class World:
        enemies = [f"Enemy_{i}" for i in range(5)]
    return World()
