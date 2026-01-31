"""Baseline screenshot management."""
from pathlib import Path
import shutil

class BaselineManager:
    """Manage baseline screenshots for visual regression testing."""
    def __init__(self, baseline_dir: str = "test_results/baselines"):
        self.baseline_dir = Path(baseline_dir)
        self.baseline_dir.mkdir(parents=True, exist_ok=True)

    def get_baseline(self, test_id: str) -> Path:
        """Get baseline screenshot path for test."""
        return self.baseline_dir / f"{test_id}.png"

    def save_baseline(self, test_id: str, screenshot_path: str):
        """Save screenshot as baseline."""
        baseline_path = self.get_baseline(test_id)
        shutil.copy(screenshot_path, baseline_path)

    def has_baseline(self, test_id: str) -> bool:
        """Check if baseline exists."""
        return self.get_baseline(test_id).exists()
