"""Player-related test fixtures."""
import pytest

@pytest.fixture
def player_with_full_health(single_editor):
    """Provide player with full health."""
    # Skeleton implementation
    class Player:
        health = 100
        max_health = 100
    return Player()

@pytest.fixture
def player_at_spawn(single_editor):
    """Provide player at spawn location."""
    class Player:
        position = (0, 0, 0)
    return Player()

@pytest.fixture
def player_with_weapon(single_editor):
    """Provide player with equipped weapon."""
    class Player:
        weapon = "sword"
    return Player()
