# System Anomaly Detector

A sophisticated real-time system monitoring tool that detects anomalies in system metrics using EWMA (Exponentially Weighted Moving Average) and z-score analysis with advanced hysteresis and per-metric tuning.

**🖥️ Cross-Platform Support**: macOS, Linux, and Windows

## Features

### 🚨 Real-time Monitoring
- **Live Metrics Display**: Color-coded real-time monitoring of 5 system metrics
- **Visual Progress Bars**: For percentage-based metrics (CPU, RAM)
- **Status Indicators**: Normal/Warning/Anomaly status with color coding
- **Warm-up Period**: System learns baseline behavior before raising alerts (2 minutes)
- **Interactive CLI**: Press 'i' for menu system with timeline and statistics

### 🎯 Advanced Anomaly Detection
- **EWMA Algorithm**: Exponentially Weighted Moving Average for trend analysis
- **Z-Score Analysis**: Statistical anomaly detection with per-metric thresholds
- **Multi-Metric Monitoring**: CPU, RAM, Disk I/O, Heap, Uptime
- **Per-Metric Tuning**: Each metric has optimized sensitivity levels
- **Hysteresis System**: Prevents rapid on/off alerts with sophisticated state tracking

### 🚨 Sophisticated Alarm System
- **Visual Alarms**: Blinking indicators and color-coded alerts
- **Audio Alarms**: System bell notifications for anomalies
- **Real-time Alerts**: Immediate notification when anomalies are detected
- **Hysteresis Control**: 30-second minimum between alerts, 10 consecutive normal samples to clear
- **Per-Metric Thresholds**: Different sensitivity for each metric type

### 📊 Anomaly Timeline
- **Event Recording**: All anomalies are stored with timestamps
- **Timeline Display**: View recent anomalies in chronological order
- **Statistics**: Detailed analysis of anomaly patterns
- **Export Capability**: Save anomaly data for further analysis
- **Interactive Management**: Clear timeline, view statistics, export data

### 🎮 Enhanced Interactive CLI
- **Menu System**: Press 'i' to access interactive menu
- **Help System**: Comprehensive documentation and usage guide
- **Statistics View**: Detailed anomaly analysis and metrics
- **Timeline Management**: View, clear, and export anomaly data
- **Real-time Status**: Live updates with color-coded indicators

### 🖥️ Cross-Platform Support
- **macOS**: Native Mach API integration for optimal performance
- **Linux**: /proc filesystem and sysinfo API support
- **Windows**: Performance Data Helper (PDH) API integration
- **Unified Interface**: Same CLI experience across all platforms
- **Platform Detection**: Automatic OS detection and appropriate implementation

## Monitored Metrics

| Metric | Description | Unit | Threshold | Hysteresis | Reasoning |
|--------|-------------|------|-----------|------------|-----------|
| CPU Utilization | CPU usage percentage | % | 6.0 | 5.0 | CPU can spike naturally |
| RAM Usage | Memory usage percentage | % | 4.5 | 3.5 | High usage might be normal |
| Disk I/O Rate | Disk read/write rate | bytes/sec | 5.0 | 4.0 | Disk I/O can be bursty |
| Heap Free | Available heap memory | bytes | 8.0 | 6.0 | Heap varies significantly |
| Uptime | System uptime | hours | 4.0 | 3.0 | Should be stable |

## Quick Start

### Prerequisites
- CMake 3.10 or higher
- C++17 compiler
- Platform-specific headers (automatically detected)

### Build and Run

#### Option 1: Automated Build Script (Recommended)
```bash
# Clone the repository
git clone https://github.com/yourusername/ml-system-anomoly-detector.git
cd ml-system-anomoly-detector

# Build for your platform
./build.sh

# Run the application
./build/bin/anom_detect_macos    # macOS
./build/bin/anom_detect_linux    # Linux
./build/bin/anom_detect_windows  # Windows
```

#### Option 2: Manual CMake Build
```bash
# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make

# Run
./bin/anom_detect_macos    # macOS
./bin/anom_detect_linux    # Linux
./bin/anom_detect_windows  # Windows
```

### Platform-Specific Notes

#### macOS
- Uses Mach API for system metrics
- Native heap statistics via malloc_zone_statistics
- Optimal performance with minimal overhead

#### Linux
- Reads from /proc filesystem for system stats
- Uses sysinfo API for memory information
- Process-specific metrics from /proc/self/status

#### Windows
- Performance Data Helper (PDH) API for metrics
- Windows Management Instrumentation (WMI) integration
- Native Windows performance counters

## Configuration

### Core Parameters
```cpp
constexpr float EWMA_ALPHA = 0.005f;        // Very slow adaptation for stability
constexpr float Z_THRESHOLD = 5.0f;         // Global anomaly detection threshold
constexpr unsigned WARMUP_SAMPLES = 120;     // 2 minutes of baseline learning
constexpr unsigned SAMPLE_MS = 500;          // Sampling interval (ms)
```

### Hysteresis Settings
```cpp
constexpr float HYSTERESIS_THRESHOLD = 4.0f;  // Lower threshold for clearing alerts
constexpr unsigned MIN_QUIET_TIME_MS = 30000;  // 30 seconds minimum between alerts
constexpr unsigned HYSTERESIS_SAMPLES = 10;    // Need 10 consecutive normal samples
```

### Per-Metric Thresholds
Each metric has optimized sensitivity based on its characteristics:
- **CPU**: Higher threshold (6.0) due to natural spikes
- **RAM**: Moderate threshold (4.5) for high-usage systems
- **Disk I/O**: Standard threshold (5.0) for bursty operations
- **Heap Free**: Very high threshold (8.0) due to significant variation
- **Uptime**: Lower threshold (4.0) for stability

## Building and Running

### Prerequisites
- CMake 3.10 or higher
- C++17 compiler
- Platform-specific development tools

### Build
```bash
mkdir build
cd build
cmake ..
make
```

### Run
```bash
./bin/anom_detect_macos    # macOS
./bin/anom_detect_linux    # Linux
./bin/anom_detect_windows  # Windows
```

## Usage

### Real-time Monitoring
The system starts in monitoring mode with a beautiful real-time display:

```
╔══════════════════════════════════════════════════════════════════════════════╗
║                    SYSTEM ANOMALY DETECTOR - Real-time Monitor                    ║
╚══════════════════════════════════════════════════════════════════════════════╝

┌─ METRICS PANEL  [ACTIVE MONITORING]
│ CPU Utilization     45.2 % [z= 0.12] ✓ NORMAL [████████████░░░░░░░░]
│ RAM Usage          67.8 % [z= 1.45] ⚠ WARNING [████████████████░░░░]
│ Disk I/O Rate       0.00 B/s [z= 0.00] ✓ NORMAL
│ Heap Free         1024.0 B [z= 0.23] ✓ NORMAL
│ Uptime              2.3 hrs [z= 0.01] ✓ NORMAL
└─────────────────────────────────────────────────────────────────────────────

┌─ STATUS BAR
│ ✅ System Normal - No anomalies detected
│ 📊 Anomaly Timeline: 0 events recorded
└─────────────────────────────────────────────────────────────────────────────

┌─ ANOMALY TIMELINE
│ No anomalies detected yet
└─────────────────────────────────────────────────────────────────────────────
```

### Interactive Menu
Press `i` to access the interactive menu:

```
╔══════════════════════════════════════════════════════════════════════════════╗
║                        INTERACTIVE MENU - Anomaly Detector                    ║
╚══════════════════════════════════════════════════════════════════════════════╝

Available Commands:
  h - Show this help menu
  t - View detailed anomaly timeline
  s - Show statistics
  c - Clear timeline
  e - Export timeline to file
  q - Return to monitoring
  x - Exit program

Enter command:
```

### Alarm Effects
When anomalies are detected:

1. **Visual Alarm**: Blinking red indicators and reverse video
2. **Audio Alarm**: System bell notification
3. **Detailed Alert**: Comprehensive anomaly information with per-metric thresholds
4. **Timeline Update**: Event recorded with timestamp and z-score
5. **Hysteresis Control**: Prevents rapid on/off alerts

Example anomaly alert:
```
🚨 ANOMALY DETECTED! 🚨
Metric: CPU Utilization
Value: 95.6 %
Z-Score: 4.23
Threshold: 6.0 (per-metric)
Timestamp: 14:32:15
```

## Architecture

### Core Components

1. **Platform Abstraction** (`platform_metrics.hpp`, `platform_*.cpp`)
   - Cross-platform interface for system metrics
   - OS-specific implementations for macOS, Linux, Windows
   - Factory pattern for platform detection and instantiation

2. **Metrics Collection** (`platform_*.cpp`)
   - Platform-specific system metric sampling
   - Unified interface across all operating systems
   - Real-time CPU, memory, disk I/O, heap, and system stats

3. **Anomaly Detection** (`detector.hpp`, `stats.hpp`)
   - EWMA algorithm implementation with slow adaptation (α=0.005)
   - Z-score calculation with per-metric thresholds
   - Multi-stream anomaly detection with hysteresis
   - State tracking for each metric

4. **CLI Monitor** (`cli_monitor.hpp`, `cli_monitor_impl.cpp`)
   - Real-time terminal UI with color coding
   - Alarm effects and notifications
   - Interactive menu system
   - Timeline management and statistics
   - Per-metric threshold display

5. **Configuration** (`config.hpp`)
   - Tunable parameters with per-metric optimization
   - Hysteresis settings for stability
   - Warm-up and sampling configuration

### Algorithm Details

**EWMA (Exponentially Weighted Moving Average)**:
- Smoothing factor α = 0.005 (very slow adaptation)
- Updates mean and variance incrementally
- Provides robust baseline for anomaly detection
- 2-minute warm-up period for baseline learning

**Z-Score Analysis**:
- Statistical measure of deviation from baseline
- Per-metric thresholds based on metric characteristics
- Hysteresis system prevents rapid alerts
- 30-second minimum between alerts

**Hysteresis System**:
- Prevents rapid on/off alerts
- Each metric has independent state tracking
- Requires 10 consecutive normal samples to clear
- Per-metric hysteresis thresholds

## Advanced Features

### Per-Metric Tuning
Each metric is optimized for its characteristics:
- **CPU**: Tolerant of spikes (threshold 6.0)
- **RAM**: Moderate sensitivity (threshold 4.5)
- **Disk I/O**: Standard sensitivity (threshold 5.0)
- **Heap Free**: Very tolerant (threshold 8.0)
- **Uptime**: Stable expectation (threshold 4.0)

### Hysteresis Control
- **Alert Triggering**: Only triggers if z-score > threshold AND 30s since last alert
- **Alert Clearing**: Only clears if z-score < hysteresis threshold AND 10 consecutive normal samples
- **State Tracking**: Each metric maintains independent anomaly state

### Interactive Features
- **Real-time Statistics**: View anomaly patterns and trends
- **Timeline Management**: Clear, export, and analyze anomaly history
- **Help System**: Comprehensive documentation and usage guide
- **Export Capability**: Save anomaly data for external analysis

### Cross-Platform Compatibility
- **Unified Interface**: Same CLI experience across all platforms
- **Platform Detection**: Automatic OS detection and appropriate implementation
- **Native APIs**: Uses platform-specific APIs for optimal performance
- **Consistent Metrics**: Same 5 metrics available on all platforms

## Extending the System

### Adding New Metrics
1. Update `N_METRICS` in `config.hpp`
2. Add metric enum in `metrics.hpp`
3. Implement sampling in all platform files (`platform_*.cpp`)
4. Add per-metric thresholds in `config.hpp`
5. Update display names in `cli_monitor_impl.cpp`

### Custom Alarm Actions
1. Modify `CLIMonitor::trigger_alarm()` in `cli_monitor_impl.cpp`
2. Add network notifications, log files, or external integrations
3. Implement custom visual/audio effects
4. Add persistence for alarm state

### Export Functionality
1. Implement `CLIMonitor::export_timeline()` in `cli_monitor_impl.cpp`
2. Add CSV/JSON export formats
3. Include metadata and statistics
4. Add filtering and search capabilities

### Advanced Tuning
1. **Adjust per-metric thresholds** in `config.hpp`
2. **Modify hysteresis settings** for different environments
3. **Change warm-up period** for different baseline requirements
4. **Add persistence** to save EWMA state between restarts

## Troubleshooting

### Common Issues

1. **Too many false positives**: 
   - Increase per-metric thresholds in `config.hpp`
   - Extend warm-up period (`WARMUP_SAMPLES`)
   - Reduce EWMA alpha for more stability

2. **Missing real anomalies**:
   - Decrease per-metric thresholds
   - Increase EWMA alpha for faster adaptation
   - Reduce hysteresis requirements

3. **Display issues**: 
   - Ensure terminal supports ANSI color codes
   - Check terminal size for proper layout

4. **Build errors**: 
   - Verify CMake version and C++17 support
   - Check platform-specific headers availability
   - Ensure proper development tools installed

### Performance Tuning

**For High-Sensitivity Environments**:
- Decrease per-metric thresholds (e.g., CPU_THRESHOLD = 4.0)
- Increase EWMA alpha (α = 0.01)
- Reduce warm-up samples (WARMUP_SAMPLES = 60)

**For Stable Production Systems**:
- Increase per-metric thresholds (e.g., CPU_THRESHOLD = 8.0)
- Decrease EWMA alpha (α = 0.002)
- Extend warm-up period (WARMUP_SAMPLES = 180)

**For Development/Testing**:
- Use moderate thresholds
- Standard EWMA alpha (α = 0.005)
- Standard warm-up (WARMUP_SAMPLES = 120)

## Automated Releases

The project includes GitHub Actions workflows that automatically:

1. **Build for all platforms** (macOS, Linux, Windows)
2. **Run tests** on each platform
3. **Create release packages** with proper naming
4. **Generate GitHub releases** when tags are pushed

### Creating a Release
```bash
# Tag a new version
git tag v1.0.0

# Push the tag to trigger automated release
git push origin v1.0.0
```

This will automatically:
- Build executables for all three platforms
- Create release packages (ZIP for macOS/Windows, TGZ for Linux)
- Generate a GitHub release with all artifacts
- Include release notes and changelog

## License

This project is open source and available under the MIT License.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for:

- Bug fixes
- New features
- Documentation improvements
- Performance optimizations
- Additional metric types
- Enhanced export formats
- Platform-specific improvements 