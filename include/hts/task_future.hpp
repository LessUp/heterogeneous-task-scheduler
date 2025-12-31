#pragma once

#include "hts/types.hpp"
#include "hts/task.hpp"
#include <future>
#include <memory>
#include <any>
#include <optional>
#include <chrono>
#include <mutex>
#include <condition_variable>

namespace hts {

/// TaskFuture provides a way to get results from completed tasks
template<typename T>
class TaskFuture {
public:
    TaskFuture() = default;
    
    /// Create a future associated with a task
    explicit TaskFuture(std::shared_ptr<Task> task)
        : task_(task)
        , state_(std::make_shared<SharedState>()) {}
    
    /// Check if the future is valid
    bool valid() const { return task_ != nullptr && state_ != nullptr; }
    
    /// Check if the result is ready
    bool ready() const {
        if (!valid()) return false;
        std::lock_guard<std::mutex> lock(state_->mutex);
        return state_->has_value || state_->has_error;
    }
    
    /// Wait for the result
    void wait() const {
        if (!valid()) return;
        std::unique_lock<std::mutex> lock(state_->mutex);
        state_->cv.wait(lock, [this] { 
            return state_->has_value || state_->has_error; 
        });
    }
    
    /// Wait for the result with timeout
    template<typename Rep, typename Period>
    bool wait_for(const std::chrono::duration<Rep, Period>& timeout) const {
        if (!valid()) return false;
        std::unique_lock<std::mutex> lock(state_->mutex);
        return state_->cv.wait_for(lock, timeout, [this] { 
            return state_->has_value || state_->has_error; 
        });
    }
    
    /// Get the result (blocks until ready)
    T get() {
        wait();
        std::lock_guard<std::mutex> lock(state_->mutex);
        if (state_->has_error) {
            throw std::runtime_error(state_->error_message);
        }
        return std::any_cast<T>(state_->value);
    }
    
    /// Try to get the result without blocking
    std::optional<T> try_get() {
        if (!ready()) return std::nullopt;
        std::lock_guard<std::mutex> lock(state_->mutex);
        if (state_->has_error) {
            throw std::runtime_error(state_->error_message);
        }
        return std::any_cast<T>(state_->value);
    }
    
    /// Set the result value (called by execution engine)
    void set_value(T value) {
        if (!state_) return;
        std::lock_guard<std::mutex> lock(state_->mutex);
        state_->value = std::move(value);
        state_->has_value = true;
        state_->cv.notify_all();
    }
    
    /// Set an error (called by execution engine)
    void set_error(const std::string& message) {
        if (!state_) return;
        std::lock_guard<std::mutex> lock(state_->mutex);
        state_->error_message = message;
        state_->has_error = true;
        state_->cv.notify_all();
    }
    
    /// Get the associated task
    std::shared_ptr<Task> task() const { return task_; }

private:
    struct SharedState {
        std::mutex mutex;
        std::condition_variable cv;
        std::any value;
        std::string error_message;
        bool has_value = false;
        bool has_error = false;
    };
    
    std::shared_ptr<Task> task_;
    std::shared_ptr<SharedState> state_;
};

/// Specialization for void
template<>
class TaskFuture<void> {
public:
    TaskFuture() = default;
    
    explicit TaskFuture(std::shared_ptr<Task> task)
        : task_(task)
        , state_(std::make_shared<SharedState>()) {}
    
    bool valid() const { return task_ != nullptr && state_ != nullptr; }
    
    bool ready() const {
        if (!valid()) return false;
        std::lock_guard<std::mutex> lock(state_->mutex);
        return state_->completed || state_->has_error;
    }
    
    void wait() const {
        if (!valid()) return;
        std::unique_lock<std::mutex> lock(state_->mutex);
        state_->cv.wait(lock, [this] { 
            return state_->completed || state_->has_error; 
        });
    }
    
    template<typename Rep, typename Period>
    bool wait_for(const std::chrono::duration<Rep, Period>& timeout) const {
        if (!valid()) return false;
        std::unique_lock<std::mutex> lock(state_->mutex);
        return state_->cv.wait_for(lock, timeout, [this] { 
            return state_->completed || state_->has_error; 
        });
    }
    
    void get() {
        wait();
        std::lock_guard<std::mutex> lock(state_->mutex);
        if (state_->has_error) {
            throw std::runtime_error(state_->error_message);
        }
    }
    
    void set_value() {
        if (!state_) return;
        std::lock_guard<std::mutex> lock(state_->mutex);
        state_->completed = true;
        state_->cv.notify_all();
    }
    
    void set_error(const std::string& message) {
        if (!state_) return;
        std::lock_guard<std::mutex> lock(state_->mutex);
        state_->error_message = message;
        state_->has_error = true;
        state_->cv.notify_all();
    }
    
    std::shared_ptr<Task> task() const { return task_; }

private:
    struct SharedState {
        std::mutex mutex;
        std::condition_variable cv;
        std::string error_message;
        bool completed = false;
        bool has_error = false;
    };
    
    std::shared_ptr<Task> task_;
    std::shared_ptr<SharedState> state_;
};

} // namespace hts
