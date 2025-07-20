#pragma once
#include <cstddef>

// number of streams/metrics we track
constexpr std::size_t N_METRICS = 5;

// sampling interval in milliseconds (used in main.cpp)
constexpr unsigned SAMPLE_MS = 500;

// EWMA smoothing factor α (0 < α < 1). 
// Smaller α → slower adaptation, larger α → faster adaptation
// Tuned to be more stable and less sensitive to noise
constexpr float EWMA_ALPHA = 0.005f;  // Further reduced for much more stability

// Global z-score threshold for flagging an anomaly
constexpr float Z_THRESHOLD = 5.0f;  // Increased significantly for less sensitivity

// small epsilon to avoid divide-by-zero in z-score
constexpr float EPSILON = 1e-6f;

// # of initial samples to learn a baseline before raising alerts
constexpr unsigned WARMUP_SAMPLES = 120;  // Much longer baseline learning

// Hysteresis settings to prevent rapid on/off alerts
constexpr float HYSTERESIS_THRESHOLD = 4.0f;  // Lower threshold for clearing alerts
constexpr unsigned MIN_QUIET_TIME_MS = 30000;  // 30 seconds minimum between alerts
constexpr unsigned HYSTERESIS_SAMPLES = 10;    // Need 10 consecutive normal samples to clear

// Per-metric thresholds (more conservative for each metric)
constexpr float CPU_THRESHOLD = 6.0f;      // CPU can spike, be more tolerant
constexpr float RAM_THRESHOLD = 4.5f;      // RAM at 94% might be normal for your system
constexpr float DISK_THRESHOLD = 5.0f;     // Disk I/O can be bursty
constexpr float HEAP_THRESHOLD = 8.0f;     // Heap free varies a lot, be very tolerant
constexpr float UPTIME_THRESHOLD = 4.0f;   // Uptime should be stable

// Per-metric hysteresis thresholds
constexpr float CPU_HYSTERESIS = 5.0f;
constexpr float RAM_HYSTERESIS = 3.5f;
constexpr float DISK_HYSTERESIS = 4.0f;
constexpr float HEAP_HYSTERESIS = 6.0f;
constexpr float UPTIME_HYSTERESIS = 3.0f;