---
layout: home
hero:
  name: HTS
  text: Heterogeneous Task Scheduler
  tagline: C++17 DAG Scheduling for CPU/GPU
  actions:
    - theme: brand
      text: Get Started
      link: /en/guide/
    - theme: alt
      text: Architecture
      link: /en/guide/architecture
features:
  - icon: ⚡
    title: DAG-First Execution
    details: Build dependency-aware pipelines with TaskGraph and TaskBuilder. Automatic topological ordering ensures correct execution sequence.
    link: /en/guide/architecture
  - icon: 🖥️
    title: Heterogeneous Computing
    details: Seamlessly mix CPU and GPU tasks in the same graph. CUDA streams and memory pools managed automatically when available.
    link: /en/guide/scheduling
  - icon: 🧩
    title: Memory Pool
    details: GPU memory pooling reduces allocation overhead. Buddy system allocator with automatic defragmentation.
    link: /en/guide/memory
---

## Quick Start

::: code-group
```bash [Clone]
git clone https://github.com/AICL-Lab/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler
```

```bash [Build (CPU-only)]
scripts/build.sh --cpu-only
```

```bash [Build (with CUDA)]
scripts/build.sh -DHTS_ENABLE_CUDA=ON
```
:::

## Architecture Overview

```mermaid
flowchart TB
    subgraph Builder["Builder Layer"]
        TG[TaskGraph]
        TB[TaskBuilder]
    end

    subgraph Scheduler["Scheduler Layer"]
        SCH[Scheduler]
        POL[SchedulingPolicy]
        RET[RetryPolicy]
    end

    subgraph Execution["Execution Layer"]
        CPU[CPU Thread Pool]
        GPU[CUDA Streams]
        MP[MemoryPool]
    end

    App[User Application] --> TG
    App --> TB
    TG --> SCH
    TB --> TG
    SCH --> POL
    SCH --> RET
    SCH --> CPU
    SCH --> GPU
    GPU --> MP
```

## Key Metrics

| Feature | Description |
|---------|-------------|
| **C++17 Native** | Modern C++ with zero-overhead abstractions |
| **DAG-First** | Dependency-aware task scheduling |
| **CPU + GPU** | Heterogeneous execution support |
| **Memory Pool** | Buddy allocator for GPU memory |
| **Profiling** | Built-in performance monitoring |
