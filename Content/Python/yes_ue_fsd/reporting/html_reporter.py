"""
Enhanced HTML reporter with screenshot integration and custom styling.
"""

import os
import time
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Any, Optional


class EnhancedHTMLReporter:
    """
    Enhanced HTML test reporter with game-specific features.

    Features:
    - Automatic screenshot embedding on failure
    - Performance metrics visualization
    - Test execution timeline
    - Failure analytics dashboard
    - Custom styling for game testing
    """

    def __init__(self, output_dir: str = "test_results"):
        """
        Initialize reporter.

        Args:
            output_dir: Directory for test reports
        """
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(parents=True, exist_ok=True)

        self.tests: List[Dict[str, Any]] = []
        self.start_time: Optional[float] = None
        self.end_time: Optional[float] = None
        self.screenshots: Dict[str, List[str]] = {}
        self.performance_metrics: Dict[str, Dict[str, Any]] = {}

    def on_test_start(self, test_id: str):
        """Called when a test starts."""
        self.tests.append({
            "id": test_id,
            "start_time": time.time(),
            "status": "running",
            "screenshots": [],
            "metrics": {},
        })

    def on_test_end(self, test_id: str, status: str, error: Optional[str] = None):
        """Called when a test ends."""
        for test in self.tests:
            if test["id"] == test_id:
                test["end_time"] = time.time()
                test["status"] = status
                test["duration"] = test["end_time"] - test["start_time"]
                if error:
                    test["error"] = error
                break

    def attach_screenshot(self, test_id: str, screenshot_path: str):
        """Attach screenshot to test."""
        for test in self.tests:
            if test["id"] == test_id:
                test["screenshots"].append(screenshot_path)
                break

    def add_performance_metric(self, test_id: str, metric_name: str, value: Any):
        """Add performance metric to test."""
        for test in self.tests:
            if test["id"] == test_id:
                test["metrics"][metric_name] = value
                break

    def generate_report(self, report_path: Optional[str] = None) -> str:
        """
        Generate HTML report.

        Args:
            report_path: Output file path (default: test_results/report.html)

        Returns:
            Path to generated report
        """
        if report_path is None:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            report_path = self.output_dir / f"report_{timestamp}.html"

        html = self._generate_html()

        with open(report_path, "w", encoding="utf-8") as f:
            f.write(html)

        return str(report_path)

    def _generate_html(self) -> str:
        """Generate HTML content."""
        # Calculate statistics
        total_tests = len(self.tests)
        passed = sum(1 for t in self.tests if t["status"] == "passed")
        failed = sum(1 for t in self.tests if t["status"] == "failed")
        skipped = sum(1 for t in self.tests if t["status"] == "skipped")

        total_duration = sum(t.get("duration", 0) for t in self.tests)

        # Generate HTML
        html = f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Yes UE FSD Test Report</title>
    <style>
        {self._get_css()}
    </style>
</head>
<body>
    <div class="header">
        <h1>🎮 Yes UE FSD Test Report</h1>
        <div class="meta">
            Generated: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}
        </div>
    </div>

    <div class="summary">
        <div class="summary-card total">
            <div class="summary-value">{total_tests}</div>
            <div class="summary-label">Total Tests</div>
        </div>
        <div class="summary-card passed">
            <div class="summary-value">{passed}</div>
            <div class="summary-label">Passed</div>
        </div>
        <div class="summary-card failed">
            <div class="summary-value">{failed}</div>
            <div class="summary-label">Failed</div>
        </div>
        <div class="summary-card skipped">
            <div class="summary-value">{skipped}</div>
            <div class="summary-label">Skipped</div>
        </div>
        <div class="summary-card duration">
            <div class="summary-value">{total_duration:.2f}s</div>
            <div class="summary-label">Duration</div>
        </div>
    </div>

    <div class="test-results">
        <h2>Test Results</h2>
        {self._generate_test_list()}
    </div>

    <script>
        {self._get_javascript()}
    </script>
</body>
</html>
"""
        return html

    def _generate_test_list(self) -> str:
        """Generate HTML for test list."""
        html = []
        for test in self.tests:
            status_class = test["status"]
            status_icon = {
                "passed": "✅",
                "failed": "❌",
                "skipped": "⏭️",
                "running": "🔄",
            }.get(status_class, "❓")

            duration = test.get("duration", 0)
            error = test.get("error", "")
            screenshots = test.get("screenshots", [])
            metrics = test.get("metrics", {})

            test_html = f"""
            <div class="test-item {status_class}">
                <div class="test-header" onclick="toggleTest(this)">
                    <span class="test-icon">{status_icon}</span>
                    <span class="test-name">{test['id']}</span>
                    <span class="test-duration">{duration:.3f}s</span>
                </div>
                <div class="test-details" style="display: none;">
            """

            # Add error if present
            if error:
                test_html += f"""
                    <div class="test-error">
                        <h4>Error</h4>
                        <pre>{error}</pre>
                    </div>
                """

            # Add screenshots if present
            if screenshots:
                test_html += """
                    <div class="test-screenshots">
                        <h4>Screenshots</h4>
                        <div class="screenshot-gallery">
                """
                for screenshot in screenshots:
                    test_html += f"""
                        <div class="screenshot-item">
                            <img src="{screenshot}" alt="Screenshot">
                        </div>
                    """
                test_html += """
                        </div>
                    </div>
                """

            # Add performance metrics if present
            if metrics:
                test_html += """
                    <div class="test-metrics">
                        <h4>Performance Metrics</h4>
                        <table class="metrics-table">
                """
                for metric_name, metric_value in metrics.items():
                    test_html += f"""
                        <tr>
                            <td>{metric_name}</td>
                            <td>{metric_value}</td>
                        </tr>
                    """
                test_html += """
                        </table>
                    </div>
                """

            test_html += """
                </div>
            </div>
            """
            html.append(test_html)

        return "\n".join(html)

    def _get_css(self) -> str:
        """Get CSS styles."""
        return """
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            padding: 20px;
            color: #333;
        }

        .header {
            background: white;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            margin-bottom: 20px;
        }

        .header h1 {
            color: #667eea;
            margin-bottom: 10px;
        }

        .meta {
            color: #666;
            font-size: 14px;
        }

        .summary {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
            gap: 15px;
            margin-bottom: 20px;
        }

        .summary-card {
            background: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            text-align: center;
        }

        .summary-value {
            font-size: 32px;
            font-weight: bold;
            margin-bottom: 5px;
        }

        .summary-label {
            font-size: 14px;
            color: #666;
        }

        .summary-card.passed .summary-value { color: #10b981; }
        .summary-card.failed .summary-value { color: #ef4444; }
        .summary-card.skipped .summary-value { color: #f59e0b; }
        .summary-card.total .summary-value { color: #667eea; }
        .summary-card.duration .summary-value { color: #8b5cf6; }

        .test-results {
            background: white;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
        }

        .test-results h2 {
            color: #667eea;
            margin-bottom: 20px;
        }

        .test-item {
            border: 1px solid #e5e7eb;
            border-radius: 8px;
            margin-bottom: 10px;
            overflow: hidden;
            transition: all 0.3s ease;
        }

        .test-item:hover {
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }

        .test-header {
            padding: 15px;
            cursor: pointer;
            display: flex;
            align-items: center;
            gap: 10px;
            user-select: none;
        }

        .test-item.passed .test-header { background: #f0fdf4; }
        .test-item.failed .test-header { background: #fef2f2; }
        .test-item.skipped .test-header { background: #fffbeb; }

        .test-icon {
            font-size: 20px;
        }

        .test-name {
            flex: 1;
            font-weight: 500;
        }

        .test-duration {
            color: #666;
            font-size: 14px;
        }

        .test-details {
            padding: 15px;
            border-top: 1px solid #e5e7eb;
            background: #f9fafb;
        }

        .test-error {
            margin-bottom: 15px;
        }

        .test-error h4 {
            color: #ef4444;
            margin-bottom: 10px;
        }

        .test-error pre {
            background: #fef2f2;
            border: 1px solid #fecaca;
            padding: 10px;
            border-radius: 5px;
            overflow-x: auto;
            font-size: 12px;
        }

        .test-screenshots h4,
        .test-metrics h4 {
            margin-bottom: 10px;
            color: #667eea;
        }

        .screenshot-gallery {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(200px, 1fr));
            gap: 10px;
        }

        .screenshot-item img {
            width: 100%;
            height: auto;
            border-radius: 5px;
            border: 1px solid #e5e7eb;
            cursor: pointer;
            transition: transform 0.3s ease;
        }

        .screenshot-item img:hover {
            transform: scale(1.05);
        }

        .metrics-table {
            width: 100%;
            border-collapse: collapse;
        }

        .metrics-table tr {
            border-bottom: 1px solid #e5e7eb;
        }

        .metrics-table td {
            padding: 8px;
        }

        .metrics-table td:first-child {
            font-weight: 500;
            color: #666;
        }

        .metrics-table td:last-child {
            text-align: right;
            font-family: 'Courier New', monospace;
        }
        """

    def _get_javascript(self) -> str:
        """Get JavaScript for interactivity."""
        return """
        function toggleTest(header) {
            const details = header.nextElementSibling;
            if (details.style.display === 'none') {
                details.style.display = 'block';
            } else {
                details.style.display = 'none';
            }
        }

        // Click on screenshot to view full size
        document.addEventListener('DOMContentLoaded', function() {
            const screenshots = document.querySelectorAll('.screenshot-item img');
            screenshots.forEach(img => {
                img.addEventListener('click', function() {
                    window.open(this.src, '_blank');
                });
            });
        });
        """
