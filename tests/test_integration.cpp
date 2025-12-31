#include <gtest/gtest.h>
#include "hts/heterogeneous_task_scheduler.hpp"
#include <atomic>
#include <thread>
#include <chrono>

using namespace hts;

TEST(IntegrationTest, SimpleLinearExecution) {
    Scheduler scheduler;
    
    std::vector<int> execution_order;
    std::mutex order_mutex;
    
    auto t1 = scheduler.graph().add_task(DeviceType::CPU);
    auto t2 = scheduler.graph().add_task(DeviceType::CPU);
    auto t3 = scheduler.graph().add_task(DeviceType::CPU);
    
    t1->set_cpu_function([&](TaskContext& ctx) {
        std::lock_guard<std::mutex> lock(order_mutex);
        execution_order.push_back(1);
    });
    
    t2->set_cpu_function([&](TaskContext& ctx) {
        std::lock_guard<std::mutex> lock(order_mutex);
        execution_order.push_back(2);
    });
    
    t3->set_cpu_function([&](TaskContext& ctx) {
        std::lock_guard<std::mutex> lock(order_mutex);
        execution_order.push_back(3);
    });
    
    scheduler.graph().add_dependency(t1->id(), t2->id());
    scheduler.graph().add_dependency(t2->id(), t3->id());
    
    scheduler.execute();
    
    ASSERT_EQ(execution_order.size(), 3);
    EXPECT_EQ(execution_order[0], 1);
    EXPECT_EQ(execution_order[1], 2);
    EXPECT_EQ(execution_order[2], 3);
}

TEST(IntegrationTest, ParallelExecution) {
    Scheduler scheduler;
    
    std::atomic<int> concurrent_count{0};
    std::atomic<int> max_concurrent{0};
    
    auto t1 = scheduler.graph().add_task(DeviceType::CPU);
    auto t2 = scheduler.graph().add_task(DeviceType::CPU);
    auto t3 = scheduler.graph().add_task(DeviceType::CPU);
    
    auto parallel_func = [&](TaskContext& ctx) {
        int current = ++concurrent_count;
        int expected = max_concurrent.load();
        while (current > expected && !max_concurrent.compare_exchange_weak(expected, current)) {}
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        --concurrent_count;
    };
    
    t1->set_cpu_function(parallel_func);
    t2->set_cpu_function(parallel_func);
    t3->set_cpu_function(parallel_func);
    
    // No dependencies - all can run in parallel
    
    scheduler.execute();
    
    // With 4 CPU threads, all 3 tasks should be able to run concurrently
    EXPECT_GE(max_concurrent.load(), 2);
}

TEST(IntegrationTest, DependencyRespected) {
    Scheduler scheduler;
    
    std::atomic<bool> t1_completed{false};
    std::atomic<bool> t2_started_before_t1{false};
    
    auto t1 = scheduler.graph().add_task(DeviceType::CPU);
    auto t2 = scheduler.graph().add_task(DeviceType::CPU);
    
    t1->set_cpu_function([&](TaskContext& ctx) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        t1_completed = true;
    });
    
    t2->set_cpu_function([&](TaskContext& ctx) {
        if (!t1_completed) {
            t2_started_before_t1 = true;
        }
    });
    
    scheduler.graph().add_dependency(t1->id(), t2->id());
    
    scheduler.execute();
    
    EXPECT_TRUE(t1_completed);
    EXPECT_FALSE(t2_started_before_t1);
}

TEST(IntegrationTest, ErrorCallback) {
    Scheduler scheduler;
    
    std::atomic<bool> error_received{false};
    TaskId failed_task_id = 0;
    
    scheduler.set_error_callback([&](TaskId id, const std::string& msg) {
        error_received = true;
        failed_task_id = id;
    });
    
    auto t1 = scheduler.graph().add_task(DeviceType::CPU);
    
    t1->set_cpu_function([](TaskContext& ctx) {
        throw std::runtime_error("Intentional failure");
    });
    
    scheduler.execute();
    
    EXPECT_TRUE(error_received);
    EXPECT_EQ(failed_task_id, t1->id());
}

TEST(IntegrationTest, FailurePropagation) {
    Scheduler scheduler;
    
    std::atomic<bool> t2_executed{false};
    
    auto t1 = scheduler.graph().add_task(DeviceType::CPU);
    auto t2 = scheduler.graph().add_task(DeviceType::CPU);
    
    t1->set_cpu_function([](TaskContext& ctx) {
        throw std::runtime_error("Failure");
    });
    
    t2->set_cpu_function([&](TaskContext& ctx) {
        t2_executed = true;
    });
    
    scheduler.graph().add_dependency(t1->id(), t2->id());
    
    scheduler.execute();
    
    // t2 should not execute because t1 failed
    EXPECT_FALSE(t2_executed);
    EXPECT_EQ(t2->state(), TaskState::Blocked);
}

TEST(IntegrationTest, ExecutionStats) {
    Scheduler scheduler;
    
    auto t1 = scheduler.graph().add_task(DeviceType::CPU);
    
    t1->set_cpu_function([](TaskContext& ctx) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    });
    
    scheduler.execute();
    
    auto stats = scheduler.get_stats();
    
    EXPECT_GT(stats.total_time.count(), 0);
    EXPECT_EQ(stats.task_times.size(), 1);
    EXPECT_GT(stats.task_times[t1->id()].count(), 0);
}

TEST(IntegrationTest, TimelineGeneration) {
    Scheduler scheduler;
    
    auto t1 = scheduler.graph().add_task(DeviceType::CPU);
    t1->set_cpu_function([](TaskContext& ctx) {});
    
    scheduler.execute();
    
    std::string timeline = scheduler.generate_timeline_json();
    
    EXPECT_FALSE(timeline.empty());
    EXPECT_NE(timeline.find("events"), std::string::npos);
    EXPECT_NE(timeline.find("task_id"), std::string::npos);
}

TEST(IntegrationTest, AsyncExecution) {
    Scheduler scheduler;
    
    std::atomic<bool> completed{false};
    
    auto t1 = scheduler.graph().add_task(DeviceType::CPU);
    t1->set_cpu_function([&](TaskContext& ctx) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        completed = true;
    });
    
    auto future = scheduler.execute_async();
    
    // Should return immediately
    EXPECT_FALSE(completed);
    
    future.get();
    
    EXPECT_TRUE(completed);
}

TEST(IntegrationTest, DiamondPattern) {
    Scheduler scheduler;
    
    std::atomic<int> t4_value{0};
    std::atomic<int> t2_value{0};
    std::atomic<int> t3_value{0};
    
    auto t1 = scheduler.graph().add_task(DeviceType::CPU);
    auto t2 = scheduler.graph().add_task(DeviceType::CPU);
    auto t3 = scheduler.graph().add_task(DeviceType::CPU);
    auto t4 = scheduler.graph().add_task(DeviceType::CPU);
    
    t1->set_cpu_function([](TaskContext& ctx) {
        // Initialize
    });
    
    t2->set_cpu_function([&](TaskContext& ctx) {
        t2_value = 10;
    });
    
    t3->set_cpu_function([&](TaskContext& ctx) {
        t3_value = 20;
    });
    
    t4->set_cpu_function([&](TaskContext& ctx) {
        t4_value = t2_value + t3_value;
    });
    
    scheduler.graph().add_dependency(t1->id(), t2->id());
    scheduler.graph().add_dependency(t1->id(), t3->id());
    scheduler.graph().add_dependency(t2->id(), t4->id());
    scheduler.graph().add_dependency(t3->id(), t4->id());
    
    scheduler.execute();
    
    EXPECT_EQ(t4_value, 30);
}

TEST(IntegrationTest, LargeGraph) {
    Scheduler scheduler;
    
    const int NUM_TASKS = 100;
    std::atomic<int> completed_count{0};
    
    std::vector<std::shared_ptr<Task>> tasks;
    for (int i = 0; i < NUM_TASKS; ++i) {
        auto task = scheduler.graph().add_task(DeviceType::CPU);
        task->set_cpu_function([&](TaskContext& ctx) {
            ++completed_count;
        });
        tasks.push_back(task);
    }
    
    // Create a chain
    for (int i = 0; i < NUM_TASKS - 1; ++i) {
        scheduler.graph().add_dependency(tasks[i]->id(), tasks[i+1]->id());
    }
    
    scheduler.execute();
    
    EXPECT_EQ(completed_count, NUM_TASKS);
}

TEST(IntegrationTest, Version) {
    EXPECT_EQ(hts::VERSION_MAJOR, 1);
    EXPECT_EQ(hts::VERSION_MINOR, 0);
    EXPECT_EQ(hts::get_version(), "1.0.0");
}
