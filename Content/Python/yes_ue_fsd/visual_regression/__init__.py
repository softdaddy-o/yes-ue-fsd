"""Visual regression testing support."""
from .image_comparator import ImageComparator, ComparisonResult
from .baseline_manager import BaselineManager

__all__ = ["ImageComparator", "ComparisonResult", "BaselineManager"]
