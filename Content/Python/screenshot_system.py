"""
Screenshot System for Automated Testing

This module provides Python bindings for the UE screenshot capture system,
enabling visual testing and regression detection.

Example:
    import screenshot_system

    # Configure screenshot system
    screenshot_system.configure(
        output_dir="Saved/Screenshots/MyTest",
        capture_on_failure=True
    )

    # Capture a screenshot
    screenshot_system.capture("MyTest", "Setup")

    # Generate manifest and report
    screenshot_system.generate_manifest()
    screenshot_system.generate_html_report()
"""

import unreal
from typing import Dict, List, Optional


class ScreenshotSystem:
    """Python interface for UE screenshot capture system."""

    def __init__(self):
        """Initialize the screenshot system."""
        self.helper_class = unreal.ScreenshotHelper

    def capture(
        self,
        test_name: str,
        phase: str = "Execution",
        metadata: Optional[Dict[str, str]] = None
    ) -> bool:
        """
        Capture a screenshot with metadata.

        Args:
            test_name: Name of the test
            phase: Test phase (e.g., "Setup", "Execution", "Teardown")
            metadata: Additional metadata to store with the screenshot

        Returns:
            True if screenshot was captured successfully

        Example:
            >>> screenshot_system.capture("TestMovement", "Execution", {"speed": "fast"})
            True
        """
        if metadata is None:
            metadata = {}

        try:
            return self.helper_class.capture_screenshot(test_name, phase, metadata)
        except Exception as e:
            unreal.log_error(f"Failed to capture screenshot: {e}")
            return False

    def capture_on_failure(self, test_name: str, error_message: str) -> bool:
        """
        Capture screenshot on test failure.

        Args:
            test_name: Name of the failed test
            error_message: Error message from the test

        Returns:
            True if screenshot was captured successfully
        """
        try:
            return self.helper_class.capture_screenshot_on_failure(test_name, error_message)
        except Exception as e:
            unreal.log_error(f"Failed to capture failure screenshot: {e}")
            return False

    def generate_manifest(self, output_path: str = "") -> bool:
        """
        Generate JSON manifest file with all captured screenshots.

        Args:
            output_path: Path to write the manifest file (defaults to configured output directory)

        Returns:
            True if manifest was generated successfully

        Example:
            >>> screenshot_system.generate_manifest()
            True
        """
        try:
            return self.helper_class.generate_manifest(output_path)
        except Exception as e:
            unreal.log_error(f"Failed to generate manifest: {e}")
            return False

    def generate_html_report(self, output_path: str = "") -> bool:
        """
        Generate HTML report with embedded screenshots.

        Args:
            output_path: Path to write the HTML report (defaults to configured output directory)

        Returns:
            True if report was generated successfully

        Example:
            >>> screenshot_system.generate_html_report()
            True
        """
        try:
            return self.helper_class.generate_htmlreport(output_path)
        except Exception as e:
            unreal.log_error(f"Failed to generate HTML report: {e}")
            return False

    def configure(
        self,
        output_dir: Optional[str] = None,
        naming_pattern: Optional[str] = None,
        capture_on_failure: Optional[bool] = None,
        capture_on_success: Optional[bool] = None,
        generate_manifest: Optional[bool] = None,
        max_screenshots_per_test: Optional[int] = None
    ) -> None:
        """
        Configure screenshot capture settings.

        Args:
            output_dir: Output directory for screenshots
            naming_pattern: Naming pattern for screenshot files
            capture_on_failure: Whether to capture screenshots on test failure
            capture_on_success: Whether to capture screenshots on test success
            generate_manifest: Whether to generate manifest automatically
            max_screenshots_per_test: Maximum screenshots per test

        Example:
            >>> screenshot_system.configure(
            ...     output_dir="Saved/Screenshots/Tests",
            ...     capture_on_failure=True,
            ...     max_screenshots_per_test=5
            ... )
        """
        config = self.get_configuration()

        if output_dir is not None:
            config.output_directory = output_dir
        if naming_pattern is not None:
            config.naming_pattern = naming_pattern
        if capture_on_failure is not None:
            config.capture_on_test_failure = capture_on_failure
        if capture_on_success is not None:
            config.capture_on_test_success = capture_on_success
        if generate_manifest is not None:
            config.generate_manifest = generate_manifest
        if max_screenshots_per_test is not None:
            config.max_screenshots_per_test = max_screenshots_per_test

        try:
            self.helper_class.configure(config)
        except Exception as e:
            unreal.log_error(f"Failed to configure screenshot system: {e}")

    def get_configuration(self):
        """
        Get current screenshot capture configuration.

        Returns:
            Current configuration object
        """
        try:
            return self.helper_class.get_configuration()
        except Exception as e:
            unreal.log_error(f"Failed to get configuration: {e}")
            return None

    def set_output_directory(self, directory: str) -> None:
        """
        Set output directory for screenshots.

        Args:
            directory: Path to output directory
        """
        try:
            self.helper_class.set_output_directory(directory)
        except Exception as e:
            unreal.log_error(f"Failed to set output directory: {e}")

    def set_naming_pattern(self, pattern: str) -> None:
        """
        Set naming pattern for screenshot files.

        Supports placeholders: {TestName}, {Timestamp}, {Phase}, {Index}

        Args:
            pattern: Naming pattern string

        Example:
            >>> screenshot_system.set_naming_pattern("{TestName}_{Phase}_{Index}")
        """
        try:
            self.helper_class.set_naming_pattern(pattern)
        except Exception as e:
            unreal.log_error(f"Failed to set naming pattern: {e}")

    def clear_screenshots(self) -> None:
        """Clear all captured screenshots for current session."""
        try:
            self.helper_class.clear_screenshots()
        except Exception as e:
            unreal.log_error(f"Failed to clear screenshots: {e}")

    def get_captured_screenshots(self) -> List:
        """
        Get list of all captured screenshots.

        Returns:
            Array of screenshot metadata objects
        """
        try:
            return self.helper_class.get_captured_screenshots()
        except Exception as e:
            unreal.log_error(f"Failed to get captured screenshots: {e}")
            return []

    def get_screenshots_for_test(self, test_name: str) -> List:
        """
        Get screenshots for a specific test.

        Args:
            test_name: Name of the test

        Returns:
            Array of screenshot metadata objects for the test
        """
        try:
            return self.helper_class.get_screenshots_for_test(test_name)
        except Exception as e:
            unreal.log_error(f"Failed to get screenshots for test: {e}")
            return []

    def set_enabled(self, enabled: bool) -> None:
        """
        Enable or disable screenshot capture.

        Args:
            enabled: True to enable, false to disable
        """
        try:
            self.helper_class.set_enabled(enabled)
        except Exception as e:
            unreal.log_error(f"Failed to set enabled state: {e}")

    def is_enabled(self) -> bool:
        """
        Check if screenshot capture is enabled.

        Returns:
            True if enabled, false otherwise
        """
        try:
            return self.helper_class.is_enabled()
        except Exception as e:
            unreal.log_error(f"Failed to check enabled state: {e}")
            return False


# Global instance
_screenshot_system = None


def get_instance() -> ScreenshotSystem:
    """
    Get global screenshot system instance.

    Returns:
        Global ScreenshotSystem instance
    """
    global _screenshot_system
    if _screenshot_system is None:
        _screenshot_system = ScreenshotSystem()
    return _screenshot_system


# Convenience functions
def capture(test_name: str, phase: str = "Execution", metadata: Optional[Dict[str, str]] = None) -> bool:
    """Capture a screenshot. See ScreenshotSystem.capture() for details."""
    return get_instance().capture(test_name, phase, metadata)


def capture_on_failure(test_name: str, error_message: str) -> bool:
    """Capture screenshot on failure. See ScreenshotSystem.capture_on_failure() for details."""
    return get_instance().capture_on_failure(test_name, error_message)


def generate_manifest(output_path: str = "") -> bool:
    """Generate manifest. See ScreenshotSystem.generate_manifest() for details."""
    return get_instance().generate_manifest(output_path)


def generate_html_report(output_path: str = "") -> bool:
    """Generate HTML report. See ScreenshotSystem.generate_html_report() for details."""
    return get_instance().generate_html_report(output_path)


def configure(**kwargs) -> None:
    """Configure screenshot system. See ScreenshotSystem.configure() for details."""
    get_instance().configure(**kwargs)


def get_configuration():
    """Get configuration. See ScreenshotSystem.get_configuration() for details."""
    return get_instance().get_configuration()


def set_output_directory(directory: str) -> None:
    """Set output directory. See ScreenshotSystem.set_output_directory() for details."""
    get_instance().set_output_directory(directory)


def set_naming_pattern(pattern: str) -> None:
    """Set naming pattern. See ScreenshotSystem.set_naming_pattern() for details."""
    get_instance().set_naming_pattern(pattern)


def clear_screenshots() -> None:
    """Clear screenshots. See ScreenshotSystem.clear_screenshots() for details."""
    get_instance().clear_screenshots()


def get_captured_screenshots() -> List:
    """Get captured screenshots. See ScreenshotSystem.get_captured_screenshots() for details."""
    return get_instance().get_captured_screenshots()


def get_screenshots_for_test(test_name: str) -> List:
    """Get screenshots for test. See ScreenshotSystem.get_screenshots_for_test() for details."""
    return get_instance().get_screenshots_for_test(test_name)


def set_enabled(enabled: bool) -> None:
    """Set enabled state. See ScreenshotSystem.set_enabled() for details."""
    get_instance().set_enabled(enabled)


def is_enabled() -> bool:
    """Check enabled state. See ScreenshotSystem.is_enabled() for details."""
    return get_instance().is_enabled()


if __name__ == "__main__":
    # Example usage
    print("Screenshot System Example")

    # Configure
    configure(
        output_dir="Saved/Screenshots/Example",
        capture_on_failure=True,
        max_screenshots_per_test=5
    )

    # Capture screenshots
    capture("ExampleTest", "Setup")
    capture("ExampleTest", "Execution", {"action": "movement"})
    capture("ExampleTest", "Teardown")

    # Generate reports
    generate_manifest()
    generate_html_report()

    print(f"Captured {len(get_captured_screenshots())} screenshots")
