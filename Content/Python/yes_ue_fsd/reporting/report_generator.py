"""
Unified report generator supporting multiple formats.
"""

import json
from pathlib import Path
from typing import Dict, List, Any, Optional
from datetime import datetime


class ReportGenerator:
    """
    Generates test reports in multiple formats.

    Supports:
    - HTML (with custom styling)
    - JSON (for programmatic access)
    - JUnit XML (for CI/CD integration)
    - Allure (rich interactive reports)
    """

    def __init__(self, output_dir: str = "test_results"):
        """
        Initialize report generator.

        Args:
            output_dir: Directory for generated reports
        """
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(parents=True, exist_ok=True)

    def generate_json_report(
        self,
        test_results: List[Dict[str, Any]],
        output_file: Optional[str] = None
    ) -> str:
        """
        Generate JSON report.

        Args:
            test_results: List of test result dictionaries
            output_file: Output filename (default: report.json)

        Returns:
            Path to generated report
        """
        if output_file is None:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            output_file = f"report_{timestamp}.json"

        report_path = self.output_dir / output_file

        report_data = {
            "generated_at": datetime.now().isoformat(),
            "total_tests": len(test_results),
            "passed": sum(1 for t in test_results if t.get("status") == "passed"),
            "failed": sum(1 for t in test_results if t.get("status") == "failed"),
            "skipped": sum(1 for t in test_results if t.get("status") == "skipped"),
            "tests": test_results,
        }

        with open(report_path, "w", encoding="utf-8") as f:
            json.dump(report_data, f, indent=2)

        return str(report_path)

    def generate_junit_xml(
        self,
        test_results: List[Dict[str, Any]],
        output_file: Optional[str] = None
    ) -> str:
        """
        Generate JUnit XML report.

        Args:
            test_results: List of test result dictionaries
            output_file: Output filename (default: junit.xml)

        Returns:
            Path to generated report
        """
        if output_file is None:
            output_file = "junit.xml"

        report_path = self.output_dir / output_file

        # Calculate statistics
        total_tests = len(test_results)
        failures = sum(1 for t in test_results if t.get("status") == "failed")
        skipped = sum(1 for t in test_results if t.get("status") == "skipped")
        total_time = sum(t.get("duration", 0) for t in test_results)

        # Generate XML
        xml_lines = [
            '<?xml version="1.0" encoding="UTF-8"?>',
            f'<testsuites tests="{total_tests}" failures="{failures}" skipped="{skipped}" time="{total_time:.3f}">',
            f'  <testsuite name="Yes UE FSD Tests" tests="{total_tests}" failures="{failures}" skipped="{skipped}" time="{total_time:.3f}">',
        ]

        for test in test_results:
            test_name = test.get("id", "unknown")
            duration = test.get("duration", 0)
            status = test.get("status", "unknown")

            xml_lines.append(f'    <testcase name="{test_name}" time="{duration:.3f}">')

            if status == "failed":
                error = test.get("error", "Unknown error")
                xml_lines.append(f'      <failure message="Test failed">{error}</failure>')
            elif status == "skipped":
                xml_lines.append('      <skipped />')

            xml_lines.append('    </testcase>')

        xml_lines.append('  </testsuite>')
        xml_lines.append('</testsuites>')

        with open(report_path, "w", encoding="utf-8") as f:
            f.write("\n".join(xml_lines))

        return str(report_path)

    def generate_markdown_summary(
        self,
        test_results: List[Dict[str, Any]],
        output_file: Optional[str] = None
    ) -> str:
        """
        Generate Markdown summary for GitHub PR comments.

        Args:
            test_results: List of test result dictionaries
            output_file: Output filename (default: summary.md)

        Returns:
            Path to generated report
        """
        if output_file is None:
            output_file = "summary.md"

        report_path = self.output_dir / output_file

        # Calculate statistics
        total_tests = len(test_results)
        passed = sum(1 for t in test_results if t.get("status") == "passed")
        failed = sum(1 for t in test_results if t.get("status") == "failed")
        skipped = sum(1 for t in test_results if t.get("status") == "skipped")
        total_time = sum(t.get("duration", 0) for t in test_results)

        # Generate Markdown
        markdown = f"""## 🧪 Test Results

✅ {passed} passed | ❌ {failed} failed | ⏭️ {skipped} skipped | 📊 {total_tests} total

**Duration:** {total_time:.2f}s

"""

        if failed > 0:
            markdown += "### Failed Tests\n\n"
            for test in test_results:
                if test.get("status") == "failed":
                    markdown += f"- ❌ `{test.get('id')}` - {test.get('error', 'Unknown error')}\n"
            markdown += "\n"

        markdown += f"""### Summary

| Metric | Value |
|--------|-------|
| Total Tests | {total_tests} |
| Passed | {passed} ({passed/total_tests*100:.1f}%) |
| Failed | {failed} ({failed/total_tests*100:.1f}%) |
| Skipped | {skipped} ({skipped/total_tests*100:.1f}%) |
| Duration | {total_time:.2f}s |
| Average Test Time | {total_time/total_tests:.2f}s |

---
*Generated by Yes UE FSD Test Framework*
"""

        with open(report_path, "w", encoding="utf-8") as f:
            f.write(markdown)

        return str(report_path)


# Pytest plugin integration
import pytest
from typing import Dict, Any


class ReportingPlugin:
    """Pytest plugin for automatic report generation."""

    def __init__(self):
        self.test_results = []
        self.generator = ReportGenerator()

    @pytest.hookimpl(hookwrapper=True)
    def pytest_runtest_makereport(self, item, call):
        """Collect test results."""
        outcome = yield
        report = outcome.get_result()

        if report.when == "call":
            test_result = {
                "id": item.nodeid,
                "status": "passed" if report.passed else ("failed" if report.failed else "skipped"),
                "duration": report.duration,
            }

            if report.failed:
                test_result["error"] = str(report.longrepr)

            self.test_results.append(test_result)

    def pytest_sessionfinish(self, session):
        """Generate reports at end of session."""
        if self.test_results:
            # Generate JSON report
            self.generator.generate_json_report(self.test_results)

            # Generate JUnit XML
            self.generator.generate_junit_xml(self.test_results)

            # Generate Markdown summary
            self.generator.generate_markdown_summary(self.test_results)


def pytest_configure(config):
    """Configure pytest plugin."""
    config._reporting_plugin = ReportingPlugin()
    config.pluginmanager.register(config._reporting_plugin)


def pytest_unconfigure(config):
    """Unconfigure pytest plugin."""
    plugin = getattr(config, "_reporting_plugin", None)
    if plugin:
        config.pluginmanager.unregister(plugin)
