#ifdef __linux__
#include "platform_metrics.hpp"
#include "metrics.hpp"

// Linux headers
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <sys/resource.h>
#include <time.h>
#include <cstring>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <string>

class LinuxMetrics : public PlatformMetrics {
private:
    // For CPU util tracking
    unsigned long long prev_total_ = 0;
    unsigned long long prev_idle_ = 0;
    bool have_prev_cpu_ = false;

    // For Disk I/O rate
    struct rusage prev_rusage_;
    bool have_prev_rusage_ = false;

    // For rate timing
    struct timespec prev_ts_;
    bool have_prev_ts_ = false;

public:
    LinuxMetrics() = default;
    
    bool initialize() override {
        have_prev_ts_ = false;
        have_prev_cpu_ = false;
        have_prev_rusage_ = false;
        return true;
    }
    
    void sample_system_metrics(float out[N_METRICS]) override {
        // ----- 1) UPTIME_MS -----
        struct timespec ts;
        if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
            out[UPTIME_MS] = ts.tv_sec * 1000.0f + ts.tv_nsec / 1e6f;
        } else {
            out[UPTIME_MS] = 0.0f;
        }

        // Compute delta‐time in seconds
        float dt = 0.0f;
        if (have_prev_ts_) {
            dt = (ts.tv_sec - prev_ts_.tv_sec)
                 + (ts.tv_nsec - prev_ts_.tv_nsec) / 1e9f;
        } else {
            have_prev_ts_ = true;  // skip rate on first sample
        }
        prev_ts_ = ts;

        // ----- 2) CPU_UTIL (%) -----
        {
            std::ifstream file("/proc/stat");
            if (file.is_open()) {
                std::string line;
                if (std::getline(file, line)) {
                    std::istringstream iss(line);
                    std::string cpu;
                    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
                    
                    iss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
                    
                    unsigned long long total = user + nice + system + idle + iowait + irq + softirq + steal;
                    
                    if (!have_prev_cpu_) {
                        prev_total_ = total;
                        prev_idle_ = idle;
                        have_prev_cpu_ = true;
                        out[CPU_UTIL] = 0.0f;
                    } else {
                        unsigned long long d_total = total - prev_total_;
                        unsigned long long d_idle = idle - prev_idle_;
                        
                        if (d_total > 0) {
                            out[CPU_UTIL] = 100.0f * (1.0f - float(d_idle) / float(d_total));
                        } else {
                            out[CPU_UTIL] = 0.0f;
                        }
                        
                        prev_total_ = total;
                        prev_idle_ = idle;
                    }
                }
            } else {
                out[CPU_UTIL] = 0.0f;
            }
        }

        // ----- 3) RAM_USED (%) -----
        {
            struct sysinfo si;
            if (sysinfo(&si) == 0) {
                unsigned long total_mem = si.totalram * si.mem_unit;
                unsigned long free_mem = si.freeram * si.mem_unit;
                unsigned long used_mem = total_mem - free_mem;
                
                if (total_mem > 0) {
                    out[RAM_USED] = 100.0f * float(used_mem) / float(total_mem);
                } else {
                    out[RAM_USED] = 0.0f;
                }
            } else {
                out[RAM_USED] = 0.0f;
            }
        }

        // ----- 4) DISK_IO_RATE (bytes/sec) -----
        {
            struct rusage usage;
            if (getrusage(RUSAGE_SELF, &usage) != 0 || dt <= 0.0f || !have_prev_rusage_) {
                out[DISK_IO_RATE] = 0.0f;
            } else {
                long d_in    = usage.ru_inblock  - prev_rusage_.ru_inblock;
                long d_out   = usage.ru_oublock  - prev_rusage_.ru_oublock;
                long d_blocks= (d_in < 0 || d_out < 0) ? 0 : (d_in + d_out);

                constexpr float BLOCK_SZ = 512.0f;  // bytes per block
                out[DISK_IO_RATE] = (d_blocks * BLOCK_SZ) / dt;
            }
            prev_rusage_      = usage;
            have_prev_rusage_ = true;
        }

        // ----- 5) HEAP_FREE (bytes) -----
        {
            // For Linux, we'll use process memory info from /proc/self/status
            std::ifstream file("/proc/self/status");
            if (file.is_open()) {
                std::string line;
                unsigned long vm_rss = 0;  // Resident Set Size in KB
                
                while (std::getline(file, line)) {
                    if (line.substr(0, 6) == "VmRSS:") {
                        std::istringstream iss(line.substr(6));
                        iss >> vm_rss;
                        break;
                    }
                }
                
                // Convert KB to bytes and estimate heap free
                // This is a rough estimate since Linux doesn't have a direct heap free metric
                unsigned long total_heap = vm_rss * 1024;  // Convert KB to bytes
                out[HEAP_FREE] = float(total_heap * 0.3f);  // Estimate 30% as free
            } else {
                out[HEAP_FREE] = 0.0f;
            }
        }
    }
    
    const char* get_platform_name() const override {
        return "Linux";
    }
    
    void cleanup() override {
        // No cleanup needed for Linux
    }
};
#else
// Stub implementation for non-Linux platforms
#include "platform_metrics.hpp"
#include "metrics.hpp"

class LinuxMetrics : public PlatformMetrics {
public:
    bool initialize() override { return false; }
    void sample_system_metrics(float out[N_METRICS]) override {
        for (int i = 0; i < N_METRICS; ++i) out[i] = 0.0f;
    }
    const char* get_platform_name() const override { return "Linux (stub)"; }
    void cleanup() override {}
};
#endif 