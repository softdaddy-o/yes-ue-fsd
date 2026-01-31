"""
Instance Context - Manage local instance configuration

This module provides context information about the current editor instance
when running in a multi-instance test scenario.
"""

import os
from dataclasses import dataclass


@dataclass
class InstanceContext:
    """Manages local instance configuration."""

    instance_id: int = 0
    role: str = "client"
    port: int = 7777

    @classmethod
    def from_environment(cls) -> 'InstanceContext':
        """
        Load context from environment variables.

        Returns:
            InstanceContext populated from environment
        """
        return cls(
            instance_id=int(os.getenv("UE_INSTANCE_ID", "0")),
            role=os.getenv("UE_INSTANCE_ROLE", "client"),
            port=int(os.getenv("UE_INSTANCE_PORT", "7777")),
        )

    def is_server(self) -> bool:
        """Check if this instance is a server."""
        return self.role.lower() in ["server", "host"]

    def is_client(self) -> bool:
        """Check if this instance is a client."""
        return self.role.lower() in ["client", "player"]

    def __str__(self) -> str:
        return f"Instance {self.instance_id} ({self.role}) on port {self.port}"


# Global instance context - loaded once at import
instance = InstanceContext.from_environment()
