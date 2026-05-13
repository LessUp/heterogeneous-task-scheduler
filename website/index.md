---
layout: home
---

<div class="home-header">
  <div class="home-header-left">
    <div class="home-logo">HTS</div>
    <div>
      <span class="home-title">Heterogeneous Task Scheduler</span>
      <span class="home-subtitle">C++17 DAG Scheduling for CPU/GPU</span>
    </div>
  </div>
  <div class="home-nav">
    <a href="./guide/">Guide</a>
    <a href="./examples/">Benchmarks</a>
    <a href="https://github.com/LessUp/heterogeneous-task-scheduler">GitHub</a>
    <a href="../zh/">中文</a>
  </div>
</div>

<div class="home-intro-row">
  <div class="home-intro">
    HTS is a C++17 library for building dependency-aware task pipelines that seamlessly span CPU and GPU execution. Design your workflow once, run it everywhere with automatic scheduling policies and resource management.
  </div>
  <div class="home-stats">
    <span><strong>CPU+GPU</strong> heterogeneous</span>
    <span><strong>DAG-first</strong> execution</span>
    <span><strong>C++17</strong> native</span>
  </div>
</div>

## Features

<div class="feature-map">
  <div class="feature-card feature-card-cpu">
    <div class="feature-card-title">⚡ DAG-First Execution</div>
    <div class="feature-card-desc">
      Build dependency-aware pipelines with TaskGraph and TaskBuilder. Automatic topological ordering ensures correct execution sequence.
    </div>
    <div class="feature-tags">
      <a href="./guide/architecture" class="feature-tag">Architecture</a>
      <a href="./api/" class="feature-tag">API</a>
    </div>
  </div>

  <div class="feature-card feature-card-gpu">
    <div class="feature-card-title">🔄 Heterogeneous Computing</div>
    <div class="feature-card-desc">
      Seamlessly mix CPU and GPU tasks in the same graph. CUDA streams and memory pools managed automatically when available.
    </div>
    <div class="feature-tags">
      <a href="./guide/gpu-support" class="feature-tag">GPU Support</a>
      <a href="./examples/" class="feature-tag">Examples</a>
    </div>
  </div>

  <div class="feature-card">
    <div class="feature-card-title">🎯 Scheduling Policies</div>
    <div class="feature-card-desc">
      Control execution with priority queues, resource limits, barriers, and retry policies. Event hooks enable custom monitoring.
    </div>
    <div class="feature-tags">
      <a href="./guide/scheduling" class="feature-tag">Policies</a>
      <a href="./api/scheduler" class="feature-tag">Scheduler API</a>
    </div>
  </div>

  <div class="feature-card">
    <div class="feature-card-title">💾 Memory Pool</div>
    <div class="feature-card-desc">
      GPU memory pooling reduces allocation overhead. Automatic management for temporary buffers and staging areas.
    </div>
    <div class="feature-tags">
      <a href="./guide/memory" class="feature-tag">Memory</a>
      <a href="./api/memory" class="feature-tag">Pool API</a>
    </div>
  </div>

  <div class="feature-card">
    <div class="feature-card-title">📊 Profiling</div>
    <div class="feature-card-desc">
      Export execution statistics and trace data to understand scheduler behavior. Identify bottlenecks in your pipeline.
    </div>
    <div class="feature-tags">
      <a href="./guide/profiling" class="feature-tag">Profiling</a>
      <a href="./examples/profiling" class="feature-tag">Examples</a>
    </div>
  </div>

  <div class="feature-card">
    <div class="feature-card-title">🛡️ Error Handling</div>
    <div class="feature-card-desc">
      Configurable retry policies and error callbacks. Task isolation prevents cascading failures across your graph.
    </div>
    <div class="feature-tags">
      <a href="./guide/error-handling" class="feature-tag">Error Handling</a>
      <a href="./api/error" class="feature-tag">Error API</a>
    </div>
  </div>
</div>

<div class="quick-start">
  <div class="quick-start-title">Quick Start</div>
  <div class="quick-start-content">
    <div class="command-block">
      <code>git clone https://github.com/LessUp/heterogeneous-task-scheduler.git</code>
    </div>
    <div class="command-block">
      <code>cd heterogeneous-task-scheduler && scripts/build.sh --cpu-only</code>
    </div>
    CPU-only build for development. Add CUDA support with <code>-DHTS_ENABLE_CUDA=ON</code>.
  </div>
</div>
