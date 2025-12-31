# Implementation Plan: Heterogeneous Task Scheduler

## Overview

本实现计划将异构任务图调度器分解为增量式的编码任务。每个任务构建在前一个任务之上，确保代码始终可编译和测试。实现顺序：核心数据结构 → 内存池 → 任务图 → 依赖管理 → 执行引擎 → 调度器 → 集成。

## Tasks

- [x] 1. Set up project structure and core types
  - Create CMakeLists.txt with CUDA support and Rapidcheck dependency
  - Create directory structure: src/, include/, tests/
  - Define core enums (DeviceType, TaskState) and forward declarations
  - Set up Google Test and Rapidcheck testing framework
  - _Requirements: 1.1, 8.1_

- [x] 2. Implement Task and TaskContext classes
  - [x] 2.1 Implement Task class with ID, state, device preference, and function storage
    - Implement constructor, getters, and setters
    - Support lambda functions for CPU and GPU execution
    - _Requirements: 1.1, 1.4, 5.1, 8.2_
  - [ ]* 2.2 Write property test for Task creation invariants
    - **Property 1: Task Creation Invariants**
    - **Validates: Requirements 1.1, 1.4, 5.1, 8.2**
  - [x] 2.3 Implement TaskContext class for memory allocation and data passing
    - Implement allocate_gpu_memory, free_gpu_memory
    - Implement get_input, set_output template methods
    - _Requirements: 3.2, 3.3_

- [x] 3. Implement MemoryPool
  - [x] 3.1 Implement MemoryPool with buddy system allocator
    - Implement constructor with configurable initial size
    - Implement allocate() and free() methods
    - Implement block splitting and coalescing
    - _Requirements: 3.1, 3.2, 3.3_
  - [ ]* 3.2 Write property test for Memory Pool round trip
    - **Property 6: Memory Pool Round Trip**
    - **Validates: Requirements 3.1, 3.2, 3.3, 3.5**
  - [x] 3.3 Implement MemoryStats tracking
    - Track total_bytes, used_bytes, peak_bytes, allocation_count
    - Implement fragmentation_ratio calculation
    - _Requirements: 3.5, 7.5_
  - [ ]* 3.4 Write property test for Memory Pool growth
    - **Property 7: Memory Pool Growth**
    - **Validates: Requirements 3.4**

- [x] 4. Checkpoint - Ensure all tests pass
  - Ensure all tests pass, ask the user if questions arise.

- [x] 5. Implement TaskGraph with cycle detection
  - [x] 5.1 Implement TaskGraph core structure
    - Implement add_task() with unique ID generation
    - Implement adjacency list and reverse adjacency storage
    - Implement get_successors() and get_predecessors()
    - _Requirements: 1.1, 1.2_
  - [ ]* 5.2 Write property test for Dependency Recording Round Trip
    - **Property 3: Dependency Recording Round Trip**
    - **Validates: Requirements 1.2**
  - [x] 5.3 Implement cycle detection using DFS
    - Implement has_cycle() private method
    - Implement add_dependency() with cycle check
    - Implement validate() method
    - _Requirements: 1.3, 1.5_
  - [ ]* 5.4 Write property test for Cycle Detection
    - **Property 2: Cycle Detection**
    - **Validates: Requirements 1.3, 1.5**
  - [x] 5.5 Implement topological sort and root task retrieval
    - Implement topological_sort() using Kahn's algorithm
    - Implement get_root_tasks()
    - _Requirements: 2.2_

- [x] 6. Implement DependencyManager
  - [x] 6.1 Implement DependencyManager core functionality
    - Implement constructor from TaskGraph
    - Implement mark_completed() with notification to dependents
    - Implement mark_failed() with blocking of dependents
    - Implement is_ready() and get_ready_tasks()
    - _Requirements: 2.1, 2.2, 2.5_
  - [ ]* 6.2 Write property test for Dependency Execution Order
    - **Property 4: Dependency Execution Order**
    - **Validates: Requirements 2.1, 2.2, 2.3**
  - [ ]* 6.3 Write property test for Failure Propagation
    - **Property 5: Failure Propagation**
    - **Validates: Requirements 2.5**

- [x] 7. Checkpoint - Ensure all tests pass
  - Ensure all tests pass, ask the user if questions arise.

- [ ] 8. Implement StreamManager
  - [ ] 8.1 Implement StreamManager for CUDA stream management
    - Implement constructor with configurable stream count
    - Implement acquire_stream() and release_stream()
    - Implement synchronize_all()
    - Implement create_event() and destroy_event()
    - _Requirements: 4.1, 4.3, 4.5_

- [ ] 9. Implement ExecutionEngine
  - [ ] 9.1 Implement CPU thread pool
    - Implement worker thread creation and management
    - Implement task queue with mutex and condition variable
    - Implement execute_on_cpu() method
    - _Requirements: 2.4, 4.2_
  - [ ] 9.2 Implement GPU execution with streams
    - Implement execute_on_gpu() method using StreamManager
    - Implement proper CUDA error handling
    - _Requirements: 4.1, 4.3, 6.1_
  - [ ] 9.3 Implement execute_task() and load tracking
    - Implement execute_task() returning future
    - Implement get_cpu_load() and get_gpu_load()
    - Implement wait_all()
    - _Requirements: 5.3, 5.4_
  - [ ]* 9.4 Write property test for Concurrent Execution
    - **Property 8: Concurrent Execution**
    - **Validates: Requirements 4.2, 4.3, 4.4, 5.4**

- [ ] 10. Implement Scheduler
  - [ ] 10.1 Implement Scheduler core with configuration
    - Implement constructor with SchedulerConfig
    - Initialize MemoryPool, ExecutionEngine, DependencyManager
    - Implement graph() accessor
    - _Requirements: 3.1, 8.1_
  - [ ] 10.2 Implement device selection and load balancing
    - Implement select_device() with load-based selection for Any tasks
    - _Requirements: 5.2, 5.3_
  - [ ]* 10.3 Write property test for Load Balancing
    - **Property 9: Load Balancing**
    - **Validates: Requirements 5.2, 5.3**
  - [ ] 10.4 Implement synchronous and asynchronous execution
    - Implement execute() blocking method
    - Implement execute_async() returning future
    - Implement schedule_ready_tasks() internal method
    - _Requirements: 8.3, 8.4, 8.5_
  - [ ]* 10.5 Write property test for Execution Mode Correctness
    - **Property 14: Execution Mode Correctness**
    - **Validates: Requirements 8.3, 8.4, 8.5**

- [ ] 11. Checkpoint - Ensure all tests pass
  - Ensure all tests pass, ask the user if questions arise.

- [ ] 12. Implement Error Handling
  - [ ] 12.1 Implement error callback and TaskError structure
    - Implement set_error_callback()
    - Implement TaskError with all required fields
    - Wire error propagation through ExecutionEngine
    - _Requirements: 6.1, 6.2, 6.3_
  - [ ]* 12.2 Write property test for Error Propagation
    - **Property 10: Error Propagation**
    - **Validates: Requirements 6.1, 6.2, 6.3, 6.4**
  - [ ] 12.3 Implement graceful shutdown
    - Implement destructor with in-flight task completion
    - _Requirements: 6.5_
  - [ ]* 12.4 Write property test for Graceful Shutdown
    - **Property 11: Graceful Shutdown**
    - **Validates: Requirements 6.5**

- [ ] 13. Implement Performance Monitoring
  - [ ] 13.1 Implement execution time tracking
    - Add timing instrumentation to task execution
    - Implement ExecutionStats structure
    - Implement get_stats() method
    - _Requirements: 7.1, 7.2, 7.3_
  - [ ]* 13.2 Write property test for Statistics Accuracy
    - **Property 12: Statistics Accuracy**
    - **Validates: Requirements 7.1, 7.2, 7.3, 7.5**
  - [ ] 13.3 Implement timeline generation
    - Implement TimelineEvent and ExecutionTimeline structures
    - Implement generate_timeline_json() method
    - _Requirements: 7.4_
  - [ ]* 13.4 Write property test for Timeline Generation
    - **Property 13: Timeline Generation**
    - **Validates: Requirements 7.4**

- [ ] 14. Final integration and wiring
  - [ ] 14.1 Create public header with complete API
    - Create heterogeneous_task_scheduler.hpp with all public types
    - Add documentation comments
    - _Requirements: 8.1_
  - [ ] 14.2 Write integration tests
    - Test complete task graph execution
    - Test CPU-GPU concurrent execution
    - Test error scenarios end-to-end
    - _Requirements: All_

- [ ] 15. Final checkpoint - Ensure all tests pass
  - Ensure all tests pass, ask the user if questions arise.

## Notes

- Tasks marked with `*` are optional and can be skipped for faster MVP
- Each task references specific requirements for traceability
- Checkpoints ensure incremental validation
- Property tests use Rapidcheck library as specified in design
- CUDA code requires nvcc compiler and CUDA toolkit
