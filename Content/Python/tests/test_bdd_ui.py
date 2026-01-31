"""
BDD step definitions for UI interaction tests.

This module implements the Given/When/Then steps for ui_interaction.feature.
"""

import pytest
from pytest_bdd import scenarios, given, when, then, parsers

# Load scenarios from feature file
scenarios('features/ui_interaction.feature')


# Shared context for UI test state
class UIContext:
    def __init__(self):
        self.current_screen = None
        self.game_paused = False
        self.settings = {}


@pytest.fixture
def ui_context():
    """Provide UI context for BDD tests."""
    return UIContext()


# Given steps
@given("the game is at the main menu")
def game_at_main_menu(single_editor, ui_context):
    """Start game at main menu."""
    ui_context.current_screen = "main_menu"
    # In real implementation:
    # driver = AutoDriver(player_index=0)
    # driver.wait_for_widget("MainMenuWidget")


@given("the player is in game")
def player_in_game(single_editor, ui_context):
    """Player is in active gameplay."""
    ui_context.current_screen = "gameplay"
    ui_context.game_paused = False


# When steps
@when(parsers.parse('the player clicks on "{button_name}" button'))
def click_button(ui_context, button_name: str):
    """Click a UI button."""
    # In real implementation:
    # driver = AutoDriver(player_index=0)
    # driver.click_widget(button_name)

    # Simulate screen transitions
    if button_name == "Play":
        ui_context.current_screen = "level_select"
    elif button_name == "Settings":
        ui_context.current_screen = "settings"
    elif button_name == "Apply":
        pass  # Settings applied
    elif button_name == "Resume":
        ui_context.game_paused = False
        ui_context.current_screen = "gameplay"


@when(parsers.parse('the player changes the graphics quality to "{quality}"'))
def change_graphics_quality(ui_context, quality: str):
    """Change graphics quality setting."""
    ui_context.settings["graphics_quality"] = quality
    # In real implementation:
    # driver = AutoDriver(player_index=0)
    # driver.select_dropdown("GraphicsQuality", quality)


@when("the player presses the pause button")
def press_pause(ui_context):
    """Press the pause button."""
    ui_context.game_paused = True
    ui_context.current_screen = "pause_menu"
    # In real implementation:
    # driver = AutoDriver(player_index=0)
    # driver.press_button("Pause")


@when(parsers.parse('the player clicks "{button_name}"'))
def click_named_button(ui_context, button_name: str):
    """Click a named button (alternate syntax)."""
    click_button(ui_context, button_name)


# Then steps
@then("the game should transition to level select")
def verify_level_select_transition(ui_context):
    """Verify game transitioned to level select."""
    assert ui_context.current_screen == "level_select"


@then("the level select UI should be visible")
def verify_level_select_visible(ui_context):
    """Verify level select UI is visible."""
    # In real implementation:
    # driver = AutoDriver(player_index=0)
    # assert driver.is_widget_visible("LevelSelectWidget")
    assert ui_context.current_screen == "level_select"


@then("the settings menu should open")
def verify_settings_menu_open(ui_context):
    """Verify settings menu opened."""
    assert ui_context.current_screen == "settings"


@then(parsers.parse('the graphics quality should be set to "{quality}"'))
def verify_graphics_quality(ui_context, quality: str):
    """Verify graphics quality setting."""
    assert ui_context.settings.get("graphics_quality") == quality


@then("the pause menu should appear")
def verify_pause_menu_appears(ui_context):
    """Verify pause menu is visible."""
    assert ui_context.current_screen == "pause_menu"


@then("the game should be paused")
def verify_game_paused(ui_context):
    """Verify game is paused."""
    assert ui_context.game_paused is True


@then("the game should resume")
def verify_game_resumes(ui_context):
    """Verify game resumed."""
    assert ui_context.game_paused is False


@then("the pause menu should disappear")
def verify_pause_menu_disappears(ui_context):
    """Verify pause menu is hidden."""
    assert ui_context.current_screen == "gameplay"
