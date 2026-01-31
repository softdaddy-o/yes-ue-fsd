"""Performance profiling for tests."""
import time
from contextlib import contextmanager
from typing import Dict, Any

class PerformanceProfiler:
    """Profile test performance metrics."""
    def __init__(self):
        self.metrics: Dict[str, Any] = {}

    @contextmanager
    def profile(self, name: str):
        """Profile a code block."""
        start = time.time()
        yield
        duration = time.time() - start
        self.metrics[f"{name}_duration"] = duration

    def get_metrics(self) -> Dict[str, Any]:
        """Get collected metrics."""
        return self.metrics.copy()
