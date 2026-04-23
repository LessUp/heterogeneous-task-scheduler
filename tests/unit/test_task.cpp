#include "hts/task.hpp"
#include "hts/task_context.hpp"
#include <gtest/gtest.h>

using namespace hts;

TEST(TaskTest, CreateTaskWithId) {
    Task task(1, DeviceType::CPU);
    EXPECT_EQ(task.id(), 1);
    EXPECT_EQ(task.preferred_device(), DeviceType::CPU);
    EXPECT_EQ(task.state(), TaskState::Pending);
}

TEST(TaskTest, CreateTaskWithGPU) {
    Task task(2, DeviceType::GPU);
    EXPECT_EQ(task.id(), 2);
    EXPECT_EQ(task.preferred_device(), DeviceType::GPU);
}

TEST(TaskTest, CreateTaskWithAny) {
    Task task(3, DeviceType::Any);
    EXPECT_EQ(task.preferred_device(), DeviceType::Any);
}

TEST(TaskTest, SetCpuFunction) {
    Task task(1, DeviceType::CPU);
    bool executed = false;

    task.set_cpu_function([&executed](TaskContext &ctx) { executed = true; });

    EXPECT_TRUE(task.has_cpu_function());
    EXPECT_FALSE(task.has_gpu_function());
}

TEST(TaskTest, SetGpuFunction) {
    Task task(1, DeviceType::GPU);

    task.set_gpu_function([](TaskContext &ctx, cudaStream_t stream) {
        // GPU function
    });

    EXPECT_FALSE(task.has_cpu_function());
    EXPECT_TRUE(task.has_gpu_function());
}

TEST(TaskTest, SetBothFunctions) {
    Task task(1, DeviceType::Any);

    task.set_cpu_function([](TaskContext &ctx) {});
    task.set_gpu_function([](TaskContext &ctx, cudaStream_t stream) {});

    EXPECT_TRUE(task.has_cpu_function());
    EXPECT_TRUE(task.has_gpu_function());
}

TEST(TaskTest, StateTransitions) {
    Task task(1, DeviceType::CPU);

    EXPECT_EQ(task.state(), TaskState::Pending);

    task.set_state(TaskState::Ready);
    EXPECT_EQ(task.state(), TaskState::Ready);

    task.set_state(TaskState::Running);
    EXPECT_EQ(task.state(), TaskState::Running);

    task.set_state(TaskState::Completed);
    EXPECT_EQ(task.state(), TaskState::Completed);
}

TEST(TaskTest, ExecutionTime) {
    Task task(1, DeviceType::CPU);

    EXPECT_EQ(task.execution_time().count(), 0);

    task.set_execution_time(std::chrono::nanoseconds(1000000));
    EXPECT_EQ(task.execution_time().count(), 1000000);
}

TEST(TaskContextTest, ErrorReporting) {
    TaskContext ctx;

    EXPECT_FALSE(ctx.has_error());

    ctx.report_error("Test error");

    EXPECT_TRUE(ctx.has_error());
    EXPECT_EQ(ctx.error_message(), "Test error");
}

TEST(TaskContextTest, InputOutput) {
    TaskContext ctx;

    int data = 42;
    ctx.set_output("result", &data, 1);

    auto outputs = ctx.get_outputs();
    EXPECT_EQ(outputs.size(), 1);
    EXPECT_TRUE(outputs.find("result") != outputs.end());
}

TEST(TaskContextTest, AddInput) {
    TaskContext ctx;

    int data = 100;
    ctx.add_input("input", &data, sizeof(int));

    EXPECT_TRUE(ctx.has_input("input"));
    EXPECT_EQ(*ctx.get_input<int>("input"), 100);
}

TEST(TaskContextTest, MissingInput) {
    TaskContext ctx;

    EXPECT_FALSE(ctx.has_input("nonexistent"));
    EXPECT_THROW(ctx.get_input<int>("nonexistent"), std::runtime_error);
}
