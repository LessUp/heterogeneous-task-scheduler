// Auto-generated CUDA stubs for CPU-only build
// This file provides CPU implementations of CUDA-dependent components
// for testing purposes only. It should NOT be used in production.

#include "hts/internal/cuda_stubs.hpp"
#include "hts/execution_engine.hpp"
#include "hts/memory_pool.hpp"
#include "hts/scheduler.hpp"
#include "hts/scheduling_policy.hpp"
#include "hts/stream_manager.hpp"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <mutex>
#include <queue>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <vector>

namespace hts {

// ============================================================================
// MemoryPool Implementation (CPU-only)
// ============================================================================

MemoryPool::MemoryPool(size_t initial_size_bytes, bool allow_growth)
    : pool_size_(initial_size_bytes), base_pool_size_(initial_size_bytes),
      allow_growth_(allow_growth), base_ptr_(nullptr) {
    base_ptr_ = std::malloc(initial_size_bytes);
    if (base_ptr_) {
        init_free_list();
    }
}

MemoryPool::~MemoryPool() {
    if (base_ptr_) {
        std::free(base_ptr_);
    }
    for (void *region : additional_regions_) {
        std::free(region);
    }
}

void *MemoryPool::allocate(size_t bytes) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (bytes == 0)
        return nullptr;

    size_t block_size = round_up_power_of_two(std::max(bytes, MIN_BLOCK_SIZE));

    auto claim_block = [&](MemoryBlock block) -> void * {
        block.in_use = true;
        allocated_blocks_[block.ptr] = block;
        stats_.used_bytes += block.size;
        stats_.allocation_count++;
        stats_.peak_bytes = std::max(stats_.peak_bytes, stats_.used_bytes);
        return block.ptr;
    };

    auto try_allocate = [&]() -> void * {
        auto exact = free_lists_.find(block_size);
        if (exact != free_lists_.end() && !exact->second.empty()) {
            MemoryBlock block = exact->second.front();
            exact->second.pop_front();
            return claim_block(block);
        }

        for (auto &[size, blocks] : free_lists_) {
            if (size > block_size && !blocks.empty()) {
                MemoryBlock block = split_block(blocks.front(), block_size);
                blocks.pop_front();
                return claim_block(block);
            }
        }

        return nullptr;
    };

    if (void *ptr = try_allocate()) {
        return ptr;
    }

    if (allow_growth_ && expand_pool(block_size)) {
        if (void *ptr = try_allocate()) {
            return ptr;
        }
    }

    return nullptr;
}

void MemoryPool::free(void *ptr) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = allocated_blocks_.find(ptr);
    if (it == allocated_blocks_.end())
        return;

    MemoryBlock block = it->second;
    block.in_use = false;
    stats_.used_bytes -= block.size;
    stats_.free_count++;
    allocated_blocks_.erase(it);
    free_lists_[block.size].push_back(block);
    coalesce_blocks();
}

MemoryStats MemoryPool::get_stats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    update_fragmentation();
    return stats_;
}

void MemoryPool::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    allocated_blocks_.clear();
    free_lists_.clear();
    if (base_ptr_) {
        init_free_list();
    }
    stats_.used_bytes = 0;
    stats_.allocation_count = 0;
    stats_.free_count = 0;
}

size_t MemoryPool::round_up_power_of_two(size_t size) {
    if (size == 0)
        return MIN_BLOCK_SIZE;
    size--;
    size |= size >> 1;
    size |= size >> 2;
    size |= size >> 4;
    size |= size >> 8;
    size |= size >> 16;
    size |= size >> 32;
    return size + 1;
}

void MemoryPool::init_free_list() {
    MemoryBlock block;
    block.ptr = base_ptr_;
    block.size = pool_size_;
    block.offset = 0;
    block.in_use = false;
    size_t block_size = round_up_power_of_two(pool_size_);
    free_lists_[block_size].push_back(block);
    stats_.total_bytes = pool_size_;
}

MemoryBlock MemoryPool::split_block(MemoryBlock &block, size_t target_size) {
    // Simplified split - just return the block for stub
    return block;
}

void MemoryPool::coalesce_blocks() {
    // Simplified coalescing for stub
}

bool MemoryPool::expand_pool(size_t min_size) {
    size_t new_size = std::max(min_size, pool_size_);
    void *new_region = std::malloc(new_size);
    if (!new_region)
        return false;

    additional_regions_.push_back(new_region);

    MemoryBlock block;
    block.ptr = new_region;
    block.size = new_size;
    block.offset = 0;
    block.in_use = false;
    free_lists_[round_up_power_of_two(new_size)].push_back(block);

    pool_size_ += new_size;
    stats_.total_bytes += new_size;
    return true;
}

void MemoryPool::update_fragmentation() const {
    if (stats_.total_bytes == 0) {
        stats_.fragmentation_ratio = 0.0;
        return;
    }
    stats_.fragmentation_ratio =
        1.0 - (static_cast<double>(stats_.used_bytes) / stats_.total_bytes);
}

// ============================================================================
// StreamManager Implementation (CPU-only with CUDA stubs)
// ============================================================================

StreamManager::StreamManager(size_t num_streams) {
    for (size_t i = 0; i < num_streams; ++i) {
        cudaStream_t stream;
        cudaStreamCreate(&stream);
        streams_.push_back(stream);
        available_streams_.push(stream);
    }
}

StreamManager::~StreamManager() {
    for (auto stream : streams_) {
        cudaStreamDestroy(stream);
    }
    for (auto event : events_) {
        cudaEventDestroy(event);
    }
}

cudaStream_t StreamManager::acquire_stream() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return !available_streams_.empty(); });
    cudaStream_t stream = available_streams_.front();
    available_streams_.pop();
    return stream;
}

void StreamManager::release_stream(cudaStream_t stream) {
    std::lock_guard<std::mutex> lock(mutex_);
    available_streams_.push(stream);
    cv_.notify_one();
}

void StreamManager::synchronize_all() {
    for (auto stream : streams_) {
        cudaStreamSynchronize(stream);
    }
}

cudaEvent_t StreamManager::create_event() {
    cudaEvent_t event;
    cudaEventCreate(&event);
    std::lock_guard<std::mutex> lock(mutex_);
    events_.insert(event);
    return event;
}

void StreamManager::destroy_event(cudaEvent_t event) {
    std::lock_guard<std::mutex> lock(mutex_);
    events_.erase(event);
    cudaEventDestroy(event);
}

void StreamManager::record_event(cudaEvent_t event, cudaStream_t stream) {
    cudaEventRecord(event, stream);
}

void StreamManager::stream_wait_event(cudaStream_t stream, cudaEvent_t event) {
    cudaStreamWaitEvent(stream, event, 0);
}

size_t StreamManager::available_streams() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return available_streams_.size();
}

// ============================================================================
// ExecutionEngine Implementation (CPU-only with CUDA stubs)
// ============================================================================

ExecutionEngine::ExecutionEngine(MemoryPool &memory_pool, size_t cpu_threads, size_t gpu_streams)
    : memory_pool_(memory_pool), stream_manager_(gpu_streams), cpu_thread_count_(cpu_threads) {
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
        if (worker.joinable())
            worker.join();
    }
    {
        std::lock_guard<std::mutex> lock(gpu_mutex_);
        for (auto &future : gpu_futures_) {
            if (future.valid())
                future.wait();
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
            if (stop_flag_ && cpu_task_queue_.empty())
                return;
            task = std::move(cpu_task_queue_.front());
            cpu_task_queue_.pop();
        }
        task();
    }
}

std::future<void> ExecutionEngine::execute_task(std::shared_ptr<Task> task, DeviceType device) {
    auto promise_ptr = std::make_shared<std::promise<void>>();
    std::future<void> future = promise_ptr->get_future();
    task->set_actual_device(device);
    task->set_state(TaskState::Running);
    if (device == DeviceType::CPU) {
        active_cpu_tasks_++;
        auto task_func = [this, task, promise_ptr]() {
            execute_on_cpu(task, std::move(*promise_ptr));
        };
        {
            std::lock_guard<std::mutex> lock(cpu_mutex_);
            cpu_task_queue_.push(std::move(task_func));
        }
        cpu_cv_.notify_one();
    } else {
        active_gpu_tasks_++;
        auto gpu_future = std::async(std::launch::async, [this, task, promise_ptr]() {
            execute_on_gpu(task, std::move(*promise_ptr));
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
    } catch (const std::exception &) {
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
        cudaStreamSynchronize(stream);
        auto end = std::chrono::high_resolution_clock::now();
        task->set_execution_time(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start));
        task->set_state(TaskState::Completed);
        promise.set_value();
    } catch (const std::exception &) {
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
    {
        std::unique_lock<std::mutex> lock(cpu_mutex_);
        cpu_cv_.wait(lock, [this]() { return cpu_task_queue_.empty() && active_cpu_tasks_ == 0; });
    }
    {
        std::lock_guard<std::mutex> lock(gpu_mutex_);
        for (auto &future : gpu_futures_) {
            if (future.valid())
                future.wait();
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

// ============================================================================
// Scheduler Implementation (CPU-only with CUDA stubs)
// ============================================================================

Scheduler::Scheduler(const SchedulerConfig &config)
    : config_(config), policy_(std::make_unique<DefaultSchedulingPolicy>()) {
    memory_pool_ =
        std::make_unique<MemoryPool>(config.memory_pool_size, config.allow_memory_growth);
    engine_ = std::make_unique<ExecutionEngine>(*memory_pool_, config.cpu_thread_count,
                                                config.gpu_stream_count);
}

Scheduler::~Scheduler() {
    if (executing_)
        engine_->wait_all();
}

void Scheduler::execute() {
    auto future = execute_async();
    future.get();
}

std::future<void> Scheduler::execute_async() {
    return std::async(std::launch::async, [this]() { execute_internal(); });
}

void Scheduler::execute_internal() {
    std::lock_guard<std::mutex> lock(execution_mutex_);
    if (!graph_.validate()) {
        throw std::runtime_error("Task graph validation failed - contains cycles");
    }
    executing_ = true;
    if (profiling_enabled_)
        profiler_.start();
    dep_manager_ = std::make_unique<DependencyManager>(graph_);
    stats_ = ExecutionStats{};
    timeline_ = ExecutionTimeline{};
    timeline_.graph_start = std::chrono::high_resolution_clock::now();
    std::unordered_map<TaskId, std::future<void>> task_futures;
    auto ready = dep_manager_->get_ready_tasks();
    for (TaskId id : ready) {
        auto task = graph_.get_task(id);
        if (task) {
            DeviceType device = select_device(*task);
            if (profiling_enabled_)
                profiler_.record_task_start(id, task->name(), device);
            task_futures[id] = engine_->execute_task(task, device);
        }
    }
    while (!task_futures.empty()) {
        std::vector<TaskId> completed;
        for (auto &[id, future] : task_futures) {
            if (future.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready) {
                auto task = graph_.get_task(id);
                try {
                    future.get();
                    on_task_completed(id);
                    if (task) {
                        auto end = std::chrono::high_resolution_clock::now();
                        auto start = end - task->execution_time();
                        record_timeline_event(id, task->actual_device(), start, end,
                                              TaskState::Completed);
                        stats_.task_times[id] = task->execution_time();
                        if (profiling_enabled_)
                            profiler_.record_task_end(id, TaskState::Completed);
                    }
                } catch (const std::exception &e) {
                    on_task_failed(id, e.what());
                    if (task) {
                        auto end = std::chrono::high_resolution_clock::now();
                        record_timeline_event(id, task->actual_device(), end, end,
                                              TaskState::Failed);
                        if (profiling_enabled_)
                            profiler_.record_task_end(id, TaskState::Failed);
                    }
                }
                completed.push_back(id);
            }
        }
        for (TaskId id : completed)
            task_futures.erase(id);
        auto newly_ready = dep_manager_->get_ready_tasks();
        for (TaskId id : newly_ready) {
            if (task_futures.find(id) == task_futures.end()) {
                auto task = graph_.get_task(id);
                if (task && task->state() == TaskState::Pending) {
                    DeviceType device = select_device(*task);
                    if (profiling_enabled_)
                        profiler_.record_task_start(id, task->name(), device);
                    task_futures[id] = engine_->execute_task(task, device);
                }
            }
        }
        if (task_futures.empty())
            break;
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    timeline_.graph_end = std::chrono::high_resolution_clock::now();
    stats_.total_time = std::chrono::duration_cast<std::chrono::nanoseconds>(timeline_.graph_end -
                                                                             timeline_.graph_start);
    stats_.cpu_utilization = engine_->get_cpu_load();
    stats_.gpu_utilization = engine_->get_gpu_load();
    stats_.memory_stats = memory_pool_->get_stats();
    if (profiling_enabled_)
        profiler_.stop();
    executing_ = false;
}

DeviceType Scheduler::select_device(const Task &task) {
    return policy_->select_device(task, engine_->get_cpu_load(), engine_->get_gpu_load());
}

void Scheduler::on_task_completed(TaskId id) {
    dep_manager_->mark_completed(id);
}

void Scheduler::on_task_failed(TaskId id, const std::string &error) {
    dep_manager_->mark_failed(id);
    if (error_callback_)
        error_callback_(id, error);
}

void Scheduler::set_error_callback(ErrorCallback callback) {
    error_callback_ = std::move(callback);
}

ExecutionStats Scheduler::get_stats() const {
    return stats_;
}

void Scheduler::record_timeline_event(TaskId id, DeviceType device,
                                      std::chrono::high_resolution_clock::time_point start,
                                      std::chrono::high_resolution_clock::time_point end,
                                      TaskState state) {
    TimelineEvent event;
    event.task_id = id;
    event.device = device;
    event.start_time = start;
    event.end_time = end;
    event.final_state = state;
    timeline_.events.push_back(event);
}

std::string Scheduler::generate_timeline_json() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6);
    auto to_ms = [&](auto tp) {
        auto duration = tp - timeline_.graph_start;
        return std::chrono::duration<double, std::milli>(duration).count();
    };
    oss << "{\n";
    oss << "  \"graph_start_ms\": 0,\n";
    oss << "  \"graph_end_ms\": " << to_ms(timeline_.graph_end) << ",\n";
    oss << "  \"events\": [\n";
    for (size_t i = 0; i < timeline_.events.size(); ++i) {
        const auto &event = timeline_.events[i];
        oss << "    {\n";
        oss << "      \"task_id\": " << event.task_id << ",\n";
        oss << "      \"device\": \"" << (event.device == DeviceType::GPU ? "GPU" : "CPU")
            << "\",\n";
        oss << "      \"start_ms\": " << to_ms(event.start_time) << ",\n";
        oss << "      \"end_ms\": " << to_ms(event.end_time) << ",\n";
        oss << "      \"state\": \"" << static_cast<int>(event.final_state) << "\"\n";
        oss << "    }";
        if (i < timeline_.events.size() - 1)
            oss << ",";
        oss << "\n";
    }
    oss << "  ]\n";
    oss << "}\n";
    return oss.str();
}

void Scheduler::set_policy(std::unique_ptr<SchedulingPolicy> policy) {
    policy_ = std::move(policy);
}

const char *Scheduler::policy_name() const {
    return policy_ ? policy_->name() : "None";
}

} // namespace hts
