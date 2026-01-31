"""
Enhanced reporting system for Yes UE FSD test framework.

This module provides rich HTML reports with:
- Screenshot integration
- Performance metrics visualization
- Test execution timelines
- Failure analytics
- Trend analysis
"""

from .html_reporter import EnhancedHTMLReporter
from .screenshot_integration import ScreenshotAttachment
from .performance_metrics import PerformanceMetrics
from .report_generator import ReportGenerator

__all__ = [
    "EnhancedHTMLReporter",
    "ScreenshotAttachment",
    "PerformanceMetrics",
    "ReportGenerator",
]
