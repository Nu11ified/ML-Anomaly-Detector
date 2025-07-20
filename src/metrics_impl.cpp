#include "metrics.hpp"
#include <cstring>

void sample_system_metrics(float out_vals[N_METRICS]) {
  // stub: zero all metrics
  std::memset(out_vals, 0, sizeof(float)*N_METRICS);
  // later: #ifdef __linux__ / _WIN32 / __APPLE__ / MCU
}