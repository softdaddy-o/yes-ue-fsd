"""
Editor Launcher - Launch and manage UE editor instances

This module provides tools for starting, stopping, and managing
multiple Unreal Engine editor instances for automated testing.
"""

import asyncio
import os
import subprocess
import sys
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Dict, List, Optional
import psutil


@dataclass
class EditorConfig:
    """Configuration for an editor instance."""

    project_path: str
    editor_path: Optional[str] = None
    role: str = "client"
    instance_id: int = 0
    port: Optional[int] = None
    python_script: Optional[str] = None
    extra_args: List[str] = field(default_factory=list)
    env_vars: Dict[str, str] = field(default_factory=dict)

    def __post_init__(self):
        """Validate configuration."""
        if not os.path.exists(self.project_path):
            raise FileNotFoundError(f"Project not found: {self.project_path}")

        if self.editor_path and not os.path.exists(self.editor_path):
            raise FileNotFoundError(f"Editor not found: {self.editor_path}")


class EditorInstance:
    """Represents a running UE editor instance."""

    def __init__(self, config: EditorConfig):
        """
        Initialize editor instance configuration.

        Args:
            config: Editor configuration
        """
        self.config = config
        self.process: Optional[subprocess.Popen] = None
        self._logs: List[str] = []
        self._ready = False
        self._start_time: Optional[float] = None

    async def start(self) -> bool:
        """
        Start the editor instance.

        Returns:
            True if started successfully
        """
        if self.process is not None:
            print(f"[Instance {self.config.instance_id}] Already running")
            return False

        # Find editor executable if not specified
        editor_path = self.config.editor_path or self._find_editor()
        if not editor_path:
            raise RuntimeError("Could not find UnrealEditor executable")

        # Build command line
        cmd = [
            editor_path,
            self.config.project_path,
        ]

        # Add Python script execution if specified
        if self.config.python_script:
            cmd.extend(["-ExecCmds", f"py {self.config.python_script}"])

        # Add network port if specified
        if self.config.port:
            cmd.extend(["-port", str(self.config.port)])

        # Add extra arguments
        cmd.extend(self.config.extra_args)

        # Prepare environment variables
        env = os.environ.copy()
        env["UE_INSTANCE_ID"] = str(self.config.instance_id)
        env["UE_INSTANCE_ROLE"] = self.config.role
        if self.config.port:
            env["UE_INSTANCE_PORT"] = str(self.config.port)
        env.update(self.config.env_vars)

        # Start process
        print(f"[Instance {self.config.instance_id}] Starting editor...")
        print(f"[Instance {self.config.instance_id}] Command: {' '.join(cmd)}")

        try:
            self.process = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                env=env,
                text=True,
                bufsize=1
            )
            self._start_time = time.time()

            # Start log reader task
            asyncio.create_task(self._read_logs())

            print(f"[Instance {self.config.instance_id}] Started (PID: {self.process.pid})")
            return True

        except Exception as e:
            print(f"[Instance {self.config.instance_id}] Failed to start: {e}")
            return False

    async def stop(self, timeout: float = 10.0) -> bool:
        """
        Stop the editor instance gracefully.

        Args:
            timeout: Maximum time to wait for graceful shutdown

        Returns:
            True if stopped successfully
        """
        if self.process is None:
            print(f"[Instance {self.config.instance_id}] Not running")
            return True

        print(f"[Instance {self.config.instance_id}] Stopping...")

        try:
            # Try graceful termination first
            self.process.terminate()

            # Wait for process to exit
            try:
                self.process.wait(timeout=timeout)
                print(f"[Instance {self.config.instance_id}] Stopped gracefully")
            except subprocess.TimeoutExpired:
                print(f"[Instance {self.config.instance_id}] Forcing kill...")
                self.process.kill()
                self.process.wait()
                print(f"[Instance {self.config.instance_id}] Killed")

            self.process = None
            self._ready = False
            return True

        except Exception as e:
            print(f"[Instance {self.config.instance_id}] Error stopping: {e}")
            return False

    async def wait_for_ready(self, timeout: float = 60.0) -> bool:
        """
        Wait for editor to be ready for automation.

        Args:
            timeout: Maximum time to wait

        Returns:
            True if editor is ready
        """
        print(f"[Instance {self.config.instance_id}] Waiting for ready state...")

        start_time = time.time()
        while time.time() - start_time < timeout:
            if not self.is_running():
                print(f"[Instance {self.config.instance_id}] Process died")
                return False

            # Check logs for ready indicators
            if self._check_ready_in_logs():
                self._ready = True
                elapsed = time.time() - self._start_time
                print(f"[Instance {self.config.instance_id}] Ready! ({elapsed:.1f}s)")
                return True

            await asyncio.sleep(0.5)

        print(f"[Instance {self.config.instance_id}] Timeout waiting for ready state")
        return False

    def is_running(self) -> bool:
        """
        Check if editor is still running.

        Returns:
            True if running
        """
        if self.process is None:
            return False
        return self.process.poll() is None

    def is_ready(self) -> bool:
        """
        Check if editor is ready for automation.

        Returns:
            True if ready
        """
        return self._ready and self.is_running()

    def get_logs(self, last_n: Optional[int] = None) -> List[str]:
        """
        Get editor output logs.

        Args:
            last_n: Return only last N lines (None for all)

        Returns:
            List of log lines
        """
        if last_n is None:
            return self._logs.copy()
        return self._logs[-last_n:]

    def execute_python(self, script: str) -> Any:
        """
        Execute Python script in the editor.

        Note: This is a placeholder. In reality, we would need:
        - Named pipes or sockets for communication
        - Or write script to file and signal editor to execute it

        Args:
            script: Python code to execute

        Returns:
            Execution result (placeholder)
        """
        # TODO: Implement actual remote execution mechanism
        raise NotImplementedError("Remote Python execution not yet implemented")

    @property
    def pid(self) -> Optional[int]:
        """
        Process ID of the editor.

        Returns:
            PID or None if not running
        """
        return self.process.pid if self.process else None

    def get_memory_usage(self) -> Optional[float]:
        """
        Get memory usage in MB.

        Returns:
            Memory usage or None if not running
        """
        if not self.is_running():
            return None

        try:
            process = psutil.Process(self.pid)
            return process.memory_info().rss / 1024 / 1024  # Convert to MB
        except (psutil.NoSuchProcess, psutil.AccessDenied):
            return None

    def get_cpu_percent(self) -> Optional[float]:
        """
        Get CPU usage percentage.

        Returns:
            CPU usage or None if not running
        """
        if not self.is_running():
            return None

        try:
            process = psutil.Process(self.pid)
            return process.cpu_percent(interval=0.1)
        except (psutil.NoSuchProcess, psutil.AccessDenied):
            return None

    # Private methods

    def _find_editor(self) -> Optional[str]:
        """Find UnrealEditor executable."""
        # Check common locations
        possible_paths = [
            "C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/Win64/UnrealEditor.exe",
            "C:/Program Files/Epic Games/UE_5.6/Engine/Binaries/Win64/UnrealEditor.exe",
            "C:/Program Files/Epic Games/UE_5.5/Engine/Binaries/Win64/UnrealEditor.exe",
            "C:/Program Files/Epic Games/UE_5.4/Engine/Binaries/Win64/UnrealEditor.exe",
        ]

        for path in possible_paths:
            if os.path.exists(path):
                return path

        # Check environment variable
        ue_root = os.getenv("UE_ROOT")
        if ue_root:
            editor_path = os.path.join(ue_root, "Engine/Binaries/Win64/UnrealEditor.exe")
            if os.path.exists(editor_path):
                return editor_path

        return None

    async def _read_logs(self):
        """Read logs from subprocess asynchronously."""
        if not self.process or not self.process.stdout:
            return

        try:
            while self.is_running():
                line = await asyncio.get_event_loop().run_in_executor(
                    None, self.process.stdout.readline
                )
                if line:
                    line = line.strip()
                    self._logs.append(line)
                    # Print important logs
                    if any(keyword in line.lower() for keyword in ["error", "warning", "crash"]):
                        print(f"[Instance {self.config.instance_id}] {line}")
                else:
                    break
        except Exception as e:
            print(f"[Instance {self.config.instance_id}] Log reader error: {e}")

    def _check_ready_in_logs(self) -> bool:
        """Check if editor is ready based on log output."""
        # Look for indicators that editor is ready
        ready_indicators = [
            "LogInit: Display: Engine is initialized",
            "LogWorld: Bringing World",
            "LogLoad: Took",
        ]

        recent_logs = " ".join(self.get_logs(last_n=100))
        return any(indicator in recent_logs for indicator in ready_indicators)


class EditorLauncher:
    """Manages multiple editor instances."""

    def __init__(
        self,
        project_path: str,
        editor_path: Optional[str] = None,
        base_port: int = 7777
    ):
        """
        Initialize launcher with project configuration.

        Args:
            project_path: Path to .uproject file
            editor_path: Path to UnrealEditor.exe (auto-detect if None)
            base_port: Base port for networking (incremented per instance)
        """
        self.project_path = project_path
        self.editor_path = editor_path
        self.base_port = base_port
        self.instances: List[EditorInstance] = []

    async def launch_instances(
        self,
        count: int,
        roles: Optional[List[str]] = None,
        scripts: Optional[List[str]] = None,
        extra_args: Optional[List[List[str]]] = None
    ) -> List[EditorInstance]:
        """
        Launch multiple editor instances.

        Args:
            count: Number of instances to launch
            roles: Role for each instance (defaults to "client")
            scripts: Python script path for each instance
            extra_args: Extra command-line arguments per instance

        Returns:
            List of launched instances
        """
        if roles and len(roles) != count:
            raise ValueError(f"Roles list length ({len(roles)}) must match count ({count})")
        if scripts and len(scripts) != count:
            raise ValueError(f"Scripts list length ({len(scripts)}) must match count ({count})")
        if extra_args and len(extra_args) != count:
            raise ValueError(f"Extra args length ({len(extra_args)}) must match count ({count})")

        print(f"[Launcher] Launching {count} editor instance(s)...")

        instances = []
        for i in range(count):
            config = EditorConfig(
                project_path=self.project_path,
                editor_path=self.editor_path,
                role=roles[i] if roles else "client",
                instance_id=i,
                port=self.base_port + i,
                python_script=scripts[i] if scripts else None,
                extra_args=extra_args[i] if extra_args else []
            )

            instance = EditorInstance(config)
            instances.append(instance)

        # Launch all instances in parallel
        start_tasks = [instance.start() for instance in instances]
        await asyncio.gather(*start_tasks)

        # Wait for all to be ready
        print(f"[Launcher] Waiting for all instances to be ready...")
        ready_tasks = [instance.wait_for_ready() for instance in instances]
        results = await asyncio.gather(*ready_tasks)

        if not all(results):
            print(f"[Launcher] WARNING: Some instances failed to become ready")

        self.instances.extend(instances)
        print(f"[Launcher] Launched {len(instances)} instance(s)")
        return instances

    async def shutdown_all(self, timeout: float = 10.0):
        """
        Shutdown all running instances.

        Args:
            timeout: Maximum time to wait for each instance
        """
        if not self.instances:
            return

        print(f"[Launcher] Shutting down {len(self.instances)} instance(s)...")

        # Stop all instances in parallel
        stop_tasks = [instance.stop(timeout) for instance in self.instances]
        await asyncio.gather(*stop_tasks)

        self.instances.clear()
        print(f"[Launcher] All instances stopped")

    def get_instances(self) -> List[EditorInstance]:
        """
        Get all managed instances.

        Returns:
            List of instances
        """
        return self.instances.copy()

    def get_running_instances(self) -> List[EditorInstance]:
        """
        Get all running instances.

        Returns:
            List of running instances
        """
        return [instance for instance in self.instances if instance.is_running()]

    def get_total_memory_usage(self) -> float:
        """
        Get total memory usage of all instances in MB.

        Returns:
            Total memory usage
        """
        total = 0.0
        for instance in self.instances:
            usage = instance.get_memory_usage()
            if usage:
                total += usage
        return total
