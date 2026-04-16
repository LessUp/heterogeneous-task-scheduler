# RFC-005: Profiling and Performance Monitoring

[![Spec](https://img.shields.io/badge/Spec-RFC-blue)]()
[![Version](https://img.shields.io/badge/Version-1.2.0-green)]()
[![Status](https://img.shields.io/badge/Status-Implemented-brightgreen)]()

> Technical design for performance profiling, monitoring, and optimization capabilities.

---

## Overview

HTS provides comprehensive profiling capabilities to measure execution times, device utilization, memory usage, and generate timeline visualizations for performance optimization.

---

## Profiler Architecture

### Execution Stats

```cpp
struct ExecutionStats {
    std::chrono::nanoseconds total_execution_time;
    size_t tasks_completed;
    size_t tasks_failed;
    double cpu_utilization;    // 0.0 - 1.0
    double gpu_utilization;    // 0.0 - 1.0
    size_t peak_memory_bytes;
};
```

### Timeline Events

```cpp
struct TimelineEvent {
    Task::TaskId task_id;
    std::string task_name;
    std::string device;  // "CPU" or "GPU"
    std::chrono::nanoseconds start_time;
    std::chrono::nanoseconds end_time;
    std::string status;  // "completed", "failed", "cancelled"
};

struct ExecutionTimeline {
    std::vector<TimelineEvent> events;
    std::chrono::nanoseconds graph_start;
    std::chrono::nanoseconds graph_end;
    std::string to_json() const;  // Chrome tracing format
};
```

---

## Performance Metrics

| Metric | Description |
|--------|-------------|
| **Task Execution Time** | Per-task wall clock time |
| **Graph Execution Time** | Total DAG execution time |
| **CPU Utilization** | Thread pool utilization |
| **GPU Utilization** | CUDA stream utilization |
| **Memory Fragmentation** | Pool fragmentation ratio |
| **Scheduling Overhead** | Time spent in scheduling logic |

---

## Optimization Guide

### Typical Workloads

| Workload | CPU-only | HTS (CPU+GPU) | Speedup |
|----------|----------|---------------|---------|
| Image Processing | 1.0x | 3.5x | **3.5x** |
| ML Inference | 1.0x | 8.2x | **8.2x** |
| Data Pipeline | 1.0x | 2.1x | **2.1x** |

---

## Related Documents

- [Product Requirements](../product/001-heterogeneous-task-scheduler.md) (REQ-7)
- [Profiling Guide](../../docs/en/profiling.md)
