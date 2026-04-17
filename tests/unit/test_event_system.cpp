#include "hts/event_system.hpp"
#include <atomic>
#include <gtest/gtest.h>
#include <thread>
#include <vector>

using namespace hts;

class EventSystemTest : public ::testing::Test {
protected:
  EventSystem events;
};

TEST_F(EventSystemTest, SubscribeAndPublish) {
  std::atomic<int> count{0};

  events.subscribe(EventType::TaskStarted,
                   [&count](const Event &) { ++count; });

  events.task_started(1, DeviceType::CPU);
  events.task_started(2, DeviceType::GPU);

  EXPECT_EQ(count.load(), 2);
}

TEST_F(EventSystemTest, SubscribeAll) {
  std::atomic<int> count{0};

  events.subscribe_all([&count](const Event &) { ++count; });

  events.task_started(1, DeviceType::CPU);
  events.task_completed(1, DeviceType::CPU);
  events.task_failed(2, "error");

  EXPECT_EQ(count.load(), 3);
}

TEST_F(EventSystemTest, Unsubscribe) {
  std::atomic<int> count{0};

  auto id = events.subscribe(EventType::TaskStarted,
                             [&count](const Event &) { ++count; });

  events.task_started(1, DeviceType::CPU);
  EXPECT_EQ(count.load(), 1);

  events.unsubscribe(id);
  events.task_started(2, DeviceType::CPU);
  EXPECT_EQ(count.load(), 1); // Should not increment
}

TEST_F(EventSystemTest, MultipleHandlers) {
  std::atomic<int> count1{0};
  std::atomic<int> count2{0};

  events.subscribe(EventType::TaskCompleted,
                   [&count1](const Event &) { ++count1; });

  events.subscribe(EventType::TaskCompleted,
                   [&count2](const Event &) { ++count2; });

  events.task_completed(1, DeviceType::CPU);

  EXPECT_EQ(count1.load(), 1);
  EXPECT_EQ(count2.load(), 1);
}

TEST_F(EventSystemTest, EventData) {
  TaskId received_id = 0;
  DeviceType received_device = DeviceType::CPU;
  std::string received_message;

  events.subscribe(EventType::TaskFailed, [&](const Event &e) {
    received_id = e.task_id;
    received_device = e.device;
    received_message = e.message;
  });

  events.task_failed(42, "Test error message");

  EXPECT_EQ(received_id, 42);
  EXPECT_EQ(received_message, "Test error message");
}

TEST_F(EventSystemTest, ScopedSubscription) {
  std::atomic<int> count{0};

  {
    auto id = events.subscribe(EventType::TaskStarted,
                               [&count](const Event &) { ++count; });
    ScopedSubscription scoped(events, id);

    events.task_started(1, DeviceType::CPU);
    EXPECT_EQ(count.load(), 1);
  }

  // After scope, subscription should be removed
  events.task_started(2, DeviceType::CPU);
  EXPECT_EQ(count.load(), 1);
}

TEST_F(EventSystemTest, ThreadSafety) {
  std::atomic<int> count{0};

  events.subscribe(EventType::TaskStarted,
                   [&count](const Event &) { ++count; });

  std::vector<std::thread> threads;
  for (int i = 0; i < 10; ++i) {
    threads.emplace_back([this, i]() {
      for (int j = 0; j < 100; ++j) {
        events.task_started(i * 100 + j, DeviceType::CPU);
      }
    });
  }

  for (auto &t : threads) {
    t.join();
  }

  EXPECT_EQ(count.load(), 1000);
}

TEST_F(EventSystemTest, EventTypeName) {
  EXPECT_STREQ(EventSystem::event_type_name(EventType::TaskCreated),
               "TaskCreated");
  EXPECT_STREQ(EventSystem::event_type_name(EventType::TaskStarted),
               "TaskStarted");
  EXPECT_STREQ(EventSystem::event_type_name(EventType::TaskCompleted),
               "TaskCompleted");
  EXPECT_STREQ(EventSystem::event_type_name(EventType::TaskFailed),
               "TaskFailed");
  EXPECT_STREQ(EventSystem::event_type_name(EventType::GraphStarted),
               "GraphStarted");
  EXPECT_STREQ(EventSystem::event_type_name(EventType::GraphCompleted),
               "GraphCompleted");
}

TEST_F(EventSystemTest, Clear) {
  std::atomic<int> count{0};

  events.subscribe(EventType::TaskStarted,
                   [&count](const Event &) { ++count; });

  events.task_started(1, DeviceType::CPU);
  EXPECT_EQ(count.load(), 1);

  events.clear();
  events.task_started(2, DeviceType::CPU);
  EXPECT_EQ(count.load(), 1); // Should not increment after clear
}

TEST_F(EventSystemTest, HandlerException) {
  std::atomic<int> count{0};

  // First handler throws
  events.subscribe(EventType::TaskStarted, [](const Event &) {
    throw std::runtime_error("Handler error");
  });

  // Second handler should still be called
  events.subscribe(EventType::TaskStarted,
                   [&count](const Event &) { ++count; });

  // Should not throw, and second handler should be called
  EXPECT_NO_THROW(events.task_started(1, DeviceType::CPU));
  EXPECT_EQ(count.load(), 1);
}
