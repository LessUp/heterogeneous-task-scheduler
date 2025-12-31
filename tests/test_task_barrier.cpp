#include <gtest/gtest.h>
#include "hts/task_barrier.hpp"
#include "hts/task_graph.hpp"
#include <thread>

using namespace hts;

class TaskBarrierTest : public ::testing::Test {
protected:
    TaskGraph graph;
};

TEST_F(TaskBarrierTest, CreateBarrier) {
    TaskBarrier barrier("test_barrier", graph);
    
    EXPECT_EQ(barrier.name(), "test_barrier");
    EXPECT_NE(barrier.barrier_task(), nullptr);
    EXPECT_EQ(barrier.total_count(), 0);
    EXPECT_EQ(barrier.completed_count(), 0);
}

TEST_F(TaskBarrierTest, AddPredecessors) {
    TaskBarrier barrier("test", graph);
    
    auto task1 = graph.add_task();
    auto task2 = graph.add_task();
    
    barrier.add_predecessor(task1);
    barrier.add_predecessor(task2);
    
    EXPECT_EQ(barrier.total_count(), 2);
    EXPECT_EQ(barrier.predecessors().size(), 2);
}

TEST_F(TaskBarrierTest, AddSuccessors) {
    TaskBarrier barrier("test", graph);
    
    auto task1 = graph.add_task();
    auto task2 = graph.add_task();
    
    barrier.add_successor(task1);
    barrier.add_successor(task2);
    
    EXPECT_EQ(barrier.successors().size(), 2);
}

TEST_F(TaskBarrierTest, IsReady) {
    TaskBarrier barrier("test", graph);
    
    auto task1 = graph.add_task();
    auto task2 = graph.add_task();
    
    barrier.add_predecessor(task1);
    barrier.add_predecessor(task2);
    
    EXPECT_FALSE(barrier.is_ready());
    
    barrier.notify_completion(task1->id());
    EXPECT_FALSE(barrier.is_ready());
    
    barrier.notify_completion(task2->id());
    EXPECT_TRUE(barrier.is_ready());
}

TEST_F(TaskBarrierTest, WaitWithTimeout) {
    TaskBarrier barrier("test", graph);
    
    auto task = graph.add_task();
    barrier.add_predecessor(task);
    
    // Should timeout since task not completed
    bool result = barrier.wait_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(result);
    
    // Complete the task
    barrier.notify_completion(task->id());
    
    // Should succeed immediately
    result = barrier.wait_for(std::chrono::milliseconds(50));
    EXPECT_TRUE(result);
}

TEST_F(TaskBarrierTest, DependencyChain) {
    TaskBarrier barrier("test", graph);
    
    auto pred1 = graph.add_task();
    auto pred2 = graph.add_task();
    auto succ1 = graph.add_task();
    auto succ2 = graph.add_task();
    
    barrier.add_predecessor(pred1);
    barrier.add_predecessor(pred2);
    barrier.add_successor(succ1);
    barrier.add_successor(succ2);
    
    // Verify dependencies are set up correctly
    auto barrier_task = barrier.barrier_task();
    
    // Predecessors should have barrier as successor
    auto pred1_succs = graph.get_successors(pred1->id());
    EXPECT_EQ(pred1_succs.size(), 1);
    EXPECT_EQ(pred1_succs[0]->id(), barrier_task->id());
    
    // Barrier should have successors
    auto barrier_succs = graph.get_successors(barrier_task->id());
    EXPECT_EQ(barrier_succs.size(), 2);
}

class BarrierGroupTest : public ::testing::Test {
protected:
    TaskGraph graph;
};

TEST_F(BarrierGroupTest, CreateBarriers) {
    BarrierGroup group(graph);
    
    auto& barrier1 = group.create_barrier("barrier1");
    auto& barrier2 = group.create_barrier("barrier2");
    
    EXPECT_EQ(barrier1.name(), "barrier1");
    EXPECT_EQ(barrier2.name(), "barrier2");
    EXPECT_EQ(group.barriers().size(), 2);
}

TEST_F(BarrierGroupTest, GetBarrierByName) {
    BarrierGroup group(graph);
    
    group.create_barrier("barrier1");
    group.create_barrier("barrier2");
    
    auto* found = group.get_barrier("barrier1");
    EXPECT_NE(found, nullptr);
    EXPECT_EQ(found->name(), "barrier1");
    
    auto* not_found = group.get_barrier("nonexistent");
    EXPECT_EQ(not_found, nullptr);
}

TEST_F(BarrierGroupTest, WaitAll) {
    BarrierGroup group(graph);
    
    auto& barrier1 = group.create_barrier("barrier1");
    auto& barrier2 = group.create_barrier("barrier2");
    
    auto task1 = graph.add_task();
    auto task2 = graph.add_task();
    
    barrier1.add_predecessor(task1);
    barrier2.add_predecessor(task2);
    
    // Complete tasks in background
    std::thread t([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        barrier1.notify_completion(task1->id());
        barrier2.notify_completion(task2->id());
    });
    
    group.wait_all();
    t.join();
    
    EXPECT_TRUE(barrier1.is_ready());
    EXPECT_TRUE(barrier2.is_ready());
}
