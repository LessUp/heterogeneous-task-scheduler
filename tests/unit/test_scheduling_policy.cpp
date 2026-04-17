#include "hts/scheduling_policy.hpp"
#include "hts/task.hpp"
#include <gtest/gtest.h>
#include <memory>

using namespace hts;

class SchedulingPolicyTest : public ::testing::Test {
protected:
  void SetUp() override {
    cpu_task = std::make_shared<Task>(1, DeviceType::CPU);
    cpu_task->set_cpu_function([](TaskContext &ctx) {});

    gpu_task = std::make_shared<Task>(2, DeviceType::GPU);
    gpu_task->set_gpu_function([](TaskContext &ctx, cudaStream_t s) {});

    any_task = std::make_shared<Task>(3, DeviceType::Any);
    any_task->set_cpu_function([](TaskContext &ctx) {});
    any_task->set_gpu_function([](TaskContext &ctx, cudaStream_t s) {});
  }

  std::shared_ptr<Task> cpu_task;
  std::shared_ptr<Task> gpu_task;
  std::shared_ptr<Task> any_task;
};

TEST_F(SchedulingPolicyTest, DefaultPolicyRespectPreference) {
  DefaultSchedulingPolicy policy;

  EXPECT_EQ(policy.select_device(*cpu_task, 0.5, 0.5), DeviceType::CPU);
  EXPECT_EQ(policy.select_device(*gpu_task, 0.5, 0.5), DeviceType::GPU);
}

TEST_F(SchedulingPolicyTest, DefaultPolicyLoadBalancing) {
  DefaultSchedulingPolicy policy;

  // GPU less loaded
  EXPECT_EQ(policy.select_device(*any_task, 0.8, 0.2), DeviceType::GPU);

  // CPU less loaded
  EXPECT_EQ(policy.select_device(*any_task, 0.2, 0.8), DeviceType::CPU);
}

TEST_F(SchedulingPolicyTest, GpuFirstPolicy) {
  GpuFirstPolicy policy;

  // Should prefer GPU for Any tasks
  EXPECT_EQ(policy.select_device(*any_task, 0.0, 0.0), DeviceType::GPU);

  // Should respect CPU preference
  EXPECT_EQ(policy.select_device(*cpu_task, 0.0, 0.0), DeviceType::CPU);
}

TEST_F(SchedulingPolicyTest, CpuFirstPolicy) {
  CpuFirstPolicy policy;

  // Should prefer CPU for Any tasks
  EXPECT_EQ(policy.select_device(*any_task, 0.0, 0.0), DeviceType::CPU);

  // Should respect GPU preference
  EXPECT_EQ(policy.select_device(*gpu_task, 0.0, 0.0), DeviceType::GPU);
}

TEST_F(SchedulingPolicyTest, RoundRobinPolicy) {
  RoundRobinPolicy policy;

  // Should alternate
  DeviceType first = policy.select_device(*any_task, 0.5, 0.5);
  DeviceType second = policy.select_device(*any_task, 0.5, 0.5);

  EXPECT_NE(first, second);
}

TEST_F(SchedulingPolicyTest, PriorityOrdering) {
  DefaultSchedulingPolicy policy;

  auto low = std::make_shared<Task>(1, DeviceType::CPU);
  low->set_priority(TaskPriority::Low);

  auto high = std::make_shared<Task>(2, DeviceType::CPU);
  high->set_priority(TaskPriority::High);

  auto normal = std::make_shared<Task>(3, DeviceType::CPU);
  normal->set_priority(TaskPriority::Normal);

  std::vector<std::shared_ptr<Task>> tasks = {low, high, normal};
  policy.prioritize(tasks);

  EXPECT_EQ(tasks[0]->priority(), TaskPriority::High);
  EXPECT_EQ(tasks[1]->priority(), TaskPriority::Normal);
  EXPECT_EQ(tasks[2]->priority(), TaskPriority::Low);
}

TEST_F(SchedulingPolicyTest, PolicyNames) {
  EXPECT_STREQ(DefaultSchedulingPolicy().name(), "Default");
  EXPECT_STREQ(GpuFirstPolicy().name(), "GPU-First");
  EXPECT_STREQ(CpuFirstPolicy().name(), "CPU-First");
  EXPECT_STREQ(RoundRobinPolicy().name(), "Round-Robin");
  EXPECT_STREQ(ShortestJobFirstPolicy().name(), "Shortest-Job-First");
}
