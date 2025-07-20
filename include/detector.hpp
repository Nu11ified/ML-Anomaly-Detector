#pragma once
#include "config.hpp"
#include "stats.hpp"
#include <array>
#include <cstddef>
#include <cmath>

// Online anomaly detector over N_METRICS streams.
class AnomalyDetector {
  std::array<EWMA, N_METRICS> stats_;

public:
  // Feed raw metrics; outputs per-metric z-scores.
  // Returns true if any |z| > Z_THRESHOLD.
  bool feed(const float vals[N_METRICS],
            float zscores[N_METRICS]) {
    bool any_anom = false;
    for (std::size_t i = 0; i < N_METRICS; ++i) {
      stats_[i].update(vals[i]);
      float z = stats_[i].z_score(vals[i]);
      zscores[i] = z;
      if (std::fabs(z) > Z_THRESHOLD) {
        any_anom = true;
      }
    }
    return any_anom;
  }
};