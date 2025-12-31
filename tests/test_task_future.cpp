#include <gtest/gtest.h>
#include "hts/task_future.hpp"
#include "hts/task.hpp"
#include <thread>

using namespace hts;

class TaskFutureTest : public ::testing::Test {
protected:
    std::shared_ptr<Task> task;
    
    void SetUp() override {
        task = std::make_shared<Task>(1);
    }
};

TEST_F(TaskFutureTest, DefaultConstruction) {
    TaskFuture<int> future;
    EXPECT_FALSE(future.valid());
}

TEST_F(TaskFutureTest, ValidFuture) {
    TaskFuture<int> future(task);
    EXPECT_TRUE(future.valid());
    EXPECT_FALSE(future.ready());
}

TEST_F(TaskFutureTest, SetAndGetValue) {
    TaskFuture<int> future(task);
    
    future.set_value(42);
    
    EXPECT_TRUE(future.ready());
    EXPECT_EQ(future.get(), 42);
}

TEST_F(TaskFutureTest, TryGet) {
    TaskFuture<int> future(task);
    
    auto result = future.try_get();
    EXPECT_FALSE(result.has_value());
    
    future.set_value(42);
    
    result = future.try_get();
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 42);
}

TEST_F(TaskFutureTest, SetError) {
    TaskFuture<int> future(task);
    
    future.set_error("Test error");
    
    EXPECT_TRUE(future.ready());
    EXPECT_THROW(future.get(), std::runtime_error);
}

TEST_F(TaskFutureTest, WaitForValue) {
    TaskFuture<int> future(task);
    
    std::thread t([&future]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        future.set_value(42);
    });
    
    future.wait();
    EXPECT_EQ(future.get(), 42);
    
    t.join();
}

TEST_F(TaskFutureTest, WaitForTimeout) {
    TaskFuture<int> future(task);
    
    bool result = future.wait_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(result);
    
    future.set_value(42);
    result = future.wait_for(std::chrono::milliseconds(50));
    EXPECT_TRUE(result);
}

TEST_F(TaskFutureTest, GetTask) {
    TaskFuture<int> future(task);
    EXPECT_EQ(future.task(), task);
}

// Tests for void specialization
class TaskFutureVoidTest : public ::testing::Test {
protected:
    std::shared_ptr<Task> task;
    
    void SetUp() override {
        task = std::make_shared<Task>(1);
    }
};

TEST_F(TaskFutureVoidTest, DefaultConstruction) {
    TaskFuture<void> future;
    EXPECT_FALSE(future.valid());
}

TEST_F(TaskFutureVoidTest, ValidFuture) {
    TaskFuture<void> future(task);
    EXPECT_TRUE(future.valid());
    EXPECT_FALSE(future.ready());
}

TEST_F(TaskFutureVoidTest, SetValue) {
    TaskFuture<void> future(task);
    
    future.set_value();
    
    EXPECT_TRUE(future.ready());
    EXPECT_NO_THROW(future.get());
}

TEST_F(TaskFutureVoidTest, SetError) {
    TaskFuture<void> future(task);
    
    future.set_error("Test error");
    
    EXPECT_TRUE(future.ready());
    EXPECT_THROW(future.get(), std::runtime_error);
}

TEST_F(TaskFutureVoidTest, WaitForValue) {
    TaskFuture<void> future(task);
    
    std::thread t([&future]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        future.set_value();
    });
    
    future.wait();
    EXPECT_TRUE(future.ready());
    
    t.join();
}

TEST_F(TaskFutureVoidTest, WaitForTimeout) {
    TaskFuture<void> future(task);
    
    bool result = future.wait_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(result);
    
    future.set_value();
    result = future.wait_for(std::chrono::milliseconds(50));
    EXPECT_TRUE(result);
}

// Test with different types
TEST(TaskFutureTypesTest, StringValue) {
    auto task = std::make_shared<Task>(1);
    TaskFuture<std::string> future(task);
    
    future.set_value("Hello, World!");
    EXPECT_EQ(future.get(), "Hello, World!");
}

TEST(TaskFutureTypesTest, VectorValue) {
    auto task = std::make_shared<Task>(1);
    TaskFuture<std::vector<int>> future(task);
    
    future.set_value({1, 2, 3, 4, 5});
    
    auto result = future.get();
    EXPECT_EQ(result.size(), 5);
    EXPECT_EQ(result[0], 1);
    EXPECT_EQ(result[4], 5);
}

TEST(TaskFutureTypesTest, DoubleValue) {
    auto task = std::make_shared<Task>(1);
    TaskFuture<double> future(task);
    
    future.set_value(3.14159);
    EXPECT_DOUBLE_EQ(future.get(), 3.14159);
}
