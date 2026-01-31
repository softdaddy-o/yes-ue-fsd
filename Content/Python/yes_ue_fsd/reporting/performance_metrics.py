"""
Performance metrics collection and reporting.
"""

import time
from typing import Dict, List, Any, Optional
from dataclasses import dataclass, field
from statistics import mean, median, stdev


@dataclass
class PerformanceMetrics:
    """Container for performance metrics."""

    test_id: str
    metrics: Dict[str, List[float]] = field(default_factory=dict)
    start_time: Optional[float] = None
    end_time: Optional[float] = None

    def record_metric(self, name: str, value: float):
        """
        Record a performance metric.

        Args:
            name: Metric name (e.g., "fps", "memory_mb", "frame_time_ms")
            value: Metric value
        """
        if name not in self.metrics:
            self.metrics[name] = []
        self.metrics[name].append(value)

    def get_statistics(self, metric_name: str) -> Dict[str, float]:
        """
        Get statistics for a metric.

        Args:
            metric_name: Name of the metric

        Returns:
            Dictionary with mean, median, min, max, stdev
        """
        if metric_name not in self.metrics or not self.metrics[metric_name]:
            return {}

        values = self.metrics[metric_name]
        stats = {
            "mean": mean(values),
            "median": median(values),
            "min": min(values),
            "max": max(values),
            "count": len(values),
        }

        if len(values) > 1:
            stats["stdev"] = stdev(values)
        else:
            stats["stdev"] = 0.0

        return stats

    def get_summary(self) -> Dict[str, Dict[str, float]]:
        """
        Get summary of all metrics.

        Returns:
            Dictionary mapping metric names to their statistics
        """
        return {
            metric_name: self.get_statistics(metric_name)
            for metric_name in self.metrics.keys()
        }

    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary for serialization."""
        return {
            "test_id": self.test_id,
            "metrics": self.metrics,
            "summary": self.get_summary(),
            "start_time": self.start_time,
            "end_time": self.end_time,
            "duration": self.end_time - self.start_time if self.start_time and self.end_time else None,
        }


class PerformanceCollector:
    """
    Collects performance metrics during test execution.

    Example:
        collector = PerformanceCollector()
        collector.start_test("test_movement")

        with collector.profile("navigation"):
            # Test code here
            collector.record_metric("test_movement", "fps", 60.5)
            collector.record_metric("test_movement", "memory_mb", 1024.5)

        collector.end_test("test_movement")
        summary = collector.get_test_metrics("test_movement")
    """

    def __init__(self):
        """Initialize performance collector."""
        self.test_metrics: Dict[str, PerformanceMetrics] = {}
        self.current_test: Optional[str] = None

    def start_test(self, test_id: str):
        """
        Start collecting metrics for a test.

        Args:
            test_id: Test identifier
        """
        self.current_test = test_id
        self.test_metrics[test_id] = PerformanceMetrics(
            test_id=test_id,
            start_time=time.time()
        )

    def end_test(self, test_id: str):
        """
        Stop collecting metrics for a test.

        Args:
            test_id: Test identifier
        """
        if test_id in self.test_metrics:
            self.test_metrics[test_id].end_time = time.time()
        self.current_test = None

    def record_metric(self, test_id: str, metric_name: str, value: float):
        """
        Record a metric value.

        Args:
            test_id: Test identifier
            metric_name: Metric name
            value: Metric value
        """
        if test_id not in self.test_metrics:
            self.start_test(test_id)

        self.test_metrics[test_id].record_metric(metric_name, value)

    def get_test_metrics(self, test_id: str) -> Optional[PerformanceMetrics]:
        """
        Get metrics for a specific test.

        Args:
            test_id: Test identifier

        Returns:
            PerformanceMetrics instance or None
        """
        return self.test_metrics.get(test_id)

    def get_all_metrics(self) -> Dict[str, PerformanceMetrics]:
        """Get all collected metrics."""
        return self.test_metrics

    def get_summary_report(self) -> Dict[str, Any]:
        """
        Generate summary report of all metrics.

        Returns:
            Dictionary with summary statistics for all tests
        """
        return {
            test_id: metrics.to_dict()
            for test_id, metrics in self.test_metrics.items()
        }


# Global instance for easy access
_global_collector = PerformanceCollector()


def get_performance_collector() -> PerformanceCollector:
    """Get the global performance collector instance."""
    return _global_collector


# Pytest fixtures for performance testing
import pytest


@pytest.fixture
def performance_metrics(request):
    """
    Pytest fixture for automatic performance metric collection.

    Usage:
        def test_movement(performance_metrics):
            # Metrics are automatically collected
            performance_metrics.record("fps", 60.0)
            performance_metrics.record("memory_mb", 1024.0)
    """
    collector = get_performance_collector()
    test_id = request.node.nodeid

    class MetricsRecorder:
        def record(self, metric_name: str, value: float):
            collector.record_metric(test_id, metric_name, value)

        def get_summary(self):
            metrics = collector.get_test_metrics(test_id)
            return metrics.get_summary() if metrics else {}

    collector.start_test(test_id)
    recorder = MetricsRecorder()

    yield recorder

    collector.end_test(test_id)

    # Attach metrics to test report
    if hasattr(request.node, "user_properties"):
        metrics = collector.get_test_metrics(test_id)
        if metrics:
            for metric_name, values in metrics.metrics.items():
                stats = metrics.get_statistics(metric_name)
                request.node.user_properties.append(
                    (f"metric_{metric_name}_avg", stats.get("mean", 0))
                )
                request.node.user_properties.append(
                    (f"metric_{metric_name}_max", stats.get("max", 0))
                )
