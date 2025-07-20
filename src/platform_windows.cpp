#ifdef _WIN32
#include "platform_metrics.hpp"
#include "metrics.hpp"

// Windows headers
#include <windows.h>
#include <psapi.h>
#include <pdh.h>
#include <time.h>
#include <cstring>

#pragma comment(lib, "pdh.lib")

class WindowsMetrics : public PlatformMetrics {
private:
    // For CPU util tracking
    PDH_HQUERY cpu_query_ = nullptr;
    PDH_HCOUNTER cpu_counter_ = nullptr;
    bool cpu_initialized_ = false;

    // For Disk I/O rate
    PDH_HQUERY disk_query_ = nullptr;
    PDH_HCOUNTER disk_counter_ = nullptr;
    bool disk_initialized_ = false;

    // For rate timing
    LARGE_INTEGER prev_ts_;
    bool have_prev_ts_ = false;

public:
    WindowsMetrics() = default;
    
    bool initialize() override {
        // Initialize CPU performance counter
        if (PdhOpenQuery(nullptr, 0, &cpu_query_) == ERROR_SUCCESS) {
            if (PdhAddCounterA(cpu_query_, "\\Processor(_Total)\\% Processor Time", 0, &cpu_counter_) == ERROR_SUCCESS) {
                PdhCollectQueryData(cpu_query_);
                cpu_initialized_ = true;
            }
        }

        // Initialize disk performance counter
        if (PdhOpenQuery(nullptr, 0, &disk_query_) == ERROR_SUCCESS) {
            if (PdhAddCounterA(disk_query_, "\\PhysicalDisk(_Total)\\Disk Bytes/sec", 0, &disk_counter_) == ERROR_SUCCESS) {
                PdhCollectQueryData(disk_query_);
                disk_initialized_ = true;
            }
        }

        have_prev_ts_ = false;
        return true;
    }
    
    void sample_system_metrics(float out[N_METRICS]) override {
        // ----- 1) UPTIME_MS -----
        LARGE_INTEGER freq, ts;
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&ts);
        
        if (have_prev_ts_) {
            LONGLONG delta = ts.QuadPart - prev_ts_.QuadPart;
            out[UPTIME_MS] = float(delta * 1000.0 / freq.QuadPart);
        } else {
            out[UPTIME_MS] = 0.0f;
            have_prev_ts_ = true;
        }
        prev_ts_ = ts;

        // ----- 2) CPU_UTIL (%) -----
        if (cpu_initialized_) {
            PDH_FMT_COUNTERVALUE value;
            PdhCollectQueryData(cpu_query_);
            if (PdhGetFormattedCounterValue(cpu_counter_, PDH_FMT_DOUBLE, nullptr, &value) == ERROR_SUCCESS) {
                out[CPU_UTIL] = float(value.doubleValue);
            } else {
                out[CPU_UTIL] = 0.0f;
            }
        } else {
            out[CPU_UTIL] = 0.0f;
        }

        // ----- 3) RAM_USED (%) -----
        {
            MEMORYSTATUSEX memInfo;
            memInfo.dwLength = sizeof(MEMORYSTATUSEX);
            if (GlobalMemoryStatusEx(&memInfo)) {
                DWORDLONG total_mem = memInfo.ullTotalPhys;
                DWORDLONG free_mem = memInfo.ullAvailPhys;
                DWORDLONG used_mem = total_mem - free_mem;
                
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
        if (disk_initialized_) {
            PDH_FMT_COUNTERVALUE value;
            PdhCollectQueryData(disk_query_);
            if (PdhGetFormattedCounterValue(disk_counter_, PDH_FMT_DOUBLE, nullptr, &value) == ERROR_SUCCESS) {
                out[DISK_IO_RATE] = float(value.doubleValue);
            } else {
                out[DISK_IO_RATE] = 0.0f;
            }
        } else {
            out[DISK_IO_RATE] = 0.0f;
        }

        // ----- 5) HEAP_FREE (bytes) -----
        {
            PROCESS_MEMORY_COUNTERS_EX pmc;
            if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
                // Working set size as an approximation of heap usage
                SIZE_T working_set = pmc.WorkingSetSize;
                out[HEAP_FREE] = float(working_set * 0.3f);  // Estimate 30% as free
            } else {
                out[HEAP_FREE] = 0.0f;
            }
        }
    }
    
    const char* get_platform_name() const override {
        return "Windows";
    }
    
    void cleanup() override {
        if (cpu_query_) {
            PdhCloseQuery(cpu_query_);
        }
        if (disk_query_) {
            PdhCloseQuery(disk_query_);
        }
    }
};
#else
// Stub implementation for non-Windows platforms
#include "platform_metrics.hpp"
#include "metrics.hpp"

class WindowsMetrics : public PlatformMetrics {
public:
    bool initialize() override { return false; }
    void sample_system_metrics(float out[N_METRICS]) override {
        for (int i = 0; i < N_METRICS; ++i) out[i] = 0.0f;
    }
    const char* get_platform_name() const override { return "Windows (stub)"; }
    void cleanup() override {}
};
#endif 