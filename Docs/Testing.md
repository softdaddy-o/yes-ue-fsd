# Testing Guide for Yes UE FSD

This document describes the comprehensive testing infrastructure for the Yes UE FSD plugin.

## Table of Contents

1. [Test Categories](#test-categories)
2. [Running Tests](#running-tests)
3. [CI/CD Integration](#cicd-integration)
4. [Coverage Analysis](#coverage-analysis)
5. [Writing New Tests](#writing-new-tests)

---

## Test Categories

### 1. C++ Unit Tests

Located in `Source/YesUeFsd/Tests/` and `Source/YesUeFsdEditor/Tests/`.

#### AutoDriverComponent Tests (`AutoDriverComponentTests.cpp`)
- Component creation and lifecycle
- Enable/disable functionality
- Movement commands (MoveToLocation, RotateTo)
- Command stopping and queuing
- Input simulation
- Callback execution
- Speed modifiers

#### Navigation Tests (`NavigationTests.cpp`)
- NavigationHelper creation
- NavigationCache functionality
- LRU eviction
- Cache invalidation
- Reachability queries
- Path length calculations
- Random location generation
- Cache performance benchmarks
- Cache statistics

#### HTTP Server Tests (`McpServerTests.cpp`)
- Server creation and lifecycle
- Start/stop functionality
- Port validation
- JSON-RPC request parsing
- JSON-RPC response generation
- Error handling
- Endpoint definition
- Invalid JSON handling

#### Integration Tests (`IntegrationTests.cpp`)
- Complete movement scenarios
- Navigation + movement combinations
- Recording and playback workflows
- Multi-command sequences with callbacks
- Subsystem coordination
- Input simulation sequences
- Performance under load
- End-to-end scenarios (basic and advanced)

#### Performance Tests (`PerformanceTests.cpp`)
- Component creation benchmarks
- Command execution overhead
- Tick performance (multi-component)
- Navigation cache performance
- Input simulation performance
- Memory usage analysis
- Rotation calculation performance
- Concurrent operations performance
- Stats system overhead

#### Memory Leak Tests (`MemoryLeakTests.cpp`)
- Component lifecycle memory management
- Command execution memory leaks
- Navigation cache growth
- Recording/playback memory retention
- Callback retention
- Subsystem lifecycle

#### Thread Safety Tests (`ThreadSafetyTests.cpp`)
- Navigation cache concurrent access
- Component command queue thread safety
- Subsystem access thread safety
- Callback execution thread safety
- Concurrent component operations
- Cache statistics thread safety
- Input simulation thread safety

---

### 2. Python Integration Tests

Located in `Content/Python/tests/`.

#### Movement Tests (`test_movement.py`)
- Basic movement commands
- Movement at different speeds
- Stopping movement
- Patrol route following

#### Navigation Tests (`test_navigation.py`)
- Location reachability queries
- Path length calculations
- Random location generation
- Navigation query integration

#### Example Tests (`test_example.py`)
- Standalone test examples
- Movement, rotation, and look-at commands
- Input simulation
- Navigation integration

---

## Running Tests

### Running All C++ Tests

```bash
# From Unreal Editor
Automation RunTests YesUeFsd

# From command line
UnrealEditor-Cmd.exe -ExecCmds="Automation RunTests YesUeFsd" -unattended -nopause -NullRHI -log
```

### Running Specific Test Categories

```bash
# Component tests only
Automation RunTests YesUeFsd.AutoDriver.Component

# Navigation tests only
Automation RunTests YesUeFsd.Navigation

# Performance tests
Automation RunTests YesUeFsd.Performance

# Integration tests
Automation RunTests YesUeFsd.Integration

# Memory leak tests
Automation RunTests YesUeFsd.MemoryLeak

# Thread safety tests
Automation RunTests YesUeFsd.ThreadSafety
```

### Running Python Tests

```bash
cd Content/Python
pytest tests/ -v

# With coverage
pytest tests/ --cov=. --cov-report=html --cov-report=term

# Specific test file
pytest tests/test_movement.py -v

# Specific test function
pytest tests/test_movement.py::test_basic_movement -v
```

### Running Tests with Markers

```python
# Movement tests only
pytest -m movement

# Navigation tests only
pytest -m navigation

# Slow tests only
pytest -m slow

# Skip slow tests
pytest -m "not slow"
```

---

## CI/CD Integration

The project uses GitHub Actions for continuous integration.

### Workflow File

`.github/workflows/tests.yml` defines the CI pipeline with the following jobs:

1. **cpp-tests**: Runs all C++ automation tests
2. **python-tests**: Runs Python integration tests with coverage
3. **performance-tests**: Runs performance benchmarks
4. **memory-leak-tests**: Runs memory leak detection
5. **integration-tests**: Runs integration tests
6. **thread-safety-tests**: Runs thread safety tests
7. **test-summary**: Combines results and posts to PR

### Triggering CI

CI runs automatically on:
- Push to `master` or `develop` branches
- Pull requests to `master` or `develop`
- Manual workflow dispatch

### Viewing Results

- Test results are uploaded as artifacts
- Coverage reports are sent to Codecov
- PR comments include test summaries

---

## Coverage Analysis

### Coverage Goals

- **Target**: 80%+ code coverage
- **Threshold**: 2% tolerance
- **Patch Coverage**: 75%+ for new code

### Python Coverage

Generated using `pytest-cov`:

```bash
cd Content/Python
pytest tests/ --cov=. --cov-report=html
# Open htmlcov/index.html to view results
```

### C++ Coverage

Unreal Engine automation framework tracks test execution. Use stats commands:

```
stat AutoDriver
stat AutoDriverDetailed
```

### Codecov Integration

Configuration in `.codecov.yml`:
- Automatic PR comments with coverage diffs
- Coverage badges in README
- Trend tracking over time

---

## Writing New Tests

### C++ Test Template

```cpp
#include "Misc/AutomationTest.h"
#include "AutoDriver/AutoDriverComponent.h"
#include "Tests/AutomationCommon.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FYourTestName,
    "YesUeFsd.Category.TestName",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FYourTestName::RunTest(const FString& Parameters)
{
    // Setup
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    if (!TestNotNull(TEXT("World Created"), World))
    {
        return false;
    }

    // Test logic
    // ...

    // Assertions
    TestTrue(TEXT("Description"), bCondition);
    TestEqual(TEXT("Description"), ActualValue, ExpectedValue);
    TestNotNull(TEXT("Description"), Pointer);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
```

### Python Test Template

```python
import pytest
from autodriver_helpers import AutoDriver

@pytest.mark.category_name
def test_your_feature(autodriver, starting_position):
    """Test description."""
    # Setup
    driver = autodriver

    # Execute
    success = driver.move_to_location(100, 0, 0)

    # Assert
    assert success, "Movement should succeed"

    # Wait for completion
    driver.wait(2.0)

    # Verify
    final_pos = starting_position
    assert final_pos.x > 50, "Should have moved forward"
```

### Test Best Practices

1. **Isolation**: Each test should be independent
2. **Cleanup**: Clean up resources (use fixtures)
3. **Descriptive Names**: Use clear, descriptive test names
4. **Single Purpose**: One test, one concept
5. **Fast Execution**: Keep tests fast (< 1 second when possible)
6. **Deterministic**: Tests should always produce same result
7. **Coverage**: Aim for both happy and error paths

### Performance Test Guidelines

- Use `EAutomationTestFlags::PerfFilter` flag
- Measure time with `FPlatformTime::Seconds()`
- Set reasonable thresholds
- Add informative output with `AddInfo()`
- Test scalability (vary input sizes)

### Memory Leak Test Guidelines

- Force garbage collection: `CollectGarbage(RF_NoFlags, true)`
- Get baseline: `FPlatformMemory::GetStats()`
- Perform operations
- Measure delta
- Allow small tolerance (< 5-10 MB)

### Thread Safety Test Guidelines

- Use `FThreadSafeBool` for inter-thread communication
- Use `FThreadSafeCounter` for counts
- Create worker threads with `FRunnableThread`
- Run concurrent operations
- Verify no crashes or data corruption
- Clean up threads: `WaitForCompletion()`

---

## Test Markers

Python tests support markers for categorization:

- `@pytest.mark.movement` - Movement tests
- `@pytest.mark.navigation` - Navigation tests
- `@pytest.mark.rotation` - Rotation tests
- `@pytest.mark.input` - Input simulation tests
- `@pytest.mark.recording` - Recording/playback tests
- `@pytest.mark.slow` - Slow-running tests (> 5 seconds)

---

## Troubleshooting

### Tests Not Running

1. Check that test files are in correct locations
2. Verify `#if WITH_DEV_AUTOMATION_TESTS` wrapper
3. Ensure tests are registered with `IMPLEMENT_*_AUTOMATION_TEST`
4. Check module dependencies in `.Build.cs` files

### Python Tests Failing

1. Ensure Unreal Editor is running with plugin loaded
2. Check MCP server is started (port 8081)
3. Verify Python dependencies are installed
4. Check logs in `Saved/Logs/`

### CI Failures

1. Check artifact uploads for detailed logs
2. Verify UE version matches project requirements
3. Check for platform-specific issues
4. Review failed test output in job logs

---

## Future Improvements

- [ ] Add C++ code coverage reporting
- [ ] Integrate static analysis (Clang-Tidy)
- [ ] Add stress testing for long-running scenarios
- [ ] Implement fuzzing for input validation
- [ ] Add UI automation visual regression tests
- [ ] Create test data generators
- [ ] Add performance regression tracking

---

## References

- [Unreal Engine Automation System](https://docs.unrealengine.com/5.6/en-US/automation-system-in-unreal-engine/)
- [Pytest Documentation](https://docs.pytest.org/)
- [GitHub Actions](https://docs.github.com/en/actions)
- [Codecov Documentation](https://docs.codecov.com/)
