#include "platform_metrics.hpp"

// Include platform-specific implementations with conditional compilation
#ifdef _WIN32
    #include "platform_windows.cpp"
#elif defined(__APPLE__)
    #include "platform_macos.cpp"
#elif defined(__linux__)
    #include "platform_linux.cpp"
#else
    #error "Unsupported platform - only Windows, macOS, and Linux are supported"
#endif

PlatformMetrics* create_platform_metrics() {
#ifdef _WIN32
    return new WindowsMetrics();
#elif defined(__APPLE__)
    return new MacOSMetrics();
#elif defined(__linux__)
    return new LinuxMetrics();
#else
    return nullptr;
#endif
} 