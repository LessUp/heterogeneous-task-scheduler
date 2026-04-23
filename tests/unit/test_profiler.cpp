#include "hts/profiler.hpp"
#include <chrono>
#include <gtest/gtest.h>
#include <thread>

using namespace hts;

TEST(ProfilerTest, StartStop) {
    Profiler profiler;

    profiler.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    profiler.stop();

    auto summary = profiler.generate_summary();
    EXPECT_GE(summary.total_time.count(), 10000000); // At least 10ms in ns
}

TEST(ProfilerTest, RecordTasks) {
    Profiler profiler;

    profiler.start();

    profiler.record_task_start(1, "Task1", DeviceType::CPU);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    profiler.record_task_end(1, TaskState::Completed);

    profiler.record_task_start(2, "Task2", DeviceType::GPU);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    profiler.record_task_end(2, TaskState::Completed);

    profiler.stop();

    EXPECT_EQ(profiler.records().size(), 2);

    auto summary = profiler.generate_summary();
    EXPECT_EQ(summary.total_tasks, 2);
    EXPECT_EQ(summary.completed_tasks, 2);
}

TEST(ProfilerTest, FailedTasks) {
    Profiler profiler;

    profiler.start();

    profiler.record_task_start(1, "Task1", DeviceType::CPU);
    profiler.record_task_end(1, TaskState::Completed);

    profiler.record_task_start(2, "Task2", DeviceType::CPU);
    profiler.record_task_end(2, TaskState::Failed);

    profiler.stop();

    auto summary = profiler.generate_summary();
    EXPECT_EQ(summary.completed_tasks, 1);
    EXPECT_EQ(summary.failed_tasks, 1);
}

TEST(ProfilerTest, GenerateReport) {
    Profiler profiler;

    profiler.start();

    profiler.record_task_start(1, "TestTask", DeviceType::CPU);
    profiler.record_task_end(1, TaskState::Completed);

    profiler.stop();

    std::string report = profiler.generate_report();

    EXPECT_FALSE(report.empty());
    EXPECT_NE(report.find("Performance Report"), std::string::npos);
    EXPECT_NE(report.find("TestTask"), std::string::npos);
}

TEST(ProfilerTest, MemoryTracking) {
    Profiler profiler;

    profiler.start();

    profiler.record_task_start(1, "Task1", DeviceType::GPU);
    profiler.record_allocation(1, 1024);
    profiler.record_allocation(1, 2048);
    profiler.record_free(1, 1024);
    profiler.record_task_end(1, TaskState::Completed);

    profiler.stop();

    auto summary = profiler.generate_summary();
    EXPECT_EQ(summary.total_allocations, 2);
}

TEST(ProfilerTest, InactiveRecording) {
    Profiler profiler;

    // Recording without start should be ignored
    profiler.record_task_start(1, "Task1", DeviceType::CPU);
    profiler.record_task_end(1, TaskState::Completed);

    EXPECT_EQ(profiler.records().size(), 0);
}
