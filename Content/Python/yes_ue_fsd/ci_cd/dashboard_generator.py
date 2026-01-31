"""Dashboard generator for CI/CD test results."""
from typing import List, Dict, Any
from pathlib import Path

class DashboardGenerator:
    """Generate test result dashboards for CI/CD."""
    def __init__(self, output_dir: str = "dashboard"):
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(parents=True, exist_ok=True)

    def generate_dashboard(self, test_results: List[Dict[str, Any]]) -> str:
        """Generate HTML dashboard."""
        html = f"""
        <!DOCTYPE html>
        <html>
        <head><title>Test Dashboard</title></head>
        <body>
            <h1>Test Results Dashboard</h1>
            <p>Total Tests: {len(test_results)}</p>
        </body>
        </html>
        """

        dashboard_path = self.output_dir / "dashboard.html"
        dashboard_path.write_text(html)
        return str(dashboard_path)
