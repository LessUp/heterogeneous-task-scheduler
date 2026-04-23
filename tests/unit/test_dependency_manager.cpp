#include "hts/dependency_manager.hpp"
#include <gtest/gtest.h>

using namespace hts;

TEST(DependencyManagerTest, InitialState) {
    TaskGraph graph;
    auto t1 = graph.add_task();
    auto t2 = graph.add_task();
    graph.add_dependency(t1->id(), t2->id());

    DependencyManager dm(graph);

    // t1 has no dependencies, should be ready
    EXPECT_TRUE(dm.is_ready(t1->id()));
    // t2 depends on t1, should not be ready
    EXPECT_FALSE(dm.is_ready(t2->id()));
}

TEST(DependencyManagerTest, MarkCompleted) {
    TaskGraph graph;
    auto t1 = graph.add_task();
    auto t2 = graph.add_task();
    graph.add_dependency(t1->id(), t2->id());

    DependencyManager dm(graph);

    EXPECT_FALSE(dm.is_ready(t2->id()));

    dm.mark_completed(t1->id());

    EXPECT_TRUE(dm.is_completed(t1->id()));
    EXPECT_TRUE(dm.is_ready(t2->id()));
}

TEST(DependencyManagerTest, MarkFailed) {
    TaskGraph graph;
    auto t1 = graph.add_task();
    auto t2 = graph.add_task();
    auto t3 = graph.add_task();
    graph.add_dependency(t1->id(), t2->id());
    graph.add_dependency(t2->id(), t3->id());

    DependencyManager dm(graph);

    dm.mark_failed(t1->id());

    EXPECT_TRUE(dm.is_failed(t1->id()));
    EXPECT_TRUE(dm.is_blocked(t2->id()));
    EXPECT_TRUE(dm.is_blocked(t3->id()));
}

TEST(DependencyManagerTest, GetReadyTasks) {
    TaskGraph graph;
    auto t1 = graph.add_task();
    auto t2 = graph.add_task();
    auto t3 = graph.add_task();
    graph.add_dependency(t1->id(), t3->id());
    graph.add_dependency(t2->id(), t3->id());

    DependencyManager dm(graph);

    auto ready = dm.get_ready_tasks();
    EXPECT_EQ(ready.size(), 2); // t1 and t2 are ready

    dm.mark_completed(t1->id());
    ready = dm.get_ready_tasks();
    EXPECT_EQ(ready.size(), 1); // Only t2 is ready (t3 still waiting)

    dm.mark_completed(t2->id());
    ready = dm.get_ready_tasks();
    EXPECT_EQ(ready.size(), 1); // Now t3 is ready
}

TEST(DependencyManagerTest, PendingDependencyCount) {
    TaskGraph graph;
    auto t1 = graph.add_task();
    auto t2 = graph.add_task();
    auto t3 = graph.add_task();
    graph.add_dependency(t1->id(), t3->id());
    graph.add_dependency(t2->id(), t3->id());

    DependencyManager dm(graph);

    EXPECT_EQ(dm.pending_dependency_count(t1->id()), 0);
    EXPECT_EQ(dm.pending_dependency_count(t2->id()), 0);
    EXPECT_EQ(dm.pending_dependency_count(t3->id()), 2);

    dm.mark_completed(t1->id());
    EXPECT_EQ(dm.pending_dependency_count(t3->id()), 1);

    dm.mark_completed(t2->id());
    EXPECT_EQ(dm.pending_dependency_count(t3->id()), 0);
}

TEST(DependencyManagerTest, Reset) {
    TaskGraph graph;
    auto t1 = graph.add_task();
    auto t2 = graph.add_task();
    graph.add_dependency(t1->id(), t2->id());

    DependencyManager dm(graph);

    dm.mark_completed(t1->id());
    EXPECT_TRUE(dm.is_completed(t1->id()));

    dm.reset();

    EXPECT_FALSE(dm.is_completed(t1->id()));
    EXPECT_TRUE(dm.is_ready(t1->id()));
}

TEST(DependencyManagerTest, DiamondDependency) {
    // Diamond pattern: t1 -> t2, t1 -> t3, t2 -> t4, t3 -> t4
    TaskGraph graph;
    auto t1 = graph.add_task();
    auto t2 = graph.add_task();
    auto t3 = graph.add_task();
    auto t4 = graph.add_task();

    graph.add_dependency(t1->id(), t2->id());
    graph.add_dependency(t1->id(), t3->id());
    graph.add_dependency(t2->id(), t4->id());
    graph.add_dependency(t3->id(), t4->id());

    DependencyManager dm(graph);

    EXPECT_EQ(dm.pending_dependency_count(t4->id()), 2);

    dm.mark_completed(t1->id());
    EXPECT_TRUE(dm.is_ready(t2->id()));
    EXPECT_TRUE(dm.is_ready(t3->id()));
    EXPECT_FALSE(dm.is_ready(t4->id()));

    dm.mark_completed(t2->id());
    EXPECT_FALSE(dm.is_ready(t4->id())); // Still waiting for t3

    dm.mark_completed(t3->id());
    EXPECT_TRUE(dm.is_ready(t4->id()));
}
