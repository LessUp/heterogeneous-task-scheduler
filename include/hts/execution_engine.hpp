#pragma once

#include "hts/memory_pool.hpp"
#include "hts/stream_manager.hpp"
#include "hts/task.hpp"
#include "hts/task_context.hpp"
#include "hts/types.hpp"
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace hts {

/// ExecutionEngine runs tasks on CPU and GPU
class ExecutionEngine {
  public:
    /// Construct engine with memory pool and thread/stream counts
    ExecutionEngine(MemoryPool &memory_pool, size_t cpu_threads = 4, size_t gpu_streams = 4);

    /// Destructor - waits for all tasks to complete
    ~ExecutionEngine();

    // Non-copyable, non-movable
    ExecutionEngine(const ExecutionEngine &) = delete;
    ExecutionEngine &operator=(const ExecutionEngine &) = delete;
    ExecutionEngine(ExecutionEngine &&) = delete;
    ExecutionEngine &operator=(ExecutionEngine &&) = delete;

    /// Execute a task on specified device
    /// @param task Task to execute
    /// @param device Device to execute on
    /// @return Future that resolves when task completes
    std::future<void> execute_task(std::shared_ptr<Task> task, DeviceType device);

    /// Wait for all submitted tasks to complete
    void wait_all();

    /// Get CPU load (0.0 to 1.0)
    double get_cpu_load() const;

    /// Get GPU load (0.0 to 1.0)
    double get_gpu_load() const;

    /// Get number of active CPU tasks
    size_t active_cpu_tasks() const { return active_cpu_tasks_.load(); }

    /// Get number of active GPU tasks
    size_t active_gpu_tasks() const { return active_gpu_tasks_.load(); }

    /// Get memory pool reference
    MemoryPool &memory_pool() { return memory_pool_; }

    /// Get stream manager reference
    StreamManager &stream_manager() { return stream_manager_; }

  private:
    MemoryPool &memory_pool_;
    StreamManager stream_manager_;

    // CPU thread pool
    std::vector<std::thread> cpu_workers_;
    std::queue<std::function<void()>> cpu_task_queue_;
    std::mutex cpu_mutex_;
    std::condition_variable cpu_cv_;
    std::atomic<bool> stop_flag_{false};
    std::atomic<size_t> active_cpu_tasks_{0};
    size_t cpu_thread_count_;

    // GPU task tracking
    std::atomic<size_t> active_gpu_tasks_{0};
    std::mutex gpu_mutex_;
    std::vector<std::future<void>> gpu_futures_;

    // Task contexts (one per task)
    std::mutex context_mutex_;
    std::unordered_map<TaskId, std::unique_ptr<TaskContext>> contexts_;

    /// CPU worker thread function
    void cpu_worker_loop();

    /// Execute task on CPU
    void execute_on_cpu(std::shared_ptr<Task> task, std::promise<void> promise);

    /// Execute task on GPU
    void execute_on_gpu(std::shared_ptr<Task> task, std::promise<void> promise);

    /// Get or create task context
    TaskContext &get_context(TaskId id);
};

} // namespace hts
