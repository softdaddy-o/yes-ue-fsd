#!/usr/bin/env python3
"""
Combine all test reports into a single summary.
"""

import sys
import glob
from pathlib import Path


def read_summary_files(results_dir):
    """Read all summary.md files from subdirectories."""
    summaries = {}

    for summary_file in Path(results_dir).rglob("summary.md"):
        category = summary_file.parent.name
        with open(summary_file, 'r') as f:
            summaries[category] = f.read()

    for perf_file in Path(results_dir).rglob("performance_report.md"):
        with open(perf_file, 'r') as f:
            summaries['performance'] = f.read()

    return summaries


def combine_reports(summaries):
    """Combine all summaries into one report."""
    report = """# 🧪 Yes UE FSD - Test Results

## Overview

This report combines results from all test suites.

---

"""

    # Add each summary
    for category, content in summaries.items():
        report += f"## {category.replace('-', ' ').title()}\n\n"
        report += content
        report += "\n---\n\n"

    # Add timestamp
    from datetime import datetime
    report += f"\n*Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}*\n"

    return report


def generate_html_report(combined_report):
    """Generate HTML version of the report."""
    html = f"""<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Yes UE FSD - Test Results</title>
    <style>
        body {{
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, sans-serif;
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
            background: #f5f5f5;
        }}
        .container {{
            background: white;
            padding: 30px;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }}
        h1 {{
            color: #333;
            border-bottom: 3px solid #4CAF50;
            padding-bottom: 10px;
        }}
        h2 {{
            color: #555;
            margin-top: 30px;
        }}
        .pass {{ color: #4CAF50; }}
        .fail {{ color: #f44336; }}
        .warn {{ color: #ff9800; }}
        code {{
            background: #f4f4f4;
            padding: 2px 6px;
            border-radius: 3px;
            font-family: 'Courier New', monospace;
        }}
        pre {{
            background: #f4f4f4;
            padding: 15px;
            border-radius: 5px;
            overflow-x: auto;
        }}
        hr {{
            border: none;
            border-top: 1px solid #ddd;
            margin: 30px 0;
        }}
    </style>
</head>
<body>
    <div class="container">
        <div id="content"></div>
    </div>
    <script src="https://cdn.jsdelivr.net/npm/marked/marked.min.js"></script>
    <script>
        const markdown = `{combined_report.replace('`', '\\`')}`;
        document.getElementById('content').innerHTML = marked.parse(markdown);
    </script>
</body>
</html>"""
    return html


def main():
    if len(sys.argv) < 2:
        print("Usage: combine_test_reports.py <results_directory>")
        sys.exit(1)

    results_dir = sys.argv[1]

    print(f"📋 Combining test reports from {results_dir}")

    summaries = read_summary_files(results_dir)
    combined_report = combine_reports(summaries)

    # Write markdown version
    markdown_file = Path(results_dir) / "summary.md"
    with open(markdown_file, 'w') as f:
        f.write(combined_report)

    print(f"✅ Markdown report written to {markdown_file}")

    # Write HTML version
    html_report = generate_html_report(combined_report)
    html_file = Path(results_dir) / "combined-report.html"
    with open(html_file, 'w') as f:
        f.write(html_report)

    print(f"✅ HTML report written to {html_file}")

    # Print summary to console
    print("\n" + combined_report)


if __name__ == "__main__":
    main()
