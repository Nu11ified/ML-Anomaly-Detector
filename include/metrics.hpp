#pragma once
#include "config.hpp"

// fixed indices 0..N_METRICS-1
enum Metric : std::size_t {
  CPU_UTIL = 0,
  RAM_USED,
  DISK_IO_RATE,
  HEAP_FREE,
  UPTIME_MS,
  METRIC_COUNT = N_METRICS
};

// platform‐specific fill
void sample_system_metrics(float out_vals[N_METRICS]);