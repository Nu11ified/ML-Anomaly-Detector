#pragma once
#include "config.hpp"
#include <cmath>

// Exponentially‐weighted moving average & variance
struct EWMA {
  float mean{0.0f};
  float var{0.0f};
  bool  initialized{false};

  // Update with new sample x_t
  void update(float x) {
    if (!initialized) {
      // Initialize on first sample
      mean = x;
      var  = 0.0f;
      initialized = true;
      return;
    }
    float delta = x - mean;
    mean += EWMA_ALPHA * delta;
    var  = EWMA_ALPHA * (delta*delta) + (1.0f - EWMA_ALPHA) * var;
  }

  // Compute z-score; guard against tiny variance
  float z_score(float x) const {
    if (!initialized || var < EPSILON) return 0.0f;
    return (x - mean) / std::sqrt(var + EPSILON);
  }
};