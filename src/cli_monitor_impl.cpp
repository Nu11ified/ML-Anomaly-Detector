#include "cli_monitor.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <thread>
#include <chrono>

// Get metric name for display
std::string AnomalyEvent::get_metric_name(std::size_t idx) {
    static const std::string names[] = {
        "CPU Utilization",
        "RAM Usage", 
        "Disk I/O Rate",
        "Heap Free",
        "Uptime"
    };
    return (idx < N_METRICS) ? names[idx] : "Unknown";
}

// Setup display - clear screen and draw initial layout
void CLIMonitor::setup_display() {
    std::cout << CLEAR_SCREEN << CURSOR_HOME;
    draw_header();
    std::cout << "\n\n";
}

// Main display update
void CLIMonitor::update_display(const float vals[N_METRICS], 
                               const float zscores[N_METRICS],
                               unsigned sample_count,
                               bool warming_up) {
    // Move cursor to top
    std::cout << CURSOR_HOME;
    
    // Redraw all panels
    draw_header();
    std::cout << "\n";
    draw_metrics_panel(vals, zscores, sample_count, warming_up);
    std::cout << "\n";
    draw_status_bar();
    std::cout << "\n";
    draw_timeline_panel();
    
    // Flush output
    std::cout.flush();
}

// Draw the main header
void CLIMonitor::draw_header() {
    std::cout << BOLD << CYAN << "╔══════════════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                    " << YELLOW << "SYSTEM ANOMALY DETECTOR" << CYAN << " - Real-time Monitor                    ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════════════════╝" << RESET << "\n";
}

// Draw the metrics panel with real-time values
void CLIMonitor::draw_metrics_panel(const float vals[N_METRICS], 
                                   const float zscores[N_METRICS],
                                   unsigned sample_count,
                                   bool warming_up) {
    std::cout << BOLD << BLUE << "┌─ METRICS PANEL " << RESET;
    if (warming_up) {
        std::cout << YELLOW << " [WARMING UP: " << sample_count << "/" << WARMUP_SAMPLES << "]" << RESET;
    } else {
        std::cout << GREEN << " [ACTIVE MONITORING]" << RESET;
    }
    std::cout << "\n";
    
    static const std::string metric_names[] = {
        "CPU Utilization",
        "RAM Usage",
        "Disk I/O Rate", 
        "Heap Free",
        "Uptime"
    };
    
    for (std::size_t i = 0; i < N_METRICS; ++i) {
        std::string status_color = get_status_color(zscores[i]);
        std::string value_str = format_value(vals[i], i);
        std::string unit = get_metric_unit(i);
        
        // Format the metric line
        std::cout << "│ " << std::setw(15) << std::left << metric_names[i] << " ";
        std::cout << status_color << std::setw(10) << std::right << value_str << " " << unit << RESET;
        
        // Z-score indicator
        std::cout << " [z=" << std::fixed << std::setprecision(2) << zscores[i] << "] ";
        
        // Visual indicator with per-metric thresholds
        float threshold = get_metric_threshold(i);
        float hysteresis_threshold = get_hysteresis_threshold(i);
        
        if (std::fabs(zscores[i]) > threshold) {
            std::cout << RED << BLINK << "⚠ ANOMALY" << RESET;
        } else if (std::fabs(zscores[i]) > hysteresis_threshold) {
            std::cout << YELLOW << "⚠ WARNING" << RESET;
        } else {
            std::cout << GREEN << "✓ NORMAL" << RESET;
        }
        
        // Progress bar for percentage metrics
        if (i == CPU_UTIL || i == RAM_USED) {
            std::cout << " ";
            draw_progress_bar(vals[i]);
        }
        
        std::cout << "\n";
    }
    std::cout << "└─────────────────────────────────────────────────────────────────────────────\n";
}

// Draw status bar with alarm information
void CLIMonitor::draw_status_bar() {
    std::cout << BOLD << MAGENTA << "┌─ STATUS BAR" << RESET << "\n";
    
    // Alarm status
    if (alarm_active_) {
        std::cout << "│ " << RED << BLINK << "🚨 ALARM ACTIVE - " << alarm_count_ << " anomalies detected" << RESET << "\n";
    } else {
        std::cout << "│ " << GREEN << "✅ System Normal - No anomalies detected" << RESET << "\n";
    }
    
    // Timeline info
    std::cout << "│ " << CYAN << "📊 Anomaly Timeline: " << anomaly_timeline_.size() << " events recorded" << RESET << "\n";
    
    std::cout << "└─────────────────────────────────────────────────────────────────────────────\n";
}

// Draw the anomaly timeline panel
void CLIMonitor::draw_timeline_panel() {
    std::cout << BOLD << YELLOW << "┌─ ANOMALY TIMELINE" << RESET << "\n";
    
    if (anomaly_timeline_.empty()) {
        std::cout << "│ " << GREEN << "No anomalies detected yet" << RESET << "\n";
    } else {
        // Show last 10 events (most recent first)
        auto start = anomaly_timeline_.rbegin();
        auto end = anomaly_timeline_.rend();
        int count = 0;
        
        for (auto it = start; it != end && count < 10; ++it, ++count) {
            std::string timestamp = format_timestamp(it->timestamp);
            std::string status_color = get_status_color(it->z_score);
            
            std::cout << "│ " << timestamp << " ";
            std::cout << status_color << it->metric_name << RESET;
            std::cout << " = " << std::fixed << std::setprecision(2) << it->value;
            std::cout << " (z=" << it->z_score << ")\n";
        }
        
        if (anomaly_timeline_.size() > 10) {
            std::cout << "│ " << CYAN << "... and " << (anomaly_timeline_.size() - 10) << " more events" << RESET << "\n";
        }
    }
    
    std::cout << "└─────────────────────────────────────────────────────────────────────────────\n";
}

// Handle anomaly detection with alarm effects
void CLIMonitor::handle_anomaly(std::size_t metric_idx, float value, float z_score) {
    // Add to timeline
    anomaly_timeline_.emplace_back(metric_idx, value, z_score);
    
    // Trigger alarm effects
    trigger_alarm();
    
    // Print detailed alert
    std::cout << "\n" << RED << BLINK << "🚨 ANOMALY DETECTED! 🚨" << RESET << "\n";
    std::cout << "Metric: " << BOLD << AnomalyEvent::get_metric_name(metric_idx) << RESET << "\n";
    std::cout << "Value: " << std::fixed << std::setprecision(2) << value << " " << get_metric_unit(metric_idx) << "\n";
    std::cout << "Z-Score: " << std::fixed << std::setprecision(2) << z_score << "\n";
    std::cout << "Threshold: " << Z_THRESHOLD << "\n";
    std::cout << "Timestamp: " << format_timestamp(std::chrono::system_clock::now()) << "\n";
    std::cout << "\n";
}

// Trigger alarm effects
void CLIMonitor::trigger_alarm() {
    alarm_active_ = true;
    alarm_count_++;
    last_alarm_time_ = std::chrono::system_clock::now();
    
    // Audio alarm
    std::cout << ALARM_BELL;
    
    // Visual alarm effect
    for (int i = 0; i < 3; ++i) {
        std::cout << RED << REVERSE << " ALARM! ANOMALY DETECTED! " << RESET << "\r";
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        std::cout << "                                \r";
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

// Clear alarm status
void CLIMonitor::clear_alarm() {
    alarm_active_ = false;
}

// Format value for display
std::string CLIMonitor::format_value(float val, std::size_t metric_idx) {
    std::ostringstream oss;
    
    switch (metric_idx) {
        case CPU_UTIL:
        case RAM_USED:
            oss << std::fixed << std::setprecision(1) << val;
            break;
        case DISK_IO_RATE:
            // Format bytes per second with appropriate units
            if (val >= 1e9) {
                oss << std::fixed << std::setprecision(1) << (val / 1e9) << "G";
            } else if (val >= 1e6) {
                oss << std::fixed << std::setprecision(1) << (val / 1e6) << "M";
            } else if (val >= 1e3) {
                oss << std::fixed << std::setprecision(1) << (val / 1e3) << "K";
            } else {
                oss << std::fixed << std::setprecision(0) << val;
            }
            break;
        case HEAP_FREE:
            // Format heap free bytes with appropriate units
            if (val >= 1e9) {
                oss << std::fixed << std::setprecision(1) << (val / 1e9) << "G";
            } else if (val >= 1e6) {
                oss << std::fixed << std::setprecision(1) << (val / 1e6) << "M";
            } else if (val >= 1e3) {
                oss << std::fixed << std::setprecision(1) << (val / 1e3) << "K";
            } else {
                oss << std::fixed << std::setprecision(0) << val;
            }
            break;
        case UPTIME_MS:
            // Convert to hours
            oss << std::fixed << std::setprecision(1) << (val / 3600000.0f);
            break;
        default:
            oss << std::fixed << std::setprecision(2) << val;
    }
    
    return oss.str();
}

// Get status color based on z-score
std::string CLIMonitor::get_status_color(float z_score) {
    float abs_z = std::fabs(z_score);
    if (abs_z > Z_THRESHOLD) {
        return RED;
    } else if (abs_z > Z_THRESHOLD * 0.7) {
        return YELLOW;
    } else {
        return GREEN;
    }
}

// Get metric unit
std::string CLIMonitor::get_metric_unit(std::size_t metric_idx) {
    static const std::string units[] = {
        "%",     // CPU
        "%",     // RAM
        "B/s",   // Disk I/O (bytes per second)
        "B",     // Heap (bytes)
        "hrs"    // Uptime
    };
    return (metric_idx < N_METRICS) ? units[metric_idx] : "";
}

// Draw a progress bar
void CLIMonitor::draw_progress_bar(float percentage, int width) {
    int filled = static_cast<int>((percentage / 100.0f) * width);
    filled = std::max(0, std::min(filled, width));
    
    std::cout << "[";
    for (int i = 0; i < width; ++i) {
        if (i < filled) {
            std::cout << "█";
        } else {
            std::cout << "░";
        }
    }
    std::cout << "]";
}

// Format timestamp for display
std::string CLIMonitor::format_timestamp(const std::chrono::system_clock::time_point& tp) {
    auto time_t = std::chrono::system_clock::to_time_t(tp);
    
#ifdef _WIN32
    struct tm tm;
    localtime_s(&tm, &time_t);
#else
    auto tm = *std::localtime(&time_t);
#endif
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S");
    return oss.str();
}

// Show detailed timeline
void CLIMonitor::show_timeline() {
    std::cout << CLEAR_SCREEN << CURSOR_HOME;
    std::cout << BOLD << CYAN << "ANOMALY TIMELINE - Detailed View\n";
    std::cout << "══════════════════════════════════════════════════════════════════════════════\n" << RESET;
    
    if (anomaly_timeline_.empty()) {
        std::cout << GREEN << "No anomalies detected in the timeline.\n" << RESET;
        return;
    }
    
    // Show all events in chronological order
    for (const auto& event : anomaly_timeline_) {
        std::string timestamp = format_timestamp(event.timestamp);
        std::string status_color = get_status_color(event.z_score);
        
        std::cout << timestamp << " | ";
        std::cout << status_color << event.metric_name << RESET;
        std::cout << " = " << std::fixed << std::setprecision(2) << event.value;
        std::cout << " " << get_metric_unit(event.metric_index);
        std::cout << " (z=" << std::fixed << std::setprecision(2) << event.z_score << ")\n";
    }
    
    std::cout << "\n" << CYAN << "Total anomalies: " << anomaly_timeline_.size() << RESET << "\n";
}

// Interactive menu system
void CLIMonitor::show_interactive_menu() {
    std::cout << CLEAR_SCREEN << CURSOR_HOME;
    std::cout << BOLD << CYAN << "╔══════════════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                        " << YELLOW << "INTERACTIVE MENU" << CYAN << " - Anomaly Detector                    ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════════════════╝" << RESET << "\n\n";
    
    std::cout << BOLD << "Available Commands:\n" << RESET;
    std::cout << "  " << GREEN << "h" << RESET << " - Show this help menu\n";
    std::cout << "  " << GREEN << "t" << RESET << " - View detailed anomaly timeline\n";
    std::cout << "  " << GREEN << "s" << RESET << " - Show statistics\n";
    std::cout << "  " << GREEN << "c" << RESET << " - Clear timeline\n";
    std::cout << "  " << GREEN << "e" << RESET << " - Export timeline to file\n";
    std::cout << "  " << GREEN << "q" << RESET << " - Return to monitoring\n";
    std::cout << "  " << GREEN << "x" << RESET << " - Exit program\n\n";
    
    std::cout << YELLOW << "Enter command: " << RESET;
}

void CLIMonitor::handle_user_input() {
    if (!interactive_mode_) return;
    
    std::string input;
    std::getline(std::cin, input);
    
    if (input.empty()) return;
    
    char cmd = std::tolower(input[0]);
    
    switch (cmd) {
        case 'h':
            show_help();
            break;
        case 't':
            show_timeline();
            std::cout << "\n" << YELLOW << "Press Enter to continue..." << RESET;
            std::cin.get();
            break;
        case 's':
            show_statistics();
            std::cout << "\n" << YELLOW << "Press Enter to continue..." << RESET;
            std::cin.get();
            break;
        case 'c':
            clear_timeline();
            break;
        case 'e':
            {
                std::cout << YELLOW << "Enter filename to export: " << RESET;
                std::string filename;
                std::getline(std::cin, filename);
                if (!filename.empty()) {
                    export_timeline(filename);
                }
            }
            break;
        case 'q':
            set_interactive_mode(false);
            break;
        case 'x':
            std::cout << "\n" << GREEN << "Exiting...\n" << RESET;
            exit(0);
            break;
        default:
            std::cout << RED << "Unknown command. Type 'h' for help.\n" << RESET;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            break;
    }
}

void CLIMonitor::show_help() {
    std::cout << CLEAR_SCREEN << CURSOR_HOME;
    std::cout << BOLD << CYAN << "HELP - System Anomaly Detector\n";
    std::cout << "══════════════════════════════════════════════════════════════════════════════\n" << RESET;
    
    std::cout << "\n" << BOLD << "About this system:\n" << RESET;
    std::cout << "This anomaly detector monitors system metrics using EWMA (Exponentially\n";
    std::cout << "Weighted Moving Average) and z-scores to identify unusual behavior.\n\n";
    
    std::cout << BOLD << "Monitored Metrics:\n" << RESET;
    std::cout << "• CPU Utilization (%)\n";
    std::cout << "• RAM Usage (%)\n";
    std::cout << "• Disk I/O Rate (bytes/sec)\n";
    std::cout << "• Heap Free Memory (bytes)\n";
    std::cout << "• System Uptime (hours)\n\n";
    
    std::cout << BOLD << "Configuration:\n" << RESET;
    std::cout << "• EWMA Alpha: " << EWMA_ALPHA << " (smoothing factor)\n";
    std::cout << "• Z-Score Threshold: " << Z_THRESHOLD << " (anomaly detection)\n";
    std::cout << "• Hysteresis Threshold: " << HYSTERESIS_THRESHOLD << " (alert clearing)\n";
    std::cout << "• Min Quiet Time: " << MIN_QUIET_TIME_MS << "ms (between alerts)\n";
    std::cout << "• Warm-up Samples: " << WARMUP_SAMPLES << " (baseline learning)\n";
    std::cout << "• Sample Interval: " << SAMPLE_MS << "ms\n\n";
    
    std::cout << BOLD << "Alarm System:\n" << RESET;
    std::cout << "• Visual alarms with blinking indicators\n";
    std::cout << "• Audio alarms (system bell)\n";
    std::cout << "• Real-time anomaly timeline\n";
    std::cout << "• Color-coded status indicators\n\n";
    
    std::cout << YELLOW << "Press Enter to continue..." << RESET;
    std::cin.get();
}

void CLIMonitor::show_statistics() {
    std::cout << CLEAR_SCREEN << CURSOR_HOME;
    std::cout << BOLD << CYAN << "STATISTICS - System Anomaly Detector\n";
    std::cout << "══════════════════════════════════════════════════════════════════════════════\n" << RESET;
    
    std::cout << "\n" << BOLD << "Timeline Statistics:\n" << RESET;
    std::cout << "• Total Anomalies: " << anomaly_timeline_.size() << "\n";
    std::cout << "• Alarm Count: " << alarm_count_ << "\n";
    std::cout << "• Current Alarm Status: " << (alarm_active_ ? "ACTIVE" : "INACTIVE") << "\n\n";
    
    if (!anomaly_timeline_.empty()) {
        // Calculate statistics
        float max_z_score = 0.0f;
        float avg_z_score = 0.0f;
        std::vector<int> metric_counts(N_METRICS, 0);
        
        for (const auto& event : anomaly_timeline_) {
            max_z_score = std::max(max_z_score, std::fabs(event.z_score));
            avg_z_score += std::fabs(event.z_score);
            metric_counts[event.metric_index]++;
        }
        avg_z_score /= anomaly_timeline_.size();
        
        std::cout << BOLD << "Anomaly Analysis:\n" << RESET;
        std::cout << "• Maximum Z-Score: " << std::fixed << std::setprecision(2) << max_z_score << "\n";
        std::cout << "• Average Z-Score: " << std::fixed << std::setprecision(2) << avg_z_score << "\n";
        std::cout << "• Most Anomalous Metric: ";
        
        auto max_it = std::max_element(metric_counts.begin(), metric_counts.end());
        std::size_t max_idx = std::distance(metric_counts.begin(), max_it);
        std::cout << AnomalyEvent::get_metric_name(max_idx) << " (" << *max_it << " events)\n\n";
        
        std::cout << BOLD << "Anomalies by Metric:\n" << RESET;
        for (std::size_t i = 0; i < N_METRICS; ++i) {
            if (metric_counts[i] > 0) {
                std::cout << "• " << AnomalyEvent::get_metric_name(i) << ": " << metric_counts[i] << " events\n";
            }
        }
    }
}

void CLIMonitor::clear_timeline() {
    anomaly_timeline_.clear();
    alarm_count_ = 0;
    alarm_active_ = false;
    std::cout << GREEN << "Timeline cleared!\n" << RESET;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void CLIMonitor::export_timeline(const std::string& filename) {
    // This would implement file export functionality
    // For now, just show a message
    std::cout << YELLOW << "Export functionality would save timeline to: " << filename << "\n" << RESET;
    std::cout << "This feature can be implemented to save anomalies to CSV/JSON format.\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
}

// Get threshold for a specific metric
float CLIMonitor::get_metric_threshold(std::size_t metric_idx) {
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
float CLIMonitor::get_hysteresis_threshold(std::size_t metric_idx) {
    switch (metric_idx) {
        case 0: return CPU_HYSTERESIS;
        case 1: return RAM_HYSTERESIS;
        case 2: return DISK_HYSTERESIS;
        case 3: return HEAP_HYSTERESIS;
        case 4: return UPTIME_HYSTERESIS;
        default: return HYSTERESIS_THRESHOLD;
    }
} 