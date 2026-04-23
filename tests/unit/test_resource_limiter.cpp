#include "hts/resource_limiter.hpp"
#include <atomic>
#include <gtest/gtest.h>
#include <thread>
#include <vector>

using namespace hts;

class SemaphoreTest : public ::testing::Test {
  protected:
    Semaphore sem{3};
};

TEST_F(SemaphoreTest, InitialCount) {
    EXPECT_EQ(sem.available(), 3);
    EXPECT_EQ(sem.max_count(), 3);
}

TEST_F(SemaphoreTest, AcquireRelease) {
    EXPECT_TRUE(sem.try_acquire());
    EXPECT_EQ(sem.available(), 2);

    EXPECT_TRUE(sem.try_acquire());
    EXPECT_EQ(sem.available(), 1);

    EXPECT_TRUE(sem.try_acquire());
    EXPECT_EQ(sem.available(), 0);

    EXPECT_FALSE(sem.try_acquire()); // Should fail
    EXPECT_EQ(sem.available(), 0);

    sem.release();
    EXPECT_EQ(sem.available(), 1);

    EXPECT_TRUE(sem.try_acquire());
    EXPECT_EQ(sem.available(), 0);
}

TEST_F(SemaphoreTest, BlockingAcquire) {
    sem.try_acquire();
    sem.try_acquire();
    sem.try_acquire();

    std::atomic<bool> acquired{false};
    std::thread t([this, &acquired]() {
        sem.acquire(); // Should block
        acquired = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(acquired.load());

    sem.release();
    t.join();
    EXPECT_TRUE(acquired.load());
}

TEST_F(SemaphoreTest, TryAcquireFor) {
    sem.try_acquire();
    sem.try_acquire();
    sem.try_acquire();

    auto start = std::chrono::steady_clock::now();
    bool result = sem.try_acquire_for(std::chrono::milliseconds(100));
    auto elapsed = std::chrono::steady_clock::now() - start;

    EXPECT_FALSE(result);
    EXPECT_GE(elapsed, std::chrono::milliseconds(90));
}

TEST_F(SemaphoreTest, SemaphoreGuard) {
    {
        SemaphoreGuard guard(sem);
        EXPECT_EQ(sem.available(), 2);
    }
    EXPECT_EQ(sem.available(), 3);
}

TEST_F(SemaphoreTest, ConcurrentAccess) {
    std::atomic<int> max_concurrent{0};
    std::atomic<int> current{0};

    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([this, &max_concurrent, &current]() {
            SemaphoreGuard guard(sem);

            int c = ++current;
            int expected = max_concurrent.load();
            while (c > expected && !max_concurrent.compare_exchange_weak(expected, c)) {
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            --current;
        });
    }

    for (auto &t : threads) {
        t.join();
    }

    EXPECT_LE(max_concurrent.load(), 3);
}

class ResourceLimiterTest : public ::testing::Test {
  protected:
    ResourceLimiter::Limits limits;

    void SetUp() override {
        limits.max_concurrent_cpu_tasks = 2;
        limits.max_concurrent_gpu_tasks = 1;
        limits.max_memory_bytes = 1024;
        limits.max_gpu_memory_bytes = 2048;
        limits.max_total_tasks = 10;
    }
};

TEST_F(ResourceLimiterTest, CpuSlots) {
    ResourceLimiter limiter(limits);

    EXPECT_TRUE(limiter.acquire_cpu_slot());
    EXPECT_TRUE(limiter.acquire_cpu_slot());
    EXPECT_FALSE(limiter.acquire_cpu_slot());

    limiter.release_cpu_slot();
    EXPECT_TRUE(limiter.acquire_cpu_slot());
}

TEST_F(ResourceLimiterTest, GpuSlots) {
    ResourceLimiter limiter(limits);

    EXPECT_TRUE(limiter.acquire_gpu_slot());
    EXPECT_FALSE(limiter.acquire_gpu_slot());

    limiter.release_gpu_slot();
    EXPECT_TRUE(limiter.acquire_gpu_slot());
}

TEST_F(ResourceLimiterTest, MemoryAllocation) {
    ResourceLimiter limiter(limits);

    EXPECT_TRUE(limiter.try_allocate_memory(512));
    EXPECT_TRUE(limiter.try_allocate_memory(512));
    EXPECT_FALSE(limiter.try_allocate_memory(1)); // Over limit

    limiter.free_memory(256);
    EXPECT_TRUE(limiter.try_allocate_memory(256));
}

TEST_F(ResourceLimiterTest, GpuMemoryAllocation) {
    ResourceLimiter limiter(limits);

    EXPECT_TRUE(limiter.try_allocate_gpu_memory(1024));
    EXPECT_TRUE(limiter.try_allocate_gpu_memory(1024));
    EXPECT_FALSE(limiter.try_allocate_gpu_memory(1));

    limiter.free_gpu_memory(512);
    EXPECT_TRUE(limiter.try_allocate_gpu_memory(512));
}

TEST_F(ResourceLimiterTest, TaskCount) {
    ResourceLimiter limiter(limits);

    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(limiter.can_add_task());
        limiter.add_task();
    }

    EXPECT_FALSE(limiter.can_add_task());

    limiter.remove_task();
    EXPECT_TRUE(limiter.can_add_task());
}

TEST_F(ResourceLimiterTest, Stats) {
    ResourceLimiter limiter(limits);

    limiter.acquire_cpu_slot();
    limiter.acquire_gpu_slot();
    limiter.try_allocate_memory(512);
    limiter.try_allocate_gpu_memory(1024);
    limiter.add_task();
    limiter.add_task();

    auto stats = limiter.get_stats();
    EXPECT_EQ(stats.current_cpu_tasks, 1);
    EXPECT_EQ(stats.current_gpu_tasks, 1);
    EXPECT_EQ(stats.current_memory, 512);
    EXPECT_EQ(stats.current_gpu_memory, 1024);
    EXPECT_EQ(stats.total_tasks, 2);
}

TEST_F(ResourceLimiterTest, PeakMemory) {
    ResourceLimiter limiter(limits);

    limiter.try_allocate_memory(512);
    limiter.try_allocate_memory(256);
    limiter.free_memory(512);

    auto stats = limiter.get_stats();
    EXPECT_EQ(stats.current_memory, 256);
    EXPECT_EQ(stats.peak_memory, 768);
}

TEST_F(ResourceLimiterTest, UnlimitedResources) {
    ResourceLimiter::Limits unlimited; // All zeros = unlimited
    ResourceLimiter limiter(unlimited);

    // Should always succeed
    for (int i = 0; i < 100; ++i) {
        EXPECT_TRUE(limiter.acquire_cpu_slot());
        EXPECT_TRUE(limiter.acquire_gpu_slot());
        EXPECT_TRUE(limiter.try_allocate_memory(1000000));
        EXPECT_TRUE(limiter.can_add_task());
    }
}

TEST_F(ResourceLimiterTest, ResourceSlotGuard) {
    ResourceLimiter limiter(limits);

    {
        ResourceSlotGuard guard(limiter, DeviceType::CPU);
        auto stats = limiter.get_stats();
        EXPECT_EQ(stats.current_cpu_tasks, 1);
    }

    auto stats = limiter.get_stats();
    EXPECT_EQ(stats.current_cpu_tasks, 0);
}
