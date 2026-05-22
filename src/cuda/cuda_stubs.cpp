// Auto-generated CUDA stubs for CPU-only build
// This file provides CPU implementations of CUDA-dependent components
// for testing purposes only. It should NOT be used in production.

#ifndef HTS_CPU_ONLY
#error "cuda_stubs.cpp is intended for CPU-only builds only"
#endif

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
    if (num_streams == 0) {
        throw std::invalid_argument("StreamManager requires at least one stream");
    }

    for (size_t i = 0; i < num_streams; ++i) {
        cudaStream_t stream;
        cudaStreamCreate(&stream);
        streams_.push_back(stream);
        owned_streams_.insert(stream);
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
    leased_streams_.insert(stream);
    return stream;
}

void StreamManager::release_stream(cudaStream_t stream) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!owned_streams_.count(stream)) {
        throw std::invalid_argument("Cannot release foreign CUDA stream");
    }
    if (!leased_streams_.count(stream)) {
        throw std::invalid_argument("Cannot release CUDA stream that is not currently leased");
    }

    leased_streams_.erase(stream);
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

} // namespace hts
