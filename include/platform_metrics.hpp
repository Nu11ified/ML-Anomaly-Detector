#pragma once
#include "config.hpp"
#include <cstddef>

// Generic platform metrics interface
class PlatformMetrics {
public:
    virtual ~PlatformMetrics() = default;
    
    // Initialize platform-specific resources
    virtual bool initialize() = 0;
    
    // Sample all system metrics
    virtual void sample_system_metrics(float out[N_METRICS]) = 0;
    
    // Get platform name for display
    virtual const char* get_platform_name() const = 0;
    
    // Cleanup platform-specific resources
    virtual void cleanup() = 0;
};

// Factory function to create appropriate platform implementation
PlatformMetrics* create_platform_metrics(); 