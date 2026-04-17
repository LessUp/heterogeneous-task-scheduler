#include "hts/task_group.hpp"
#include <gtest/gtest.h>

using namespace hts;

TEST(TaskGroupTest, CreateGroup) {
  TaskGraph graph;
  TaskGroup group("TestGroup", graph);

  EXPECT_EQ(group.name(), "TestGroup");
  EXPECT_TRUE(group.empty());
  EXPECT_EQ(group.size(), 0);
}

TEST(TaskGroupTest, AddTasks) {
  TaskGraph graph;
  TaskGroup group("TestGroup", graph);

  auto t1 = group.add_task(DeviceType::CPU);
  auto t2 = group.add_task(DeviceType::GPU);

  EXPECT_EQ(group.size(), 2);
  EXPECT_FALSE(group.empty());
}

TEST(TaskGroupTest, DependsOn) {
  TaskGraph graph;

  auto init = graph.add_task();

  TaskGroup group("Workers", graph);
  group.add_task();
  group.add_task();

  group.depends_on(init);

  // All tasks in group should depend on init
  for (const auto &task : group.tasks()) {
    auto preds = graph.get_predecessors(task->id());
    EXPECT_EQ(preds.size(), 1);
    EXPECT_EQ(preds[0]->id(), init->id());
  }
}

TEST(TaskGroupTest, Then) {
  TaskGraph graph;

  TaskGroup group("Workers", graph);
  group.add_task();
  group.add_task();

  auto final_task = graph.add_task();
  group.then(final_task);

  // Final task should depend on all tasks in group
  auto preds = graph.get_predecessors(final_task->id());
  EXPECT_EQ(preds.size(), 2);
}

TEST(TaskGroupTest, GroupToGroup) {
  TaskGraph graph;

  TaskGroup group1("Stage1", graph);
  group1.add_task();
  group1.add_task();

  TaskGroup group2("Stage2", graph);
  group2.add_task();
  group2.add_task();

  group1.then(group2);

  // Each task in group2 should depend on all tasks in group1
  for (const auto &task : group2.tasks()) {
    auto preds = graph.get_predecessors(task->id());
    EXPECT_EQ(preds.size(), 2);
  }
}

TEST(TaskGroupTest, SetPriority) {
  TaskGraph graph;
  TaskGroup group("TestGroup", graph);

  group.add_task();
  group.add_task();

  group.set_priority(TaskPriority::Critical);

  for (const auto &task : group.tasks()) {
    EXPECT_EQ(task->priority(), TaskPriority::Critical);
  }
}

TEST(TaskGroupTest, Cancel) {
  TaskGraph graph;
  TaskGroup group("TestGroup", graph);

  group.add_task();
  group.add_task();

  group.cancel();

  for (const auto &task : group.tasks()) {
    EXPECT_TRUE(task->is_cancelled());
  }
}

TEST(TaskGroupTest, ForEach) {
  TaskGraph graph;
  TaskGroup group("TestGroup", graph);

  group.add_task();
  group.add_task();
  group.add_task();

  int count = 0;
  group.for_each([&count](auto task) { count++; });

  EXPECT_EQ(count, 3);
}
