#include "hts/scheduler.hpp"
#include <atomic>
#include <chrono>
#include <gtest/gtest.h>
#include <thread>

#ifdef HTS_CPU_ONLY
#include "hts/internal/cuda_stubs.hpp"
#endif

using namespace hts;

class SchedulerTest : public ::testing::Test {
protected:
  void SetUp() override {
    SchedulerConfig config;
    config.memory_pool_size = 1024 * 1024; // 1MB
    config.cpu_thread_count = 4;
    config.gpu_stream_count = 4;
    scheduler_ = std::make_unique<Scheduler>(config);
  }

  void TearDown() override { scheduler_.reset(); }

  std::unique_ptr<Scheduler> scheduler_;
};

// Test 1: Construction and destruction
TEST_F(SchedulerTest, Construction) { EXPECT_NE(scheduler_, nullptr); }

// Test 2: Execute empty graph
TEST_F(SchedulerTest, EmptyGraph) {
  // Empty graph should execute without error
  scheduler_->execute();
}

// Test 3: Single CPU task execution
TEST_F(SchedulerTest, SingleCPUTask) {
  std::atomic<bool> executed{false};

  auto task = scheduler_->graph().add_task(DeviceType::CPU);
  task->set_cpu_function([&executed](TaskContext &ctx) { executed = true; });

  scheduler_->execute();

  EXPECT_TRUE(executed);
  EXPECT_EQ(task->state(), TaskState::Completed);
}

// Test 4: Single GPU task execution
TEST_F(SchedulerTest, SingleGPUTask) {
  std::atomic<bool> executed{false};

  auto task = scheduler_->graph().add_task(DeviceType::GPU);
  task->set_gpu_function(
      [&executed](TaskContext &ctx, cudaStream_t stream) { executed = true; });

  scheduler_->execute();

  EXPECT_TRUE(executed);
  EXPECT_EQ(task->state(), TaskState::Completed);
}

// Test 5: Linear dependency chain
TEST_F(SchedulerTest, LinearDependencyChain) {
  std::vector<int> execution_order;
  std::mutex order_mutex;

  auto t1 = scheduler_->graph().add_task(DeviceType::CPU);
  auto t2 = scheduler_->graph().add_task(DeviceType::CPU);
  auto t3 = scheduler_->graph().add_task(DeviceType::CPU);

  t1->set_cpu_function([&](TaskContext &ctx) {
    std::lock_guard<std::mutex> lock(order_mutex);
    execution_order.push_back(1);
  });

  t2->set_cpu_function([&](TaskContext &ctx) {
    std::lock_guard<std::mutex> lock(order_mutex);
    execution_order.push_back(2);
  });

  t3->set_cpu_function([&](TaskContext &ctx) {
    std::lock_guard<std::mutex> lock(order_mutex);
    execution_order.push_back(3);
  });

  scheduler_->graph().add_dependency(t1->id(), t2->id());
  scheduler_->graph().add_dependency(t2->id(), t3->id());

  scheduler_->execute();

  ASSERT_EQ(execution_order.size(), 3);
  EXPECT_EQ(execution_order[0], 1);
  EXPECT_EQ(execution_order[1], 2);
  EXPECT_EQ(execution_order[2], 3);
}

// Test 6: Error callback
TEST_F(SchedulerTest, ErrorCallback) {
  std::atomic<bool> error_received{false};
  TaskId failed_task_id = 0;

  scheduler_->set_error_callback([&](TaskId id, const std::string &msg) {
    error_received = true;
    failed_task_id = id;
  });

  auto task = scheduler_->graph().add_task(DeviceType::CPU);
  task->set_cpu_function(
      [](TaskContext &ctx) { throw std::runtime_error("Test error"); });

  scheduler_->execute();

  EXPECT_TRUE(error_received);
  EXPECT_EQ(failed_task_id, task->id());
  EXPECT_EQ(task->state(), TaskState::Failed);
}

// Test 7: Failure propagation
TEST_F(SchedulerTest, FailurePropagation) {
  std::atomic<bool> t2_executed{false};

  auto t1 = scheduler_->graph().add_task(DeviceType::CPU);
  auto t2 = scheduler_->graph().add_task(DeviceType::CPU);

  t1->set_cpu_function(
      [](TaskContext &ctx) { throw std::runtime_error("Failure"); });

  t2->set_cpu_function([&](TaskContext &ctx) { t2_executed = true; });

  scheduler_->graph().add_dependency(t1->id(), t2->id());

  scheduler_->execute();

  EXPECT_FALSE(t2_executed);
  EXPECT_EQ(t2->state(), TaskState::Blocked);
}

// Test 8: Execution stats
TEST_F(SchedulerTest, ExecutionStats) {
  auto task = scheduler_->graph().add_task(DeviceType::CPU);
  task->set_cpu_function([](TaskContext &ctx) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  });

  scheduler_->execute();

  auto stats = scheduler_->get_stats();
  EXPECT_GT(stats.total_time.count(), 0);
  EXPECT_EQ(stats.task_times.size(), 1);
}

// Test 9: Timeline generation
TEST_F(SchedulerTest, TimelineGeneration) {
  auto task = scheduler_->graph().add_task(DeviceType::CPU);
  task->set_cpu_function([](TaskContext &ctx) {});

  scheduler_->execute();

  std::string timeline = scheduler_->generate_timeline_json();

  EXPECT_FALSE(timeline.empty());
  EXPECT_NE(timeline.find("events"), std::string::npos);
  EXPECT_NE(timeline.find("task_id"), std::string::npos);
}

// Test 10: Async execution
TEST_F(SchedulerTest, AsyncExecution) {
  std::atomic<bool> completed{false};

  auto task = scheduler_->graph().add_task(DeviceType::CPU);
  task->set_cpu_function([&](TaskContext &ctx) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    completed = true;
  });

  auto future = scheduler_->execute_async();

  // Should return immediately before task completes
  EXPECT_FALSE(completed);

  future.get();

  EXPECT_TRUE(completed);
}

// Test 11: Diamond pattern
TEST_F(SchedulerTest, DiamondPattern) {
  std::atomic<int> t4_value{0};
  std::atomic<int> t2_value{0};
  std::atomic<int> t3_value{0};

  auto t1 = scheduler_->graph().add_task(DeviceType::CPU);
  auto t2 = scheduler_->graph().add_task(DeviceType::CPU);
  auto t3 = scheduler_->graph().add_task(DeviceType::CPU);
  auto t4 = scheduler_->graph().add_task(DeviceType::CPU);

  t1->set_cpu_function([](TaskContext &ctx) {});
  t2->set_cpu_function([&](TaskContext &ctx) { t2_value = 10; });
  t3->set_cpu_function([&](TaskContext &ctx) { t3_value = 20; });
  t4->set_cpu_function(
      [&](TaskContext &ctx) { t4_value = t2_value + t3_value; });

  scheduler_->graph().add_dependency(t1->id(), t2->id());
  scheduler_->graph().add_dependency(t1->id(), t3->id());
  scheduler_->graph().add_dependency(t2->id(), t4->id());
  scheduler_->graph().add_dependency(t3->id(), t4->id());

  scheduler_->execute();

  EXPECT_EQ(t4_value, 30);
}

// Test 12: Policy setting
TEST_F(SchedulerTest, PolicySetting) {
  // Default policy name
  EXPECT_STREQ(scheduler_->policy_name(), "Default");

  // Set a different policy
  scheduler_->set_policy(std::make_unique<GPUFirstSchedulingPolicy>());
  EXPECT_STREQ(scheduler_->policy_name(), "GPUFirst");

  scheduler_->set_policy(std::make_unique<CPUFirstSchedulingPolicy>());
  EXPECT_STREQ(scheduler_->policy_name(), "CPUFirst");

  scheduler_->set_policy(std::make_unique<RoundRobinSchedulingPolicy>());
  EXPECT_STREQ(scheduler_->policy_name(), "RoundRobin");
}

// Test 13: Multiple independent tasks
TEST_F(SchedulerTest, MultipleIndependentTasks) {
  const int NUM_TASKS = 10;
  std::atomic<int> counter{0};

  for (int i = 0; i < NUM_TASKS; ++i) {
    auto task = scheduler_->graph().add_task(DeviceType::CPU);
    task->set_cpu_function([&counter](TaskContext &ctx) { counter++; });
  }

  scheduler_->execute();

  EXPECT_EQ(counter, NUM_TASKS);
}

// Test 14: Mixed CPU/GPU tasks
TEST_F(SchedulerTest, MixedCPUGPUTasks) {
  std::atomic<int> cpu_count{0};
  std::atomic<int> gpu_count{0};

  for (int i = 0; i < 3; ++i) {
    auto cpu_task = scheduler_->graph().add_task(DeviceType::CPU);
    cpu_task->set_cpu_function([&cpu_count](TaskContext &ctx) { cpu_count++; });

    auto gpu_task = scheduler_->graph().add_task(DeviceType::GPU);
    gpu_task->set_gpu_function(
        [&gpu_count](TaskContext &ctx, cudaStream_t stream) { gpu_count++; });
  }

  scheduler_->execute();

  EXPECT_EQ(cpu_count, 3);
  EXPECT_EQ(gpu_count, 3);
}

// Test 15: Profiling toggle
TEST_F(SchedulerTest, ProfilingToggle) {
  scheduler_->set_profiling_enabled(true);

  auto task = scheduler_->graph().add_task(DeviceType::CPU);
  task->set_cpu_function([](TaskContext &ctx) {});

  scheduler_->execute();

  // Should have profiling data
  std::string timeline = scheduler_->generate_timeline_json();
  EXPECT_FALSE(timeline.empty());
}
