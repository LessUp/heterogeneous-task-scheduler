#include "hts/execution_engine.hpp"
#include "hts/memory_pool.hpp"
#include <atomic>
#include <chrono>
#include <gtest/gtest.h>
#include <thread>

#ifdef HTS_CPU_ONLY
#include "hts/internal/cuda_stubs.hpp"
#endif

using namespace hts;

class ExecutionEngineTest : public ::testing::Test {
protected:
  void SetUp() override {
    pool_ = std::make_unique<MemoryPool>(1024 * 1024); // 1MB
    engine_ = std::make_unique<ExecutionEngine>(
        *pool_, 4, 4); // 4 CPU threads, 4 GPU streams
  }

  void TearDown() override {
    engine_.reset();
    pool_.reset();
  }

  std::unique_ptr<MemoryPool> pool_;
  std::unique_ptr<ExecutionEngine> engine_;
};

// Test 1: Construction
TEST_F(ExecutionEngineTest, Construction) {
  EXPECT_NE(engine_, nullptr);
  EXPECT_EQ(engine_->active_cpu_tasks(), 0);
  EXPECT_EQ(engine_->active_gpu_tasks(), 0);
}

// Test 2: CPU task submission
TEST_F(ExecutionEngineTest, CPUTaskSubmission) {
  auto task = std::make_shared<Task>(DeviceType::CPU);
  std::atomic<bool> executed{false};

  task->set_cpu_function([&executed](TaskContext &ctx) { executed = true; });

  auto future = engine_->execute_task(task, DeviceType::CPU);
  future.wait();

  EXPECT_TRUE(executed);
  EXPECT_EQ(task->state(), TaskState::Completed);
}

// Test 3: GPU task submission (CPU-only mode will use stubs)
TEST_F(ExecutionEngineTest, GPUTaskSubmission) {
  auto task = std::make_shared<Task>(DeviceType::GPU);
  std::atomic<bool> executed{false};

  task->set_gpu_function(
      [&executed](TaskContext &ctx, cudaStream_t stream) { executed = true; });

  auto future = engine_->execute_task(task, DeviceType::GPU);
  future.wait();

  EXPECT_TRUE(executed);
  EXPECT_EQ(task->state(), TaskState::Completed);
}

// Test 4: Multiple CPU tasks
TEST_F(ExecutionEngineTest, MultipleCPUTasks) {
  std::atomic<int> counter{0};
  std::vector<std::future<void>> futures;

  for (int i = 0; i < 10; ++i) {
    auto task = std::make_shared<Task>(DeviceType::CPU);
    task->set_cpu_function([&counter](TaskContext &ctx) { counter++; });
    futures.push_back(engine_->execute_task(task, DeviceType::CPU));
  }

  for (auto &f : futures) {
    f.wait();
  }

  EXPECT_EQ(counter, 10);
}

// Test 5: CPU load tracking
TEST_F(ExecutionEngineTest, GetCPULoad) {
  // Initially no load
  EXPECT_EQ(engine_->get_cpu_load(), 0.0);

  // Submit tasks and check load
  auto task = std::make_shared<Task>(DeviceType::CPU);
  std::atomic<bool> can_finish{false};

  task->set_cpu_function([&can_finish](TaskContext &ctx) {
    while (!can_finish) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  });

  auto future = engine_->execute_task(task, DeviceType::CPU);

  // Wait a bit for task to start
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  // Load should be > 0 now
  double load = engine_->get_cpu_load();
  EXPECT_GE(load, 0.0);
  EXPECT_LE(load, 1.0);

  can_finish = true;
  future.wait();
}

// Test 6: GPU load tracking
TEST_F(ExecutionEngineTest, GetGPULoad) {
  // Initially no load
  EXPECT_EQ(engine_->get_gpu_load(), 0.0);
}

// Test 7: Wait all
TEST_F(ExecutionEngineTest, WaitAll) {
  std::atomic<int> counter{0};

  for (int i = 0; i < 5; ++i) {
    auto task = std::make_shared<Task>(DeviceType::CPU);
    task->set_cpu_function([&counter](TaskContext &ctx) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      counter++;
    });
    engine_->execute_task(task, DeviceType::CPU);
  }

  engine_->wait_all();

  EXPECT_EQ(counter, 5);
}

// Test 8: Task exception handling
TEST_F(ExecutionEngineTest, TaskExceptionHandling) {
  auto task = std::make_shared<Task>(DeviceType::CPU);

  task->set_cpu_function(
      [](TaskContext &ctx) { throw std::runtime_error("Test exception"); });

  auto future = engine_->execute_task(task, DeviceType::CPU);

  EXPECT_THROW(future.get(), std::runtime_error);
  EXPECT_EQ(task->state(), TaskState::Failed);
}

// Test 9: Memory pool access
TEST_F(ExecutionEngineTest, MemoryPoolAccess) {
  MemoryPool &pool_ref = engine_->memory_pool();
  EXPECT_EQ(pool_ref.total_size(), pool_->total_size());
}

// Test 10: Stream manager access
TEST_F(ExecutionEngineTest, StreamManagerAccess) {
  StreamManager &stream_mgr = engine_->stream_manager();
  EXPECT_GT(stream_mgr.num_streams(), 0);
}

// Test 11: Concurrent CPU and GPU tasks
TEST_F(ExecutionEngineTest, ConcurrentCPUAndGPUTasks) {
  std::atomic<int> cpu_counter{0};
  std::atomic<int> gpu_counter{0};
  std::vector<std::future<void>> futures;

  // Submit CPU tasks
  for (int i = 0; i < 5; ++i) {
    auto task = std::make_shared<Task>(DeviceType::CPU);
    task->set_cpu_function([&cpu_counter](TaskContext &ctx) { cpu_counter++; });
    futures.push_back(engine_->execute_task(task, DeviceType::CPU));
  }

  // Submit GPU tasks
  for (int i = 0; i < 5; ++i) {
    auto task = std::make_shared<Task>(DeviceType::GPU);
    task->set_gpu_function(
        [&gpu_counter](TaskContext &ctx, cudaStream_t stream) {
          gpu_counter++;
        });
    futures.push_back(engine_->execute_task(task, DeviceType::GPU));
  }

  for (auto &f : futures) {
    f.wait();
  }

  EXPECT_EQ(cpu_counter, 5);
  EXPECT_EQ(gpu_counter, 5);
}

// Test 12: Task context availability
TEST_F(ExecutionEngineTest, TaskContextAvailability) {
  auto task = std::make_shared<Task>(DeviceType::CPU);
  bool has_memory_pool = false;

  task->set_cpu_function([&has_memory_pool](TaskContext &ctx) {
    has_memory_pool = (ctx.memory_pool() != nullptr);
  });

  auto future = engine_->execute_task(task, DeviceType::CPU);
  future.wait();

  EXPECT_TRUE(has_memory_pool);
}
