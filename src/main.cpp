#include "metrics.hpp"
#include "detector.hpp"
#include "alert.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
  float vals[N_METRICS];
  float zscores[N_METRICS];
  AnomalyDetector det;

  while (true) {
    // 1) Sample raw metrics
    sample_system_metrics(vals);

    // 2) Compute anomalies
    bool is_anom = det.feed(vals, zscores);

    // 3) Print raw metrics
    std::cout << "[";
    for (std::size_t i = 0; i < N_METRICS; ++i) {
      std::cout << vals[i] 
                << (i+1 < N_METRICS ? ", " : "");
    }
    std::cout << "]";

    // 4) If anomaly, alert for each offending metric
    if (is_anom) {
      std::cout << "  !!!";
      for (std::size_t i = 0; i < N_METRICS; ++i) {
        if (std::fabs(zscores[i]) > Z_THRESHOLD) {
          Alert::raise(i, vals[i], zscores[i]);
        }
      }
    }
    std::cout << "\n";

    std::this_thread::sleep_for(
      std::chrono::milliseconds(SAMPLE_MS)
    );
  }
  return 0;
}