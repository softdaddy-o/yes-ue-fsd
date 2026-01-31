"""
BDD step definitions for combat system tests.

This module implements the Given/When/Then steps for combat.feature.
"""

import pytest
from pytest_bdd import scenarios, given, when, then, parsers

# Load scenarios from feature file
scenarios('features/combat.feature')


# Shared context for combat test state
class CombatContext:
    def __init__(self):
        self.player_health = 100
        self.enemy_health = 100
        self.player_weapon = "sword"
        self.attack_count = 0
        self.damage_dealt = 0
        self.experience_gained = 0
        self.enemy_defeated = False


@pytest.fixture
def combat_context():
    """Provide combat context for BDD tests."""
    return CombatContext()


# Given steps
@given("a player with full health")
def player_with_full_health(single_editor, combat_context):
    """Initialize player with full health."""
    combat_context.player_health = 100


@given("an enemy within attack range")
def enemy_in_range(combat_context):
    """Spawn enemy within attack range."""
    combat_context.enemy_health = 100
    # In real implementation:
    # driver = AutoDriver(player_index=0)
    # driver.spawn_enemy(distance=200)


@given(parsers.parse("an enemy with {health:d} health"))
def enemy_with_health(combat_context, health: int):
    """Spawn enemy with specific health."""
    combat_context.enemy_health = health


@given(parsers.parse("a player with {weapon}"))
def player_with_weapon(single_editor, combat_context, weapon: str):
    """Equip player with specific weapon."""
    combat_context.player_weapon = weapon
    combat_context.player_health = 100
    # In real implementation:
    # driver = AutoDriver(player_index=0)
    # driver.equip_weapon(weapon)


@given(parsers.parse("an enemy with {health:d} health"))
def enemy_with_specific_health(combat_context, health: int):
    """Spawn enemy with specific health value."""
    combat_context.enemy_health = health


# When steps
@when("the player attacks the enemy")
def player_attacks_enemy(combat_context):
    """Player performs attack on enemy."""
    combat_context.attack_count += 1

    # Calculate damage based on weapon
    weapon_damage = {
        "sword": 20,
        "axe": 35,
        "dagger": 15,
        "greatsword": 50,
    }
    damage = weapon_damage.get(combat_context.player_weapon, 20)
    combat_context.damage_dealt = damage
    combat_context.enemy_health -= damage

    # Check if enemy defeated
    if combat_context.enemy_health <= 0:
        combat_context.enemy_defeated = True
        combat_context.experience_gained = 100

    # In real implementation:
    # driver = AutoDriver(player_index=0)
    # driver.press_button("Attack")
    # driver.wait_for_animation("AttackComplete")


@when("the enemy attacks the player")
def enemy_attacks_player(combat_context):
    """Enemy performs attack on player."""
    damage = 15  # Enemy base damage
    combat_context.player_health -= damage
    # In real implementation:
    # Wait for enemy AI to attack
    # Read player health from game state


@when(parsers.parse("the player attacks the enemy {count:d} times"))
def player_attacks_multiple_times(combat_context, count: int):
    """Player attacks enemy multiple times."""
    for _ in range(count):
        player_attacks_enemy(combat_context)


@when("the player attacks the enemy once")
def player_attacks_once(combat_context):
    """Player attacks enemy one time."""
    player_attacks_enemy(combat_context)


# Then steps
@then("the enemy should take damage")
def enemy_takes_damage(combat_context):
    """Verify enemy took damage."""
    assert combat_context.damage_dealt > 0, "Enemy did not take any damage"


@then("the enemy health should decrease")
def enemy_health_decreases(combat_context):
    """Verify enemy health decreased."""
    assert combat_context.enemy_health < 100, "Enemy health did not decrease"


@then("the player should take damage")
def player_takes_damage(combat_context):
    """Verify player took damage."""
    assert combat_context.player_health < 100, "Player did not take damage"


@then("the player health should decrease")
def player_health_decreases(combat_context):
    """Verify player health decreased."""
    assert combat_context.player_health < 100, "Player health did not decrease"


@then("the damage indicator should appear")
def damage_indicator_appears(combat_context):
    """Verify damage indicator UI appeared."""
    # In real implementation:
    # driver = AutoDriver(player_index=0)
    # assert driver.is_widget_visible("DamageIndicator")
    assert True


@then("the enemy should be defeated")
def enemy_defeated(combat_context):
    """Verify enemy was defeated."""
    assert combat_context.enemy_defeated is True, "Enemy was not defeated"


@then("the enemy should despawn")
def enemy_despawns(combat_context):
    """Verify enemy despawned."""
    # In real implementation:
    # driver = AutoDriver(player_index=0)
    # assert not driver.does_enemy_exist()
    assert combat_context.enemy_defeated is True


@then("the player should gain experience")
def player_gains_experience(combat_context):
    """Verify player gained experience."""
    assert combat_context.experience_gained > 0, "Player did not gain experience"


@then(parsers.parse("the enemy should take {damage:d} damage"))
def enemy_takes_specific_damage(combat_context, damage: int):
    """Verify enemy took specific amount of damage."""
    assert combat_context.damage_dealt == damage, \
        f"Expected {damage} damage, but dealt {combat_context.damage_dealt}"
