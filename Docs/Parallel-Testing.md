# Parallel Test Execution Guide

## Overview

Yes UE FSD supports parallel test execution using pytest-xdist, enabling tests to run concurrently across multiple editor instances. This can reduce test suite execution time by 4x-8x or more.

## Installation

Parallel execution support is automatically included in requirements.txt:

```bash
cd Content/Python
pip install -r requirements.txt  # Includes pytest-xdist
```

## Quick Start

### Run Tests in Parallel

```bash
# Use all CPU cores
pytest tests/ -n auto

# Use specific number of workers
pytest tests/ -n 4

# Use optimal distribution strategy
pytest tests/ -n auto --dist loadscope
```

### Expected Speedup

| Workers | Speedup | Example Time |
|---------|---------|--------------|
| 1 (serial) | 1x | 16 minutes |
| 2 | 1.8x | 9 minutes |
| 4 | 3.5x | 4.5 minutes |
| 8 | 6x | 2.7 minutes |

Actual speedup depends on:
- Test complexity
- Editor startup overhead
- Available CPU cores
- Disk I/O speed

## Distribution Strategies

### loadscope (Recommended)

Groups tests by module/class to maximize fixture reuse.

```bash
pytest tests/ -n auto --dist loadscope
```

Good for:
- Tests with expensive setup fixtures
- Test suites with logical groupings
- Maximizing instance reuse

### load

Distributes tests evenly across workers as they become available.

```bash
pytest tests/ -n auto --dist loadfile
```

Good for:
- Tests with varying execution times
- Maximizing CPU utilization
- Independent test cases

### loadfile

Distributes entire test files to workers.

```bash
pytest tests/ -n auto --dist loadfile
```

Good for:
- File-level fixtures
- Related tests in same file
- Simpler distribution logic

## Instance Pooling

Yes UE FSD uses instance pooling to reduce editor startup overhead:

```python
# conftest.py
from yes_ue_fsd.parallel import EditorInstancePool

@pytest.fixture(scope="session")
async def instance_pool(project_path):
    """Create pool of editor instances."""
    pool = EditorInstancePool(
        project_path=project_path,
        pool_size=4  # Number of instances
    )

    await pool.initialize()
    yield pool
    await pool.shutdown()

@pytest.fixture
async def pooled_editor(instance_pool):
    """Get editor from pool."""
    pooled = await instance_pool.acquire()
    try:
        yield pooled.instance
    finally:
        await instance_pool.release(pooled)

# Use in tests
def test_movement(pooled_editor):
    # Instance automatically acquired and released
    driver = AutoDriver(player_index=0)
    driver.move_to((1000, 500, 100))
```

## Resource Isolation

### Port Allocation

Each worker gets a unique port:

```python
def test_with_unique_port(worker_port):
    """Each worker gets unique port."""
    print(f"Worker using port: {worker_port}")
    # Ports: 8000, 8001, 8002, 8003, ...
```

### Temporary Directories

Each worker gets isolated temp directory:

```python
def test_with_temp_dir(worker_temp_dir):
    """Each worker gets isolated temp directory."""
    save_file = worker_temp_dir / "save.dat"
    save_file.write_text("data")
    # No conflicts with other workers
```

### Shared Resource Locking

Lock shared resources to prevent conflicts:

```python
async def test_database(shared_resource):
    """Test with exclusive database access."""
    async with shared_resource("database"):
        # Only one test can access database at a time
        connection = await connect_to_database()
        await run_queries(connection)
```

## Configuration

### Using ParallelConfig

```python
from yes_ue_fsd.parallel import ParallelConfig, PresetConfigs

# Use preset
config = PresetConfigs.fast()  # Use all cores
config = PresetConfigs.balanced()  # Use half of cores
config = PresetConfigs.conservative()  # Use 2-4 workers
config = PresetConfigs.ci()  # Optimized for CI/CD

# Custom configuration
config = ParallelConfig(
    workers=4,
    dist_mode="loadscope",
    timeout=300.0,
    instance_pool_size=4,
)

# Convert to pytest args
pytest_args = config.to_pytest_args()
# ['-n', '4', '--dist', 'loadscope', ...]
```

### pytest.ini Configuration

```ini
[pytest]
# Parallel execution options
addopts =
    -n auto  # Use all CPU cores
    --dist loadscope  # Group by test module
    --max-worker-restart 3  # Restart failed workers up to 3 times
```

## Best Practices

### 1. Design for Parallelism

Good (thread-safe):
```python
def test_independent(pooled_editor):
    # Each test gets own editor instance
    driver = AutoDriver(player_index=0)
    driver.move_to((1000, 500, 100))
```

Avoid (shared state):
```python
# Global state causes race conditions
global_counter = 0

def test_with_global_state():
    global global_counter
    global_counter += 1  # Race condition!
```

### 2. Use Appropriate Fixtures

```python
# Session-scoped for expensive setup
@pytest.fixture(scope="session")
async def instance_pool():
    # Created once per session
    pass

# Function-scoped for per-test isolation
@pytest.fixture
async def clean_editor():
    # Created for each test
    pass
```

### 3. Handle Test Dependencies

```python
# Mark dependencies explicitly
@pytest.mark.dependency(name="setup_world")
def test_setup():
    pass

@pytest.mark.dependency(depends=["setup_world"])
def test_use_world():
    pass

# Or use loadscope to group related tests
```

### 4. Monitor Resource Usage

```python
def test_check_pool_stats(instance_pool):
    """Monitor instance pool utilization."""
    stats = instance_pool.get_stats()
    print(f"Pool: {stats['pool_size']}, "
          f"Idle: {stats['idle']}, "
          f"Busy: {stats['busy']}")
```

## Troubleshooting

### Tests Hanging

**Symptom:** Tests hang indefinitely

**Solutions:**
```bash
# Reduce worker count
pytest tests/ -n 2

# Increase timeout
pytest tests/ --timeout=600

# Check for deadlocks
# - Resource locks not released
# - Circular dependencies
```

### Port Conflicts

**Symptom:** "Port already in use" errors

**Solutions:**
```python
# Use higher base port
from yes_ue_fsd.parallel import ResourceManager
resource_manager = ResourceManager(base_port=9000)

# Or let system assign ports
port = 0  # System assigns available port
```

### Instance Exhaustion

**Symptom:** "No instance available within timeout"

**Solutions:**
```bash
# Increase pool size
pytest tests/ -n 8  # More workers

# Increase timeout
# Let slower tests complete

# Reduce parallelism
pytest tests/ -n 2  # Fewer workers
```

### Flaky Tests in Parallel

**Symptom:** Tests pass serially but fail in parallel

**Causes:**
- Shared state (globals, singletons)
- Race conditions
- Resource conflicts
- Timing dependencies

**Solutions:**
```python
# Use resource locking
async with shared_resource("database"):
    # Exclusive access

# Add proper cleanup
@pytest.fixture
def clean_state():
    yield
    # Clean up after test

# Disable parallelism for specific tests
@pytest.mark.no_parallel
def test_serial_only():
    pass
```

## Advanced Usage

### Custom Worker Distribution

```python
# conftest.py
def pytest_xdist_setupnodes(config, specs):
    """Customize worker setup."""
    # Assign specific tests to specific workers
    pass

def pytest_xdist_make_scheduler(config, log):
    """Create custom test scheduler."""
    # Custom scheduling logic
    return MyCustomScheduler(config, log)
```

### Worker-Specific Setup

```python
def pytest_configure_node(node):
    """Configure each worker node."""
    worker_id = node.workerinput["workerid"]
    # Worker-specific configuration
```

### Live Worker Monitoring

```bash
# Enable live logs from all workers
pytest tests/ -n auto --log-cli-level=INFO

# Shows output from all workers in real-time
```

## Performance Tips

### 1. Minimize Editor Startups

```python
# Use instance pool (already implemented)
# Instances are reused across tests

# Group related tests in same file
# Benefits from loadscope distribution
```

### 2. Optimize Test Duration

```python
# Fast tests first
# tests/fast/test_quick.py - run first

# Slow tests last
# tests/slow/test_integration.py - run last

# pytest-xdist automatically balances load
```

### 3. Use Appropriate Worker Count

```python
# Too few: underutilized CPU
pytest tests/ -n 2  # On 8-core machine

# Too many: overhead dominates
pytest tests/ -n 32  # On 8-core machine

# Optimal: 1-2x CPU cores
pytest tests/ -n auto  # Recommended
```

### 4. Profile Parallel Execution

```bash
# Measure actual speedup
time pytest tests/  # Serial baseline
time pytest tests/ -n auto  # Parallel

# Identify bottlenecks
pytest tests/ -n auto --durations=10
```

## CI/CD Integration

### GitHub Actions

```yaml
name: Parallel Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: windows-latest

    steps:
      - uses: actions/checkout@v4

      - name: Setup Python
        uses: actions/setup-python@v4
        with:
          python-version: '3.11'

      - name: Install dependencies
        run: |
          cd Content/Python
          pip install -r requirements.txt

      - name: Run tests in parallel
        run: |
          cd Content/Python
          pytest tests/ -n auto --dist loadscope --html=report.html
```

### Jenkins

```groovy
pipeline {
    agent any

    stages {
        stage('Parallel Tests') {
            steps {
                bat 'pytest tests/ -n 4 --dist loadscope --junit-xml=junit.xml'
            }
        }
    }
}
```

## Metrics

Track parallel execution metrics:

```python
# conftest.py
def pytest_terminal_summary(terminalreporter, exitstatus, config):
    """Report parallel execution stats."""
    if hasattr(config, 'workerinput'):
        worker_id = config.workerinput['workerid']
        print(f"\nWorker {worker_id} completed")

    # Pool stats
    if hasattr(config, '_instance_pool'):
        stats = config._instance_pool.get_stats()
        print(f"\nInstance Pool Stats:")
        print(f"  Idle: {stats['idle']}")
        print(f"  Busy: {stats['busy']}")
        print(f"  Failed: {stats['failed']}")
```

## Examples

See example parallel test suites:
- `Content/Python/tests/parallel/` - Parallel test examples
- `Content/Python/tests/conftest.py` - Pool configuration
- `.github/workflows/test.yml` - CI/CD parallel execution

## References

- [pytest-xdist Documentation](https://pytest-xdist.readthedocs.io/)
- [Parallel Testing Best Practices](https://docs.pytest.org/en/stable/how-to/xdist.html)
- [Instance Pooling Patterns](https://martinfowler.com/bliki/ObjectPool.html)
