// src/metrics_impl.cpp
#include "metrics.hpp"

// macOS headers
#include <mach/mach.h>
#include <mach/mach_host.h>
#include <sys/sysctl.h>
#include <time.h>
#include <cstring>

// We keep the previous CPU ticks here to compute deltas.
static host_cpu_load_info_data_t s_prev_cpu_info;
static bool                    s_have_prev_cpu = false;

void sample_system_metrics(float out[N_METRICS]) {
  // 1) CPU_UTIL
  {
    natural_t cpu_count = HOST_CPU_LOAD_INFO_COUNT;
    host_cpu_load_info_data_t cpu_info;
    kern_return_t kr = host_statistics(
      mach_host_self(),
      HOST_CPU_LOAD_INFO,
      reinterpret_cast<host_info_t>(&cpu_info),
      &cpu_count
    );
    if (kr != KERN_SUCCESS) {
      out[CPU_UTIL] = 0.0f;
    } else {
      if (!s_have_prev_cpu) {
        s_prev_cpu_info = cpu_info;
        s_have_prev_cpu = true;
        out[CPU_UTIL] = 0.0f;
      } else {
        uint64_t prev_idle = s_prev_cpu_info.cpu_ticks[CPU_STATE_IDLE];
        uint64_t prev_user = s_prev_cpu_info.cpu_ticks[CPU_STATE_USER];
        uint64_t prev_sys  = s_prev_cpu_info.cpu_ticks[CPU_STATE_SYSTEM];
        uint64_t prev_nice= s_prev_cpu_info.cpu_ticks[CPU_STATE_NICE];

        uint64_t idle = cpu_info.cpu_ticks[CPU_STATE_IDLE];
        uint64_t user = cpu_info.cpu_ticks[CPU_STATE_USER];
        uint64_t sys  = cpu_info.cpu_ticks[CPU_STATE_SYSTEM];
        uint64_t nice = cpu_info.cpu_ticks[CPU_STATE_NICE];

        uint64_t d_idle = idle - prev_idle;
        uint64_t d_user = user - prev_user;
        uint64_t d_sys  = sys  - prev_sys;
        uint64_t d_nice= nice - prev_nice;

        uint64_t busy  = d_user + d_sys + d_nice;
        uint64_t total = busy + d_idle;
        out[CPU_UTIL] = total
          ? 100.0f * (float)busy / (float)total
          : 0.0f;

        s_prev_cpu_info = cpu_info;
      }
    }
  }

  // 2) RAM_USED (%)
  {
    // Total physical memory
    uint64_t total_mem = 0;
    size_t   len       = sizeof(total_mem);
    int      mib[2]    = { CTL_HW, HW_MEMSIZE };
    sysctl(mib, 2, &total_mem, &len, nullptr, 0);

    // Free pages
    vm_size_t page_size = 0;
    host_page_size(mach_host_self(), &page_size);

    vm_statistics64_data_t vmstat;
    natural_t count = HOST_VM_INFO64_COUNT;
    kern_return_t kr = host_statistics64(
      mach_host_self(),
      HOST_VM_INFO64,
      reinterpret_cast<host_info64_t>(&vmstat),
      &count
    );

    if (kr != KERN_SUCCESS || total_mem == 0) {
      out[RAM_USED] = 0.0f;
    } else {
      uint64_t free_mem = vmstat.free_count * (uint64_t)page_size;
      uint64_t used_mem = total_mem - free_mem;
      out[RAM_USED] = 100.0f * (float)used_mem / (float)total_mem;
    }
  }

  // 3) DISK_IO_RATE (stub for now)
  out[DISK_IO_RATE] = 0.0f;

  // 4) HEAP_FREE (stub for now)
  out[HEAP_FREE] = 0.0f;

  // 5) UPTIME_MS
  {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
      out[UPTIME_MS] = ts.tv_sec * 1000.0f + ts.tv_nsec / 1e6f;
    } else {
      out[UPTIME_MS] = 0.0f;
    }
  }
}