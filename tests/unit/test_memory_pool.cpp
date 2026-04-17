#include "hts/memory_pool.hpp"
#include <gtest/gtest.h>
#include <thread>
#include <vector>

using namespace hts;

class MemoryPoolTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Create a small pool for testing (1KB)
    pool_ = std::make_unique<MemoryPool>(1024, true);
  }

  void TearDown() override { pool_.reset(); }

  std::unique_ptr<MemoryPool> pool_;
};

// Test 1: Basic allocation and free
TEST_F(MemoryPoolTest, AllocateBasic) {
  void *ptr = pool_->allocate(64);
  EXPECT_NE(ptr, nullptr);

  pool_->free(ptr);
  // Should not crash
}

// Test 2: Multiple allocations
TEST_F(MemoryPoolTest, MultipleAllocations) {
  std::vector<void *> ptrs;

  for (int i = 0; i < 10; ++i) {
    void *ptr = pool_->allocate(32);
    EXPECT_NE(ptr, nullptr);
    ptrs.push_back(ptr);
  }

  auto stats = pool_->get_stats();
  EXPECT_EQ(stats.allocation_count, 10);

  for (void *ptr : ptrs) {
    pool_->free(ptr);
  }

  stats = pool_->get_stats();
  EXPECT_EQ(stats.free_count, 10);
}

// Test 3: Pool growth
TEST_F(MemoryPoolTest, PoolGrowth) {
  size_t initial_size = pool_->total_size();

  // Allocate more than the initial pool size
  void *large_ptr = pool_->allocate(2048); // 2KB > 1KB initial
  EXPECT_NE(large_ptr, nullptr);

  // Pool should have grown
  EXPECT_GT(pool_->total_size(), initial_size);

  pool_->free(large_ptr);
}

// Test 4: Block coalescing (basic test)
TEST_F(MemoryPoolTest, BlockCoalescing) {
  // Allocate and free to trigger coalescing
  void *ptr1 = pool_->allocate(64);
  void *ptr2 = pool_->allocate(64);
  void *ptr3 = pool_->allocate(64);

  EXPECT_NE(ptr1, nullptr);
  EXPECT_NE(ptr2, nullptr);
  EXPECT_NE(ptr3, nullptr);

  // Free in order
  pool_->free(ptr1);
  pool_->free(ptr2);
  pool_->free(ptr3);

  // Should be able to allocate again
  void *ptr4 = pool_->allocate(128);
  EXPECT_NE(ptr4, nullptr);
  pool_->free(ptr4);
}

// Test 5: Thread safety
TEST_F(MemoryPoolTest, ThreadSafety) {
  const int NUM_THREADS = 4;
  const int ALLOCS_PER_THREAD = 100;

  std::vector<std::thread> threads;

  for (int t = 0; t < NUM_THREADS; ++t) {
    threads.emplace_back([this]() {
      std::vector<void *> my_ptrs;
      for (int i = 0; i < ALLOCS_PER_THREAD; ++i) {
        void *ptr = pool_->allocate(32);
        if (ptr) {
          my_ptrs.push_back(ptr);
        }
      }
      for (void *ptr : my_ptrs) {
        pool_->free(ptr);
      }
    });
  }

  for (auto &t : threads) {
    t.join();
  }

  // Pool should still be in valid state
  auto stats = pool_->get_stats();
  EXPECT_GT(stats.allocation_count, 0);
}

// Test 6: Alignment check
TEST_F(MemoryPoolTest, AlignmentCheck) {
  void *ptr = pool_->allocate(1);
  EXPECT_NE(ptr, nullptr);

  // Minimum block size should provide some alignment
  uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
  // At least 8-byte aligned
  EXPECT_EQ(addr % 8, 0);

  pool_->free(ptr);
}

// Test 7: Fragmentation tracking
TEST_F(MemoryPoolTest, FragmentationTracking) {
  // Allocate different sizes to create fragmentation
  std::vector<void *> ptrs;
  for (int i = 0; i < 5; ++i) {
    void *ptr = pool_->allocate(64 * (i + 1));
    if (ptr)
      ptrs.push_back(ptr);
  }

  auto stats = pool_->get_stats();
  EXPECT_GE(stats.fragmentation_ratio, 0.0);
  EXPECT_LE(stats.fragmentation_ratio, 1.0);

  for (void *ptr : ptrs) {
    pool_->free(ptr);
  }
}

// Test 8: Large allocation
TEST_F(MemoryPoolTest, LargeAllocation) {
  // Allocate a large block (1MB)
  MemoryPool large_pool(1024 * 1024 * 10, true); // 10MB pool

  void *ptr = large_pool.allocate(1024 * 1024); // 1MB
  EXPECT_NE(ptr, nullptr);

  auto stats = large_pool.get_stats();
  EXPECT_GE(stats.used_bytes, 1024 * 1024);

  large_pool.free(ptr);
}

// Test 9: Zero size allocation
TEST_F(MemoryPoolTest, ZeroSizeAllocation) {
  void *ptr = pool_->allocate(0);
  // Zero-size allocation should return nullptr
  EXPECT_EQ(ptr, nullptr);
}

// Test 10: Statistics tracking
TEST_F(MemoryPoolTest, PoolStatistics) {
  size_t initial_total = pool_->total_size();
  EXPECT_GT(initial_total, 0);

  void *ptr = pool_->allocate(100);
  EXPECT_NE(ptr, nullptr);

  auto stats = pool_->get_stats();
  EXPECT_EQ(stats.allocation_count, 1);
  EXPECT_GT(stats.used_bytes, 0);
  EXPECT_GT(stats.peak_bytes, 0);

  pool_->free(ptr);

  stats = pool_->get_stats();
  EXPECT_EQ(stats.free_count, 1);
  EXPECT_EQ(stats.used_bytes, 0);
}

// Test 11: Reset pool
TEST_F(MemoryPoolTest, ResetPool) {
  // Allocate several blocks
  for (int i = 0; i < 5; ++i) {
    void *ptr = pool_->allocate(64);
    EXPECT_NE(ptr, nullptr);
  }

  auto stats = pool_->get_stats();
  EXPECT_GT(stats.used_bytes, 0);

  // Reset pool
  pool_->reset();

  stats = pool_->get_stats();
  EXPECT_EQ(stats.used_bytes, 0);
  EXPECT_EQ(stats.allocation_count, 0);

  // Should be able to allocate again after reset
  void *ptr = pool_->allocate(64);
  EXPECT_NE(ptr, nullptr);
  pool_->free(ptr);
}

// Test 12: No growth pool
TEST_F(MemoryPoolTest, NoGrowthPool) {
  MemoryPool fixed_pool(256, false); // No growth

  // Allocate until exhausted
  std::vector<void *> ptrs;
  while (true) {
    void *ptr = fixed_pool.allocate(64);
    if (!ptr)
      break;
    ptrs.push_back(ptr);
  }

  EXPECT_GT(ptrs.size(), 0);

  // Free all
  for (void *ptr : ptrs) {
    fixed_pool.free(ptr);
  }
}

// Test 13: Double free safety
TEST_F(MemoryPoolTest, DoubleFreeSafety) {
  void *ptr = pool_->allocate(64);
  EXPECT_NE(ptr, nullptr);

  pool_->free(ptr);
  // Double free should not crash
  pool_->free(ptr);
}

// Test 14: Pool with allow_growth=false
TEST_F(MemoryPoolTest, FixedPoolBehavior) {
  MemoryPool fixed_pool(512, false);

  EXPECT_FALSE(fixed_pool.allows_growth());
  EXPECT_EQ(fixed_pool.total_size(), 512);
}
