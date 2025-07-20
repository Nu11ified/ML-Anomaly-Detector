#include "metrics.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
  float vals[N_METRICS];
  while (true) {
    sample_system_metrics(vals);
    for (std::size_t i = 0; i < N_METRICS; ++i) {
      std::cout << vals[i] << (i+1<N_METRICS? ", ":"\n");
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(SAMPLE_MS));
  }
  return 0;
}