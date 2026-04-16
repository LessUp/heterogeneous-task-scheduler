# Memory Management

> Efficient GPU memory allocation and management in HTS

---

## Table of Contents

- [Overview](#overview)
- [Buddy System Allocator](#buddy-system-allocator)
- [Memory Pool Configuration](#memory-pool-configuration)
- [Allocation Strategies](#allocation-strategies)
- [Best Practices](#best-practices)
- [Debugging Memory Issues](#debugging-memory-issues)
- [Advanced Topics](#advanced-topics)

---

## Overview

HTS uses a specialized memory pool for GPU memory allocation to avoid the overhead of frequent `cudaMalloc`/`cudaFree` calls. The pool uses a **buddy system allocator** which provides:

- **O(log n)** allocation and deallocation
- **O(1)** coalescing of adjacent free blocks
- Minimal external fragmentation

### Why Memory Pool?

| Metric | cudaMalloc | HTS Memory Pool |
|--------|------------|-----------------|
| Allocation Time | ~10-100 μs | ~1 μs |
| Deallocation Time | ~10-50 μs | ~1 μs |
| Synchronization | Required | Optional |
| Fragmentation | N/A | Bounded |

### Architecture

```
┌─────────────────────────────────────────────────────────┐
│              Memory Pool (e.g., 256 MB)                  │
├─────────────────────────────────────────────────────────┤
│  Free List by Order:                                     │
│  ┌─────────────────────────────────────────────────┐    │
│  │ Order 0: [64B] → [64B] → [64B]                  │    │
│  │ Order 1: [128B] → [128B]                        │    │
│  │ Order 2: [256B]                                 │    │
│  │ Order 3: [512B] → [512B]                        │    │
│  │ ...                                             │    │
│  └─────────────────────────────────────────────────┘    │
│                    │                                     │
│                    ▼                                     │
│  ┌─────────────────────────────────────────────────┐    │
│  │           Buddy Block Structure                  │    │
│  │  ┌───────────────────┬───────────────────┐      │    │
│  │  │    Block A        │    Block B        │      │    │
│  │  │   (buddies)       │   (buddies)       │      │    │
│  │  │  Size: 256B       │  Size: 256B       │      │    │
│  │  └───────────────────┴───────────────────┘      │    │
│  │           ↓ merges into ↓                      │    │
│  │        ┌───────────────┐                        │    │
│  │        │ Block AB      │                        │    │
│  │        │ Size: 512B    │                        │    │
│  │        └───────────────┘                        │    │
│  └─────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────┘
```

---

## Buddy System Allocator

### How It Works

1. **Initial State**: One large free block
2. **Allocation**: Split blocks until reaching appropriate size
3. **Deallocation**: Merge with buddy if also free
4. **Block Size**: Powers of 2 (64B, 128B, 256B, ...)

### Allocation Example

```
Request: 100 bytes

Step 1: Round up to power of 2 → 128B (Order 1)

Step 2: Check Order 1 free list
        Empty → need to split larger block

Step 3: Take block from Order 2 (256B)
        Split into two Order 1 blocks (128B each)
        ┌───────────────────┬───────────────────┐
        │  Allocate this    │   Add to free     │
        │  (128B)           │   list (128B)     │
        └───────────────────┴───────────────────┘

Step 4: Return pointer to allocated block
```

### Deallocation Example

```
Free: 128B block (Order 1)

Step 1: Mark block as free

Step 2: Check if buddy is free
        ┌───────────────────┬───────────────────┐
        │   This block      │   Buddy block     │
        │   (now free)      │   (already free)  │
        └───────────────────┴───────────────────┘
        
Step 3: Both free → merge into Order 2 (256B)

Step 4: Recursively check higher order
```

---

## Memory Pool Configuration

### Basic Configuration

```cpp
#include <hts/scheduler.hpp>

SchedulerConfig config;

// Total memory pool size (default: 256 MB)
config.memory_pool_size = 512 * 1024 * 1024;  // 512 MB

// Allow pool to grow if needed
config.allow_memory_growth = true;

Scheduler scheduler(config);
```

### Sizing Guidelines

| Application Type | Recommended Pool Size | Notes |
|-----------------|----------------------|-------|
| Small models | 256-512 MB | Mobile/edge devices |
| Medium models | 1-2 GB | Desktop workstations |
| Large models | 4-8 GB | Server GPUs |
| Dynamic workloads | Start small, enable growth | allow_memory_growth = true |

### Memory Limits

```cpp
// Check available memory
auto& pool = scheduler.memory_pool();
auto stats = pool.get_stats();

std::cout << "Pool size: " << stats.total_bytes / (1024*1024) << " MB\n";
std::cout << "Used: " << stats.used_bytes / (1024*1024) << " MB\n";
std::cout << "Free: " << stats.free_bytes / (1024*1024) << " MB\n";
std::cout << "Peak: " << stats.peak_bytes / (1024*1024) << " MB\n";
```

---

## Allocation Strategies

### 1. Automatic Allocation (Recommended)

TaskContext automatically manages memory:

```cpp
auto task = scheduler.graph().add_task(DeviceType::GPU);

task->set_gpu_function([](TaskContext& ctx, cudaStream_t stream) {
    // Allocate through context
    void* buffer = ctx.allocate_gpu_memory(1024 * 1024);  // 1MB
    
    // Use buffer...
    
    // Automatically freed when task completes
});
```

**Benefits**:
- Automatic lifetime management
- Automatic cleanup on exceptions
- No memory leaks

### 2. Manual Allocation (Advanced)

Direct access to memory pool:

```cpp
auto& pool = scheduler.memory_pool();

// Allocate
void* mem = pool.allocate(size);

// Use...

// Must manually deallocate
pool.deallocate(mem, size);
```

### 3. RAII Wrappers

```cpp
#include <hts/cuda_utils.hpp>

// Device memory (GPU)
hts::DeviceMemory<float> d_data(pool.allocate<float>(1024), 1024);
// Automatically freed when out of scope

// Pinned memory (CPU, page-locked)
hts::PinnedMemory<float> h_data(1024);
// Fast CPU↔GPU transfers
```

### 4. Custom Allocators

```cpp
template<typename T>
class GpuAllocator {
    MemoryPool& pool_;
    
public:
    using value_type = T;
    
    GpuAllocator(MemoryPool& pool) : pool_(pool) {}
    
    T* allocate(size_t n) {
        return static_cast<T*>(pool_.allocate(n * sizeof(T)));
    }
    
    void deallocate(T* ptr, size_t n) {
        pool_.deallocate(ptr, n * sizeof(T));
    }
};

// Usage with STL containers
GpuAllocator<float> alloc(scheduler.memory_pool());
std::vector<float, GpuAllocator<float>> gpu_vector(alloc);
```

---

## Best Practices

### 1. Size Alignment

Request sizes aligned to 64 bytes for optimal performance:

```cpp
// Good: aligns to cache line
size_t size = ((requested_size + 63) / 64) * 64;

// Better: use helper
size_t aligned_size = align_up(requested_size, 64);
```

### 2. Reuse Buffers

Avoid repeated allocations:

```cpp
// Bad: allocate in loop
for (int i = 0; i < 1000; ++i) {
    void* temp = ctx.allocate_gpu_memory(1024);
    // use temp...
}

// Good: allocate once, reuse
void* buffer = ctx.allocate_gpu_memory(1024);
for (int i = 0; i < 1000; ++i) {
    // reuse buffer...
}
```

### 3. Pool Size Planning

Estimate maximum concurrent memory usage:

```cpp
// Example: Image processing pipeline
// - Input buffer: 4K RGB = 3840*2160*3 = ~24 MB
// - Working buffer: same = 24 MB
// - Output buffer: same = 24 MB
// Total per frame: ~72 MB
// For 4 frames in flight: 288 MB

SchedulerConfig config;
config.memory_pool_size = 300 * 1024 * 1024;  // 300 MB
```

### 4. Growth Strategy

```cpp
SchedulerConfig config;
config.memory_pool_size = 256 * 1024 * 1024;   // Start at 256 MB
config.allow_memory_growth = true;              // Grow if needed
config.max_memory_pool_size = 2ULL * 1024 * 1024 * 1024;  // Max 2 GB
```

### 5. Data Transfer Optimization

```cpp
// Use pinned memory for faster transfers
hts::PinnedMemory<float> pinned(1024);

// Async copy (non-blocking)
cudaMemcpyAsync(d_data.get(), pinned.get(), size, 
                cudaMemcpyHostToDevice, stream);

// Stream-ordered allocation (CUDA 11.2+)
void* ptr;
cudaMallocAsync(&ptr, size, stream);
// Automatically freed at stream synchronize
```

---

## Debugging Memory Issues

### Enable Memory Tracking

```cpp
// In debug builds
#define HTS_DEBUG_MEMORY 1

// Pool will track allocations
auto stats = pool.get_detailed_stats();
for (const auto& alloc : stats.allocations) {
    std::cout << "Block: " << alloc.ptr 
              << " Size: " << alloc.size
              << " Task: " << alloc.task_id << "\n";
}
```

### Memory Leak Detection

```cpp
{
    Scheduler scheduler;
    
    auto before = scheduler.memory_pool().get_stats();
    
    // ... run workload ...
    
    auto after = scheduler.memory_pool().get_stats();
    
    if (after.used_bytes > before.used_bytes) {
        std::cerr << "Potential leak: " 
                  << (after.used_bytes - before.used_bytes) 
                  << " bytes\n";
    }
}
```

### Out-of-Memory Handling

```cpp
task->set_gpu_function([](TaskContext& ctx, cudaStream_t stream) {
    try {
        void* large_buffer = ctx.allocate_gpu_memory(1024 * 1024 * 100);  // 100MB
        // ... use buffer ...
    } catch (const MemoryPool::OutOfMemory& e) {
        // Handle OOM gracefully
        ctx.set_error("Out of GPU memory");
        return;
    }
});
```

---

## Advanced Topics

### Memory Pools per Stream

```cpp
// Create separate pools for different workload classes
SchedulerConfig config1;
config1.memory_pool_size = 256 * 1024 * 1024;
config1.gpu_stream_count = 2;

Scheduler config2;
config2.memory_pool_size = 1024 * 1024 * 1024;
config2.gpu_stream_count = 4;

// Use scheduler1 for small, latency-critical tasks
// Use scheduler2 for large, throughput-critical tasks
```

### Memory Pressure Handling

```cpp
class MemoryPressurePolicy : public SchedulingPolicy {
public:
    DeviceType select_device(const Task& task,
                             const SystemStatus& status) override {
        // If GPU memory is low, prefer CPU
        if (status.free_gpu_memory < 100 * 1024 * 1024) {  // < 100MB
            return DeviceType::CPU;
        }
        return DeviceType::GPU;
    }
};

scheduler.set_policy(std::make_unique<MemoryPressurePolicy>());
```

### Sub-allocator Pattern

```cpp
class WorkspaceAllocator {
    MemoryPool& pool_;
    void* base_ptr_;
    size_t offset_;
    
public:
    WorkspaceAllocator(MemoryPool& pool, size_t total_size) 
        : pool_(pool), offset_(0) {
        base_ptr_ = pool.allocate(total_size);
    }
    
    ~WorkspaceAllocator() {
        pool_.deallocate(base_ptr_, total_size_);
    }
    
    void* allocate(size_t size, size_t alignment = 64) {
        offset_ = align_up(offset_, alignment);
        void* ptr = static_cast<char*>(base_ptr_) + offset_;
        offset_ += size;
        return ptr;
    }
    
    void reset() { offset_ = 0; }
};
```

---

## Performance Comparison

### Allocation Benchmark

```cpp
constexpr size_t NUM_ALLOCS = 100000;
constexpr size_t ALLOC_SIZE = 1024;

// cudaMalloc baseline
auto start = std::chrono::high_resolution_clock::now();
for (size_t i = 0; i < NUM_ALLOCS; ++i) {
    void* ptr;
    cudaMalloc(&ptr, ALLOC_SIZE);
    cudaFree(ptr);
}
auto cuda_time = std::chrono::high_resolution_clock::now() - start;

// HTS memory pool
MemoryPool pool(256 * 1024 * 1024);
start = std::chrono::high_resolution_clock::now();
for (size_t i = 0; i < NUM_ALLOCS; ++i) {
    void* ptr = pool.allocate(ALLOC_SIZE);
    pool.deallocate(ptr, ALLOC_SIZE);
}
auto pool_time = std::chrono::high_resolution_clock::now() - start;

std::cout << "cudaMalloc: " 
          << std::chrono::duration_cast<std::chrono::microseconds>(cuda_time).count()
          << " μs\n";
std::cout << "MemoryPool: " 
          << std::chrono::duration_cast<std::chrono::microseconds>(pool_time).count()
          << " μs\n";
```

Typical Results:
- `cudaMalloc`/`cudaFree`: ~50-100 μs per operation
- HTS Memory Pool: ~1-2 μs per operation
- **Speedup: 50-100x**

---

## Further Reading

- [API Reference](api-reference.md) - MemoryPool API details
- [Architecture Overview](architecture.md) - System design
- [Error Handling](error-handling.md) - Handling OOM errors
