Feature: Combat System
  As a game developer
  I want to automate combat testing
  So that I can verify combat mechanics work correctly

  Scenario: Player attacks enemy
    Given a player with full health
    And an enemy within attack range
    When the player attacks the enemy
    Then the enemy should take damage
    And the enemy health should decrease

  Scenario: Player takes damage
    Given a player with full health
    And an enemy within attack range
    When the enemy attacks the player
    Then the player should take damage
    And the player health should decrease
    And the damage indicator should appear

  Scenario: Player defeats enemy
    Given a player with full health
    And an enemy with 10 health
    When the player attacks the enemy 3 times
    Then the enemy should be defeated
    And the enemy should despawn
    And the player should gain experience

  Scenario Outline: Different weapon damage
    Given a player with <weapon>
    And an enemy with 100 health
    When the player attacks the enemy once
    Then the enemy should take <damage> damage

    Examples:
      | weapon      | damage |
      | sword       | 20     |
      | axe         | 35     |
      | dagger      | 15     |
      | greatsword  | 50     |
