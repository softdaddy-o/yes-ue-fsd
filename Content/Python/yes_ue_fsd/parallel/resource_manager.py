"""
Resource manager for parallel test execution.

Handles resource allocation and isolation across parallel test workers:
- Port allocation
- Temporary directories
- Save file isolation
- Resource locking for shared resources
"""

import os
import asyncio
import tempfile
from pathlib import Path
from typing import Dict, Optional, Set
from contextlib import asynccontextmanager


class ResourceManager:
    """
    Manages shared resources for parallel test execution.

    Ensures tests don't conflict by:
    - Allocating unique ports per worker
    - Creating isolated temporary directories
    - Managing locks for shared resources
    - Preventing save file conflicts
    """

    def __init__(
        self,
        base_port: int = 8000,
        base_temp_dir: Optional[str] = None,
    ):
        """
        Initialize resource manager.

        Args:
            base_port: Base port for allocation
            base_temp_dir: Base directory for temp files
        """
        self.base_port = base_port
        self.base_temp_dir = base_temp_dir or tempfile.gettempdir()

        self.allocated_ports: Set[int] = set()
        self.port_lock = asyncio.Lock()

        self.temp_dirs: Dict[str, Path] = {}
        self.temp_dir_lock = asyncio.Lock()

        self.resource_locks: Dict[str, asyncio.Lock] = {}

    async def allocate_port(self, worker_id: str) -> int:
        """
        Allocate a unique port for a worker.

        Args:
            worker_id: Unique worker identifier

        Returns:
            Allocated port number
        """
        async with self.port_lock:
            # Find first available port
            port = self.base_port
            while port in self.allocated_ports:
                port += 1

            self.allocated_ports.add(port)
            return port

    async def release_port(self, port: int):
        """
        Release a port back to the pool.

        Args:
            port: Port to release
        """
        async with self.port_lock:
            self.allocated_ports.discard(port)

    async def create_temp_dir(self, worker_id: str) -> Path:
        """
        Create isolated temporary directory for a worker.

        Args:
            worker_id: Unique worker identifier

        Returns:
            Path to temporary directory
        """
        async with self.temp_dir_lock:
            if worker_id in self.temp_dirs:
                return self.temp_dirs[worker_id]

            temp_dir = Path(self.base_temp_dir) / f"ue_test_{worker_id}"
            temp_dir.mkdir(parents=True, exist_ok=True)

            self.temp_dirs[worker_id] = temp_dir
            return temp_dir

    async def cleanup_temp_dir(self, worker_id: str):
        """
        Clean up temporary directory for a worker.

        Args:
            worker_id: Worker identifier
        """
        async with self.temp_dir_lock:
            if worker_id in self.temp_dirs:
                temp_dir = self.temp_dirs[worker_id]

                # Delete contents
                for item in temp_dir.iterdir():
                    if item.is_file():
                        item.unlink()
                    elif item.is_dir():
                        import shutil
                        shutil.rmtree(item)

                # Remove directory
                temp_dir.rmdir()
                del self.temp_dirs[worker_id]

    @asynccontextmanager
    async def lock_resource(self, resource_name: str):
        """
        Acquire exclusive lock on a shared resource.

        Args:
            resource_name: Name of resource to lock

        Example:
            async with resource_manager.lock_resource("database"):
                # Only one test can access database at a time
                await run_database_test()
        """
        if resource_name not in self.resource_locks:
            self.resource_locks[resource_name] = asyncio.Lock()

        async with self.resource_locks[resource_name]:
            yield

    def get_worker_id(self) -> str:
        """
        Get current pytest-xdist worker ID.

        Returns:
            Worker ID (e.g., "gw0", "gw1") or "master" for non-parallel
        """
        return os.environ.get("PYTEST_XDIST_WORKER", "master")

    def get_worker_count(self) -> int:
        """
        Get total number of parallel workers.

        Returns:
            Number of workers (1 for non-parallel)
        """
        return int(os.environ.get("PYTEST_XDIST_WORKER_COUNT", 1))

    def is_parallel_execution(self) -> bool:
        """
        Check if tests are running in parallel.

        Returns:
            True if running with pytest-xdist
        """
        return "PYTEST_XDIST_WORKER" in os.environ


# Global instance
_resource_manager = ResourceManager()


def get_resource_manager() -> ResourceManager:
    """Get the global resource manager instance."""
    return _resource_manager


# Pytest fixtures
import pytest


@pytest.fixture(scope="session")
def resource_manager():
    """Provide resource manager for test session."""
    return get_resource_manager()


@pytest.fixture
async def worker_port(resource_manager):
    """Allocate unique port for current worker."""
    worker_id = resource_manager.get_worker_id()
    port = await resource_manager.allocate_port(worker_id)

    yield port

    await resource_manager.release_port(port)


@pytest.fixture
async def worker_temp_dir(resource_manager):
    """Create isolated temp directory for current worker."""
    worker_id = resource_manager.get_worker_id()
    temp_dir = await resource_manager.create_temp_dir(worker_id)

    yield temp_dir

    await resource_manager.cleanup_temp_dir(worker_id)


@pytest.fixture
def shared_resource(resource_manager):
    """
    Provide context manager for locking shared resources.

    Usage:
        def test_database(shared_resource):
            async with shared_resource("database"):
                # Exclusive access to database
                pass
    """
    return resource_manager.lock_resource
