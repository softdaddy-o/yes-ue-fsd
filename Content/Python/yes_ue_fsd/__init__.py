"""
Yes UE FSD - Automated Test Framework for Unreal Engine

This package provides a two-layer architecture for automated testing:
- Meta Layer: Test orchestration and multi-instance management
- Local Control Layer: In-game player automation

Example:
    # Single-player test
    from yes_ue_fsd import AutoDriver
    driver = AutoDriver(player_index=0)
    driver.move_to(location=(100, 200, 50))

    # Multi-player test orchestration
    from yes_ue_fsd.meta_layer import EditorLauncher, TestRunner, TestScenario
    launcher = EditorLauncher(project_path="MyProject.uproject")
    instances = await launcher.launch_instances(count=2)
"""

__version__ = "2.0.0"
__author__ = "Yes UE FSD Team"

# Local control layer (in-game automation)
try:
    import unreal
    # Only import when running inside UE
    from .local_layer.autodriver import AutoDriver
    from .local_layer.recording import ActionRecorder, ActionPlayer
    from .local_layer.instance_context import instance
    __all__ = ["AutoDriver", "ActionRecorder", "ActionPlayer", "instance"]
except ImportError:
    # Running outside UE (meta layer)
    __all__ = []

# Meta layer (test orchestration) - always available
from .meta_layer import EditorLauncher, EditorInstance, TestRunner, TestScenario, ResultAggregator

__all__ += [
    "EditorLauncher",
    "EditorInstance",
    "TestRunner",
    "TestScenario",
    "ResultAggregator",
]
