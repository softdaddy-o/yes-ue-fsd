#!/usr/bin/env python3
"""
Parse Unreal Engine automation test results and generate summary.
"""

import sys
import json
import glob
import os
from pathlib import Path


def parse_ue_test_results(results_dir):
    """Parse UE automation test JSON results."""
    results_file = Path(results_dir) / "index.json"

    if not results_file.exists():
        print(f"❌ No test results found at {results_file}")
        return None

    with open(results_file, 'r') as f:
        data = json.load(f)

    return data


def generate_summary(test_data):
    """Generate test summary from parsed data."""
    if not test_data:
        return "No test data available"

    total_tests = test_data.get('TotalTests', 0)
    passed_tests = test_data.get('SucceededTests', 0)
    failed_tests = test_data.get('FailedTests', 0)
    warnings = test_data.get('Warnings', 0)

    success_rate = (passed_tests / total_tests * 100) if total_tests > 0 else 0

    summary = f"""
## Test Results Summary

- **Total Tests**: {total_tests}
- **Passed**: {passed_tests} ✅
- **Failed**: {failed_tests} ❌
- **Warnings**: {warnings} ⚠️
- **Success Rate**: {success_rate:.1f}%

"""

    if failed_tests > 0:
        summary += "\n### Failed Tests\n\n"
        for test in test_data.get('Tests', []):
            if test.get('State') == 'Fail':
                summary += f"- `{test.get('TestDisplayName', 'Unknown')}`\n"
                summary += f"  - Error: {test.get('Errors', ['No error message'])[0]}\n"

    return summary


def main():
    if len(sys.argv) < 2:
        print("Usage: parse_test_results.py <results_directory>")
        sys.exit(1)

    results_dir = sys.argv[1]

    print(f"📊 Parsing test results from {results_dir}")

    test_data = parse_ue_test_results(results_dir)
    summary = generate_summary(test_data)

    print(summary)

    # Write summary to file
    with open(Path(results_dir) / "summary.md", 'w') as f:
        f.write(summary)

    # Exit with error code if tests failed
    if test_data and test_data.get('FailedTests', 0) > 0:
        sys.exit(1)


if __name__ == "__main__":
    main()
