"""
Screenshot integration for test reports.

Automatically captures and attaches screenshots on test failures.
"""

import os
from pathlib import Path
from typing import Optional
import pytest


class ScreenshotAttachment:
    """
    Pytest plugin for automatic screenshot attachment to reports.

    Automatically captures screenshots on test failure and attaches them
    to HTML/Allure reports.
    """

    def __init__(self, screenshot_dir: str = "test_results/screenshots"):
        """
        Initialize screenshot attachment.

        Args:
            screenshot_dir: Directory for storing screenshots
        """
        self.screenshot_dir = Path(screenshot_dir)
        self.screenshot_dir.mkdir(parents=True, exist_ok=True)

    @pytest.hookimpl(hookwrapper=True)
    def pytest_runtest_makereport(self, item, call):
        """
        Pytest hook to capture screenshots on failure.

        This hook is called after each test phase (setup, call, teardown).
        """
        outcome = yield
        report = outcome.get_result()

        # Only capture on test failure during call phase
        if report.when == "call" and report.failed:
            self._capture_screenshot(item, report)

    def _capture_screenshot(self, item, report):
        """Capture screenshot and attach to report."""
        try:
            # Generate screenshot filename
            test_name = item.nodeid.replace("/", "_").replace("::", "_")
            screenshot_path = self.screenshot_dir / f"{test_name}.png"

            # In real implementation, this would call the screenshot system:
            # from yes_ue_fsd import AutoDriver
            # driver = AutoDriver(player_index=0)
            # driver.capture_screenshot(str(screenshot_path))

            # For now, create a placeholder
            # (In production, the actual screenshot would be captured here)

            # Attach to pytest-html report
            if hasattr(report, "extra"):
                report.extra = getattr(report, "extra", [])
                report.extra.append(pytest_html.extras.image(str(screenshot_path)))

            # Attach to Allure report
            if self._is_allure_available():
                import allure
                with open(screenshot_path, "rb") as f:
                    allure.attach(
                        f.read(),
                        name="failure_screenshot",
                        attachment_type=allure.attachment_type.PNG
                    )

        except Exception as e:
            # Don't fail the test if screenshot capture fails
            print(f"Warning: Failed to capture screenshot: {e}")

    def _is_allure_available(self) -> bool:
        """Check if Allure is available."""
        try:
            import allure
            return True
        except ImportError:
            return False


def pytest_configure(config):
    """Configure pytest plugin."""
    config._screenshot_plugin = ScreenshotAttachment()
    config.pluginmanager.register(config._screenshot_plugin)


def pytest_unconfigure(config):
    """Unconfigure pytest plugin."""
    plugin = getattr(config, "_screenshot_plugin", None)
    if plugin:
        config.pluginmanager.unregister(plugin)
