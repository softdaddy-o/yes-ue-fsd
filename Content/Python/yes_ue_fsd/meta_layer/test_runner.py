"""
Test Runner - Execute test scenarios across multiple instances

This module provides tools for defining and running test scenarios
with multiple editor instances.
"""

import asyncio
import time
from dataclasses import dataclass, field
from typing import Any, Dict, List, Optional

from .ue_launcher import EditorLauncher, EditorInstance


@dataclass
class InstanceResult:
    """Result from a single instance."""

    instance_id: int
    role: str
    success: bool
    duration: float
    errors: List[str] = field(default_factory=list)
    metrics: Dict[str, Any] = field(default_factory=dict)
    screenshots: List[str] = field(default_factory=list)
    logs: List[str] = field(default_factory=list)

    def __str__(self) -> str:
        status = "✓" if self.success else "✗"
        return f"{status} Instance {self.instance_id} ({self.role}): {self.duration:.2f}s"


@dataclass
class TestResult:
    """Test execution result."""

    scenario_name: str
    success: bool
    duration: float
    instance_results: List[InstanceResult] = field(default_factory=list)
    errors: List[str] = field(default_factory=list)
    screenshots: List[str] = field(default_factory=list)
    logs: Dict[int, List[str]] = field(default_factory=dict)

    def __str__(self) -> str:
        status = "✓ PASS" if self.success else "✗ FAIL"
        return f"{status} {self.scenario_name} ({self.duration:.2f}s, {len(self.instance_results)} instances)"

    def get_summary(self) -> Dict[str, Any]:
        """Get summary statistics."""
        return {
            "name": self.scenario_name,
            "success": self.success,
            "duration": self.duration,
            "instances": len(self.instance_results),
            "passed": sum(1 for r in self.instance_results if r.success),
            "failed": sum(1 for r in self.instance_results if not r.success),
            "errors": len(self.errors),
            "screenshots": len(self.screenshots),
        }


class TestScenario:
    """Defines a multi-instance test scenario."""

    def __init__(
        self,
        name: str,
        instances: int = 1,
        roles: Optional[Dict[int, str]] = None,
        timeout: float = 300.0
    ):
        """
        Initialize test scenario.

        Args:
            name: Scenario name
            instances: Number of instances needed
            roles: Role assignments {instance_id: role}
            timeout: Maximum execution time
        """
        self.name = name
        self.instances = instances
        self.roles = roles or {}
        self.timeout = timeout
        self.scripts: Dict[str, str] = {}
        self._shared_script: Optional[str] = None

    def set_script(self, role: str, script_path: str):
        """
        Assign a script to a role.

        Args:
            role: Role name
            script_path: Path to Python script
        """
        self.scripts[role] = script_path

    def set_script_for_all(self, script_path: str):
        """
        Use same script for all instances.

        Args:
            script_path: Path to Python script
        """
        self._shared_script = script_path

    def get_script_for_instance(self, instance_id: int) -> Optional[str]:
        """
        Get script path for an instance.

        Args:
            instance_id: Instance ID

        Returns:
            Script path or None
        """
        # Check if instance has a specific role
        role = self.roles.get(instance_id)
        if role and role in self.scripts:
            return self.scripts[role]

        # Fall back to shared script
        return self._shared_script

    def get_role_list(self) -> List[str]:
        """
        Get list of roles for all instances.

        Returns:
            List of roles (indexed by instance_id)
        """
        return [
            self.roles.get(i, f"instance_{i}")
            for i in range(self.instances)
        ]

    def get_script_list(self) -> List[Optional[str]]:
        """
        Get list of scripts for all instances.

        Returns:
            List of script paths (indexed by instance_id)
        """
        return [
            self.get_script_for_instance(i)
            for i in range(self.instances)
        ]


class TestRunner:
    """Orchestrates test execution."""

    def __init__(self, launcher: EditorLauncher):
        """
        Initialize with editor launcher.

        Args:
            launcher: EditorLauncher instance
        """
        self.launcher = launcher

    async def run_scenario(
        self,
        scenario: TestScenario,
        timeout: Optional[float] = None
    ) -> TestResult:
        """
        Run a test scenario.

        Args:
            scenario: Test scenario to run
            timeout: Override scenario timeout

        Returns:
            Test result
        """
        print(f"\n[TestRunner] Running scenario: {scenario.name}")
        print(f"[TestRunner] Instances: {scenario.instances}")
        print(f"[TestRunner] Timeout: {timeout or scenario.timeout}s")

        start_time = time.time()
        timeout = timeout or scenario.timeout

        try:
            # Launch instances
            print(f"[TestRunner] Launching {scenario.instances} instances...")
            instances = await self.launcher.launch_instances(
                count=scenario.instances,
                roles=scenario.get_role_list(),
                scripts=scenario.get_script_list()
            )

            # Wait for all instances to complete or timeout
            print(f"[TestRunner] Waiting for test completion...")
            instance_results = await self._wait_for_completion(
                instances,
                timeout=timeout
            )

            # Calculate overall result
            duration = time.time() - start_time
            success = all(r.success for r in instance_results)

            result = TestResult(
                scenario_name=scenario.name,
                success=success,
                duration=duration,
                instance_results=instance_results
            )

            # Collect logs and screenshots from all instances
            for instance in instances:
                instance_id = instance.config.instance_id
                result.logs[instance_id] = instance.get_logs()

            print(f"[TestRunner] {result}")
            return result

        except Exception as e:
            duration = time.time() - start_time
            print(f"[TestRunner] Scenario failed with exception: {e}")
            return TestResult(
                scenario_name=scenario.name,
                success=False,
                duration=duration,
                errors=[str(e)]
            )

    async def run_scenarios(
        self,
        scenarios: List[TestScenario],
        fail_fast: bool = False
    ) -> List[TestResult]:
        """
        Run multiple scenarios sequentially.

        Args:
            scenarios: List of test scenarios
            fail_fast: Stop on first failure

        Returns:
            List of test results
        """
        print(f"\n[TestRunner] Running {len(scenarios)} scenario(s)...")

        results = []
        for i, scenario in enumerate(scenarios, 1):
            print(f"\n[TestRunner] === Scenario {i}/{len(scenarios)} ===")

            result = await self.run_scenario(scenario)
            results.append(result)

            if not result.success and fail_fast:
                print(f"[TestRunner] Stopping due to failure (fail_fast=True)")
                break

            # Cleanup between scenarios
            await self.launcher.shutdown_all()
            await asyncio.sleep(2.0)  # Cool-down period

        # Print summary
        print(f"\n[TestRunner] === Summary ===")
        passed = sum(1 for r in results if r.success)
        failed = sum(1 for r in results if not r.success)
        print(f"[TestRunner] Total: {len(results)}")
        print(f"[TestRunner] Passed: {passed}")
        print(f"[TestRunner] Failed: {failed}")

        return results

    async def _wait_for_completion(
        self,
        instances: List[EditorInstance],
        timeout: float
    ) -> List[InstanceResult]:
        """
        Wait for all instances to complete their tests.

        Args:
            instances: List of instances to monitor
            timeout: Maximum time to wait

        Returns:
            List of instance results
        """
        start_time = time.time()
        results: List[InstanceResult] = []

        # Create result objects for each instance
        for instance in instances:
            results.append(InstanceResult(
                instance_id=instance.config.instance_id,
                role=instance.config.role,
                success=False,
                duration=0.0
            ))

        # Monitor instances until all complete or timeout
        while time.time() - start_time < timeout:
            all_done = True

            for i, instance in enumerate(instances):
                if results[i].success:
                    # Already completed successfully
                    continue

                if not instance.is_running():
                    # Instance stopped
                    elapsed = time.time() - start_time
                    results[i].duration = elapsed

                    # Check exit code to determine success
                    if instance.process and instance.process.returncode == 0:
                        results[i].success = True
                        print(f"[TestRunner] Instance {instance.config.instance_id} completed successfully")
                    else:
                        results[i].success = False
                        results[i].errors.append("Process exited with error")
                        print(f"[TestRunner] Instance {instance.config.instance_id} failed")
                else:
                    all_done = False

            if all_done:
                print(f"[TestRunner] All instances completed")
                break

            await asyncio.sleep(0.5)

        # Check for timeout
        if time.time() - start_time >= timeout:
            print(f"[TestRunner] Timeout reached ({timeout}s)")
            for i, result in enumerate(results):
                if not result.success and not result.errors:
                    result.errors.append("Timeout")

        # Collect final metrics
        for i, instance in enumerate(instances):
            if instance.is_running():
                results[i].metrics["memory_mb"] = instance.get_memory_usage() or 0
                results[i].metrics["cpu_percent"] = instance.get_cpu_percent() or 0

        return results
