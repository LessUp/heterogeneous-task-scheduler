#include "hts/execution_engine.hpp"

#ifdef HTS_CPU_ONLY
#include "hts/internal/cuda_stubs.hpp"
#else
#include <cuda_runtime.h>
#endif

#include <chrono>
#include <stdexcept>

namespace hts {

ExecutionEngine::ExecutionEngine(MemoryPool &memory_pool, size_t cpu_threads, size_t gpu_streams)
    : memory_pool_(memory_pool), stream_manager_(gpu_streams), cpu_thread_count_(cpu_threads) {
    if (cpu_threads == 0) {
        throw std::invalid_argument("ExecutionEngine requires at least one CPU worker thread");
    }
    cpu_workers_.reserve(cpu_threads);
    for (size_t i = 0; i < cpu_threads; ++i) {
        cpu_workers_.emplace_back(&ExecutionEngine::cpu_worker_loop, this);
    }
}

ExecutionEngine::~ExecutionEngine() {
    {
        std::lock_guard<std::mutex> lock(cpu_mutex_);
        stop_flag_ = true;
    }
    cpu_cv_.notify_all();
    for (auto &worker : cpu_workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    {
        std::lock_guard<std::mutex> lock(gpu_mutex_);
        for (auto &future : gpu_futures_) {
            if (future.valid()) {
                future.wait();
            }
        }
    }
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
    auto promise = std::make_shared<std::promise<void>>();
    auto future = promise->get_future();

    task->mark_running(device);

    if (device == DeviceType::CPU) {
        active_cpu_tasks_++;

        auto task_func = [this, task, promise]() { execute_on_cpu(task, promise); };

        {
            std::lock_guard<std::mutex> lock(cpu_mutex_);
            cpu_task_queue_.push(std::move(task_func));
        }
        cpu_cv_.notify_one();
    } else {
        active_gpu_tasks_++;

        auto gpu_future = std::async(std::launch::async,
                                     [this, task, promise]() { execute_on_gpu(task, promise); });

        {
            std::lock_guard<std::mutex> lock(gpu_mutex_);
            gpu_futures_.push_back(std::move(gpu_future));
        }
    }

    return future;
}

void ExecutionEngine::execute_on_cpu(std::shared_ptr<Task> task,
                                     const std::shared_ptr<std::promise<void>> &promise) {
    auto start = std::chrono::high_resolution_clock::now();

    try {
        TaskContext &ctx = get_context(task->id());
        task->execute_cpu(ctx);

        auto end = std::chrono::high_resolution_clock::now();
        task->mark_completed(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start));

        promise->set_value();
    } catch (...) {
        task->mark_failed();
        promise->set_exception(std::current_exception());
    }

    active_cpu_tasks_--;
    cpu_cv_.notify_all();
}

void ExecutionEngine::execute_on_gpu(std::shared_ptr<Task> task,
                                     const std::shared_ptr<std::promise<void>> &promise) {
    auto start = std::chrono::high_resolution_clock::now();
    cudaStream_t stream = stream_manager_.acquire_stream();

    try {
        TaskContext &ctx = get_context(task->id());
        task->execute_gpu(ctx, stream);

        cudaError_t err = cudaStreamSynchronize(stream);
        if (err != cudaSuccess) {
            throw std::runtime_error("CUDA stream sync failed: " +
                                     std::string(cudaGetErrorString(err)));
        }

        auto end = std::chrono::high_resolution_clock::now();
        task->mark_completed(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start));

        promise->set_value();
    } catch (...) {
        task->mark_failed();
        promise->set_exception(std::current_exception());
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
    {
        std::unique_lock<std::mutex> lock(cpu_mutex_);
        cpu_cv_.wait(lock, [this]() { return cpu_task_queue_.empty() && active_cpu_tasks_ == 0; });
    }
    {
        std::lock_guard<std::mutex> lock(gpu_mutex_);
        for (auto &future : gpu_futures_) {
            if (future.valid()) {
                future.wait();
            }
        }
        gpu_futures_.clear();
    }
    stream_manager_.synchronize_all();
}

double ExecutionEngine::get_cpu_load() const {
    return static_cast<double>(active_cpu_tasks_.load()) / cpu_thread_count_;
}

double ExecutionEngine::get_gpu_load() const {
    return static_cast<double>(active_gpu_tasks_.load()) / stream_manager_.num_streams();
}

} // namespace hts
