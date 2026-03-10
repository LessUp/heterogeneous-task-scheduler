#include "hts/task_builder.hpp"
#include <gtest/gtest.h>

using namespace hts;

TEST(TaskBuilderTest, BasicBuild) {
  TaskGraph graph;
  TaskBuilder builder(graph);

  auto task = builder.name("Test Task")
                  .device(DeviceType::CPU)
                  .cpu([](TaskContext &ctx) {})
                  .build();

  EXPECT_NE(task, nullptr);
  EXPECT_EQ(task->name(), "Test Task");
  EXPECT_EQ(task->preferred_device(), DeviceType::CPU);
  EXPECT_TRUE(task->has_cpu_function());
}

TEST(TaskBuilderTest, WithPriority) {
  TaskGraph graph;
  TaskBuilder builder(graph);

  auto task = builder.priority(TaskPriority::Critical)
                  .cpu([](TaskContext &ctx) {})
                  .build();

  EXPECT_EQ(task->priority(), TaskPriority::Critical);
}

TEST(TaskBuilderTest, WithDependencies) {
  TaskGraph graph;
  TaskBuilder builder(graph);

  auto task1 = builder.name("Task 1").cpu([](TaskContext &ctx) {}).build();

  auto task2 =
      builder.name("Task 2").after(task1).cpu([](TaskContext &ctx) {}).build();

  auto predecessors = graph.get_predecessors(task2->id());
  EXPECT_EQ(predecessors.size(), 1);
  EXPECT_EQ(predecessors[0]->id(), task1->id());
}

TEST(TaskBuilderTest, MultipleDependencies) {
  TaskGraph graph;
  TaskBuilder builder(graph);

  auto task1 = builder.cpu([](TaskContext &ctx) {}).build();
  auto task2 = builder.cpu([](TaskContext &ctx) {}).build();

  auto task3 =
      builder.after(task1).after(task2).cpu([](TaskContext &ctx) {}).build();

  auto predecessors = graph.get_predecessors(task3->id());
  EXPECT_EQ(predecessors.size(), 2);
}

TEST(TaskBuilderTest, ResetAfterBuild) {
  TaskGraph graph;
  TaskBuilder builder(graph);

  auto task1 = builder.name("Task 1")
                   .priority(TaskPriority::High)
                   .cpu([](TaskContext &ctx) {})
                   .build();

  // Second build should have default values
  auto task2 = builder.cpu([](TaskContext &ctx) {}).build();

  EXPECT_EQ(task1->name(), "Task 1");
  EXPECT_EQ(task1->priority(), TaskPriority::High);

  EXPECT_EQ(task2->name(), "");
  EXPECT_EQ(task2->priority(), TaskPriority::Normal);
}

TEST(TaskBuilderTest, BothFunctions) {
  TaskGraph graph;
  TaskBuilder builder(graph);

  auto task = builder.device(DeviceType::Any)
                  .cpu([](TaskContext &ctx) {})
                  .gpu([](TaskContext &ctx, cudaStream_t stream) {})
                  .build();

  EXPECT_TRUE(task->has_cpu_function());
  EXPECT_TRUE(task->has_gpu_function());
}
