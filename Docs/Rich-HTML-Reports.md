## Rich HTML Reports

Yes UE FSD provides beautiful, detailed HTML test reports with screenshots, performance metrics, and execution timelines.

## Features

- ✅ **Automatic Screenshot Capture**: Screenshots attached on test failure
- ✅ **Performance Metrics Visualization**: FPS, memory, frame time charts
- ✅ **Execution Timeline**: Visual timeline of test execution
- ✅ **Failure Analytics**: Detailed error messages and stack traces
- ✅ **Interactive UI**: Expandable test details, clickable screenshots
- ✅ **Multiple Formats**: HTML, JSON, JUnit XML, Markdown, Allure

## Quick Start

### Basic HTML Reports

```bash
# Generate HTML report
pytest tests/ --html=report.html --self-contained-html

# Open report in browser
start report.html  # Windows
open report.html   # macOS
xdg-open report.html  # Linux
```

### Enhanced Reports with Screenshots

```python
# conftest.py
pytest_plugins = ["yes_ue_fsd.reporting.screenshot_integration"]

# Tests automatically capture screenshots on failure
def test_movement(single_editor):
    driver = AutoDriver(player_index=0)
    driver.move_to((1000, 500, 100))
    assert driver.has_arrived()  # Screenshot captured if this fails
```

### Performance Metrics in Reports

```python
def test_performance(performance_metrics):
    """Test with automatic performance tracking."""
    driver = AutoDriver(player_index=0)

    # Record performance metrics
    performance_metrics.record("fps", 60.5)
    performance_metrics.record("memory_mb", 1024.5)
    performance_metrics.record("frame_time_ms", 16.7)

    # Metrics automatically added to report
```

## Report Formats

### 1. HTML Reports (pytest-html)

Beautiful, self-contained HTML reports with custom styling.

```bash
pytest tests/ --html=report.html --self-contained-html
```

**Features:**
- Pass/fail summary cards
- Expandable test details
- Screenshot gallery
- Performance metrics tables
- Responsive design

### 2. Allure Reports

Industry-standard rich reports with advanced visualizations.

```bash
# Run tests and generate Allure data
pytest tests/ --alluredir=allure-results

# Generate Allure HTML report
allure generate allure-results -o allure-report

# Open report
allure open allure-report
```

**Features:**
- Trend analysis across runs
- Test duration charts
- Flaky test detection
- Test categorization
- Attachments (screenshots, logs)

### 3. JSON Reports

Machine-readable reports for programmatic access.

```bash
pytest tests/ --json-report --json-report-file=report.json
```

**Use Cases:**
- CI/CD pipeline integration
- Custom dashboard development
- Trend analysis
- Test result archiving

### 4. JUnit XML

Standard format for CI/CD integration.

```bash
pytest tests/ --junit-xml=junit.xml
```

**Integrations:**
- Jenkins
- GitHub Actions
- GitLab CI
- Azure DevOps
- CircleCI

### 5. Markdown Summary

GitHub-friendly summaries for PR comments.

```bash
pytest tests/
# Generates test_results/summary.md automatically
```

Example output:
```markdown
## 🧪 Test Results

✅ 245 passed | ❌ 3 failed | ⏭️ 2 skipped | 📊 250 total

**Duration:** 127.45s

### Failed Tests

- ❌ `test_movement.py::test_navigate_to_waypoint` - AssertionError: Player did not arrive
- ❌ `test_combat.py::test_enemy_defeated` - TimeoutError: Enemy did not despawn
- ❌ `test_ui.py::test_main_menu` - WidgetNotFoundError: MainMenuWidget not visible
```

## Screenshot Integration

### Automatic Capture on Failure

Screenshots are automatically captured when tests fail:

```python
# conftest.py - Already configured by default
pytest_plugins = ["yes_ue_fsd.reporting.screenshot_integration"]

# No code changes needed - automatic!
def test_ui_navigation(single_editor):
    driver = AutoDriver(player_index=0)
    assert driver.is_widget_visible("MainMenu")  # Screenshot if fails
```

### Manual Screenshot Capture

```python
def test_manual_screenshot(single_editor):
    driver = AutoDriver(player_index=0)

    # Capture screenshot manually
    screenshot_path = driver.capture_screenshot("before_action")

    # Perform action
    driver.click_widget("PlayButton")

    # Capture another screenshot
    screenshot_path = driver.capture_screenshot("after_action")

    # Screenshots are automatically attached to report
```

### Screenshot Naming

Screenshots are automatically named based on:
- Test name
- Timestamp
- Failure location (for automatic captures)

Format: `test_<name>_<timestamp>.png`

### Viewing Screenshots in Reports

**HTML Report:**
- Click test name to expand details
- Screenshots displayed in gallery
- Click screenshot to view full size

**Allure Report:**
- Screenshots in "Attachments" tab
- Thumbnail preview
- Full size viewer

## Performance Metrics

### Collecting Metrics

```python
def test_spawn_100_npcs(performance_metrics):
    """Test NPC spawning performance."""
    driver = AutoDriver(player_index=0)

    # Record FPS before
    performance_metrics.record("fps_before", driver.get_fps())

    # Spawn NPCs
    for i in range(100):
        driver.spawn_npc(f"NPC_{i}")

    # Record FPS after
    performance_metrics.record("fps_after", driver.get_fps())

    # Record memory usage
    performance_metrics.record("memory_mb", driver.get_memory_usage_mb())

    # Assertions with metrics
    summary = performance_metrics.get_summary()
    assert summary["fps_after"]["mean"] > 30, "FPS dropped too low"
```

### Metric Types

Common performance metrics:

| Metric | Unit | Description |
|--------|------|-------------|
| `fps` | frames/sec | Frames per second |
| `frame_time_ms` | milliseconds | Frame render time |
| `memory_mb` | megabytes | Memory usage |
| `memory_delta_mb` | megabytes | Memory change |
| `draw_calls` | count | Draw calls per frame |
| `triangle_count` | count | Triangles rendered |
| `cpu_time_ms` | milliseconds | CPU time per frame |
| `gpu_time_ms` | milliseconds | GPU time per frame |

### Metric Statistics

All metrics automatically calculate:
- **Mean**: Average value
- **Median**: Middle value
- **Min**: Minimum value
- **Max**: Maximum value
- **StdDev**: Standard deviation
- **Count**: Number of samples

### Viewing Metrics in Reports

**HTML Report:**
```
Performance Metrics
┌──────────────────┬────────┐
│ fps_avg          │ 60.5   │
│ fps_min          │ 45.2   │
│ memory_mb_avg    │ 1024.5 │
│ frame_time_ms_p95│ 18.3   │
└──────────────────┴────────┘
```

**Allure Report:**
- Metrics displayed as graphs
- Trend analysis across runs
- Performance regression detection

## Custom Report Templates

### Creating Custom HTML Template

```python
# custom_reporter.py
from yes_ue_fsd.reporting import EnhancedHTMLReporter

class MyGameReporter(EnhancedHTMLReporter):
    def _get_css(self):
        """Override CSS with custom styling."""
        return super()._get_css() + """
        .test-item.passed .test-header {
            background: linear-gradient(135deg, #10b981 0%, #059669 100%);
            color: white;
        }
        """

    def _generate_test_list(self):
        """Override test list generation."""
        # Custom HTML generation
        html = super()._generate_test_list()
        # Add custom sections
        return html + "<div class='custom-section'>...</div>"

# Use custom reporter
reporter = MyGameReporter(output_dir="test_results")
reporter.generate_report()
```

### Adding Custom Sections

```python
from yes_ue_fsd.reporting import ReportGenerator

class GameReportGenerator(ReportGenerator):
    def generate_custom_report(self, test_results):
        """Generate report with game-specific sections."""
        report = self.generate_json_report(test_results)

        # Add custom analysis
        custom_data = {
            "game_version": "1.2.3",
            "engine_version": "5.4",
            "test_environment": "Production",
        }

        # Merge and save
        # ...
```

## CI/CD Integration

### GitHub Actions

```yaml
# .github/workflows/test.yml
name: Tests

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

      - name: Run tests
        run: |
          cd Content/Python
          pytest tests/ --html=report.html --self-contained-html --json-report --json-report-file=report.json

      - name: Upload test report
        if: always()
        uses: actions/upload-artifact@v3
        with:
          name: test-report
          path: |
            Content/Python/report.html
            Content/Python/report.json
            Content/Python/test_results/

      - name: Comment PR with results
        if: github.event_name == 'pull_request'
        uses: actions/github-script@v6
        with:
          script: |
            const fs = require('fs');
            const summary = fs.readFileSync('Content/Python/test_results/summary.md', 'utf8');
            github.rest.issues.createComment({
              issue_number: context.issue.number,
              owner: context.repo.owner,
              repo: context.repo.repo,
              body: summary
            });
```

### Jenkins Integration

```groovy
// Jenkinsfile
pipeline {
    agent any

    stages {
        stage('Test') {
            steps {
                bat 'pytest tests/ --junit-xml=junit.xml --html=report.html'
            }
        }
    }

    post {
        always {
            junit 'junit.xml'
            publishHTML([
                reportDir: '.',
                reportFiles: 'report.html',
                reportName: 'Test Report'
            ])
        }
    }
}
```

## Advanced Features

### Test History and Trends

Track test performance over time:

```python
from yes_ue_fsd.reporting import ReportGenerator

generator = ReportGenerator()

# Generate report with history
generator.generate_report_with_trends(
    current_results=test_results,
    history_file="test_results/history.json"
)
```

### Flaky Test Detection

Automatically identify flaky tests:

```python
# Reports track test flakiness
# A test is flagged as flaky if:
# - Passes after retry
# - Fails intermittently
# - Has high variance in execution time
```

### Performance Regression Detection

Automatically detect performance regressions:

```python
# Reports compare against baseline
# Regression detected if:
# - FPS drops by >10%
# - Memory increases by >20%
# - Test duration increases by >50%
```

## Best Practices

### 1. Use Descriptive Test Names

Good:
```python
def test_player_navigates_to_waypoint_within_10_seconds():
    pass
```

Avoid:
```python
def test_1():
    pass
```

### 2. Add Test Docstrings

```python
def test_movement():
    """
    Test player movement to waypoint.

    Verifies that:
    - Player reaches waypoint within 10 seconds
    - Player is within 50 units of target
    - Navigation path is valid
    """
    pass
```

### 3. Capture Context on Failure

```python
def test_with_context(single_editor):
    driver = AutoDriver(player_index=0)

    try:
        driver.move_to((1000, 500, 100))
        assert driver.has_arrived()
    except AssertionError:
        # Capture additional context on failure
        driver.capture_screenshot("failure_context")
        driver.log_player_state()
        driver.log_navigation_path()
        raise
```

### 4. Organize Reports by Test Suite

```bash
# Separate reports for different suites
pytest tests/movement/ --html=reports/movement.html
pytest tests/combat/ --html=reports/combat.html
pytest tests/ui/ --html=reports/ui.html
```

### 5. Archive Reports for History

```python
import shutil
from datetime import datetime

# Archive reports with timestamp
timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
shutil.copy("report.html", f"archives/report_{timestamp}.html")
```

## Troubleshooting

### Screenshots Not Appearing

**Problem:** Screenshots not showing in HTML report

**Solutions:**
1. Verify screenshot plugin is loaded:
   ```python
   pytest_plugins = ["yes_ue_fsd.reporting.screenshot_integration"]
   ```

2. Check screenshot directory exists:
   ```bash
   ls test_results/screenshots/
   ```

3. Verify pytest-html is installed:
   ```bash
   pip install pytest-html>=3.1.0
   ```

### Performance Metrics Missing

**Problem:** Metrics not in report

**Solution:** Use `performance_metrics` fixture:
```python
def test_with_metrics(performance_metrics):  # Add fixture
    performance_metrics.record("fps", 60.0)
```

### Allure Report Not Generating

**Problem:** `allure command not found`

**Solution:**
```bash
# Install Allure CLI
# macOS
brew install allure

# Windows
scoop install allure

# Or use Docker
docker run -p 8080:8080 "allegro/allure" version
```

## Examples

See example test suites with reports:
- `Content/Python/tests/test_movement.py` - Movement tests with screenshots
- `Content/Python/tests/test_performance.py` - Performance tests with metrics
- `Content/Python/tests/test_multiplayer.py` - Multiplayer tests with multi-instance reports

## References

- [pytest-html Documentation](https://pytest-html.readthedocs.io/)
- [Allure Framework](https://docs.qameta.io/allure/)
- [pytest Reporting Plugins](https://docs.pytest.org/en/stable/how-to/output.html)
