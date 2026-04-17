#include "hts/task_graph.hpp"
#include <gtest/gtest.h>

using namespace hts;

TEST(TaskGraphTest, AddTask) {
  TaskGraph graph;

  auto task = graph.add_task(DeviceType::CPU);

  EXPECT_NE(task, nullptr);
  EXPECT_EQ(task->preferred_device(), DeviceType::CPU);
  EXPECT_EQ(graph.size(), 1);
}

TEST(TaskGraphTest, AddMultipleTasks) {
  TaskGraph graph;

  auto t1 = graph.add_task(DeviceType::CPU);
  auto t2 = graph.add_task(DeviceType::GPU);
  auto t3 = graph.add_task(DeviceType::Any);

  EXPECT_EQ(graph.size(), 3);
  EXPECT_NE(t1->id(), t2->id());
  EXPECT_NE(t2->id(), t3->id());
}

TEST(TaskGraphTest, AddDependency) {
  TaskGraph graph;

  auto t1 = graph.add_task();
  auto t2 = graph.add_task();

  EXPECT_TRUE(graph.add_dependency(t1->id(), t2->id()));

  auto successors = graph.get_successors(t1->id());
  EXPECT_EQ(successors.size(), 1);
  EXPECT_EQ(successors[0]->id(), t2->id());

  auto predecessors = graph.get_predecessors(t2->id());
  EXPECT_EQ(predecessors.size(), 1);
  EXPECT_EQ(predecessors[0]->id(), t1->id());
}

TEST(TaskGraphTest, RejectSelfLoop) {
  TaskGraph graph;

  auto t1 = graph.add_task();

  EXPECT_FALSE(graph.add_dependency(t1->id(), t1->id()));
}

TEST(TaskGraphTest, RejectCycle) {
  TaskGraph graph;

  auto t1 = graph.add_task();
  auto t2 = graph.add_task();
  auto t3 = graph.add_task();

  EXPECT_TRUE(graph.add_dependency(t1->id(), t2->id()));
  EXPECT_TRUE(graph.add_dependency(t2->id(), t3->id()));

  // This would create a cycle: t1 -> t2 -> t3 -> t1
  EXPECT_FALSE(graph.add_dependency(t3->id(), t1->id()));
}

TEST(TaskGraphTest, ValidateValidDAG) {
  TaskGraph graph;

  auto t1 = graph.add_task();
  auto t2 = graph.add_task();
  auto t3 = graph.add_task();

  graph.add_dependency(t1->id(), t2->id());
  graph.add_dependency(t1->id(), t3->id());
  graph.add_dependency(t2->id(), t3->id());

  EXPECT_TRUE(graph.validate());
}

TEST(TaskGraphTest, TopologicalSort) {
  TaskGraph graph;

  auto t1 = graph.add_task();
  auto t2 = graph.add_task();
  auto t3 = graph.add_task();

  graph.add_dependency(t1->id(), t2->id());
  graph.add_dependency(t2->id(), t3->id());

  auto sorted = graph.topological_sort();

  EXPECT_EQ(sorted.size(), 3);
  EXPECT_EQ(sorted[0]->id(), t1->id());
  EXPECT_EQ(sorted[1]->id(), t2->id());
  EXPECT_EQ(sorted[2]->id(), t3->id());
}

TEST(TaskGraphTest, GetRootTasks) {
  TaskGraph graph;

  auto t1 = graph.add_task();
  auto t2 = graph.add_task();
  auto t3 = graph.add_task();

  graph.add_dependency(t1->id(), t3->id());
  graph.add_dependency(t2->id(), t3->id());

  auto roots = graph.get_root_tasks();

  EXPECT_EQ(roots.size(), 2);
}

TEST(TaskGraphTest, InOutDegree) {
  TaskGraph graph;

  auto t1 = graph.add_task();
  auto t2 = graph.add_task();
  auto t3 = graph.add_task();

  graph.add_dependency(t1->id(), t2->id());
  graph.add_dependency(t1->id(), t3->id());

  EXPECT_EQ(graph.in_degree(t1->id()), 0);
  EXPECT_EQ(graph.out_degree(t1->id()), 2);
  EXPECT_EQ(graph.in_degree(t2->id()), 1);
  EXPECT_EQ(graph.in_degree(t3->id()), 1);
}

TEST(TaskGraphTest, GetTask) {
  TaskGraph graph;

  auto t1 = graph.add_task();

  auto retrieved = graph.get_task(t1->id());
  EXPECT_EQ(retrieved, t1);

  auto nonexistent = graph.get_task(999);
  EXPECT_EQ(nonexistent, nullptr);
}

TEST(TaskGraphTest, Clear) {
  TaskGraph graph;

  graph.add_task();
  graph.add_task();

  EXPECT_EQ(graph.size(), 2);

  graph.clear();

  EXPECT_EQ(graph.size(), 0);
  EXPECT_TRUE(graph.empty());
}
