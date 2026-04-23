#pragma once

#include "hts/types.hpp"
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace hts {

class MemoryPool;

/// TaskContext provides memory allocation and data passing for task execution
class TaskContext {
  public:
    /// Construct context with optional memory pool
    explicit TaskContext(MemoryPool *memory_pool = nullptr);

    // Non-copyable
    TaskContext(const TaskContext &) = delete;
    TaskContext &operator=(const TaskContext &) = delete;

    /// Allocate GPU memory from pool
    void *allocate_gpu_memory(size_t bytes);

    /// Free GPU memory back to pool
    void free_gpu_memory(void *ptr);

    /// Get input data from upstream task
    template <typename T> T *get_input(const std::string &name) {
        std::lock_guard<std::mutex> lock(data_mutex_);
        auto it = inputs_.find(name);
        if (it == inputs_.end()) {
            throw std::runtime_error("Input '" + name + "' not found");
        }
        return static_cast<T *>(it->second.ptr);
    }

    /// Set output data for downstream tasks
    template <typename T> void set_output(const std::string &name, T *data, size_t count) {
        std::lock_guard<std::mutex> lock(data_mutex_);
        outputs_[name] = DataEntry{static_cast<void *>(data), count * sizeof(T)};
    }

    /// Check if input exists
    bool has_input(const std::string &name) const {
        std::lock_guard<std::mutex> lock(data_mutex_);
        return inputs_.find(name) != inputs_.end();
    }

    /// Add input (called by scheduler to wire task outputs to inputs)
    void add_input(const std::string &name, void *ptr, size_t size) {
        std::lock_guard<std::mutex> lock(data_mutex_);
        inputs_[name] = DataEntry{ptr, size};
    }

    /// Get all outputs (for wiring to downstream tasks)
    const std::unordered_map<std::string, std::pair<void *, size_t>> get_outputs() const {
        std::lock_guard<std::mutex> lock(data_mutex_);
        std::unordered_map<std::string, std::pair<void *, size_t>> result;
        for (const auto &[name, entry] : outputs_) {
            result[name] = {entry.ptr, entry.size};
        }
        return result;
    }

    /// Report error during execution
    void report_error(const std::string &message);

    /// Check if error was reported
    bool has_error() const { return has_error_; }

    /// Get error message
    const std::string &error_message() const { return error_message_; }

    /// Clear all data
    void clear() {
        std::lock_guard<std::mutex> lock(data_mutex_);
        inputs_.clear();
        outputs_.clear();
        has_error_ = false;
        error_message_.clear();
    }

  private:
    struct DataEntry {
        void *ptr = nullptr;
        size_t size = 0;
    };

    MemoryPool *memory_pool_;
    std::unordered_map<std::string, DataEntry> inputs_;
    std::unordered_map<std::string, DataEntry> outputs_;
    mutable std::mutex data_mutex_;
    bool has_error_ = false;
    std::string error_message_;
};

} // namespace hts
