## ADDED Requirements

### Requirement: Task graph traversal SHALL not expose backing storage
The HTS task graph module SHALL provide explicit traversal and query operations for tasks, and
callers SHALL not depend on the backing container that stores graph nodes.

#### Scenario: Internal graph consumers enumerate tasks
- **WHEN** a serializer, dependency tracker, or other caller needs to traverse the graph
- **THEN** it uses explicit `TaskGraph` query/traversal APIs rather than a returned storage map

### Requirement: Scheduler runtime configuration SHALL reject invalid worker counts
The HTS scheduler SHALL fail fast when constructed with zero CPU worker threads or zero GPU streams
instead of allowing execution to proceed with undefined utilization math.

#### Scenario: Zero CPU thread count is configured
- **WHEN** a caller constructs `Scheduler` with `cpu_thread_count` set to zero
- **THEN** construction fails with a descriptive invalid-configuration error

#### Scenario: Zero GPU stream count is configured
- **WHEN** a caller constructs `Scheduler` with `gpu_stream_count` set to zero
- **THEN** construction fails with a descriptive invalid-configuration error

### Requirement: Stream manager SHALL preserve exclusive stream ownership
The HTS stream manager SHALL only accept release of streams it owns and has currently leased, and it
SHALL reject duplicate or foreign releases instead of corrupting pool state.

#### Scenario: Duplicate release is attempted
- **WHEN** the same CUDA stream is released twice without a matching acquire
- **THEN** the stream manager rejects the second release with an explicit error

#### Scenario: Foreign release is attempted
- **WHEN** a CUDA stream that was not created and leased by the stream manager is released
- **THEN** the stream manager rejects the release with an explicit error

### Requirement: Scheduler execution state SHALL recover from exceptions
The HTS scheduler SHALL reset its internal execution-state guard even when graph validation or task
execution raises an exception, so the scheduler can be safely destroyed or reused after failure.

#### Scenario: Task execution throws
- **WHEN** a scheduled task raises an exception during execution
- **THEN** the scheduler surfaces the failure and leaves its execution guard reset afterward
