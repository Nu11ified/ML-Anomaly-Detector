// src/metrics_impl.cpp
#include "metrics.hpp"

// macOS headers
#include <mach/mach.h>
#include <mach/mach_host.h>
#include <sys/sysctl.h>
#include <time.h>
#include <sys/resource.h>
#include <malloc/malloc.h>
#include <cstring>

// For CPU util tracking
static host_cpu_load_info_data_t s_prev_cpu_info;
static bool                    s_have_prev_cpu = false;

// For Disk I/O rate
static struct rusage prev_rusage;
static bool           have_prev_rusage = false;

// For rate timing
static struct timespec prev_ts;
static bool              have_prev_ts = false;

void sample_system_metrics(float out[N_METRICS]) {
  // ----- 1) UPTIME_MS -----
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
    out[UPTIME_MS] = ts.tv_sec * 1000.0f + ts.tv_nsec / 1e6f;
  } else {
    out[UPTIME_MS] = 0.0f;
  }

  // Compute delta‐time in seconds
  float dt = 0.0f;
  if (have_prev_ts) {
    dt = (ts.tv_sec - prev_ts.tv_sec)
         + (ts.tv_nsec - prev_ts.tv_nsec) / 1e9f;
  } else {
    have_prev_ts = true;  // skip rate on first sample
  }
  prev_ts = ts;

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
    } else if (!s_have_prev_cpu) {
      s_prev_cpu_info = cpu_info;
      s_have_prev_cpu = true;
      out[CPU_UTIL]    = 0.0f;
    } else {
      auto& p = s_prev_cpu_info.cpu_ticks;
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
      s_prev_cpu_info = cpu_info;
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
    if (getrusage(RUSAGE_SELF, &usage) != 0 || dt <= 0.0f || !have_prev_rusage) {
      out[DISK_IO_RATE] = 0.0f;
    } else {
      long d_in    = usage.ru_inblock  - prev_rusage.ru_inblock;
      long d_out   = usage.ru_oublock  - prev_rusage.ru_oublock;
      long d_blocks= (d_in < 0 || d_out < 0) ? 0 : (d_in + d_out);

      constexpr float BLOCK_SZ = 512.0f;  // bytes per block
      out[DISK_IO_RATE] = (d_blocks * BLOCK_SZ) / dt;
    }
    prev_rusage      = usage;
    have_prev_rusage = true;
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