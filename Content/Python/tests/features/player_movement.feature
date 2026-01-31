Feature: Player Movement
  As a game developer
  I want to automate player movement testing
  So that I can verify navigation works correctly

  Scenario: Player navigates to waypoint
    Given a player at spawn location
    When the player moves to waypoint at coordinates (1000, 500, 100)
    Then the player should arrive within 10 seconds
    And the player should be within 50 units of the target

  Scenario: Player navigates through multiple waypoints
    Given a player at spawn location
    When the player moves to waypoint at coordinates (1000, 0, 100)
    And the player moves to waypoint at coordinates (2000, 500, 100)
    And the player moves to waypoint at coordinates (1500, 1500, 100)
    Then all waypoints should be reached
    And the total travel time should be less than 30 seconds

  Scenario: Player movement with obstacles
    Given a player at spawn location
    And there are obstacles in the environment
    When the player moves to waypoint at coordinates (3000, 3000, 100)
    Then the player should navigate around obstacles
    And the player should arrive at the destination

  Scenario Outline: Player movement at different speeds
    Given a player at spawn location
    When the player moves to waypoint at coordinates (1000, 1000, 100) with speed <speed>
    Then the player should arrive within <max_time> seconds

    Examples:
      | speed | max_time |
      | 300   | 15       |
      | 600   | 8        |
      | 900   | 5        |
