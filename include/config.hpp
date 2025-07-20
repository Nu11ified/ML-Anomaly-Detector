#pragma once
#include <cstddef>

// number of streams/metrics we track
constexpr std::size_t N_METRICS = 5;

// sampling interval in milliseconds (used in main.cpp)
constexpr unsigned SAMPLE_MS = 500;

// EWMA smoothing factor α (0 < α < 1). 
// Smaller α → slower adaptation, larger α → faster adaptation
constexpr float EWMA_ALPHA = 0.02f;

// z-score threshold for flagging an anomaly
constexpr float Z_THRESHOLD = 3.0f;

// small epsilon to avoid divide-by-zero in z-score
constexpr float EPSILON = 1e-6f;