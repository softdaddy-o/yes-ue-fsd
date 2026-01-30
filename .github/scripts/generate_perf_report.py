#!/usr/bin/env python3
"""
Generate performance report from UE automation test results.
"""

import sys
import json
from pathlib import Path
from collections import defaultdict


def parse_perf_results(results_dir):
    """Parse performance test results."""
    results_file = Path(results_dir) / "index.json"

    if not results_file.exists():
        print(f"❌ No performance results found at {results_file}")
        return None

    with open(results_file, 'r') as f:
        data = json.load(f)

    return data


def extract_perf_metrics(test_data):
    """Extract performance metrics from test entries."""
    metrics = defaultdict(list)

    for test in test_data.get('Tests', []):
        test_name = test.get('TestDisplayName', 'Unknown')
        entries = test.get('Entries', [])

        for entry in entries:
            event = entry.get('Event', {})
            message = event.get('Message', '')

            # Parse timing information from messages
            if 'ms' in message or 'seconds' in message:
                metrics[test_name].append(message)

    return metrics


def generate_perf_report(test_data):
    """Generate performance report."""
    if not test_data:
        return "No performance data available"

    metrics = extract_perf_metrics(test_data)

    report = """
## Performance Test Results

"""

    if not metrics:
        report += "No performance metrics collected.\n"
        return report

    for test_name, messages in metrics.items():
        report += f"### {test_name}\n\n"
        for message in messages:
            report += f"- {message}\n"
        report += "\n"

    return report


def check_performance_thresholds(metrics):
    """Check if performance meets thresholds."""
    warnings = []

    # Define thresholds (these can be customized)
    thresholds = {
        'Component.Creation': 1.0,  # ms per component
        'Command.Execution': 0.5,  # ms per command
        'Tick.Multiple': 16.0,      # ms per frame
    }

    # Check against thresholds
    for test_name, messages in metrics.items():
        for threshold_key, threshold_value in thresholds.items():
            if threshold_key in test_name:
                # Parse timing from messages and compare
                # (simplified - actual implementation would parse numbers)
                pass

    return warnings


def main():
    if len(sys.argv) < 2:
        print("Usage: generate_perf_report.py <results_directory>")
        sys.exit(1)

    results_dir = sys.argv[1]

    print(f"📈 Generating performance report from {results_dir}")

    test_data = parse_perf_results(results_dir)
    report = generate_perf_report(test_data)

    print(report)

    # Write report to file
    with open(Path(results_dir) / "performance_report.md", 'w') as f:
        f.write(report)

    print("✅ Performance report generated")


if __name__ == "__main__":
    main()
