"""Interactive test debugger with pause/resume and step-through."""
import pdb
from typing import Optional

class InteractiveDebugger:
    """
    Interactive test debugger.

    Features:
    - Pause/resume test execution
    - Step-through execution
    - State inspection
    - Breakpoints

    Usage:
        debugger = InteractiveDebugger()
        debugger.set_breakpoint()  # Pauses execution
        debugger.step()  # Step to next line
        debugger.inspect(driver)  # Inspect variable
    """
    def __init__(self):
        self.enabled = False
        self.breakpoints = []

    def enable(self):
        """Enable interactive debugging mode."""
        self.enabled = True

    def set_breakpoint(self):
        """Pause execution at this point."""
        if self.enabled:
            pdb.set_trace()

    def step(self):
        """Step to next line."""
        if self.enabled:
            pdb.set_trace()

    def inspect(self, obj):
        """Inspect object state."""
        if self.enabled:
            print(f"Inspecting {type(obj).__name__}: {vars(obj)}")
            pdb.set_trace()
