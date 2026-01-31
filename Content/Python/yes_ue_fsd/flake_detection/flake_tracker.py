"""
Flake detection and tracking system.

Tracks test failures and retries to identify flaky tests.
"""

import json
import time
from pathlib import Path
from dataclasses import dataclass, field, asdict
from typing import Dict, List, Optional
from datetime import datetime


@dataclass
class TestRun:
    """Record of a single test run."""
    timestamp: float
    outcome: str  # "passed", "failed", "rerun"
    duration: float
    error_message: Optional[str] = None


@dataclass
class FlakeStatistics:
    """Statistics for a potentially flaky test."""
    test_id: str
    total_runs: int = 0
    total_passes: int = 0
    total_failures: int = 0
    total_reruns: int = 0
    runs: List[TestRun] = field(default_factory=list)

    @property
    def flakiness_score(self) -> float:
        """
        Calculate flakiness score (0.0 - 1.0).

        1.0 = highly flaky (fails then passes often)
        0.0 = stable (always passes or always fails)
        """
        if self.total_runs == 0:
            return 0.0

        if self.total_reruns == 0:
            return 0.0  # Never needed retry = stable

        # Score based on retry rate
        return self.total_reruns / self.total_runs

    @property
    def is_flaky(self) -> bool:
        """Test is considered flaky if it ever needed a retry."""
        return self.total_reruns > 0

    @property
    def pass_rate(self) -> float:
        """Percentage of runs that passed."""
        if self.total_runs == 0:
            return 0.0
        return self.total_passes / self.total_runs

    def add_run(self, outcome: str, duration: float, error: Optional[str] = None):
        """Record a test run."""
        run = TestRun(
            timestamp=time.time(),
            outcome=outcome,
            duration=duration,
            error_message=error,
        )

        self.runs.append(run)
        self.total_runs += 1

        if outcome == "passed":
            self.total_passes += 1
        elif outcome == "failed":
            self.total_failures += 1
        elif outcome == "rerun":
            self.total_reruns += 1


class FlakeTracker:
    """
    Tracks flaky tests across test runs.

    Maintains a history of test outcomes to identify patterns of flakiness.
    """

    def __init__(self, history_file: str = "test_results/flake_history.json"):
        """
        Initialize flake tracker.

        Args:
            history_file: Path to flake history JSON file
        """
        self.history_file = Path(history_file)
        self.history_file.parent.mkdir(parents=True, exist_ok=True)

        self.tests: Dict[str, FlakeStatistics] = {}
        self.load_history()

    def load_history(self):
        """Load flake history from file."""
        if not self.history_file.exists():
            return

        try:
            with open(self.history_file, "r") as f:
                data = json.load(f)

            for test_id, test_data in data.items():
                stats = FlakeStatistics(
                    test_id=test_id,
                    total_runs=test_data["total_runs"],
                    total_passes=test_data["total_passes"],
                    total_failures=test_data["total_failures"],
                    total_reruns=test_data["total_reruns"],
                    runs=[
                        TestRun(**run_data)
                        for run_data in test_data.get("runs", [])
                    ],
                )
                self.tests[test_id] = stats

        except Exception as e:
            print(f"Warning: Failed to load flake history: {e}")

    def save_history(self):
        """Save flake history to file."""
        try:
            data = {}
            for test_id, stats in self.tests.items():
                data[test_id] = {
                    "test_id": stats.test_id,
                    "total_runs": stats.total_runs,
                    "total_passes": stats.total_passes,
                    "total_failures": stats.total_failures,
                    "total_reruns": stats.total_reruns,
                    "runs": [asdict(run) for run in stats.runs[-100:]],  # Keep last 100
                }

            with open(self.history_file, "w") as f:
                json.dump(data, f, indent=2)

        except Exception as e:
            print(f"Warning: Failed to save flake history: {e}")

    def record_test(
        self,
        test_id: str,
        outcome: str,
        duration: float,
        error: Optional[str] = None,
    ):
        """
        Record a test run.

        Args:
            test_id: Test identifier
            outcome: Test outcome ("passed", "failed", "rerun")
            duration: Test duration in seconds
            error: Error message if failed
        """
        if test_id not in self.tests:
            self.tests[test_id] = FlakeStatistics(test_id=test_id)

        self.tests[test_id].add_run(outcome, duration, error)

    def get_flaky_tests(self, min_flakiness: float = 0.1) -> List[FlakeStatistics]:
        """
        Get list of flaky tests.

        Args:
            min_flakiness: Minimum flakiness score to be considered flaky

        Returns:
            List of flaky test statistics, sorted by flakiness score
        """
        flaky = [
            stats
            for stats in self.tests.values()
            if stats.flakiness_score >= min_flakiness
        ]

        return sorted(flaky, key=lambda s: s.flakiness_score, reverse=True)

    def get_test_stats(self, test_id: str) -> Optional[FlakeStatistics]:
        """Get statistics for a specific test."""
        return self.tests.get(test_id)

    def generate_report(self) -> str:
        """
        Generate flakiness report.

        Returns:
            Markdown-formatted report
        """
        flaky_tests = self.get_flaky_tests(min_flakiness=0.01)

        report = f"""# Flaky Test Report

Generated: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}

## Summary

- Total tests tracked: {len(self.tests)}
- Flaky tests detected: {len(flaky_tests)}
- Flakiness threshold: 1%

## Flaky Tests

"""

        if not flaky_tests:
            report += "*No flaky tests detected!*\n"
        else:
            report += "| Test | Flakiness | Pass Rate | Runs | Reruns |\n"
            report += "|------|-----------|-----------|------|--------|\n"

            for stats in flaky_tests[:20]:  # Top 20
                report += f"| `{stats.test_id}` | "
                report += f"{stats.flakiness_score:.1%} | "
                report += f"{stats.pass_rate:.1%} | "
                report += f"{stats.total_runs} | "
                report += f"{stats.total_reruns} |\n"

        report += f"\n---\n*Full data: {self.history_file}*\n"

        return report


# Global tracker instance
_flake_tracker = FlakeTracker()


def get_flake_tracker() -> FlakeTracker:
    """Get the global flake tracker instance."""
    return _flake_tracker


# Pytest integration
import pytest


@pytest.hookimpl(hookwrapper=True)
def pytest_runtest_makereport(item, call):
    """Track test outcomes for flake detection."""
    outcome = yield
    report = outcome.get_result()

    tracker = get_flake_tracker()

    if report.when == "call":
        test_id = item.nodeid

        if report.passed:
            outcome_str = "passed"
        elif report.failed:
            outcome_str = "failed"
        else:
            outcome_str = "skipped"

        error = str(report.longrepr) if report.failed else None

        tracker.record_test(test_id, outcome_str, report.duration, error)

    # Track reruns
    if hasattr(report, "wasxfail"):
        test_id = item.nodeid
        tracker.record_test(test_id, "rerun", report.duration)


def pytest_sessionfinish(session):
    """Save flake history at end of session."""
    tracker = get_flake_tracker()
    tracker.save_history()

    # Print flaky tests summary
    flaky_tests = tracker.get_flaky_tests(min_flakiness=0.1)
    if flaky_tests:
        print("\n" + "=" * 60)
        print("⚠️  FLAKY TESTS DETECTED")
        print("=" * 60)
        for stats in flaky_tests[:5]:  # Top 5
            print(f"  {stats.test_id}")
            print(f"    Flakiness: {stats.flakiness_score:.1%}")
            print(f"    Pass rate: {stats.pass_rate:.1%}")
            print(f"    Reruns: {stats.total_reruns}/{stats.total_runs}")
        print("=" * 60)
