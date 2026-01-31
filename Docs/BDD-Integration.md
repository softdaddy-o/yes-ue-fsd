# BDD Integration Guide

## Overview

Yes UE FSD now supports Behavior-Driven Development (BDD) using Gherkin syntax and pytest-bdd. This enables writing tests in plain English that are understandable by non-technical stakeholders (QA testers, designers, producers).

## Installation

BDD support is automatically included when you install dependencies:

```bash
cd Content/Python
pip install -r requirements.txt
```

This installs `pytest-bdd>=6.1.0` which provides Gherkin support.

## Quick Start

### 1. Write a Feature File

Create a `.feature` file in `tests/features/`:

```gherkin
# tests/features/player_movement.feature
Feature: Player Movement
  As a game developer
  I want to automate player movement testing
  So that I can verify navigation works correctly

  Scenario: Player navigates to waypoint
    Given a player at spawn location
    When the player moves to waypoint at coordinates (1000, 500, 100)
    Then the player should arrive within 10 seconds
    And the player should be within 50 units of the target
```

### 2. Implement Step Definitions

Create a test file that implements the steps:

```python
# tests/test_bdd_movement.py
import pytest
from pytest_bdd import scenarios, given, when, then, parsers
from yes_ue_fsd import AutoDriver

# Load all scenarios from feature file
scenarios('features/player_movement.feature')

@given("a player at spawn location")
def player_at_spawn(single_editor):
    """Initialize player at spawn location."""
    driver = AutoDriver(player_index=0)
    driver.teleport_to((0, 0, 0))
    return driver

@when(parsers.parse("the player moves to waypoint at coordinates ({x:d}, {y:d}, {z:d})"))
def move_to_waypoint(player_at_spawn, x: int, y: int, z: int):
    """Move player to specified waypoint."""
    driver = player_at_spawn
    driver.move_to(location=(x, y, z))
    driver.wait_for_arrival(timeout=30)

@then(parsers.parse("the player should arrive within {seconds:d} seconds"))
def player_arrives_in_time(player_at_spawn, seconds: int):
    """Verify player arrives within specified time."""
    driver = player_at_spawn
    assert driver.has_arrived(), "Player did not arrive at destination"
```

### 3. Run BDD Tests

```bash
# Run all BDD tests
pytest tests/test_bdd_*.py -v

# Run specific feature
pytest tests/test_bdd_movement.py -v

# Run with BDD-specific reporting
pytest tests/test_bdd_*.py -v --gherkin-terminal-reporter
```

## Gherkin Syntax

### Basic Structure

```gherkin
Feature: <Feature Name>
  <Feature Description>

  Scenario: <Scenario Name>
    Given <precondition>
    When <action>
    Then <expected result>
    And <additional assertion>
```

### Scenario Outlines (Parameterized Tests)

```gherkin
Scenario Outline: Player movement at different speeds
  Given a player at spawn location
  When the player moves to waypoint at coordinates (1000, 1000, 100) with speed <speed>
  Then the player should arrive within <max_time> seconds

  Examples:
    | speed | max_time |
    | 300   | 15       |
    | 600   | 8        |
    | 900   | 5        |
```

This generates 3 separate test cases, one for each row in the Examples table.

### Background (Shared Setup)

```gherkin
Feature: Combat System

  Background:
    Given a player with full health
    And an enemy within attack range

  Scenario: Player attacks enemy
    When the player attacks the enemy
    Then the enemy should take damage

  Scenario: Player takes damage
    When the enemy attacks the player
    Then the player should take damage
```

## Step Definition Patterns

### Simple Steps

```python
@given("a player at spawn location")
def player_at_spawn():
    # Implementation
    pass
```

### Parameterized Steps

```python
# Parse integers
@when(parsers.parse("the player moves to waypoint at coordinates ({x:d}, {y:d}, {z:d})"))
def move_to_waypoint(x: int, y: int, z: int):
    pass

# Parse strings
@when(parsers.parse('the player clicks on "{button_name}" button'))
def click_button(button_name: str):
    pass

# Parse floats
@then(parsers.parse("the health should be {value:f}"))
def verify_health(value: float):
    pass
```

### Regular Expression Steps

```python
import re
from pytest_bdd import parsers

@when(parsers.re(r'the player moves to (?P<location>.+)'))
def move_to_location(location: str):
    pass
```

### Reusable Steps

```python
# Same step can be reused across multiple scenarios
@given("a player at spawn location")
@given("the player is at the starting position")
def player_at_spawn():
    # Implementation
    pass
```

## Best Practices

### 1. Use Shared Context

```python
class TestContext:
    def __init__(self):
        self.player = None
        self.enemy = None
        self.start_time = None

@pytest.fixture
def context():
    return TestContext()

@given("a player at spawn location")
def player_at_spawn(single_editor, context):
    context.player = AutoDriver(player_index=0)
    context.start_time = time.time()
```

### 2. Keep Steps Atomic

Good:
```gherkin
Given a player at spawn location
When the player moves to waypoint at coordinates (1000, 500, 100)
Then the player should arrive at the destination
```

Avoid:
```gherkin
Given a player at spawn location
When the player moves to waypoint, waits, and then checks arrival
Then everything should work
```

### 3. Make Steps Reusable

Write steps that can be combined in different scenarios:

```python
@given("a player at spawn location")
@given("an enemy within attack range")
@when("the player attacks the enemy")
@then("the enemy should take damage")
```

These 4 steps can create dozens of different scenarios.

### 4. Use Descriptive Names

Good:
```gherkin
Scenario: Player navigates to waypoint with obstacles
```

Avoid:
```gherkin
Scenario: Test 1
```

### 5. Focus on Behavior, Not Implementation

Good:
```gherkin
When the player opens the settings menu
Then the graphics quality option should be visible
```

Avoid:
```gherkin
When the player clicks button ID 42 at pixel coordinates (500, 300)
Then widget "WidgetBP_Settings_C" should exist in the viewport
```

## Example Use Cases

### QA Testers

QA team members can write test scenarios without programming knowledge:

```gherkin
Feature: Main Menu
  Scenario: Start new game
    Given the game is at the main menu
    When the player clicks "New Game"
    Then the character creation screen should appear
```

The QA tester writes the scenario, and a developer implements the step definitions once.

### Designers

Game designers can specify expected behavior:

```gherkin
Feature: Boss Fight
  Scenario: Boss enters second phase
    Given the boss has 50% health remaining
    When the boss health drops below 50%
    Then the boss should enter rage mode
    And the boss attack speed should increase by 50%
```

### Living Documentation

Feature files serve as living documentation that stays in sync with the code:

```gherkin
Feature: Inventory System
  The inventory system allows players to collect and manage items.
  Maximum capacity is 20 items. Items can stack if they are stackable.

  Scenario: Adding items to inventory
    Given an empty inventory
    When the player picks up 5 health potions
    Then the inventory should contain 5 health potions
    And they should be stacked in one slot
```

## Integration with Yes UE FSD

### Using AutoDriver in BDD Tests

```python
from yes_ue_fsd import AutoDriver

@given("a player at spawn location")
def player_at_spawn(single_editor):
    driver = AutoDriver(player_index=0)
    driver.teleport_to((0, 0, 0))
    return driver

@when("the player moves forward")
def move_forward(player_at_spawn):
    driver = player_at_spawn
    driver.set_axis_value("MoveForward", 1.0)
```

### Multiplayer BDD Tests

```python
@pytest.mark.multiplayer
@given("two players in the same level")
def two_players(multiplayer_editors):
    player1 = AutoDriver(player_index=0)
    player2 = AutoDriver(player_index=1)
    return player1, player2

@when("player 1 sends a message to player 2")
def send_message(two_players):
    player1, player2 = two_players
    player1.send_chat_message("Hello!")

@then("player 2 should receive the message")
def verify_message_received(two_players):
    player1, player2 = two_players
    messages = player2.get_chat_messages()
    assert "Hello!" in messages
```

## Reporting

### Terminal Reporter

```bash
pytest tests/test_bdd_*.py -v --gherkin-terminal-reporter
```

Output:
```
Feature: Player Movement
  Scenario: Player navigates to waypoint
    Given a player at spawn location ... PASSED
    When the player moves to waypoint at coordinates (1000, 500, 100) ... PASSED
    Then the player should arrive within 10 seconds ... PASSED
```

### HTML Reports

```bash
pytest tests/test_bdd_*.py --html=report.html --self-contained-html
```

Generates an HTML report showing all scenarios and their pass/fail status.

### Cucumber JSON (for CI/CD)

```bash
pytest tests/test_bdd_*.py --cucumberjson=cucumber.json
```

Generates Cucumber-compatible JSON for integration with CI/CD dashboards.

## Troubleshooting

### Step Not Found

**Error:**
```
StepDefinitionNotFoundError: Step definition is not found: Given "a player at spawn location"
```

**Solution:**
Make sure you've imported `scenarios()` and the step is decorated with `@given`, `@when`, or `@then`.

### Multiple Step Definitions

**Error:**
```
StepDefinitionAlreadyUsedError: Step definition "player at spawn location" is already used
```

**Solution:**
Each step pattern can only be defined once. Use fixtures to share logic instead of duplicating steps.

### Feature File Not Found

**Error:**
```
FeatureExecution error: Feature file not found
```

**Solution:**
Check the path in `scenarios()`. It's relative to the test file location.

```python
# If test file is in tests/test_bdd_movement.py
# Feature file should be in tests/features/player_movement.feature
scenarios('features/player_movement.feature')
```

## Advanced Topics

### Tags

```gherkin
@slow @integration
Scenario: Long running test
  Given a player at spawn location
  When the player completes a full playthrough
  Then the game should end successfully
```

Run tagged scenarios:
```bash
pytest tests/ -m "slow"
pytest tests/ -m "not slow"
```

### Hooks

```python
# conftest.py
from pytest_bdd import given, when, then

@pytest.hookimpl(tryfirst=True)
def pytest_bdd_before_scenario(request, feature, scenario):
    print(f"Starting scenario: {scenario.name}")

@pytest.hookimpl(tryfirst=True)
def pytest_bdd_after_scenario(request, feature, scenario):
    print(f"Finished scenario: {scenario.name}")
```

### Custom Types

```python
from pytest_bdd import parsers

@parsers.parse("the player equips a {weapon_type:WeaponType}")
class WeaponTypeParser:
    @staticmethod
    def parse(text):
        return WeaponType[text.upper()]

@when(parsers.parse("the player equips a {weapon_type:WeaponType}"))
def equip_weapon(weapon_type: WeaponType):
    # weapon_type is now a WeaponType enum
    pass
```

## References

- [pytest-bdd Documentation](https://pytest-bdd.readthedocs.io/)
- [Gherkin Reference](https://cucumber.io/docs/gherkin/reference/)
- [Behavior-Driven Development (BDD)](https://cucumber.io/docs/bdd/)

## Next Steps

- Review example scenarios in `tests/features/`
- Implement step definitions for your game's features
- Run BDD tests in CI/CD pipelines
- Share feature files with QA and design teams
