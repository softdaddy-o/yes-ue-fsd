"""HTTP API client for testing backend APIs."""
import requests
from typing import Dict, Any, Optional

class APIClient:
    """Client for testing backend REST/GraphQL APIs."""
    def __init__(self, base_url: str, auth_token: Optional[str] = None):
        self.base_url = base_url.rstrip('/')
        self.auth_token = auth_token
        self.session = requests.Session()
        if auth_token:
            self.session.headers['Authorization'] = f'Bearer {auth_token}'

    def get(self, endpoint: str, **kwargs) -> requests.Response:
        """HTTP GET request."""
        url = f"{self.base_url}/{endpoint.lstrip('/')}"
        return self.session.get(url, **kwargs)

    def post(self, endpoint: str, data: Optional[Dict[str, Any]] = None, **kwargs) -> requests.Response:
        """HTTP POST request."""
        url = f"{self.base_url}/{endpoint.lstrip('/')}"
        return self.session.post(url, json=data, **kwargs)

    def put(self, endpoint: str, data: Optional[Dict[str, Any]] = None, **kwargs) -> requests.Response:
        """HTTP PUT request."""
        url = f"{self.base_url}/{endpoint.lstrip('/')}"
        return self.session.put(url, json=data, **kwargs)

    def delete(self, endpoint: str, **kwargs) -> requests.Response:
        """HTTP DELETE request."""
        url = f"{self.base_url}/{endpoint.lstrip('/')}"
        return self.session.delete(url, **kwargs)
