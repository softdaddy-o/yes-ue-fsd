"""Game state test fixtures."""
import pytest

@pytest.fixture
def paused_game(single_editor):
    """Provide game in paused state."""
    class Game:
        paused = True
    return Game()

@pytest.fixture
def game_at_menu(single_editor):
    """Provide game at main menu."""
    class Game:
        current_screen = "main_menu"
    return Game()
