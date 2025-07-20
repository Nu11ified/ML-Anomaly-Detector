#include "detector.hpp"
#include "cli_monitor.hpp"
#include "platform_metrics.hpp"
#include "config.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory>

// Global monitor for signal handling
static CLIMonitor* g_monitor = nullptr;

// Signal handler for clean exit
void signal_handler(int signal) {
    if (g_monitor) {
        std::cout << "\n\n" << "\033[1m\033[33m" << "Shutting down anomaly detector...\n" << "\033[0m";
        g_monitor->show_timeline();
    }
    exit(0);
}

// Check for keyboard input (non-blocking)
bool check_keyboard_input() {
    struct termios old_tio, new_tio;
    tcgetattr(STDIN_FILENO, &old_tio);
    new_tio = old_tio;
    new_tio.c_lflag &= (~ICANON & ~ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
    
    int old_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, old_flags | O_NONBLOCK);
    
    char ch;
    bool has_input = (read(STDIN_FILENO, &ch, 1) == 1);
    
    fcntl(STDIN_FILENO, F_SETFL, old_flags);
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
    
    return has_input && (ch == 'i' || ch == 'I');
}

int main() {
    // Setup signal handling
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Create platform-specific metrics
    std::unique_ptr<PlatformMetrics> platform = std::unique_ptr<PlatformMetrics>(create_platform_metrics());
    if (!platform || !platform->initialize()) {
        std::cerr << "Failed to initialize platform metrics\n";
        return 1;
    }
    
    std::cout << "Platform: " << platform->get_platform_name() << "\n";
    
    float vals[N_METRICS], zscores[N_METRICS];
    AnomalyDetector det;
    CLIMonitor monitor;
    g_monitor = &monitor;
    
    unsigned sample_count = 0;
    
    // Setup the display
    monitor.setup_display();
    
    std::cout << "\033[1m\033[32m" << "Starting System Anomaly Detector...\n" << "\033[0m";
    std::cout << "Press Ctrl+C to exit and view timeline\n";
    std::cout << "Press 'i' for interactive menu\n\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    while (true) {
        platform->sample_system_metrics(vals);
        bool has_anomaly = det.feed(vals, zscores);
        
        ++sample_count;
        bool ready = (sample_count > WARMUP_SAMPLES);
        
        // Update the real-time display
        monitor.update_display(vals, zscores, sample_count, !ready);
        
        // Handle anomalies after warm-up (using hysteresis-aware detection)
        if (ready && has_anomaly) {
            for (std::size_t i = 0; i < N_METRICS; ++i) {
                if (det.is_anomaly_active(i)) {
                    monitor.handle_anomaly(i, vals[i], zscores[i]);
                }
            }
        }
        
        // Check for keyboard input
        if (check_keyboard_input()) {
            monitor.set_interactive_mode(true);
        }
        
        // Check for interactive mode toggle
        if (monitor.is_interactive_mode()) {
            monitor.show_interactive_menu();
            monitor.handle_user_input();
        }
        
        // Sleep between samples
        std::this_thread::sleep_for(
            std::chrono::milliseconds(SAMPLE_MS)
        );
    }
    
    platform->cleanup();
    return 0;
}