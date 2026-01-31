"""
Editor instance pool for parallel test execution.

Manages a pool of editor instances that can be reused across tests
to reduce startup overhead.
"""

import asyncio
import logging
from typing import List, Optional, Dict, Any
from pathlib import Path
from dataclasses import dataclass, field
from enum import Enum


logger = logging.getLogger(__name__)


class InstanceState(Enum):
    """State of an editor instance."""
    IDLE = "idle"
    BUSY = "busy"
    STARTING = "starting"
    STOPPING = "stopping"
    FAILED = "failed"


@dataclass
class PooledInstance:
    """Wrapper for a pooled editor instance."""
    instance_id: int
    instance: Any  # EditorInstance
    state: InstanceState = InstanceState.IDLE
    port: int = 0
    temp_dir: Optional[Path] = None
    current_test: Optional[str] = None
    lock: asyncio.Lock = field(default_factory=asyncio.Lock)


class EditorInstancePool:
    """
    Pool of editor instances for parallel test execution.

    Features:
    - Reuses instances across tests
    - Automatic instance allocation and cleanup
    - Resource isolation (ports, temp dirs, save files)
    - Health monitoring and recovery
    - Graceful shutdown

    Example:
        pool = EditorInstancePool(
            project_path="MyProject.uproject",
            pool_size=4
        )

        await pool.initialize()

        # Acquire instance for test
        instance = await pool.acquire()
        try:
            # Run test
            pass
        finally:
            await pool.release(instance)

        await pool.shutdown()
    """

    def __init__(
        self,
        project_path: str,
        pool_size: int = 4,
        base_port: int = 8000,
        max_retries: int = 3,
    ):
        """
        Initialize instance pool.

        Args:
            project_path: Path to .uproject file
            pool_size: Number of instances in pool
            base_port: Base port for instance allocation
            max_retries: Maximum retries for failed instances
        """
        self.project_path = project_path
        self.pool_size = pool_size
        self.base_port = base_port
        self.max_retries = max_retries

        self.instances: List[PooledInstance] = []
        self.available_queue: asyncio.Queue = asyncio.Queue()
        self.initialized = False
        self._shutdown_event = asyncio.Event()

    async def initialize(self):
        """Initialize the instance pool."""
        if self.initialized:
            return

        logger.info(f"Initializing instance pool with {self.pool_size} instances")

        # Import here to avoid circular dependency
        from yes_ue_fsd.meta_layer import EditorLauncher

        launcher = EditorLauncher(project_path=self.project_path)

        # Pre-launch instances
        for i in range(self.pool_size):
            port = self.base_port + i
            temp_dir = Path(f"temp/instance_{i}")
            temp_dir.mkdir(parents=True, exist_ok=True)

            logger.info(f"Starting instance {i} on port {port}")

            # Launch instance
            instances = await launcher.launch_instances(
                count=1,
                base_port=port,
            )

            pooled = PooledInstance(
                instance_id=i,
                instance=instances[0],
                state=InstanceState.IDLE,
                port=port,
                temp_dir=temp_dir,
            )

            self.instances.append(pooled)
            await self.available_queue.put(pooled)

        self.initialized = True
        logger.info("Instance pool initialized successfully")

    async def acquire(self, timeout: float = 60.0) -> PooledInstance:
        """
        Acquire an available instance from the pool.

        Args:
            timeout: Maximum time to wait for instance (seconds)

        Returns:
            PooledInstance ready for use

        Raises:
            asyncio.TimeoutError: If no instance available within timeout
        """
        if not self.initialized:
            await self.initialize()

        logger.debug("Acquiring instance from pool")

        try:
            pooled = await asyncio.wait_for(
                self.available_queue.get(),
                timeout=timeout
            )
        except asyncio.TimeoutError:
            raise RuntimeError(
                f"No instance available within {timeout}s. "
                f"Pool size: {self.pool_size}, "
                f"Available: {self.available_queue.qsize()}"
            )

        async with pooled.lock:
            pooled.state = InstanceState.BUSY
            logger.debug(f"Acquired instance {pooled.instance_id}")

        return pooled

    async def release(self, pooled: PooledInstance):
        """
        Release an instance back to the pool.

        Args:
            pooled: Instance to release
        """
        async with pooled.lock:
            # Clean up instance state
            await self._cleanup_instance(pooled)

            pooled.state = InstanceState.IDLE
            pooled.current_test = None

            logger.debug(f"Released instance {pooled.instance_id}")

        # Return to pool
        await self.available_queue.put(pooled)

    async def _cleanup_instance(self, pooled: PooledInstance):
        """
        Clean up instance state between tests.

        Args:
            pooled: Instance to clean up
        """
        # Clear temp files
        if pooled.temp_dir and pooled.temp_dir.exists():
            for file in pooled.temp_dir.glob("*"):
                try:
                    file.unlink()
                except Exception as e:
                    logger.warning(f"Failed to delete {file}: {e}")

        # Reset game state
        # In real implementation, this would call instance.reset_state()

    async def shutdown(self):
        """Shutdown all instances in the pool."""
        if not self.initialized:
            return

        logger.info("Shutting down instance pool")

        # Stop all instances
        shutdown_tasks = []
        for pooled in self.instances:
            shutdown_tasks.append(self._shutdown_instance(pooled))

        await asyncio.gather(*shutdown_tasks, return_exceptions=True)

        self.instances.clear()
        self.initialized = False

        logger.info("Instance pool shutdown complete")

    async def _shutdown_instance(self, pooled: PooledInstance):
        """Shutdown a single instance."""
        async with pooled.lock:
            try:
                pooled.state = InstanceState.STOPPING
                await pooled.instance.stop()
            except Exception as e:
                logger.error(f"Error stopping instance {pooled.instance_id}: {e}")

    def get_stats(self) -> Dict[str, Any]:
        """
        Get pool statistics.

        Returns:
            Dictionary with pool stats
        """
        idle = sum(1 for p in self.instances if p.state == InstanceState.IDLE)
        busy = sum(1 for p in self.instances if p.state == InstanceState.BUSY)
        failed = sum(1 for p in self.instances if p.state == InstanceState.FAILED)

        return {
            "pool_size": self.pool_size,
            "idle": idle,
            "busy": busy,
            "failed": failed,
            "available": self.available_queue.qsize(),
        }


# Pytest integration
import pytest


@pytest.fixture(scope="session")
async def instance_pool(project_path):
    """
    Provide instance pool for test session.

    This fixture creates and manages a pool of editor instances
    that can be shared across parallel tests.
    """
    import os
    workers = int(os.environ.get("PYTEST_XDIST_WORKER_COUNT", 1))

    pool = EditorInstancePool(
        project_path=project_path,
        pool_size=workers
    )

    await pool.initialize()

    yield pool

    await pool.shutdown()


@pytest.fixture
async def pooled_editor(instance_pool):
    """
    Provide a pooled editor instance for a test.

    Automatically acquires and releases instance from pool.
    """
    pooled = await instance_pool.acquire()

    try:
        yield pooled.instance
    finally:
        await instance_pool.release(pooled)
