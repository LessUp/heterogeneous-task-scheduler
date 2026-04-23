# Proposal: Memory Pool

## Summary
Implement a GPU memory pool using buddy system allocator to eliminate the overhead of frequent cudaMalloc/cudaFree calls.

## Motivation
Direct cudaMalloc/cudaFree calls are expensive (~50μs per allocation). A memory pool reduces this to ~1μs by pre-allocating and managing memory internally.

## Scope

### In Scope
- Buddy system allocator
- Block splitting and coalescing
- Pool growth when full
- Memory statistics tracking
- RAII-style memory guards

### Out of Scope
- Multi-GPU memory pools
- Unified memory support
- Persistent memory across executions

## Approach
Pre-allocate a large GPU memory block (default 256MB) and manage sub-allocations using buddy system algorithm. Split blocks when needed, coalesce when freed.

## Impact
- **Code**: `src/cuda/memory_pool.cu`, `include/hts/memory_pool.hpp`
- **Tests**: `test_memory_pool.cu`
- **Docs**: Memory management guide

## Related
- Requirements: REQ-3 (Memory Pool Management)
- RFC: RFC-002 Memory Pool Design
