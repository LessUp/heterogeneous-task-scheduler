# Introduction

Welcome to **HTS (Heterogeneous Task Scheduler)** — a high-performance C++ framework for scheduling and executing task DAGs across CPU and GPU devices.

## What is HTS?

HTS enables efficient parallel execution of complex task dependencies across heterogeneous computing environments. It provides:

- **DAG-based task management** with automatic dependency resolution
- **Intelligent scheduling** that optimizes CPU/GPU utilization
- **High-performance memory pooling** for GPU allocations
- **Comprehensive observability** with built-in profiling

## Core Architecture

```
User Application
       ↓
  TaskGraph Builder API
       ↓
    Scheduler
       ↓
Execution Engine (CPU Threads + GPU Streams)
       ↓
   Memory Pool
```

## Next Steps

- [Installation](/guide/installation) — Set up HTS in your project
- [Quick Start](/guide/quickstart) — Build your first DAG
