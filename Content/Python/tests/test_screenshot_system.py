"""
Tests for screenshot capture system
"""

import pytest
import screenshot_system
import os


@pytest.fixture
def screenshot_config():
    """Configure screenshot system for testing."""
    screenshot_system.configure(
        output_dir="Saved/Screenshots/Tests",
        capture_on_failure=True,
        capture_on_success=True,
        max_screenshots_per_test=10
    )
    screenshot_system.clear_screenshots()
    yield
    # Cleanup after test
    screenshot_system.clear_screenshots()


@pytest.mark.screenshot
def test_capture_screenshot(screenshot_config):
    """Test basic screenshot capture."""
    result = screenshot_system.capture("TestCapture", "Execution")
    assert result, "Screenshot capture should succeed"

    screenshots = screenshot_system.get_captured_screenshots()
    assert len(screenshots) > 0, "Should have captured at least one screenshot"


@pytest.mark.screenshot
def test_capture_with_metadata(screenshot_config):
    """Test screenshot capture with custom metadata."""
    metadata = {
        "test_type": "integration",
        "feature": "movement",
        "priority": "high"
    }

    result = screenshot_system.capture("TestMetadata", "Execution", metadata)
    assert result, "Screenshot capture with metadata should succeed"

    screenshots = screenshot_system.get_screenshots_for_test("TestMetadata")
    assert len(screenshots) > 0, "Should have captured screenshot for test"


@pytest.mark.screenshot
def test_capture_multiple_phases(screenshot_config):
    """Test capturing screenshots in different test phases."""
    test_name = "TestMultiPhase"

    # Capture in different phases
    screenshot_system.capture(test_name, "Setup")
    screenshot_system.capture(test_name, "Execution")
    screenshot_system.capture(test_name, "Teardown")

    screenshots = screenshot_system.get_screenshots_for_test(test_name)
    assert len(screenshots) >= 3, "Should have captured screenshots in all phases"


@pytest.mark.screenshot
def test_capture_on_failure(screenshot_config):
    """Test capturing screenshot on test failure."""
    result = screenshot_system.capture_on_failure("TestFailure", "Test failed due to assertion error")
    assert result, "Failure screenshot capture should succeed"

    screenshots = screenshot_system.get_screenshots_for_test("TestFailure")
    assert len(screenshots) > 0, "Should have captured failure screenshot"


@pytest.mark.screenshot
def test_generate_manifest(screenshot_config):
    """Test manifest generation."""
    # Capture some screenshots
    screenshot_system.capture("TestManifest", "Setup")
    screenshot_system.capture("TestManifest", "Execution")

    # Generate manifest
    result = screenshot_system.generate_manifest()
    assert result, "Manifest generation should succeed"


@pytest.mark.screenshot
def test_generate_html_report(screenshot_config):
    """Test HTML report generation."""
    # Capture some screenshots
    screenshot_system.capture("TestReport", "Setup")
    screenshot_system.capture("TestReport", "Execution")

    # Generate HTML report
    result = screenshot_system.generate_html_report()
    assert result, "HTML report generation should succeed"


@pytest.mark.screenshot
def test_max_screenshots_limit(screenshot_config):
    """Test that max screenshots per test limit is enforced."""
    test_name = "TestLimit"

    # Try to capture more than the limit
    for i in range(15):
        screenshot_system.capture(test_name, f"Iteration_{i}")

    screenshots = screenshot_system.get_screenshots_for_test(test_name)
    assert len(screenshots) <= 10, "Should not exceed max screenshots per test"


@pytest.mark.screenshot
def test_enable_disable(screenshot_config):
    """Test enabling and disabling screenshot capture."""
    # Disable screenshot capture
    screenshot_system.set_enabled(False)
    assert not screenshot_system.is_enabled(), "Should be disabled"

    # Try to capture (should fail)
    result = screenshot_system.capture("TestDisabled", "Execution")
    assert not result, "Should not capture when disabled"

    # Re-enable
    screenshot_system.set_enabled(True)
    assert screenshot_system.is_enabled(), "Should be enabled"

    # Capture should work now
    result = screenshot_system.capture("TestEnabled", "Execution")
    assert result, "Should capture when enabled"


@pytest.mark.screenshot
def test_clear_screenshots(screenshot_config):
    """Test clearing screenshot cache."""
    # Capture some screenshots
    screenshot_system.capture("TestClear", "Setup")
    screenshot_system.capture("TestClear", "Execution")

    screenshots_before = screenshot_system.get_captured_screenshots()
    assert len(screenshots_before) > 0, "Should have screenshots before clearing"

    # Clear screenshots
    screenshot_system.clear_screenshots()

    screenshots_after = screenshot_system.get_captured_screenshots()
    assert len(screenshots_after) == 0, "Should have no screenshots after clearing"


@pytest.mark.screenshot
def test_configuration(screenshot_config):
    """Test screenshot system configuration."""
    screenshot_system.configure(
        output_dir="Saved/Screenshots/Custom",
        naming_pattern="{TestName}_{Index}_{Phase}",
        max_screenshots_per_test=5
    )

    config = screenshot_system.get_configuration()
    assert config is not None, "Should get configuration"
    assert config.output_directory == "Saved/Screenshots/Custom", "Output directory should be set"
    assert config.max_screenshots_per_test == 5, "Max screenshots should be set"


@pytest.mark.screenshot
def test_get_screenshots_for_test(screenshot_config):
    """Test getting screenshots for a specific test."""
    # Capture screenshots for multiple tests
    screenshot_system.capture("TestA", "Execution")
    screenshot_system.capture("TestB", "Execution")
    screenshot_system.capture("TestA", "Teardown")

    # Get screenshots for TestA only
    screenshots_a = screenshot_system.get_screenshots_for_test("TestA")
    assert len(screenshots_a) == 2, "Should have 2 screenshots for TestA"

    # Get screenshots for TestB only
    screenshots_b = screenshot_system.get_screenshots_for_test("TestB")
    assert len(screenshots_b) == 1, "Should have 1 screenshot for TestB"


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
