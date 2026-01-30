"""
AutoDriver - Player control and automation API

This module wraps the existing autodriver_helpers.py functionality
into the new two-layer architecture.
"""

import sys
from pathlib import Path

# Import existing autodriver_helpers
parent_dir = Path(__file__).parent.parent.parent
if str(parent_dir) not in sys.path:
    sys.path.insert(0, str(parent_dir))

try:
    from autodriver_helpers import AutoDriver as _AutoDriver

    # Re-export with same interface
    class AutoDriver(_AutoDriver):
        """
        AutoDriver for player control and automation.

        This is a wrapper around the existing AutoDriver implementation,
        enhanced for the two-layer architecture.
        """
        pass

except ImportError:
    # Fallback if running outside UE
    class AutoDriver:
        """Placeholder AutoDriver when not in UE."""
        def __init__(self, player_index: int = 0):
            raise RuntimeError("AutoDriver requires running inside Unreal Engine")
