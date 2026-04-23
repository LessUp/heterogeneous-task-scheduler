---
layout: home

hero:
  name: "HTS"
  text: "Heterogeneous Task Scheduler"
  tagline: C++17 DAG scheduling for mixed CPU and GPU workloads
  image:
    src: /logo.svg
    alt: HTS Logo
  actions:
    - theme: brand
      text: Get Started
      link: /guide/
    - theme: alt
      text: Browse Examples
      link: /examples/
    - theme: alt
      text: View on GitHub
      link: https://github.com/LessUp/heterogeneous-task-scheduler

features:
  - icon: ⚡
    title: DAG-first execution
    details: Build dependency-aware pipelines with TaskGraph, TaskBuilder, and scheduling policies that span CPU and GPU work.
  - icon: 🔄
    title: CPU-only fallback
    details: Validate and iterate without CUDA hardware by using the project's CPU-only build and test path.
  - icon: 🎯
    title: Scheduling and control
    details: Combine scheduling policies, retry behavior, barriers, resource limits, and event hooks in one runtime surface.
  - icon: 💾
    title: GPU runtime services
    details: Use the CUDA execution path, stream manager, and memory pool for accelerator-backed stages when available.
  - icon: 📊
    title: Profiling and inspection
    details: Export execution statistics and trace-oriented profiling data to understand scheduler behavior.
  - icon: 🛡️
    title: Source-guided adoption
    details: Start from runnable examples, public headers, and GitHub Pages guides instead of a README mirror.

head:
  - - meta
    - property: og:title
      content: HTS - Heterogeneous Task Scheduler
  - - meta
    - property: og:description
      content: C++17 DAG scheduling library for mixed CPU and GPU workloads
---

<style>
:root {
  --vp-home-hero-name-color: transparent;
  --vp-home-hero-name-background: -webkit-linear-gradient(120deg, #3EAF7C 30%, #41D1FF);
  --vp-home-hero-image-background-image: linear-gradient(-45deg, #3EAF7C 50%, #41D1FF 50%);
  --vp-home-hero-image-filter: blur(40px);
}

.VPImage.logo {
  animation: float 6s ease-in-out infinite;
}

@keyframes float {
  0%, 100% { transform: translateY(0px); }
  50% { transform: translateY(-20px); }
}

.VPFeature {
  transition: transform 0.3s ease, box-shadow 0.3s ease;
}

.VPFeature:hover {
  transform: translateY(-5px);
  box-shadow: 0 10px 40px rgba(0,0,0,0.1);
}
</style>

## Why use HTS

- Compose dependency-aware workloads with a single C++ API.
- Keep the default contributor path CPU-only while still supporting CUDA-backed execution.
- Use examples and public headers to evaluate the library without reverse-engineering the repository.

## Architecture at a glance

| Layer | Responsibilities |
|------|------------------|
| Builder API | `TaskGraph`, `TaskBuilder`, task groups, barriers, and futures |
| Scheduler | Dependency resolution, policy selection, retries, profiling, and events |
| Execution | CPU worker threads, CUDA execution path, stream management, and resource limits |
| Device services | Memory pool, CUDA helpers, CPU-only stubs, and runtime utilities |

## Quick start

```bash
git clone https://github.com/LessUp/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler
scripts/build.sh --cpu-only
scripts/test.sh
```

For editor/LSP integration, the repository provides a `cpu-only-debug` CMake preset and workspace
settings in `.vscode/`.

## Where to go next

- [Guide](/guide/) - installation, quick start, and architecture
- [API](/api/) - public API orientation
- [Examples](/examples/) - runnable usage patterns
- [GitHub repository](https://github.com/LessUp/heterogeneous-task-scheduler) - source, issues, and releases

---

MIT Licensed | Copyright © 2024-2026 [HTS Contributors](https://github.com/LessUp/heterogeneous-task-scheduler/graphs/contributors)
