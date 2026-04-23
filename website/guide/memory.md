# Memory Pool

HTS includes a high-performance GPU memory pool that eliminates the overhead of `cudaMalloc` and `cudaFree` calls.

## The Problem with cudaMalloc/cudaFree

Standard CUDA memory allocation has significant overhead:

- **cudaMalloc**: ~50 μs per call
- **cudaFree**: ~25 μs per call
- **Synchronization**: Can block the entire GPU

For workloads with frequent small allocations, this overhead can dominate execution time.

## HTS Memory Pool Solution

HTS uses a **buddy system allocator** that:

- Allocates from pre-allocated memory pool
- O(log n) allocation time
- O(1) deallocation time
- Automatic defragmentation
- Zero synchronization overhead

## Architecture

```
┌────────────────────────────────────────────┐
│         Memory Pool (e.g., 4 GB)            │
│                                            │
│  ┌──────────────────────────────────────┐  │
│  │  Block 0: 1 GB (allocated)           │  │
│  ├──────────────────────────────────────┤  │
│  │  Block 1: 512 MB (free)              │  │
│  ├──────────────────────────────────────┤  │
│  │  Block 2: 256 MB (allocated)         │  │
│  ├──────────────────────────────────────┤  │
│  │  Block 3: 256 MB (free)              │  │
│  ├──────────────────────────────────────┤  │
│  │  ...                                  │  │
│  └──────────────────────────────────────┘  │
└────────────────────────────────────────────┘
```

## Buddy System Explained

The buddy system divides memory into blocks that are powers of 2:

### Allocation

1. Find the smallest block that fits the request
2. If block is too large, split it in half
3. Repeat until block is the right size
4. Return pointer to allocated block

### Deallocation

1. Mark block as free
2. Check if buddy (adjacent block) is also free
3. If yes, merge buddies into larger block
4. Repeat to reduce fragmentation

### Example

```
Request: 300 MB
Pool:    4 GB

Step 1: Find 512 MB block (smallest power of 2 ≥ 300 MB)
Step 2: Split 1 GB → 512 MB + 512 MB
Step 3: Allocate first 512 MB
Step 4: Return pointer

Request: 128 MB
Step 1: Use remaining 512 MB from split
Step 2: Split 512 MB → 256 MB + 256 MB
Step 3: Split 256 MB → 128 MB + 128 MB
Step 4: Allocate first 128 MB
Step 5: Return pointer
```

## Usage

### Basic Usage

Memory allocation is handled automatically by HTS when tasks request GPU memory:

```cpp
auto gpu_task = graph.add_task(DeviceType::GPU, "GPU_Work");
gpu_task->set_gpu_function([](TaskContext& ctx, cudaStream_t stream) {
    // Request memory from pool (automatic)
    void* ptr = ctx.allocate_gpu(1024 * 1024); // 1 MB
    
    // Use memory for GPU computation
    my_kernel<<<blocks, threads, 0, stream>>>(ptr);
    
    // Memory automatically returned to pool on task completion
});
```

### Pool Configuration

Configure the memory pool during scheduler initialization:

```cpp
#include <hts/memory_pool.hpp>

MemoryPoolConfig config;
config.pool_size_mb = 4096;           // 4 GB pool
config.min_block_size_kb = 4;         // Minimum 4 KB blocks
config.max_block_size_mb = 1024;      // Maximum 1 GB blocks
config.enable_defragmentation = true; // Enable auto defrag

scheduler.configure_memory_pool(config);
```

### Manual Allocation

For fine-grained control:

```cpp
#include <hts/memory_pool.hpp>

// Get memory pool instance
auto& pool = scheduler.get_memory_pool();

// Allocate memory
void* ptr = pool.allocate(1024 * 1024); // 1 MB

// Use memory...

// Free memory (returns to pool, not to OS)
pool.free(ptr);
```

## Operational Characteristics

The memory pool is designed to reduce repeated allocation churn and expose runtime statistics you can
inspect in your own workload.

### Fragmentation

HTS monitors and manages fragmentation:

```cpp
auto stats = pool.get_stats();
std::cout << "Fragmentation: " << stats.fragmentation_ratio * 100 << "%" << std::endl;
std::cout << "Total allocated: " << stats.allocated_bytes / 1024 / 1024 << " MB" << std::endl;
std::cout << "Total free: " << stats.free_bytes / 1024 / 1024 << " MB" << std::endl;
std::cout << "Largest block: " << stats.largest_free_block / 1024 / 1024 << " MB" << std::endl;
```

## Defragmentation

When fragmentation becomes high, HTS can defragment the pool:

### Automatic Defragmentation

Enabled by default, runs periodically:

```cpp
config.enable_defragmentation = true;
config.defrag_threshold = 0.3; // Trigger when 30% fragmented
```

### Manual Defragmentation

```cpp
// Trigger defragmentation
pool.defragment();

// Check if defragmentation is needed
if (pool.get_stats().fragmentation_ratio > 0.3) {
    pool.defragment();
}
```

### How Defragmentation Works

1. Pause new allocations briefly
2. Identify allocated blocks
3. Move blocks to consolidate free space
4. Update pointers (handled automatically)
5. Resume allocations

**Note:** Defragmentation may briefly pause allocation, but is optimized to minimize impact.

## Best Practices

### 1. Size the Pool Appropriately

```cpp
// Good: Size based on workload
size_t estimated_memory = num_tasks * avg_memory_per_task;
config.pool_size_mb = estimated_memory * 1.2 / 1024 / 1024; // 20% headroom
```

### 2. Use Task-Level Memory Requests

Let HTS manage memory per task rather than manual allocation:

```cpp
// Recommended: Automatic
task->set_memory_requirement(256 * 1024 * 1024); // 256 MB

// Avoid: Manual (unless you need fine control)
void* ptr = pool.allocate(256 * 1024 * 1024);
```

### 3. Monitor Fragmentation

```cpp
// Periodically check fragmentation
auto check_fragmentation = [&]() {
    auto stats = pool.get_stats();
    if (stats.fragmentation_ratio > 0.25) {
        std::cout << "High fragmentation detected: " 
                  << stats.fragmentation_ratio * 100 << "%" << std::endl;
    }
};
```

### 4. Pre-allocate for Large Tasks

For tasks that need large contiguous blocks:

```cpp
// Reserve memory before task execution
void* reserved = pool.reserve(512 * 1024 * 1024); // 512 MB
task->set_preallocated_ptr(reserved);
```

### 5. Avoid Allocation in Hot Path

Don't allocate memory in performance-critical loops:

```cpp
// Bad: Allocation in loop
for (int i = 0; i < 1000; i++) {
    void* ptr = pool.allocate(size); // 50 μs overhead each time!
    kernel<<<...>>>(ptr);
    pool.free(ptr);
}

// Good: Allocate once, reuse
void* ptr = pool.allocate(size * 1000);
for (int i = 0; i < 1000; i++) {
    kernel<<<...>>>(ptr + i * size);
}
```

## Troubleshooting

### Out of Memory Errors

If you see `HTS_ERROR_OOM`:

1. **Increase pool size**: `config.pool_size_mb = 8192;`
2. **Enable defragmentation**: `config.enable_defragmentation = true;`
3. **Reduce task memory usage**: Optimize kernels
4. **Check for memory leaks**: Ensure all allocations are freed

### High Fragmentation

If fragmentation > 30%:

1. **Trigger defragmentation**: `pool.defragment();`
2. **Increase pool size**: More headroom reduces fragmentation
3. **Use larger block sizes**: `config.min_block_size_kb = 64;`
4. **Batch allocations**: Allocate larger chunks upfront

### Performance Issues

If memory allocation is slow:

1. **Check block size configuration**: Larger min blocks can speed up searches
2. **Use thread-local pools**: Avoid contention (advanced)
3. **Profile allocation hotspots**: Use the profiler
4. **Consider arena allocation**: For many small objects

## Next Steps

- [Error Handling](/guide/error-handling) — Handling failures
- [Scheduling](/guide/scheduling) — Task scheduling policies
- [API Reference](/api/) — Complete API documentation
- [Examples](/examples/) — See memory pool in action
