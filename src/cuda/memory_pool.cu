#include "hts/memory_pool.hpp"
#include <algorithm>
#include <cmath>
#include <cuda_runtime.h>
#include <stdexcept>

namespace hts {

size_t MemoryPool::round_up_power_of_two(size_t size) {
    if (size <= MIN_BLOCK_SIZE)
        return MIN_BLOCK_SIZE;
    size_t power = 1;
    while (power < size) {
        power <<= 1;
    }
    return power;
}

MemoryPool::MemoryPool(size_t initial_size_bytes, bool allow_growth)
    : pool_size_(round_up_power_of_two(initial_size_bytes)), base_pool_size_(pool_size_),
      allow_growth_(allow_growth) {
    cudaError_t err = cudaMalloc(&base_ptr_, pool_size_);
    if (err != cudaSuccess) {
        throw std::runtime_error("Failed to allocate GPU memory pool: " +
                                 std::string(cudaGetErrorString(err)));
    }

    stats_.total_bytes = pool_size_;
    init_free_list();
}

MemoryPool::~MemoryPool() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (base_ptr_) {
        cudaFree(base_ptr_);
        base_ptr_ = nullptr;
    }

    for (void *region : additional_regions_) {
        cudaFree(region);
    }
    additional_regions_.clear();
}

void MemoryPool::init_free_list() {
    MemoryBlock block;
    block.ptr = base_ptr_;
    block.size = pool_size_;
    block.offset = 0;
    block.in_use = false;

    free_lists_[pool_size_].push_back(block);
}

void *MemoryPool::allocate(size_t bytes) {
    if (bytes == 0)
        return nullptr;

    std::unique_lock<std::mutex> lock(mutex_);

    size_t required_size = round_up_power_of_two(bytes);

    // Find smallest free block that fits
    for (auto it = free_lists_.lower_bound(required_size); it != free_lists_.end(); ++it) {
        if (!it->second.empty()) {
            MemoryBlock block = it->second.front();
            it->second.pop_front();

            // Split if block is larger than needed
            while (block.size > required_size && block.size / 2 >= required_size) {
                size_t half_size = block.size / 2;

                // Create buddy block
                MemoryBlock buddy;
                buddy.ptr = static_cast<char *>(block.ptr) + half_size;
                buddy.size = half_size;
                buddy.offset = block.offset + half_size;
                buddy.in_use = false;

                free_lists_[half_size].push_back(buddy);
                block.size = half_size;
            }

            block.in_use = true;
            allocated_blocks_[block.ptr] = block;

            stats_.used_bytes += block.size;
            stats_.peak_bytes = std::max(stats_.peak_bytes, stats_.used_bytes);
            stats_.allocation_count++;
            update_fragmentation();

            return block.ptr;
        }
    }

    // No suitable block found, try to expand
    if (allow_growth_) {
        lock.unlock();
        if (expand_pool(required_size)) {
            return allocate(bytes); // Retry allocation
        }
    }

    if (!allow_growth_) {
        return nullptr;
    }

    return nullptr; // Allocation failed
}

void MemoryPool::free(void *ptr) {
    if (!ptr)
        return;

    std::lock_guard<std::mutex> lock(mutex_);

    auto it = allocated_blocks_.find(ptr);
    if (it == allocated_blocks_.end()) {
        // Double free or invalid pointer - log warning but don't crash
        return;
    }

    MemoryBlock block = it->second;
    allocated_blocks_.erase(it);

    block.in_use = false;
    stats_.used_bytes -= block.size;
    stats_.free_count++;

    // Add back to free list
    free_lists_[block.size].push_back(block);

    // Try to coalesce
    coalesce_blocks();
    update_fragmentation();

    cv_.notify_one();
}

void MemoryPool::coalesce_blocks() {
    // Simple coalescing: merge adjacent buddies
    for (auto &[size, blocks] : free_lists_) {
        if (blocks.size() < 2)
            continue;

        bool merged = true;
        while (merged) {
            merged = false;
            for (auto it1 = blocks.begin(); it1 != blocks.end(); ++it1) {
                for (auto it2 = std::next(it1); it2 != blocks.end(); ++it2) {
                    // Check if blocks are buddies (adjacent and same size)
                    size_t offset1 = it1->offset;
                    size_t offset2 = it2->offset;

                    // Buddies have offsets that differ by exactly their size
                    // and the lower offset is aligned to 2*size
                    size_t lower_offset = std::min(offset1, offset2);
                    size_t higher_offset = std::max(offset1, offset2);

                    if (higher_offset - lower_offset == size && (lower_offset % (2 * size)) == 0) {
                        // Merge buddies
                        MemoryBlock merged_block;
                        merged_block.ptr = (offset1 < offset2) ? it1->ptr : it2->ptr;
                        merged_block.size = 2 * size;
                        merged_block.offset = lower_offset;
                        merged_block.in_use = false;

                        blocks.erase(it2);
                        blocks.erase(it1);
                        free_lists_[2 * size].push_back(merged_block);

                        merged = true;
                        break;
                    }
                }
                if (merged)
                    break;
            }
        }
    }
}

bool MemoryPool::expand_pool(size_t min_size) {
    std::lock_guard<std::mutex> lock(mutex_);

    size_t expand_size = std::max(pool_size_, round_up_power_of_two(min_size));

    void *new_region = nullptr;
    cudaError_t err = cudaMalloc(&new_region, expand_size);
    if (err != cudaSuccess) {
        return false;
    }

    additional_regions_.push_back(new_region);

    MemoryBlock block;
    block.ptr = new_region;
    block.size = expand_size;
    block.offset = stats_.total_bytes; // Logical offset
    block.in_use = false;

    free_lists_[expand_size].push_back(block);

    stats_.total_bytes += expand_size;
    pool_size_ += expand_size;

    return true;
}

MemoryStats MemoryPool::get_stats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

void MemoryPool::reset() {
    std::lock_guard<std::mutex> lock(mutex_);

    allocated_blocks_.clear();
    free_lists_.clear();

    // Free additional regions first
    for (void *region : additional_regions_) {
        cudaFree(region);
    }
    additional_regions_.clear();

    // Restore pool_size_ to original base region size
    pool_size_ = base_pool_size_;

    stats_.total_bytes = pool_size_;
    stats_.used_bytes = 0;
    stats_.allocation_count = 0;
    stats_.free_count = 0;
    stats_.fragmentation_ratio = 0.0;

    // Re-initialize free list with base region only
    if (base_ptr_) {
        MemoryBlock block;
        block.ptr = base_ptr_;
        block.size = pool_size_;
        block.offset = 0;
        block.in_use = false;
        free_lists_[pool_size_].push_back(block);
    }

    cv_.notify_all();
}

void MemoryPool::update_fragmentation() {
    if (stats_.total_bytes == 0) {
        stats_.fragmentation_ratio = 0.0;
        return;
    }

    size_t largest_free = 0;
    size_t total_free = stats_.total_bytes - stats_.used_bytes;

    for (const auto &[size, blocks] : free_lists_) {
        for (const auto &block : blocks) {
            largest_free = std::max(largest_free, block.size);
        }
    }

    if (total_free == 0) {
        stats_.fragmentation_ratio = 0.0;
    } else {
        // Fragmentation = 1 - (largest_free / total_free)
        stats_.fragmentation_ratio = 1.0 - (static_cast<double>(largest_free) / total_free);
    }
}

} // namespace hts
