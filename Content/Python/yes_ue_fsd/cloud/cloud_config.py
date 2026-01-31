"""Cloud testing configuration."""
from dataclasses import dataclass
from typing import Optional, List

@dataclass
class CloudConfig:
    """
    Configuration for cloud-distributed testing.

    Supports:
    - GitHub Actions
    - AWS Device Farm
    - Azure DevOps
    - Google Cloud Build
    """
    provider: str = "github_actions"  # github_actions, aws, azure, gcp
    workers: int = 4
    regions: List[str] = None
    instance_type: str = "standard"

    def __post_init__(self):
        if self.regions is None:
            self.regions = ["us-east-1"]
