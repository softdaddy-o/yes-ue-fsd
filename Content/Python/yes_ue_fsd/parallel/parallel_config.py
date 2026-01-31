"""
Configuration for parallel test execution.
"""

from dataclasses import dataclass
from typing import Optional


@dataclass
class ParallelConfig:
    """
    Configuration for parallel test execution.

    Attributes:
        workers: Number of parallel workers (-1 = auto-detect CPUs)
        dist_mode: Distribution strategy ("load", "loadscope", "loadfile")
        max_worker_restart: Maximum worker restarts before giving up
        timeout: Worker timeout in seconds
        instance_pool_size: Size of editor instance pool
        base_port: Base port for instance allocation
        enable_live_logs: Show live logs from all workers
        report_mode: Reporting mode ("individual", "aggregated")
    """

    workers: int = -1  # Auto-detect
    dist_mode: str = "loadscope"  # Group by test module
    max_worker_restart: int = 3
    timeout: float = 300.0
    instance_pool_size: Optional[int] = None  # Same as workers if None
    base_port: int = 8000
    enable_live_logs: bool = False
    report_mode: str = "aggregated"

    @property
    def worker_count(self) -> int:
        """Get actual worker count."""
        if self.workers <= 0:
            import multiprocessing
            return multiprocessing.cpu_count()
        return self.workers

    @property
    def pool_size(self) -> int:
        """Get instance pool size."""
        if self.instance_pool_size is None:
            return self.worker_count
        return self.instance_pool_size

    def to_pytest_args(self) -> list:
        """
        Convert to pytest command-line arguments.

        Returns:
            List of pytest-xdist arguments
        """
        args = []

        # Worker count
        if self.workers == -1:
            args.extend(["-n", "auto"])
        else:
            args.extend(["-n", str(self.workers)])

        # Distribution mode
        args.extend(["--dist", self.dist_mode])

        # Worker restart
        args.extend(["--max-worker-restart", str(self.max_worker_restart)])

        # Live logs
        if self.enable_live_logs:
            args.append("--log-cli-level=INFO")

        return args

    @classmethod
    def from_pytest_args(cls, args: list) -> "ParallelConfig":
        """
        Parse from pytest arguments.

        Args:
            args: pytest command-line arguments

        Returns:
            ParallelConfig instance
        """
        config = cls()

        # Parse -n/--numprocesses
        if "-n" in args:
            idx = args.index("-n")
            if idx + 1 < len(args):
                value = args[idx + 1]
                if value == "auto":
                    config.workers = -1
                else:
                    config.workers = int(value)

        # Parse --dist
        if "--dist" in args:
            idx = args.index("--dist")
            if idx + 1 < len(args):
                config.dist_mode = args[idx + 1]

        return config


# Preset configurations
class PresetConfigs:
    """Preset parallel execution configurations."""

    @staticmethod
    def fast() -> ParallelConfig:
        """Fast configuration: Use all CPU cores."""
        return ParallelConfig(
            workers=-1,
            dist_mode="load",
            timeout=60.0,
        )

    @staticmethod
    def balanced() -> ParallelConfig:
        """Balanced configuration: Use half of CPU cores."""
        import multiprocessing
        return ParallelConfig(
            workers=max(1, multiprocessing.cpu_count() // 2),
            dist_mode="loadscope",
            timeout=120.0,
        )

    @staticmethod
    def conservative() -> ParallelConfig:
        """Conservative configuration: Use 2-4 workers."""
        import multiprocessing
        return ParallelConfig(
            workers=min(4, multiprocessing.cpu_count()),
            dist_mode="loadscope",
            timeout=300.0,
        )

    @staticmethod
    def ci() -> ParallelConfig:
        """CI configuration: Optimized for CI/CD environments."""
        return ParallelConfig(
            workers=4,
            dist_mode="loadfile",
            max_worker_restart=5,
            timeout=600.0,
            enable_live_logs=True,
        )
