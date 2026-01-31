"""Data loaders for data-driven testing."""
import csv
import json
from pathlib import Path
from typing import List, Dict, Any

class CSVLoader:
    """Load test data from CSV files."""
    @staticmethod
    def load(file_path: str) -> List[Dict[str, Any]]:
        """Load CSV file as list of dictionaries."""
        with open(file_path, 'r') as f:
            reader = csv.DictReader(f)
            return list(reader)

class JSONLoader:
    """Load test data from JSON files."""
    @staticmethod
    def load(file_path: str) -> Any:
        """Load JSON file."""
        with open(file_path, 'r') as f:
            return json.load(f)

class YAMLLoader:
    """Load test data from YAML files."""
    @staticmethod
    def load(file_path: str) -> Any:
        """Load YAML file."""
        try:
            import yaml
            with open(file_path, 'r') as f:
                return yaml.safe_load(f)
        except ImportError:
            raise ImportError("PyYAML not installed. Run: pip install pyyaml")
