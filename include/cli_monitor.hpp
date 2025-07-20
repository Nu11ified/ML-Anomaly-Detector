#pragma once
#include "config.hpp"
#include "metrics.hpp"
#include <vector>
#include <string>
#include <chrono>
#include <memory>

// Anomaly event record
struct AnomalyEvent {
    std::chrono::system_clock::time_point timestamp;
    std::size_t metric_index;
    float value;
    float z_score;
    std::string metric_name;
    
    AnomalyEvent(std::size_t idx, float val, float z) 
        : timestamp(std::chrono::system_clock::now())
        , metric_index(idx)
        , value(val)
        , z_score(z)
        , metric_name(get_metric_name(idx)) {}
    
    // Get metric name for display
    static std::string get_metric_name(std::size_t idx);
};

// Enhanced CLI Monitor with real-time display and alarm effects
class CLIMonitor {
private:
    std::vector<AnomalyEvent> anomaly_timeline_;
    std::chrono::system_clock::time_point last_alarm_time_;
    bool alarm_active_{false};
    unsigned alarm_count_{0};
    bool interactive_mode_{false};
    
    // Terminal control sequences
    static constexpr const char* CLEAR_SCREEN = "\033[2J";
    static constexpr const char* CLEAR_LINE = "\033[K";
    static constexpr const char* CURSOR_HOME = "\033[H";
    static constexpr const char* BOLD = "\033[1m";
    static constexpr const char* RED = "\033[31m";
    static constexpr const char* GREEN = "\033[32m";
    static constexpr const char* YELLOW = "\033[33m";
    static constexpr const char* BLUE = "\033[34m";
    static constexpr const char* MAGENTA = "\033[35m";
    static constexpr const char* CYAN = "\033[36m";
    static constexpr const char* RESET = "\033[0m";
    static constexpr const char* REVERSE = "\033[7m";
    static constexpr const char* BLINK = "\033[5m";
    
    // Audio alarm (ASCII bell)
    static constexpr const char* ALARM_BELL = "\a";
    
public:
    CLIMonitor() = default;
    
    // Main display update
    void update_display(const float vals[N_METRICS], 
                       const float zscores[N_METRICS],
                       unsigned sample_count,
                       bool warming_up);
    
    // Handle anomaly detection
    void handle_anomaly(std::size_t metric_idx, float value, float z_score);
    
    // Display anomaly timeline
    void show_timeline();
    
    // Clear screen and setup
    void setup_display();
    
    // Get alarm status
    bool is_alarm_active() const { return alarm_active_; }
    
    // Interactive menu system
    void show_interactive_menu();
    void handle_user_input();
    void show_help();
    void show_statistics();
    void clear_timeline();
    void export_timeline(const std::string& filename);
    
    // Toggle interactive mode
    void set_interactive_mode(bool enabled) { interactive_mode_ = enabled; }
    bool is_interactive_mode() const { return interactive_mode_; }
    
private:
    // Helper methods
    void draw_header();
    void draw_metrics_panel(const float vals[N_METRICS], 
                           const float zscores[N_METRICS],
                           unsigned sample_count,
                           bool warming_up);
    void draw_status_bar();
    void draw_timeline_panel();
    void trigger_alarm();
    void clear_alarm();
    std::string format_value(float val, std::size_t metric_idx);
    std::string get_status_color(float z_score);
    std::string get_metric_unit(std::size_t metric_idx);
    void draw_progress_bar(float percentage, int width = 20);
    std::string format_timestamp(const std::chrono::system_clock::time_point& tp);
    float get_metric_threshold(std::size_t metric_idx);
    float get_hysteresis_threshold(std::size_t metric_idx);
}; 