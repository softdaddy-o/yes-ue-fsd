"""
Recording - Action recording and playback API

This module wraps the existing autodriver_helpers.py recording functionality
into the new two-layer architecture.
"""

import sys
from pathlib import Path

# Import existing autodriver_helpers
parent_dir = Path(__file__).parent.parent.parent
if str(parent_dir) not in sys.path:
    sys.path.insert(0, str(parent_dir))

try:
    from autodriver_helpers import ActionRecorder as _ActionRecorder
    from autodriver_helpers import ActionPlayer as _ActionPlayer

    # Re-export with same interface
    class ActionRecorder(_ActionRecorder):
        """
        ActionRecorder for recording player actions.

        This is a wrapper around the existing ActionRecorder implementation,
        enhanced for the two-layer architecture.
        """
        pass

    class ActionPlayer(_ActionPlayer):
        """
        ActionPlayer for playing back recorded actions.

        This is a wrapper around the existing ActionPlayer implementation,
        enhanced for the two-layer architecture.
        """
        pass

except ImportError:
    # Fallback if running outside UE
    class ActionRecorder:
        """Placeholder ActionRecorder when not in UE."""
        def __init__(self, player_index: int = 0):
            raise RuntimeError("ActionRecorder requires running inside Unreal Engine")

    class ActionPlayer:
        """Placeholder ActionPlayer when not in UE."""
        def __init__(self, player_index: int = 0):
            raise RuntimeError("ActionPlayer requires running inside Unreal Engine")
