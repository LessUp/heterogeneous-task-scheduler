#pragma once

#include "hts/dependency_manager.hpp"
#include "hts/execution_engine.hpp"
#include "hts/memory_pool.hpp"
#include "hts/profiler.hpp"
#include "hts/scheduling_policy.hpp"
#include "hts/task_graph.hpp"
#include "hts/types.hpp"
#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <string>

namespace hts {

/// Scheduler coordinates task graph execution across CPU and GPU
class Scheduler {
  public:
    using ErrorCallback = std::function<void(TaskId, const std::string &)>;

    /// Construct scheduler with configuration
    explicit Scheduler(const SchedulerConfig &config = {});

    /// Destructor - ensures graceful shutdown
    ~Scheduler();

    // Non-copyable, non-movable
    Scheduler(const Scheduler &) = delete;
    Scheduler &operator=(const Scheduler &) = delete;
    Scheduler(Scheduler &&) = delete;
    Scheduler &operator=(Scheduler &&) = delete;

    /// Get task graph for building
    TaskGraph &graph() { return graph_; }
    const TaskGraph &graph() const { return graph_; }

    /// Execute graph synchronously (blocks until complete)
    void execute();

    /// Execute graph asynchronously
    /// @return Future that resolves when execution completes
    std::future<void> execute_async();

    /// Set error callback
    void set_error_callback(ErrorCallback callback);

    /// Get execution statistics
    ExecutionStats get_stats() const;

    /// Generate timeline JSON for visualization
    std::string generate_timeline_json() const;

    /// Get memory pool
    MemoryPool &memory_pool() { return *memory_pool_; }

    /// Get configuration
    const SchedulerConfig &config() const { return config_; }

    /// Set scheduling policy
    void set_policy(std::unique_ptr<SchedulingPolicy> policy);

    /// Get current policy name
    const char *policy_name() const;

    /// Get profiler
    Profiler &profiler() { return profiler_; }
    const Profiler &profiler() const { return profiler_; }

    /// Enable/disable profiling
    void set_profiling(bool enabled) { profiling_enabled_ = enabled; }
    bool profiling_enabled() const { return profiling_enabled_; }

  private:
    SchedulerConfig config_;
    TaskGraph graph_;
    std::unique_ptr<MemoryPool> memory_pool_;
    std::unique_ptr<ExecutionEngine> engine_;
    std::unique_ptr<DependencyManager> dep_manager_;

    ErrorCallback error_callback_;
    mutable ExecutionStats stats_;
    mutable ExecutionTimeline timeline_;

    std::unique_ptr<SchedulingPolicy> policy_;
    Profiler profiler_;
    bool profiling_enabled_ = false;

    std::mutex execution_mutex_;
    std::atomic<bool> executing_{false};

    /// Internal execution implementation
    void execute_internal();

    /// Select device for task execution
    DeviceType select_device(const Task &task);

    /// Handle task completion
    void on_task_completed(TaskId id);

    /// Handle task failure
    void on_task_failed(TaskId id, const std::string &error);

    /// Record timeline event
    void record_timeline_event(TaskId id, DeviceType device,
                               std::chrono::high_resolution_clock::time_point start,
                               std::chrono::high_resolution_clock::time_point end, TaskState state);
};

} // namespace hts
