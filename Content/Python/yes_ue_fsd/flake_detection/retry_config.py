"""
Retry configuration for flaky tests.
"""

from dataclasses import dataclass
from typing import Optional, List


@dataclass
class RetryConfig:
    """
    Configuration for test retry behavior.

    Attributes:
        max_reruns: Maximum number of times to retry a failed test
        reruns_delay: Delay between retries in seconds
        only_rerun: List of test IDs to apply retry (None = all tests)
        no_rerun: List of test IDs to never retry
        rerun_except: Exception types that should not trigger retry
    """

    max_reruns: int = 3
    reruns_delay: float = 1.0
    only_rerun: Optional[List[str]] = None
    no_rerun: Optional[List[str]] = None
    rerun_except: Optional[List[str]] = None

    def to_pytest_args(self) -> List[str]:
        """
        Convert to pytest-rerunfailures arguments.

        Returns:
            List of pytest arguments
        """
        args = []

        # Max reruns
        args.extend(["--reruns", str(self.max_reruns)])

        # Delay between reruns
        if self.reruns_delay > 0:
            args.extend(["--reruns-delay", str(self.reruns_delay)])

        # Only rerun specific tests
        if self.only_rerun:
            args.extend(["--only-rerun", ",".join(self.only_rerun)])

        return args


# Preset configurations
class RetryPresets:
    """Preset retry configurations."""

    @staticmethod
    def aggressive() -> RetryConfig:
        """Aggressive retries for flaky tests."""
        return RetryConfig(
            max_reruns=5,
            reruns_delay=2.0,
        )

    @staticmethod
    def balanced() -> RetryConfig:
        """Balanced retry configuration."""
        return RetryConfig(
            max_reruns=3,
            reruns_delay=1.0,
        )

    @staticmethod
    def conservative() -> RetryConfig:
        """Conservative retries."""
        return RetryConfig(
            max_reruns=1,
            reruns_delay=0.5,
        )

    @staticmethod
    def ci() -> RetryConfig:
        """CI/CD optimized retries."""
        return RetryConfig(
            max_reruns=2,
            reruns_delay=1.0,
        )
