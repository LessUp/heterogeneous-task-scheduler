#include "hts/execution_engine.hpp"
#include <chrono>
#include <cuda_runtime.h>

namespace hts {

ExecutionEngine::ExecutionEngine(MemoryPool &memory_pool, size_t cpu_threads, size_t gpu_streams)
    : memory_pool_(memory_pool), stream_manager_(gpu_streams), cpu_thread_count_(cpu_threads) {
    // Start CPU worker threads
    cpu_workers_.reserve(cpu_threads);
    for (size_t i = 0; i < cpu_threads; ++i) {
        cpu_workers_.emplace_back(&ExecutionEngine::cpu_worker_loop, this);
    }
}

ExecutionEngine::~ExecutionEngine() {
    // Signal workers to stop
    {
        std::lock_guard<std::mutex> lock(cpu_mutex_);
        stop_flag_ = true;
    }
    cpu_cv_.notify_all();

    // Wait for CPU workers
    for (auto &worker : cpu_workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    // Wait for GPU tasks
    {
        std::lock_guard<std::mutex> lock(gpu_mutex_);
        for (auto &future : gpu_futures_) {
            if (future.valid()) {
                future.wait();
            }
        }
    }

    // Synchronize GPU
    stream_manager_.synchronize_all();
}

void ExecutionEngine::cpu_worker_loop() {
    while (true) {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(cpu_mutex_);
            cpu_cv_.wait(lock, [this]() { return stop_flag_ || !cpu_task_queue_.empty(); });

            if (stop_flag_ && cpu_task_queue_.empty()) {
                return;
            }

            task = std::move(cpu_task_queue_.front());
            cpu_task_queue_.pop();
        }

        task();
    }
}

std::future<void> ExecutionEngine::execute_task(std::shared_ptr<Task> task, DeviceType device) {
    std::promise<void> promise;
    std::future<void> future = promise.get_future();

    task->set_actual_device(device);
    task->set_state(TaskState::Running);

    if (device == DeviceType::CPU) {
        active_cpu_tasks_++;

        auto task_func = [this, task, p = std::move(promise)]() mutable {
            execute_on_cpu(task, std::move(p));
        };

        {
            std::lock_guard<std::mutex> lock(cpu_mutex_);
            cpu_task_queue_.push(std::move(task_func));
        }
        cpu_cv_.notify_one();
    } else {
        active_gpu_tasks_++;

        // Execute GPU task in a separate thread to not block
        auto gpu_future =
            std::async(std::launch::async, [this, task, p = std::move(promise)]() mutable {
                execute_on_gpu(task, std::move(p));
            });

        {
            std::lock_guard<std::mutex> lock(gpu_mutex_);
            gpu_futures_.push_back(std::move(gpu_future));
        }
    }

    return future;
}

void ExecutionEngine::execute_on_cpu(std::shared_ptr<Task> task, std::promise<void> promise) {
    auto start = std::chrono::high_resolution_clock::now();

    try {
        TaskContext &ctx = get_context(task->id());
        task->execute_cpu(ctx);

        auto end = std::chrono::high_resolution_clock::now();
        task->set_execution_time(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start));
        task->set_state(TaskState::Completed);

        promise.set_value();
    } catch (const std::exception &e) {
        task->set_state(TaskState::Failed);
        promise.set_exception(std::current_exception());
    }

    active_cpu_tasks_--;
    cpu_cv_.notify_all();
}

void ExecutionEngine::execute_on_gpu(std::shared_ptr<Task> task, std::promise<void> promise) {
    auto start = std::chrono::high_resolution_clock::now();

    cudaStream_t stream = stream_manager_.acquire_stream();

    try {
        TaskContext &ctx = get_context(task->id());
        task->execute_gpu(ctx, stream);

        // Synchronize stream to ensure completion
        cudaError_t err = cudaStreamSynchronize(stream);
        if (err != cudaSuccess) {
            throw std::runtime_error("CUDA stream sync failed: " +
                                     std::string(cudaGetErrorString(err)));
        }

        auto end = std::chrono::high_resolution_clock::now();
        task->set_execution_time(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start));
        task->set_state(TaskState::Completed);

        promise.set_value();
    } catch (const std::exception &e) {
        task->set_state(TaskState::Failed);
        promise.set_exception(std::current_exception());
    }

    stream_manager_.release_stream(stream);
    active_gpu_tasks_--;
}

TaskContext &ExecutionEngine::get_context(TaskId id) {
    std::lock_guard<std::mutex> lock(context_mutex_);

    auto it = contexts_.find(id);
    if (it == contexts_.end()) {
        auto [inserted, _] = contexts_.emplace(id, std::make_unique<TaskContext>(&memory_pool_));
        return *inserted->second;
    }
    return *it->second;
}

void ExecutionEngine::wait_all() {
    // Wait for CPU queue to drain
    {
        std::unique_lock<std::mutex> lock(cpu_mutex_);
        cpu_cv_.wait(lock, [this]() { return cpu_task_queue_.empty() && active_cpu_tasks_ == 0; });
    }

    // Wait for GPU tasks
    {
        std::lock_guard<std::mutex> lock(gpu_mutex_);
        for (auto &future : gpu_futures_) {
            if (future.valid()) {
                future.wait();
            }
        }
        gpu_futures_.clear();
    }

    // Final GPU sync
    stream_manager_.synchronize_all();
}

double ExecutionEngine::get_cpu_load() const {
    return static_cast<double>(active_cpu_tasks_.load()) / cpu_thread_count_;
}

double ExecutionEngine::get_gpu_load() const {
    return static_cast<double>(active_gpu_tasks_.load()) / stream_manager_.num_streams();
}

} // namespace hts
