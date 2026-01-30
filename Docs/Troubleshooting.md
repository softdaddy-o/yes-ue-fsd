# Troubleshooting Guide

This guide helps resolve common issues with the Yes UE FSD plugin.

## Table of Contents

1. [Installation Issues](#installation-issues)
2. [Build Errors](#build-errors)
3. [Runtime Errors](#runtime-errors)
4. [Python Integration Issues](#python-integration-issues)
5. [HTTP Server Issues](#http-server-issues)
6. [Navigation Issues](#navigation-issues)
7. [Performance Issues](#performance-issues)
8. [Test Failures](#test-failures)

---

## Installation Issues

### Plugin Not Showing in Plugin List

**Symptoms**: Yes UE FSD doesn't appear in Edit → Plugins

**Solutions**:
1. Check plugin location: Should be in `<Project>/Plugins/YesUeFsd/` or `<Engine>/Engine/Plugins/YesUeFsd/`
2. Verify `.uplugin` file exists and is valid JSON
3. Check Unreal Engine version: Plugin requires UE 5.6+
4. Restart Unreal Editor
5. Regenerate project files: Right-click `.uproject` → Generate Visual Studio Project Files

### Plugin Won't Enable

**Symptoms**: Error when trying to enable plugin

**Solutions**:
1. Check Output Log for specific error message
2. Verify all module dependencies are available:
   - Core, CoreUObject, Engine
   - InputCore, EnhancedInput
   - NavigationSystem, AIModule
   - UMG, Slate, SlateCore
   - HTTPServer, Json, JsonUtilities
   - PythonScriptPlugin (for Python support)
3. Build from source if using source distribution

---

## Build Errors

### Missing Header Files

**Error**: `fatal error: 'AutoDriver/AutoDriverComponent.h' file not found`

**Solutions**:
1. Check file exists at `Source/YesUeFsd/Public/AutoDriver/AutoDriverComponent.h`
2. Regenerate project files
3. Clean and rebuild:
   - Build → Clean Solution
   - Build → Rebuild Solution
4. Check module dependencies in `.Build.cs` files

### Linker Errors

**Error**: `unresolved external symbol` or `LNK2019`

**Solutions**:
1. Ensure all required modules are listed in `PublicDependencyModuleNames` or `PrivateDependencyModuleNames`
2. Rebuild from scratch
3. Delete `Intermediate` and `Binaries` folders, regenerate project files
4. Check for circular dependencies between modules

### Compile Errors in Test Files

**Error**: Compile errors in `*Tests.cpp` files

**Solutions**:
1. Ensure `WITH_DEV_AUTOMATION_TESTS` is defined
2. Check test framework headers are included:
   ```cpp
   #include "Misc/AutomationTest.h"
   #include "Tests/AutomationCommon.h"
   ```
3. Verify test macros are correct: `IMPLEMENT_SIMPLE_AUTOMATION_TEST` or `IMPLEMENT_COMPLEX_AUTOMATION_TEST`

---

## Runtime Errors

### AutoDriverComponent Not Working

**Symptoms**: Component added but commands don't execute

**Solutions**:
1. Check component is enabled:
   ```cpp
   AutoDriverComponent->SetEnabled(true);
   ```
2. Verify actor has valid world
3. Check output log for error messages
4. Ensure Enhanced Input System is properly configured
5. Test with simple command:
   ```cpp
   AutoDriverComponent->MoveToLocation(FVector(100, 0, 0));
   ```

### Commands Execute But Nothing Happens

**Symptoms**: Commands return `true` but no visible effect

**Solutions**:
1. Check Enhanced Input is configured:
   - Input Mapping Context is set
   - Input Actions are defined
2. Verify pawn has controller attached
3. Check movement component exists and is configured
4. Enable debug logging:
   ```
   Log LogAutoDriver Verbose
   ```
5. Use stats to verify execution:
   ```
   stat AutoDriver
   stat AutoDriverDetailed
   ```

### Crash on Startup

**Symptoms**: Editor crashes when opening project or PIE

**Solutions**:
1. Check crash logs in `Saved/Logs/`
2. Disable plugin, restart, re-enable
3. Verify plugin compiled for correct Unreal Engine version
4. Check for conflicting plugins
5. Run in Debug mode to get detailed crash info

---

## Python Integration Issues

### Python Scripts Not Running

**Symptoms**: `py` command not found or scripts don't execute

**Solutions**:
1. Enable Python Script Plugin: Edit → Plugins → Search "Python" → Enable
2. Verify Python is installed (3.9+ recommended)
3. Check Python path in Editor Preferences: Editor Preferences → Plugins → Python
4. Restart Unreal Editor after enabling Python plugin
5. Test Python works:
   ```
   py print("Hello from Python")
   ```

### Cannot Import autodriver_helpers

**Error**: `ModuleNotFoundError: No module named 'autodriver_helpers'`

**Solutions**:
1. Ensure `Content/Python` is in Python path:
   ```python
   import sys
   print(sys.path)
   ```
2. Add to Python path in Editor Preferences: Editor Preferences → Plugins → Python → Additional Paths → Add `<Project>/Content/Python`
3. Restart editor after changing Python paths
4. Verify file exists: `Content/Python/autodriver_helpers.py`

### HTTP Requests Failing

**Error**: `ConnectionRefusedError` or `requests.exceptions.ConnectionError`

**Solutions**:
1. Check MCP server is running: Output Log should show "MCP Server started on port 8081"
2. Verify port is not blocked by firewall
3. Test server manually:
   ```python
   import requests
   response = requests.get("http://localhost:8081/health")
   print(response.status_code)  # Should be 200
   ```
4. Check server configuration: `Config/DefaultYesUeFsd.ini`
   ```ini
   [/Script/YesUeFsdEditor.McpServer]
   ServerPort=8081
   bAutoStartServer=true
   ```

---

## HTTP Server Issues

### Server Won't Start

**Symptoms**: "Failed to start MCP server" in output log

**Solutions**:
1. Check port is not in use:
   - Windows: `netstat -ano | findstr :8081`
   - Kill process if needed
2. Try different port in config
3. Run editor as Administrator (if port < 1024)
4. Check firewall settings
5. Verify HTTPServer module is loaded

### JSON-RPC Errors

**Error**: Invalid JSON or method not found errors

**Solutions**:
1. Verify JSON format:
   ```json
   {
     "jsonrpc": "2.0",
     "method": "moveToLocation",
     "params": {"x": 100, "y": 0, "z": 0},
     "id": 1
   }
   ```
2. Check method name spelling
3. Verify required parameters are provided
4. Use `autodriver_helpers.py` wrapper instead of raw HTTP

### Server Stops Responding

**Symptoms**: Server started but requests timeout

**Solutions**:
1. Check editor didn't hang (PIE running?)
2. Look for errors in output log
3. Restart MCP server via Python:
   ```python
   import unreal
   subsystem = unreal.get_editor_subsystem(unreal.McpEditorSubsystem)
   subsystem.restart_server()
   ```
4. Restart editor if necessary

---

## Navigation Issues

### Navigation Queries Always Fail

**Symptoms**: `IsLocationReachable()` always returns `false`

**Solutions**:
1. Add Nav Mesh Bounds Volume to level:
   - Place → Volumes → Nav Mesh Bounds Volume
   - Scale to cover play area
2. Rebake navigation:
   - Select NavMeshBoundsVolume
   - Right-click → Rebuild Navigation
3. Visualize nav mesh: Press `P` in viewport
4. Check navigation system is enabled: Project Settings → Engine → Navigation System
5. Verify Agent properties match your character:
   - Agent Radius
   - Agent Height
   - Max Slope Angle

### Navigation Cache Not Helping Performance

**Symptoms**: Cache hit rate very low (< 30%)

**Solutions**:
1. Check cache size: Increase in `DefaultYesUeFsd.ini`:
   ```ini
   [/Script/YesUeFsd.NavigationCache]
   MaxCacheSize=10000
   ```
2. Verify queries are using same start/end points
3. Check query frequency (too many unique queries)
4. Use stats to analyze:
   ```
   stat AutoDriver
   ```
   Look for cache hit rate in output

### Pathfinding Takes Too Long

**Symptoms**: Navigation queries cause frame drops

**Solutions**:
1. Enable navigation cache
2. Reduce query frequency:
   ```cpp
   AutoDriverComponent->SetNavigationUpdateRate(0.2f); // Query every 0.2s instead of every frame
   ```
3. Optimize nav mesh:
   - Reduce complexity of level geometry
   - Use simplified collision for navigation
   - Adjust NavMesh cell size
4. Use async queries (already built-in to AutoDriver)

---

## Performance Issues

### Low Frame Rate

**Symptoms**: FPS drops when using AutoDriver

**Solutions**:
1. Check number of active AutoDriver components:
   ```
   stat AutoDriver
   ```
2. Reduce tick frequency if possible
3. Disable debug visualization:
   ```cpp
   AutoDriverComponent->SetDebugDrawingEnabled(false);
   ```
4. Use stats to identify bottleneck:
   ```
   stat AutoDriverDetailed
   ```
5. Profile with Unreal Insights

### High Memory Usage

**Symptoms**: Memory usage grows over time

**Solutions**:
1. Run memory leak tests: `Automation RunTests YesUeFsd.MemoryLeak`
2. Check for retained callbacks:
   - Unbind callbacks after use
   - Use weak pointers if capturing objects
3. Clear navigation cache periodically:
   ```cpp
   NavigationCache->Clear();
   ```
4. Limit recording buffer size:
   ```ini
   [/Script/YesUeFsd.ActionRecorder]
   MaxRecordingEntries=1000
   ```

### Command Execution Lag

**Symptoms**: Delay between command and execution

**Solutions**:
1. Check tick rate: Higher tick rate = more responsive
2. Reduce input smoothing:
   ```ini
   [/Script/YesUeFsd.InputSimulator]
   bSmoothInput=false
   ```
3. Check for other expensive tick operations
4. Use performance tests to benchmark:
   ```
   Automation RunTests YesUeFsd.Performance
   ```

---

## Test Failures

### C++ Tests Not Running

**Symptoms**: Tests don't appear in automation window

**Solutions**:
1. Verify tests are in `Source/YesUeFsd/Tests/` or `Source/YesUeFsdEditor/Tests/`
2. Check `#if WITH_DEV_AUTOMATION_TESTS` wrapper
3. Ensure test module is linked in `.Build.cs`
4. Rebuild project
5. Refresh automation test list: Session Frontend → Automation → Refresh Tests

### Python Tests Failing

**Symptoms**: Pytest failures

**Solutions**:
1. Check prerequisites:
   - Unreal Editor running
   - Plugin enabled
   - MCP server started
   - Python dependencies installed: `pip install pytest requests`
2. Run with verbose output:
   ```bash
   pytest tests/ -v -s
   ```
3. Check individual test:
   ```bash
   pytest tests/test_movement.py::test_basic_movement -v
   ```
4. Verify fixtures are working:
   - `autodriver` fixture
   - `starting_position` fixture

### CI/CD Pipeline Failures

**Symptoms**: GitHub Actions workflow fails

**Solutions**:
1. Check workflow logs in GitHub Actions tab
2. Verify Unreal Engine version in workflow matches project
3. Check artifact uploads for detailed logs
4. Run tests locally first:
   ```bash
   pytest Content/Python/tests/ -v
   Automation RunTests YesUeFsd
   ```
5. Check for platform-specific issues (Windows vs Linux)

### Coverage Below Threshold

**Symptoms**: CI fails due to code coverage < 80%

**Solutions**:
1. Run coverage analysis locally:
   ```bash
   cd Content/Python
   pytest tests/ --cov=. --cov-report=html
   ```
2. Add missing tests for uncovered code
3. Remove dead code
4. Update `.codecov.yml` if threshold is too strict

---

## Getting Help

If your issue isn't covered here:

1. **Check Documentation**:
   - [README.md](../README.md) - Overview
   - [USAGE.md](../USAGE.md) - Usage guide
   - [API.md](../API.md) - API reference
   - [Testing.md](./Testing.md) - Testing guide

2. **Check Output Logs**:
   - Unreal Editor: Window → Developer Tools → Output Log
   - Saved Logs: `<Project>/Saved/Logs/`

3. **Enable Verbose Logging**:
   ```
   Log LogAutoDriver Verbose
   Log LogPython Verbose
   Log LogHTTPServer Verbose
   ```

4. **Use Debug Tools**:
   - Stats: `stat AutoDriver`, `stat AutoDriverDetailed`
   - Visual Debugger: Press `'` (apostrophe) in PIE
   - Python debugger: `import pdb; pdb.set_trace()`

5. **Report Issues**:
   - GitHub Issues: https://github.com/anthropics/yes-ue-fsd/issues
   - Include:
     - Unreal Engine version
     - Plugin version
     - Steps to reproduce
     - Error messages / logs
     - Screenshots if applicable

---

## Common Error Messages

| Error | Cause | Solution |
|-------|-------|----------|
| "AutoDriver component not found" | Component not added to actor | Add UAutoDriverComponent to your actor |
| "Navigation system not available" | NavMesh not set up | Add Nav Mesh Bounds Volume and rebake |
| "Command execution failed" | Invalid parameters | Check command parameters are valid |
| "HTTP server failed to start" | Port in use | Change port or kill conflicting process |
| "Python module not found" | Python path not configured | Add Content/Python to Python path |
| "Enhanced Input not configured" | Input mapping missing | Set up Input Mapping Context |
| "Test world creation failed" | Editor not ready | Wait for editor to fully load |

---

## Performance Benchmarks

Expected performance on a typical development machine:

| Metric | Expected Value | How to Check |
|--------|----------------|--------------|
| Component Creation | < 1 ms | Run performance tests |
| Command Execution | < 0.5 ms | Use `stat AutoDriverDetailed` |
| Tick Performance (100 components) | < 16 ms/frame | Run performance tests |
| Navigation Cache Hit Rate | > 60% | Use `stat AutoDriver` |
| Memory per Component | < 100 KB | Run memory tests |
| HTTP Request | < 50 ms | Test with Python client |

If your metrics are significantly worse, check the Performance Issues section above.

---

## FAQ

**Q: Can I use AutoDriver in shipping builds?**
A: Yes, but the HTTP server and Python scripting are editor-only. The core automation functionality (C++ and Blueprints) works in all builds.

**Q: Does AutoDriver work with multiplayer?**
A: Yes, each client can have its own AutoDriver components. The HTTP server is editor-only and doesn't work in packaged multiplayer games.

**Q: Can I record and play back in packaged builds?**
A: Yes, the recording/playback system works in all builds.

**Q: How do I disable AutoDriver in shipping builds?**
A: Wrap your code in `#if !UE_BUILD_SHIPPING` or check `IsRunningCommandlet()` at runtime.

**Q: Can I use AutoDriver with AI characters?**
A: Yes! AutoDriver has full Behavior Tree integration. See [BehaviorTreeIntegration.md](./BehaviorTreeIntegration.md).

**Q: Does this work with Unreal Engine 4?**
A: No, this plugin requires Unreal Engine 5.6+. The Enhanced Input System is a hard requirement.

**Q: Can I extend AutoDriver with custom commands?**
A: Yes! Implement the `IAutoDriverCommand` interface. See examples in `Source/YesUeFsd/Public/AutoDriver/Commands/`.

---

## Debug Checklist

When troubleshooting, go through this checklist:

- [ ] Plugin is enabled (Edit → Plugins → Yes UE FSD → Enabled)
- [ ] Project compiled successfully (no build errors)
- [ ] AutoDriverComponent added to actor
- [ ] Component is enabled (`SetEnabled(true)`)
- [ ] Enhanced Input configured (Input Mapping Context set)
- [ ] Navigation mesh exists and covers play area (visualize with `P` key)
- [ ] Output log checked for errors (Window → Developer Tools → Output Log)
- [ ] Python plugin enabled (if using Python)
- [ ] MCP server running (if using HTTP/Python)
- [ ] Latest plugin version installed

---

*Last updated: 2026-01-30*
