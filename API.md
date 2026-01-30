# Yes UE FSD - HTTP API Reference

Remote control API for the Auto Player Driver plugin via HTTP/JSON-RPC 2.0.

## Quick Start

**Server Endpoint:** `http://127.0.0.1:8081/rpc`

**Example Request:**
```bash
curl -X POST http://127.0.0.1:8081/rpc \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "tools/call",
    "params": {
      "name": "autodriver/move_to_location",
      "arguments": {
        "location": {"x": 1000, "y": 0, "z": 100},
        "acceptanceRadius": 50.0
      }
    },
    "id": 1
  }'
```

## Protocol

The API uses **JSON-RPC 2.0** over HTTP POST.

### Request Format

```json
{
  "jsonrpc": "2.0",
  "method": "<method_name>",
  "params": {<parameters>},
  "id": <request_id>
}
```

### Response Format

**Success:**
```json
{
  "jsonrpc": "2.0",
  "result": {<result_data>},
  "id": <request_id>
}
```

**Error:**
```json
{
  "jsonrpc": "2.0",
  "error": {
    "code": <error_code>,
    "message": "<error_message>"
  },
  "id": <request_id>
}
```

### Error Codes

| Code | Message | Description |
|------|---------|-------------|
| -32700 | Parse error | Invalid JSON |
| -32600 | Invalid Request | Invalid JSON-RPC format |
| -32601 | Method not found | Unknown method |
| -32602 | Invalid params | Invalid parameters |
| -32603 | Internal error | Server error |

## Methods

### tools/list

List all available tools.

**Request:**
```json
{
  "jsonrpc": "2.0",
  "method": "tools/list",
  "params": {},
  "id": 1
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "tools": [
      {
        "name": "autodriver/move_to_location",
        "description": "Move character to target location",
        "inputSchema": {
          "type": "object",
          "properties": {
            "location": {
              "type": "object",
              "properties": {
                "x": {"type": "number"},
                "y": {"type": "number"},
                "z": {"type": "number"}
              }
            },
            "acceptanceRadius": {"type": "number"},
            "movementMode": {"type": "string"}
          }
        }
      }
    ]
  },
  "id": 1
}
```

### tools/call

Execute a tool.

**Request:**
```json
{
  "jsonrpc": "2.0",
  "method": "tools/call",
  "params": {
    "name": "<tool_name>",
    "arguments": {<tool_arguments>}
  },
  "id": 1
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "content": [
      {
        "type": "text",
        "text": "<result_message>"
      }
    ],
    "isError": false
  },
  "id": 1
}
```

## Available Tools

### autodriver/move_to_location

Move the controlled character to a target location.

**Parameters:**
- `location` (object, required)
  - `x` (number): X coordinate
  - `y` (number): Y coordinate
  - `z` (number): Z coordinate
- `acceptanceRadius` (number, optional): Distance threshold (default: 50.0)
- `movementMode` (string, optional): "Direct", "Navigation", or "InputSimulation"

**Example:**
```json
{
  "jsonrpc": "2.0",
  "method": "tools/call",
  "params": {
    "name": "autodriver/move_to_location",
    "arguments": {
      "location": {"x": 1000, "y": 500, "z": 100},
      "acceptanceRadius": 100.0,
      "movementMode": "Navigation"
    }
  },
  "id": 1
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "content": [{
      "type": "text",
      "text": "Moving to location (1000.0, 500.0, 100.0) with radius 100.0"
    }],
    "isError": false
  },
  "id": 1
}
```

### autodriver/query_status

Get the current automation status.

**Parameters:** None

**Example:**
```json
{
  "jsonrpc": "2.0",
  "method": "tools/call",
  "params": {
    "name": "autodriver/query_status",
    "arguments": {}
  },
  "id": 1
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "result": {
    "content": [{
      "type": "text",
      "text": "Status: Ready"
    }],
    "isError": false
  },
  "id": 1
}
```

### autodriver/stop_command

Stop the currently executing command.

**Parameters:** None

**Example:**
```json
{
  "jsonrpc": "2.0",
  "method": "tools/call",
  "params": {
    "name": "autodriver/stop_command",
    "arguments": {}
  },
  "id": 1
}
```

## Client Examples

### Python

```python
import requests
import json

class AutoDriverClient:
    def __init__(self, url="http://127.0.0.1:8081/rpc"):
        self.url = url
        self.request_id = 0

    def call_tool(self, tool_name, arguments=None):
        self.request_id += 1
        payload = {
            "jsonrpc": "2.0",
            "method": "tools/call",
            "params": {
                "name": tool_name,
                "arguments": arguments or {}
            },
            "id": self.request_id
        }

        response = requests.post(self.url, json=payload)
        return response.json()

    def move_to_location(self, x, y, z, radius=50.0):
        return self.call_tool("autodriver/move_to_location", {
            "location": {"x": x, "y": y, "z": z},
            "acceptanceRadius": radius
        })

    def query_status(self):
        return self.call_tool("autodriver/query_status")

    def stop(self):
        return self.call_tool("autodriver/stop_command")

# Usage
client = AutoDriverClient()
result = client.move_to_location(1000, 0, 100)
print(result)
```

### JavaScript/Node.js

```javascript
const fetch = require('node-fetch');

class AutoDriverClient {
    constructor(url = 'http://127.0.0.1:8081/rpc') {
        this.url = url;
        this.requestId = 0;
    }

    async callTool(toolName, arguments = {}) {
        this.requestId++;
        const payload = {
            jsonrpc: '2.0',
            method: 'tools/call',
            params: {
                name: toolName,
                arguments
            },
            id: this.requestId
        };

        const response = await fetch(this.url, {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify(payload)
        });

        return await response.json();
    }

    async moveToLocation(x, y, z, radius = 50.0) {
        return this.callTool('autodriver/move_to_location', {
            location: {x, y, z},
            acceptanceRadius: radius
        });
    }

    async queryStatus() {
        return this.callTool('autodriver/query_status');
    }

    async stop() {
        return this.callTool('autodriver/stop_command');
    }
}

// Usage
const client = new AutoDriverClient();
client.moveToLocation(1000, 0, 100).then(console.log);
```

### cURL

```bash
# List tools
curl -X POST http://127.0.0.1:8081/rpc \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"tools/list","params":{},"id":1}'

# Move to location
curl -X POST http://127.0.0.1:8081/rpc \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "tools/call",
    "params": {
      "name": "autodriver/move_to_location",
      "arguments": {
        "location": {"x": 1000, "y": 0, "z": 100}
      }
    },
    "id": 1
  }'

# Query status
curl -X POST http://127.0.0.1:8081/rpc \
  -H "Content-Type: application/json" \
  -d '{
    "jsonrpc": "2.0",
    "method": "tools/call",
    "params": {
      "name": "autodriver/query_status",
      "arguments": {}
    },
    "id": 1
  }'
```

## Configuration

Edit `Config/DefaultYesUeFsd.ini`:

```ini
[/Script/YesUeFsdEditor.AutoDriverSettings]
ServerPort=8081
bAutoStartServer=true
BindAddress=127.0.0.1
LogLevel=Log
```

**Settings:**
- `ServerPort`: HTTP server port (default: 8081)
- `bAutoStartServer`: Auto-start on editor launch (default: true)
- `BindAddress`: Bind address - use "127.0.0.1" for localhost only, "0.0.0.0" for all interfaces
- `LogLevel`: Logging verbosity (Error, Warning, Log, Verbose)

## Security Considerations

**Default Configuration:**
- Server binds to `127.0.0.1` (localhost only)
- Only accessible from the same machine
- No authentication required

**Production Use:**
- Do NOT bind to `0.0.0.0` in production
- Use firewall rules to restrict access
- Consider adding authentication if exposing externally
- Use HTTPS proxy if remote access is needed

## Troubleshooting

**Server won't start:**
- Check port 8081 isn't already in use
- Verify `bAutoStartServer=true` in config
- Check Output Log for error messages

**Connection refused:**
- Verify server is running: Check Output Log
- Confirm correct port and address
- Check firewall settings

**Invalid JSON errors:**
- Validate JSON syntax
- Ensure Content-Type header is set
- Check request format matches JSON-RPC 2.0 spec

---

**API Version:** 0.1.0
**Protocol:** JSON-RPC 2.0
**Last Updated:** 2026-01-30
