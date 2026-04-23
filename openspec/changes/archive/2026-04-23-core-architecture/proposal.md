# Proposal: Core Architecture

## Summary
Implement the core architecture for the Heterogeneous Task Scheduler (HTS), including task graph management, dependency tracking, scheduler, and execution engine coordination.

## Motivation
Enable developers to define and execute DAG-based task graphs with automatic CPU/GPU device assignment and dependency management.

## Scope

### In Scope
- Task definition with CPU/GPU execution targets
- TaskGraph with cycle detection and topological sort
- DependencyManager for tracking task completion
- Scheduler coordinating execution across components
- CPU thread pool and GPU stream management
- Memory pool for GPU allocations

### Out of Scope
- Multi-GPU support
- Distributed scheduling
- Persistent task storage

## Approach
Layered architecture with clear separation of concerns:
1. **Task Layer**: Task, TaskContext, TaskGraph
2. **Dependency Layer**: DependencyManager
3. **Memory Layer**: MemoryPool (buddy system)
4. **Execution Layer**: ExecutionEngine, StreamManager
5. **Scheduling Layer**: Scheduler, SchedulingPolicy

## Impact
- **Code**: New components in `src/core/` and `src/cuda/`
- **Tests**: 18+ test files covering all components
- **Docs**: API documentation in `docs/en/`

## Related
- Requirements: REQ-1 to REQ-8
- RFC: RFC-001 Core Architecture
