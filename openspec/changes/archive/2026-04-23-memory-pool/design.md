# Design: Memory Pool

[![Spec](https://img.shields.io/badge/Spec-RFC-blue)]()
[![Version](https://img.shields.io/badge/Version-1.2.0-green)]()
[![Status](https://img.shields.io/badge/Status-Implemented-brightgreen)]()

## Overview

The MemoryPool provides efficient GPU memory management by pre-allocating a large block and managing sub-allocations through a buddy system algorithm, eliminating the overhead of frequent `cudaMalloc`/`cudaFree` calls.

## Design

### Buddy System Algorithm

The buddy system divides memory into power-of-2 sized blocks. When allocating, it finds the smallest block that fits and splits larger blocks if necessary. When freeing, it coalesces adjacent buddy blocks.

### Memory Stats

```cpp
struct MemoryStats {
    size_t total_bytes;       // Total pool size
    size_t used_bytes;        // Currently allocated
    size_t peak_bytes;        // Peak usage
    size_t allocation_count;  // Number of allocations
    double fragmentation_ratio;  // 0.0 = no fragmentation, 1.0 = fully fragmented
};
```

### Allocation Strategy

| Strategy | Description |
|----------|-------------|
| **Automatic** | Pool handles allocation/free automatically |
| **Manual** | User explicitly calls allocate/free |
| **RAII** | Scoped guards for automatic cleanup |
| **Custom** | User-defined allocation policies |

### Pool Growth

- Initial size configurable (default: 256 MB)
- `allow_growth = true`: Pool expands when full
- `allow_growth = false`: Fixed size, returns nullptr when full

## Performance

| Operation | cudaMalloc | HTS Memory Pool | Speedup |
|-----------|------------|-----------------|---------|
| Allocate 1 MB | ~50 μs | ~1 μs | **50x** |
| Free 1 MB | ~25 μs | ~1 μs | **25x** |

## File Changes

| File | Action | Description |
|------|--------|-------------|
| `src/cuda/memory_pool.cu` | add | Memory pool implementation |
| `include/hts/memory_pool.hpp` | add | Public header |
| `tests/unit/test_memory_pool.cu` | add | Unit tests |

## Testing Strategy

- Unit tests for allocate/free operations
- Property-based tests for block coalescing
- Performance benchmarks vs cudaMalloc
