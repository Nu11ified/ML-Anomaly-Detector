#pragma once
#include <cstddef>
#include <iostream>

// Simple console alert; swap out for LEDs, network, etc.
struct Alert {
  // i = metric index, v = value, z = z-score
  static void raise(std::size_t i, float v, float z) {
    std::cout 
      << ">>> Anomaly on metric[" << i 
      << "] val=" << v 
      << " z=" << z << "\n";
  }
};