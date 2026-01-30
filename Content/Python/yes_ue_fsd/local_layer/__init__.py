"""
Local Control Layer - In-Game Automation API

This module provides the Python API for controlling player actions
within a running Unreal Engine editor instance.

Components:
- AutoDriver: Player control and automation
- ActionRecorder: Record player actions
- ActionPlayer: Playback recorded actions
- instance: Current instance context
"""

from .autodriver import AutoDriver
from .recording import ActionRecorder, ActionPlayer
from .instance_context import instance, InstanceContext

__all__ = [
    "AutoDriver",
    "ActionRecorder",
    "ActionPlayer",
    "instance",
    "InstanceContext",
]
