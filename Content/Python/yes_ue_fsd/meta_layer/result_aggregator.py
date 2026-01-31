"""
Result Aggregator - Collect and report test results

This module provides tools for aggregating results from multiple
test runs and generating reports.
"""

import json
import time
from pathlib import Path
from typing import Any, Dict, List, Optional
from xml.etree import ElementTree as ET

from .test_runner import TestResult


class ResultAggregator:
    """Aggregates results from multiple test runs."""

    def __init__(self, output_dir: str = "test_results"):
        """
        Initialize with output directory.

        Args:
            output_dir: Directory for output files
        """
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(parents=True, exist_ok=True)
        self.results: List[TestResult] = []

    def add_result(self, result: TestResult):
        """
        Add a test result.

        Args:
            result: Test result to add
        """
        self.results.append(result)

    def generate_junit_xml(self, output_path: Optional[str] = None) -> str:
        """
        Generate JUnit XML report.

        Args:
            output_path: Output file path (default: output_dir/results.xml)

        Returns:
            Path to generated file
        """
        if output_path is None:
            output_path = str(self.output_dir / "results.xml")

        # Create root element
        testsuites = ET.Element("testsuites")
        testsuites.set("tests", str(sum(len(r.instance_results) for r in self.results)))
        testsuites.set("failures", str(sum(len([ir for ir in r.instance_results if not ir.success]) for r in self.results)))
        testsuites.set("time", str(sum(r.duration for r in self.results)))

        # Add testsuite for each scenario
        for result in self.results:
            testsuite = ET.SubElement(testsuites, "testsuite")
            testsuite.set("name", result.scenario_name)
            testsuite.set("tests", str(len(result.instance_results)))
            testsuite.set("failures", str(len([r for r in result.instance_results if not r.success])))
            testsuite.set("time", str(result.duration))

            # Add testcase for each instance
            for instance_result in result.instance_results:
                testcase = ET.SubElement(testsuite, "testcase")
                testcase.set("name", f"Instance_{instance_result.instance_id}_{instance_result.role}")
                testcase.set("time", str(instance_result.duration))

                if not instance_result.success:
                    failure = ET.SubElement(testcase, "failure")
                    failure.set("message", "; ".join(instance_result.errors))
                    failure.text = "\n".join(instance_result.logs[-50:])  # Last 50 log lines

        # Write XML
        tree = ET.ElementTree(testsuites)
        ET.indent(tree, space="  ")
        tree.write(output_path, encoding="utf-8", xml_declaration=True)

        print(f"[ResultAggregator] JUnit XML written to: {output_path}")
        return output_path

    def generate_html_report(self, output_path: Optional[str] = None) -> str:
        """
        Generate HTML report with screenshots.

        Args:
            output_path: Output file path (default: output_dir/report.html)

        Returns:
            Path to generated file
        """
        if output_path is None:
            output_path = str(self.output_dir / "report.html")

        summary = self.get_summary()

        html = f"""<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>Test Report - {time.strftime('%Y-%m-%d %H:%M:%S')}</title>
    <style>
        body {{ font-family: Arial, sans-serif; margin: 20px; }}
        h1 {{ color: #333; }}
        .summary {{ background: #f0f0f0; padding: 15px; border-radius: 5px; margin: 20px 0; }}
        .pass {{ color: green; font-weight: bold; }}
        .fail {{ color: red; font-weight: bold; }}
        table {{ border-collapse: collapse; width: 100%; margin: 20px 0; }}
        th, td {{ border: 1px solid #ddd; padding: 8px; text-align: left; }}
        th {{ background-color: #4CAF50; color: white; }}
        .details {{ margin: 20px 0; }}
        .logs {{ background: #f5f5f5; padding: 10px; font-family: monospace; font-size: 12px; overflow-x: auto; }}
    </style>
</head>
<body>
    <h1>Test Report</h1>
    <div class="summary">
        <h2>Summary</h2>
        <p><strong>Total Scenarios:</strong> {summary['total_scenarios']}</p>
        <p><strong>Passed:</strong> <span class="pass">{summary['passed_scenarios']}</span></p>
        <p><strong>Failed:</strong> <span class="fail">{summary['failed_scenarios']}</span></p>
        <p><strong>Total Duration:</strong> {summary['total_duration']:.2f}s</p>
        <p><strong>Total Instances:</strong> {summary['total_instances']}</p>
    </div>

    <h2>Scenarios</h2>
    <table>
        <tr>
            <th>Scenario</th>
            <th>Status</th>
            <th>Duration</th>
            <th>Instances</th>
            <th>Passed</th>
            <th>Failed</th>
        </tr>
"""

        for result in self.results:
            status_class = "pass" if result.success else "fail"
            status_text = "PASS" if result.success else "FAIL"
            passed = sum(1 for r in result.instance_results if r.success)
            failed = sum(1 for r in result.instance_results if not r.success)

            html += f"""
        <tr>
            <td>{result.scenario_name}</td>
            <td class="{status_class}">{status_text}</td>
            <td>{result.duration:.2f}s</td>
            <td>{len(result.instance_results)}</td>
            <td class="pass">{passed}</td>
            <td class="fail">{failed}</td>
        </tr>
"""

        html += """
    </table>

    <h2>Details</h2>
"""

        for result in self.results:
            html += f"""
    <div class="details">
        <h3>{result.scenario_name}</h3>
"""
            for instance_result in result.instance_results:
                status_class = "pass" if instance_result.success else "fail"
                status_text = "✓" if instance_result.success else "✗"

                html += f"""
        <h4 class="{status_class}">{status_text} Instance {instance_result.instance_id} ({instance_result.role}) - {instance_result.duration:.2f}s</h4>
"""
                if instance_result.errors:
                    html += f"""
        <p><strong>Errors:</strong></p>
        <ul>
"""
                    for error in instance_result.errors:
                        html += f"            <li>{error}</li>\n"
                    html += "        </ul>\n"

                if instance_result.logs:
                    html += f"""
        <p><strong>Logs (last 20 lines):</strong></p>
        <div class="logs">
"""
                    for log in instance_result.logs[-20:]:
                        html += f"            {log}<br>\n"
                    html += "        </div>\n"

            html += "    </div>\n"

        html += """
</body>
</html>
"""

        Path(output_path).write_text(html, encoding="utf-8")
        print(f"[ResultAggregator] HTML report written to: {output_path}")
        return output_path

    def generate_json_report(self, output_path: Optional[str] = None) -> str:
        """
        Generate JSON report.

        Args:
            output_path: Output file path (default: output_dir/results.json)

        Returns:
            Path to generated file
        """
        if output_path is None:
            output_path = str(self.output_dir / "results.json")

        data = {
            "summary": self.get_summary(),
            "results": [
                {
                    "scenario": r.scenario_name,
                    "success": r.success,
                    "duration": r.duration,
                    "instances": [
                        {
                            "id": ir.instance_id,
                            "role": ir.role,
                            "success": ir.success,
                            "duration": ir.duration,
                            "errors": ir.errors,
                            "metrics": ir.metrics,
                        }
                        for ir in r.instance_results
                    ],
                    "errors": r.errors,
                }
                for r in self.results
            ]
        }

        Path(output_path).write_text(json.dumps(data, indent=2), encoding="utf-8")
        print(f"[ResultAggregator] JSON report written to: {output_path}")
        return output_path

    def get_summary(self) -> Dict[str, Any]:
        """
        Get test summary statistics.

        Returns:
            Summary dictionary
        """
        return {
            "total_scenarios": len(self.results),
            "passed_scenarios": sum(1 for r in self.results if r.success),
            "failed_scenarios": sum(1 for r in self.results if not r.success),
            "total_duration": sum(r.duration for r in self.results),
            "total_instances": sum(len(r.instance_results) for r in self.results),
            "total_errors": sum(len(r.errors) + sum(len(ir.errors) for ir in r.instance_results) for r in self.results),
        }
