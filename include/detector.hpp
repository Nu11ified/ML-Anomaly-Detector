#pragma once
#include "config.hpp"
#include "stats.hpp"
#include <array>
#include <cstddef>
#include <cmath>
#include <chrono>

// Online anomaly detector over N_METRICS streams with hysteresis.
class AnomalyDetector {
  std::array<EWMA, N_METRICS> stats_;
  
  // Hysteresis state tracking
  std::array<bool, N_METRICS> anomaly_active_;      // Current anomaly state per metric
  std::array<unsigned, N_METRICS> normal_samples_;  // Consecutive normal samples
  std::array<std::chrono::steady_clock::time_point, N_METRICS> last_alert_time_;
  
  // Per-metric thresholds
  std::array<float, N_METRICS> thresholds_;
  std::array<float, N_METRICS> hysteresis_thresholds_;
  
  // Get threshold for a specific metric
  float get_threshold(std::size_t metric_idx) const {
    switch (metric_idx) {
      case 0: return CPU_THRESHOLD;      // CPU_UTIL
      case 1: return RAM_THRESHOLD;      // RAM_USED
      case 2: return DISK_THRESHOLD;     // DISK_IO_RATE
      case 3: return HEAP_THRESHOLD;     // HEAP_FREE
      case 4: return UPTIME_THRESHOLD;   // UPTIME_MS
      default: return Z_THRESHOLD;
    }
  }
  
  // Get hysteresis threshold for a specific metric
  float get_hysteresis_threshold(std::size_t metric_idx) const {
    switch (metric_idx) {
      case 0: return CPU_HYSTERESIS;
      case 1: return RAM_HYSTERESIS;
      case 2: return DISK_HYSTERESIS;
      case 3: return HEAP_HYSTERESIS;
      case 4: return UPTIME_HYSTERESIS;
      default: return HYSTERESIS_THRESHOLD;
    }
  }
  
  // Initialize hysteresis state
  void init_hysteresis() {
    for (std::size_t i = 0; i < N_METRICS; ++i) {
      anomaly_active_[i] = false;
      normal_samples_[i] = 0;
      last_alert_time_[i] = std::chrono::steady_clock::now();
      thresholds_[i] = get_threshold(i);
      hysteresis_thresholds_[i] = get_hysteresis_threshold(i);
    }
  }

public:
  AnomalyDetector() {
    init_hysteresis();
  }

  // Feed raw metrics; outputs per-metric z-scores.
  // Returns true if any anomaly is active (considering hysteresis).
  bool feed(const float vals[N_METRICS],
            float zscores[N_METRICS]) {
    bool any_anom = false;
    auto now = std::chrono::steady_clock::now();
    
    for (std::size_t i = 0; i < N_METRICS; ++i) {
      stats_[i].update(vals[i]);
      float z = stats_[i].z_score(vals[i]);
      zscores[i] = z;
      
      float threshold = thresholds_[i];
      float hysteresis_threshold = hysteresis_thresholds_[i];
      
      // Check if we should trigger a new alert
      bool should_alert = false;
      if (!anomaly_active_[i]) {
        // Not currently in anomaly state - check if we should trigger
        if (std::fabs(z) > threshold) {
          auto time_since_last = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - last_alert_time_[i]).count();
          
          if (time_since_last >= MIN_QUIET_TIME_MS) {
            should_alert = true;
            anomaly_active_[i] = true;
            normal_samples_[i] = 0;
            last_alert_time_[i] = now;
          }
        }
      } else {
        // Currently in anomaly state - check if we should clear
        if (std::fabs(z) < hysteresis_threshold) {
          normal_samples_[i]++;
          if (normal_samples_[i] >= HYSTERESIS_SAMPLES) {
            anomaly_active_[i] = false;
            normal_samples_[i] = 0;
          }
        } else {
          // Still anomalous, reset normal sample counter
          normal_samples_[i] = 0;
        }
      }
      
      if (anomaly_active_[i]) {
        any_anom = true;
      }
    }
    
    return any_anom;
  }
  
  // Get current anomaly state for a specific metric
  bool is_anomaly_active(std::size_t metric_idx) const {
    return (metric_idx < N_METRICS) ? anomaly_active_[metric_idx] : false;
  }
  
  // Get threshold for a specific metric (for display purposes)
  float get_metric_threshold(std::size_t metric_idx) const {
    return (metric_idx < N_METRICS) ? thresholds_[metric_idx] : Z_THRESHOLD;
  }
  
  // Reset hysteresis state (useful for testing or system reset)
  void reset_hysteresis() {
    init_hysteresis();
  }
};