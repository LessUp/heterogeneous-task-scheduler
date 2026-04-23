#pragma once

#include "hts/types.hpp"
#include <condition_variable>
#include <list>
#include <map>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace hts {

/// Memory block descriptor
struct MemoryBlock {
    void *ptr = nullptr;
    size_t size = 0;
    size_t offset = 0; // Offset from base pointer
    bool in_use = false;
};

/// GPU Memory Pool using buddy system allocation
class MemoryPool {
  public:
    /// Construct pool with initial size
    /// @param initial_size_bytes Initial pool size in bytes
    /// @param allow_growth Whether to allow pool expansion
    explicit MemoryPool(size_t initial_size_bytes, bool allow_growth = true);

    /// Destructor - releases all GPU memory
    ~MemoryPool();

    // Non-copyable, non-movable
    MemoryPool(const MemoryPool &) = delete;
    MemoryPool &operator=(const MemoryPool &) = delete;
    MemoryPool(MemoryPool &&) = delete;
    MemoryPool &operator=(MemoryPool &&) = delete;

    /// Allocate memory from pool
    /// @param bytes Number of bytes to allocate
    /// @return Pointer to allocated memory, or nullptr on failure
    void *allocate(size_t bytes);

    /// Free memory back to pool
    /// @param ptr Pointer previously returned by allocate()
    void free(void *ptr);

    /// Get current statistics
    MemoryStats get_stats() const;

    /// Reset pool - frees all allocations
    void reset();

    /// Get total pool size
    size_t total_size() const { return pool_size_; }

    /// Check if pool allows growth
    bool allows_growth() const { return allow_growth_; }

  private:
    size_t pool_size_;
    size_t base_pool_size_; // Original base region size (immutable after
                            // construction)
    bool allow_growth_;
    void *base_ptr_ = nullptr;

    // Free lists indexed by block size (power of 2)
    std::map<size_t, std::list<MemoryBlock>> free_lists_;

    // Allocated blocks indexed by pointer
    std::unordered_map<void *, MemoryBlock> allocated_blocks_;

    // Statistics
    mutable MemoryStats stats_;

    // Thread safety
    mutable std::mutex mutex_;
    std::condition_variable cv_;

    // Additional memory regions (for growth)
    std::vector<void *> additional_regions_;

    // Minimum block size (64 bytes)
    static constexpr size_t MIN_BLOCK_SIZE = 64;

    /// Round up to next power of two
    static size_t round_up_power_of_two(size_t size);

    /// Initialize free list with initial block
    void init_free_list();

    /// Split a block to get desired size
    MemoryBlock split_block(MemoryBlock &block, size_t target_size);

    /// Try to coalesce adjacent free blocks
    void coalesce_blocks();

    /// Expand pool by allocating more memory
    bool expand_pool(size_t min_size);

    /// Calculate fragmentation ratio
    void update_fragmentation() const;
};

} // namespace hts
