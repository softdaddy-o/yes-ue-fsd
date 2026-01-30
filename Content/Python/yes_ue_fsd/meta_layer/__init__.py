"""
Meta Layer - Test Orchestration Framework

This module provides tools for launching and coordinating multiple
Unreal Engine editor instances for automated testing.

Components:
- EditorLauncher: Launch and manage editor instances
- EditorInstance: Represent a running editor
- TestRunner: Execute test scenarios
- TestScenario: Define multi-instance tests
- ResultAggregator: Collect and report results
"""

from .ue_launcher import EditorLauncher, EditorInstance
from .test_runner import TestRunner, TestScenario, TestResult, InstanceResult
from .result_aggregator import ResultAggregator

__all__ = [
    "EditorLauncher",
    "EditorInstance",
    "TestRunner",
    "TestScenario",
    "TestResult",
    "InstanceResult",
    "ResultAggregator",
]
