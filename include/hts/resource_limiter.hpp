#pragma once

#include "hts/types.hpp"
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <unordered_map>
#include <string>
#include <memory>

namespace hts {

/// Semaphore for limiting concurrent access
class Semaphore {
public:
    explicit Semaphore(size_t count) : count_(count), max_count_(count) {}
    
    /// Acquire a permit (blocks if none available)
    void acquire() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return count_ > 0; });
        --count_;
    }
    
    /// Try to acquire a permit without blocking
    bool try_acquire() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (count_ > 0) {
            --count_;
            return true;
        }
        return false;
    }
    
    /// Try to acquire with timeout
    template<typename Rep, typename Period>
    bool try_acquire_for(const std::chrono::duration<Rep, Period>& timeout) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (cv_.wait_for(lock, timeout, [this] { return count_ > 0; })) {
            --count_;
            return true;
        }
        return false;
    }
    
    /// Release a permit
    void release() {
        std::lock_guard<std::mutex> lock(mutex_);
        ++count_;
        cv_.notify_one();
    }
    
    /// Get current available count
    size_t available() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return count_;
    }
    
    /// Get maximum count
    size_t max_count() const { return max_count_; }

private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    size_t count_;
    size_t max_count_;
};

/// RAII guard for semaphore
class SemaphoreGuard {
public:
    explicit SemaphoreGuard(Semaphore& sem) : sem_(sem), acquired_(true) {
        sem_.acquire();
    }
    
    ~SemaphoreGuard() {
        if (acquired_) {
            sem_.release();
        }
    }
    
    // Non-copyable, movable
    SemaphoreGuard(const SemaphoreGuard&) = delete;
    SemaphoreGuard& operator=(const SemaphoreGuard&) = delete;
    SemaphoreGuard(SemaphoreGuard&& other) noexcept
        : sem_(other.sem_), acquired_(other.acquired_) {
        other.acquired_ = false;
    }
    SemaphoreGuard& operator=(SemaphoreGuard&&) = delete;

private:
    Semaphore& sem_;
    bool acquired_;
};

/// ResourceLimiter manages concurrent resource usage
class ResourceLimiter {
public:
    /// Resource limit configuration
    struct Limits {
        size_t max_concurrent_cpu_tasks = 0;  // 0 = unlimited
        size_t max_concurrent_gpu_tasks = 0;  // 0 = unlimited
        size_t max_memory_bytes = 0;          // 0 = unlimited
        size_t max_gpu_memory_bytes = 0;      // 0 = unlimited
        size_t max_total_tasks = 0;           // 0 = unlimited
    };
    
    explicit ResourceLimiter(const Limits& limits = {})
        : limits_(limits)
        , cpu_semaphore_(limits.max_concurrent_cpu_tasks > 0 
                         ? limits.max_concurrent_cpu_tasks : 1000000)
        , gpu_semaphore_(limits.max_concurrent_gpu_tasks > 0 
                         ? limits.max_concurrent_gpu_tasks : 1000000) {}
    
    /// Acquire CPU task slot
    bool acquire_cpu_slot() {
        if (limits_.max_concurrent_cpu_tasks == 0) return true;
        return cpu_semaphore_.try_acquire();
    }
    
    /// Acquire CPU task slot (blocking)
    void acquire_cpu_slot_blocking() {
        if (limits_.max_concurrent_cpu_tasks == 0) return;
        cpu_semaphore_.acquire();
    }
    
    /// Release CPU task slot
    void release_cpu_slot() {
        if (limits_.max_concurrent_cpu_tasks == 0) return;
        cpu_semaphore_.release();
    }
    
    /// Acquire GPU task slot
    bool acquire_gpu_slot() {
        if (limits_.max_concurrent_gpu_tasks == 0) return true;
        return gpu_semaphore_.try_acquire();
    }
    
    /// Acquire GPU task slot (blocking)
    void acquire_gpu_slot_blocking() {
        if (limits_.max_concurrent_gpu_tasks == 0) return;
        gpu_semaphore_.acquire();
    }
    
    /// Release GPU task slot
    void release_gpu_slot() {
        if (limits_.max_concurrent_gpu_tasks == 0) return;
        gpu_semaphore_.release();
    }
    
    /// Try to allocate memory
    bool try_allocate_memory(size_t bytes) {
        if (limits_.max_memory_bytes == 0) return true;
        
        std::lock_guard<std::mutex> lock(memory_mutex_);
        if (current_memory_ + bytes > limits_.max_memory_bytes) {
            return false;
        }
        current_memory_ += bytes;
        peak_memory_ = std::max(peak_memory_, current_memory_);
        return true;
    }
    
    /// Free memory
    void free_memory(size_t bytes) {
        if (limits_.max_memory_bytes == 0) return;
        
        std::lock_guard<std::mutex> lock(memory_mutex_);
        current_memory_ = (bytes > current_memory_) ? 0 : current_memory_ - bytes;
    }
    
    /// Try to allocate GPU memory
    bool try_allocate_gpu_memory(size_t bytes) {
        if (limits_.max_gpu_memory_bytes == 0) return true;
        
        std::lock_guard<std::mutex> lock(gpu_memory_mutex_);
        if (current_gpu_memory_ + bytes > limits_.max_gpu_memory_bytes) {
            return false;
        }
        current_gpu_memory_ += bytes;
        peak_gpu_memory_ = std::max(peak_gpu_memory_, current_gpu_memory_);
        return true;
    }
    
    /// Free GPU memory
    void free_gpu_memory(size_t bytes) {
        if (limits_.max_gpu_memory_bytes == 0) return;
        
        std::lock_guard<std::mutex> lock(gpu_memory_mutex_);
        current_gpu_memory_ = (bytes > current_gpu_memory_) ? 0 : current_gpu_memory_ - bytes;
    }
    
    /// Check if can add more tasks
    bool can_add_task() const {
        if (limits_.max_total_tasks == 0) return true;
        return total_tasks_.load() < limits_.max_total_tasks;
    }
    
    /// Increment task count
    void add_task() {
        ++total_tasks_;
    }
    
    /// Decrement task count
    void remove_task() {
        --total_tasks_;
    }
    
    /// Get current statistics
    struct Stats {
        size_t current_cpu_tasks;
        size_t current_gpu_tasks;
        size_t current_memory;
        size_t peak_memory;
        size_t current_gpu_memory;
        size_t peak_gpu_memory;
        size_t total_tasks;
    };
    
    Stats get_stats() const {
        Stats stats;
        stats.current_cpu_tasks = limits_.max_concurrent_cpu_tasks > 0 
            ? limits_.max_concurrent_cpu_tasks - cpu_semaphore_.available() : 0;
        stats.current_gpu_tasks = limits_.max_concurrent_gpu_tasks > 0
            ? limits_.max_concurrent_gpu_tasks - gpu_semaphore_.available() : 0;
        
        {
            std::lock_guard<std::mutex> lock(memory_mutex_);
            stats.current_memory = current_memory_;
            stats.peak_memory = peak_memory_;
        }
        
        {
            std::lock_guard<std::mutex> lock(gpu_memory_mutex_);
            stats.current_gpu_memory = current_gpu_memory_;
            stats.peak_gpu_memory = peak_gpu_memory_;
        }
        
        stats.total_tasks = total_tasks_.load();
        return stats;
    }
    
    /// Get limits
    const Limits& limits() const { return limits_; }

private:
    Limits limits_;
    Semaphore cpu_semaphore_;
    Semaphore gpu_semaphore_;
    
    mutable std::mutex memory_mutex_;
    size_t current_memory_{0};
    size_t peak_memory_{0};
    
    mutable std::mutex gpu_memory_mutex_;
    size_t current_gpu_memory_{0};
    size_t peak_gpu_memory_{0};
    
    std::atomic<size_t> total_tasks_{0};
};

/// RAII guard for resource slots
class ResourceSlotGuard {
public:
    ResourceSlotGuard(ResourceLimiter& limiter, DeviceType device)
        : limiter_(limiter), device_(device), acquired_(false) {
        if (device == DeviceType::CPU) {
            limiter_.acquire_cpu_slot_blocking();
        } else {
            limiter_.acquire_gpu_slot_blocking();
        }
        acquired_ = true;
    }
    
    ~ResourceSlotGuard() {
        if (acquired_) {
            if (device_ == DeviceType::CPU) {
                limiter_.release_cpu_slot();
            } else {
                limiter_.release_gpu_slot();
            }
        }
    }
    
    // Non-copyable, movable
    ResourceSlotGuard(const ResourceSlotGuard&) = delete;
    ResourceSlotGuard& operator=(const ResourceSlotGuard&) = delete;
    ResourceSlotGuard(ResourceSlotGuard&& other) noexcept
        : limiter_(other.limiter_), device_(other.device_), acquired_(other.acquired_) {
        other.acquired_ = false;
    }
    ResourceSlotGuard& operator=(ResourceSlotGuard&&) = delete;

private:
    ResourceLimiter& limiter_;
    DeviceType device_;
    bool acquired_;
};

} // namespace hts
