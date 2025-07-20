#include "platform_metrics.hpp"
#include "metrics.hpp"

// macOS headers
#include <mach/mach.h>
#include <mach/mach_host.h>
#include <sys/sysctl.h>
#include <time.h>
#include <sys/resource.h>
#include <malloc/malloc.h>
#include <cstring>

class MacOSMetrics : public PlatformMetrics {
private:
    // For CPU util tracking
    host_cpu_load_info_data_t prev_cpu_info_;
    bool have_prev_cpu_ = false;

    // For Disk I/O rate
    struct rusage prev_rusage_;
    bool have_prev_rusage_ = false;

    // For rate timing
    struct timespec prev_ts_;
    bool have_prev_ts_ = false;

public:
    MacOSMetrics() = default;
    
    bool initialize() override {
        // Initialize timing
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
            natural_t count = HOST_CPU_LOAD_INFO_COUNT;
            host_cpu_load_info_data_t cpu_info;
            if (host_statistics(
                  mach_host_self(),
                  HOST_CPU_LOAD_INFO,
                  reinterpret_cast<host_info_t>(&cpu_info),
                  &count) != KERN_SUCCESS) {
                out[CPU_UTIL] = 0.0f;
            } else if (!have_prev_cpu_) {
                prev_cpu_info_ = cpu_info;
                have_prev_cpu_ = true;
                out[CPU_UTIL]    = 0.0f;
            } else {
                auto& p = prev_cpu_info_.cpu_ticks;
                auto& c = cpu_info.cpu_ticks;
                uint64_t d_idle = c[CPU_STATE_IDLE]   - p[CPU_STATE_IDLE];
                uint64_t d_user = c[CPU_STATE_USER]   - p[CPU_STATE_USER];
                uint64_t d_sys  = c[CPU_STATE_SYSTEM] - p[CPU_STATE_SYSTEM];
                uint64_t d_nice = c[CPU_STATE_NICE]   - p[CPU_STATE_NICE];
                uint64_t busy   = d_user + d_sys + d_nice;
                uint64_t total  = busy + d_idle;
                out[CPU_UTIL] = total
                  ? 100.0f * float(busy) / float(total)
                  : 0.0f;
                prev_cpu_info_ = cpu_info;
            }
        }

        // ----- 3) RAM_USED (%) -----
        {
            uint64_t total_mem = 0;
            size_t   len       = sizeof(total_mem);
            int      mib[2]    = { CTL_HW, HW_MEMSIZE };
            sysctl(mib, 2, &total_mem, &len, nullptr, 0);

            vm_size_t page_sz = 0;
            host_page_size(mach_host_self(), &page_sz);

            vm_statistics64_data_t vmstat;
            natural_t cnt = HOST_VM_INFO64_COUNT;
            if (host_statistics64(
                  mach_host_self(),
                  HOST_VM_INFO64,
                  reinterpret_cast<host_info64_t>(&vmstat),
                  &cnt) != KERN_SUCCESS || total_mem == 0) {
                out[RAM_USED] = 0.0f;
            } else {
                uint64_t free_mem = vmstat.free_count * uint64_t(page_sz);
                uint64_t used_mem = total_mem - free_mem;
                out[RAM_USED] = 100.0f * float(used_mem) / float(total_mem);
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
            malloc_statistics_t ms;
            malloc_zone_statistics(malloc_default_zone(), &ms);
            // free = total VM allocated − bytes currently in use
            size_t free_bytes = ms.size_allocated - ms.size_in_use;
            out[HEAP_FREE] = float(free_bytes);
        }
    }
    
    const char* get_platform_name() const override {
        return "macOS";
    }
    
    void cleanup() override {
        // No cleanup needed for macOS
    }
}; 