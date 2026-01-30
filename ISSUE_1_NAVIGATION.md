# Issue #1: Add Navigation System Integration

**Type:** Feature Request
**Module:** Navigation
**Priority:** High
**Labels:** enhancement, navigation

## Feature Description

Integrate Unreal Engine's Navigation System to enable AI-driven pathfinding for automatic player movement. Currently, the `MoveToLocationCommand` only supports direct movement (adding movement input in a straight line). This feature will add proper navigation with obstacle avoidance and path following.

## Use Case

- Navigate characters around obstacles in complex environments
- Follow navigation paths instead of moving in straight lines
- Automatically handle blocked paths and find alternative routes
- Support navigation queries (is location reachable, find random point, etc.)
- Enable realistic AI-driven player behavior for testing

## Proposed Solution

### Components to Add:

1. **NavigationHelper** - Utility class for navigation queries
   - Check if location is reachable
   - Find random reachable point
   - Get path length
   - Visualize paths for debugging

2. **AI Controller Integration** - Use UE's AIController for pathfinding
   - Create temporary AI controller for navigation
   - Move to location using navigation system
   - Handle path following completion
   - Support move request customization

3. **Enhanced MoveToLocationCommand**
   - Use navigation when `MovementMode::Navigation` is selected
   - Fall back to direct movement if navigation fails
   - Report path-following status
   - Handle path blocked/unreachable scenarios

### Implementation Plan:

1. Create `NavigationHelper` utility class
2. Add AI controller management to `AutoDriverComponent`
3. Update `MoveToLocationCommand` to use navigation system
4. Add navigation debugging visualization
5. Add navigation query methods to component
6. Update documentation with navigation examples

## Acceptance Criteria

- [ ] Can move to location using navigation pathfinding
- [ ] Handles obstacles correctly (goes around, not through)
- [ ] Falls back gracefully if path is unreachable
- [ ] Navigation queries work (reachable check, random point, etc.)
- [ ] Debug visualization available for paths
- [ ] Documentation updated with examples
- [ ] No memory leaks from AI controller creation

## Technical Details

**Dependencies:**
- AIModule
- NavigationSystem
- Existing: AutoDriverComponent, MoveToLocationCommand

**Files to Modify:**
- `AutoDriverComponent.h/.cpp` - Add AI controller management
- `MoveToLocationCommand.cpp` - Implement navigation movement
- `AutoDriverTypes.h` - Add navigation-related types

**Files to Create:**
- `NavigationHelper.h/.cpp` - Navigation utilities
- `AutoDriverAIController.h/.cpp` - Custom AI controller (optional)

## Additional Context

This is a critical feature for realistic automated testing. Direct movement doesn't work well in real game environments with obstacles, stairs, and complex geometry. Navigation system integration will make the auto driver much more practical for actual use.

## References

- UE Navigation System: https://docs.unrealengine.com/5.0/en-US/navigation-system-in-unreal-engine/
- AIController MoveTo: https://docs.unrealengine.com/5.0/en-US/API/Runtime/AIModule/AIController/
