Feature: UI Interaction
  As a QA tester
  I want to automate UI interaction testing
  So that I can verify menus and widgets work correctly

  Scenario: Main menu navigation
    Given the game is at the main menu
    When the player clicks on "Play" button
    Then the game should transition to level select
    And the level select UI should be visible

  Scenario: Settings menu interaction
    Given the game is at the main menu
    When the player clicks on "Settings" button
    Then the settings menu should open
    When the player changes the graphics quality to "High"
    And the player clicks "Apply"
    Then the graphics quality should be set to "High"

  Scenario: In-game pause menu
    Given the player is in game
    When the player presses the pause button
    Then the pause menu should appear
    And the game should be paused
    When the player clicks "Resume"
    Then the game should resume
    And the pause menu should disappear
