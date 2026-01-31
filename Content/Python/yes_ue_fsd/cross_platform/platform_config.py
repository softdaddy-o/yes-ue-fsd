"""Cross-platform testing configuration."""
from dataclasses import dataclass
from typing import List
import platform

@dataclass
class PlatformConfig:
    """
    Configuration for cross-platform testing.

    Supports:
    - Windows
    - macOS
    - Linux
    - PlayStation 5 (future)
    - Xbox Series X/S (future)
    - Nintendo Switch (future)
    """
    target_platforms: List[str] = None
    current_platform: str = None

    def __post_init__(self):
        if self.current_platform is None:
            self.current_platform = platform.system()

        if self.target_platforms is None:
            self.target_platforms = [self.current_platform]

    def is_platform_supported(self, platform_name: str) -> bool:
        """Check if platform is supported."""
        supported = ["Windows", "Linux", "Darwin"]  # Darwin = macOS
        return platform_name in supported

    def get_platform_specific_path(self, base_path: str) -> str:
        """Get platform-specific path."""
        if self.current_platform == "Windows":
            return base_path.replace("/", "\\")
        return base_path.replace("\\", "/")
