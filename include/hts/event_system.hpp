#pragma once

#include "hts/types.hpp"
#include <any>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace hts {

/// Event types in the scheduler
enum class EventType {
  TaskCreated,
  TaskStarted,
  TaskCompleted,
  TaskFailed,
  TaskCancelled,
  TaskRetrying,
  GraphStarted,
  GraphCompleted,
  MemoryAllocated,
  MemoryFreed,
  StreamAcquired,
  StreamReleased,
  Custom
};

/// Event data structure
struct Event {
  EventType type;
  TaskId task_id{0};
  DeviceType device{DeviceType::CPU};
  std::string message;
  std::chrono::high_resolution_clock::time_point timestamp;
  std::any custom_data;

  Event(EventType t)
      : type(t), timestamp(std::chrono::high_resolution_clock::now()) {}

  Event(EventType t, TaskId id)
      : type(t), task_id(id),
        timestamp(std::chrono::high_resolution_clock::now()) {}

  Event(EventType t, TaskId id, DeviceType dev)
      : type(t), task_id(id), device(dev),
        timestamp(std::chrono::high_resolution_clock::now()) {}

  Event(EventType t, TaskId id, const std::string &msg)
      : type(t), task_id(id), message(msg),
        timestamp(std::chrono::high_resolution_clock::now()) {}
};

/// Event handler function type
using EventHandler = std::function<void(const Event &)>;

/// Subscription handle for unsubscribing
using SubscriptionId = uint64_t;

/// EventSystem provides pub/sub for scheduler events
class EventSystem {
public:
  EventSystem() = default;

  /// Subscribe to an event type
  /// @param type Event type to subscribe to
  /// @param handler Handler function
  /// @return Subscription ID for unsubscribing
  SubscriptionId subscribe(EventType type, EventHandler handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    SubscriptionId id = next_id_++;
    handlers_[type].emplace_back(id, std::move(handler));
    return id;
  }

  /// Subscribe to all events
  SubscriptionId subscribe_all(EventHandler handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    SubscriptionId id = next_id_++;
    global_handlers_.emplace_back(id, std::move(handler));
    return id;
  }

  /// Unsubscribe from events
  void unsubscribe(SubscriptionId id) {
    std::lock_guard<std::mutex> lock(mutex_);

    // Remove from type-specific handlers
    for (auto &[type, handlers] : handlers_) {
      handlers.erase(
          std::remove_if(handlers.begin(), handlers.end(),
                         [id](const auto &pair) { return pair.first == id; }),
          handlers.end());
    }

    // Remove from global handlers
    global_handlers_.erase(
        std::remove_if(global_handlers_.begin(), global_handlers_.end(),
                       [id](const auto &pair) { return pair.first == id; }),
        global_handlers_.end());
  }

  /// Publish an event
  void publish(const Event &event) {
    std::vector<EventHandler> handlers_to_call;

    {
      std::lock_guard<std::mutex> lock(mutex_);

      // Collect type-specific handlers
      auto it = handlers_.find(event.type);
      if (it != handlers_.end()) {
        for (const auto &[id, handler] : it->second) {
          handlers_to_call.push_back(handler);
        }
      }

      // Collect global handlers
      for (const auto &[id, handler] : global_handlers_) {
        handlers_to_call.push_back(handler);
      }
    }

    // Call handlers outside lock
    for (const auto &handler : handlers_to_call) {
      try {
        handler(event);
      } catch (...) {
        // Ignore handler exceptions
      }
    }
  }

  /// Convenience methods for publishing common events
  void task_created(TaskId id) { publish(Event{EventType::TaskCreated, id}); }

  void task_started(TaskId id, DeviceType device) {
    publish(Event{EventType::TaskStarted, id, device});
  }

  void task_completed(TaskId id, DeviceType device) {
    publish(Event{EventType::TaskCompleted, id, device});
  }

  void task_failed(TaskId id, const std::string &error) {
    publish(Event{EventType::TaskFailed, id, error});
  }

  void task_cancelled(TaskId id) {
    publish(Event{EventType::TaskCancelled, id});
  }

  void task_retrying(TaskId id, size_t attempt) {
    Event event{EventType::TaskRetrying, id};
    event.custom_data = attempt;
    publish(event);
  }

  void graph_started() { publish(Event{EventType::GraphStarted}); }

  void graph_completed() { publish(Event{EventType::GraphCompleted}); }

  /// Clear all subscriptions
  void clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    handlers_.clear();
    global_handlers_.clear();
  }

  /// Get event type name
  static const char *event_type_name(EventType type) {
    switch (type) {
    case EventType::TaskCreated:
      return "TaskCreated";
    case EventType::TaskStarted:
      return "TaskStarted";
    case EventType::TaskCompleted:
      return "TaskCompleted";
    case EventType::TaskFailed:
      return "TaskFailed";
    case EventType::TaskCancelled:
      return "TaskCancelled";
    case EventType::TaskRetrying:
      return "TaskRetrying";
    case EventType::GraphStarted:
      return "GraphStarted";
    case EventType::GraphCompleted:
      return "GraphCompleted";
    case EventType::MemoryAllocated:
      return "MemoryAllocated";
    case EventType::MemoryFreed:
      return "MemoryFreed";
    case EventType::StreamAcquired:
      return "StreamAcquired";
    case EventType::StreamReleased:
      return "StreamReleased";
    case EventType::Custom:
      return "Custom";
    default:
      return "Unknown";
    }
  }

private:
  std::mutex mutex_;
  SubscriptionId next_id_{1};
  std::unordered_map<EventType,
                     std::vector<std::pair<SubscriptionId, EventHandler>>>
      handlers_;
  std::vector<std::pair<SubscriptionId, EventHandler>> global_handlers_;
};

/// RAII subscription guard
class ScopedSubscription {
public:
  ScopedSubscription(EventSystem &system, SubscriptionId id)
      : system_(system), id_(id) {}

  ~ScopedSubscription() { system_.unsubscribe(id_); }

  // Non-copyable, movable
  ScopedSubscription(const ScopedSubscription &) = delete;
  ScopedSubscription &operator=(const ScopedSubscription &) = delete;
  ScopedSubscription(ScopedSubscription &&other) noexcept
      : system_(other.system_), id_(other.id_) {
    other.id_ = 0;
  }
  ScopedSubscription &operator=(ScopedSubscription &&) = delete;

private:
  EventSystem &system_;
  SubscriptionId id_;
};

} // namespace hts
