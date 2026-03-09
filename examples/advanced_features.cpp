/**
 * @file advanced_features.cpp
 * @brief Demonstrates advanced HTS features: events, barriers, retry policies
 */

#include <hts/event_system.hpp>
#include <hts/heterogeneous_task_scheduler.hpp>
#include <hts/resource_limiter.hpp>
#include <hts/retry_policy.hpp>
#include <hts/task_barrier.hpp>
#include <iomanip>
#include <iostream>
#include <random>

using namespace hts;

// Simulate a task that might fail
void unreliable_task(TaskContext &ctx, int task_num, double failure_rate) {
  static std::mt19937 rng(42);
  std::uniform_real_distribution<double> dist(0.0, 1.0);

  if (dist(rng) < failure_rate) {
    throw std::runtime_error("Transient failure in task " +
                             std::to_string(task_num));
  }

  // Simulate work
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  std::cout << "  Task " << task_num << " completed successfully\n";
}

void demo_event_system() {
  std::cout << "\n=== Event System Demo ===\n\n";

  EventSystem events;

  // Subscribe to task events
  auto sub1 = events.subscribe(EventType::TaskStarted, [](const Event &e) {
    std::cout << "  [EVENT] Task " << e.task_id << " started on "
              << (e.device == DeviceType::CPU ? "CPU" : "GPU") << "\n";
  });

  auto sub2 = events.subscribe(EventType::TaskCompleted, [](const Event &e) {
    std::cout << "  [EVENT] Task " << e.task_id << " completed\n";
  });

  auto sub3 = events.subscribe(EventType::TaskFailed, [](const Event &e) {
    std::cout << "  [EVENT] Task " << e.task_id << " failed: " << e.message
              << "\n";
  });

  // Simulate events
  events.graph_started();
  events.task_started(1, DeviceType::CPU);
  events.task_completed(1, DeviceType::CPU);
  events.task_started(2, DeviceType::GPU);
  events.task_failed(2, "Out of memory");
  events.graph_completed();

  // Unsubscribe
  events.unsubscribe(sub1);
  events.unsubscribe(sub2);
  events.unsubscribe(sub3);

  std::cout << "\n  Event system demo complete.\n";
}

void demo_task_barrier() {
  std::cout << "\n=== Task Barrier Demo ===\n\n";

  Scheduler scheduler;
  auto &graph = scheduler.graph();

  // Create phase 1 tasks
  std::cout << "  Creating phase 1 tasks (data loading)...\n";
  std::vector<std::shared_ptr<Task>> phase1_tasks;
  for (int i = 0; i < 3; ++i) {
    auto task = graph.add_task(DeviceType::CPU);
    task->set_name("load_data_" + std::to_string(i));
    task->set_cpu_function([i](TaskContext &) {
      std::cout << "    Loading data chunk " << i << "\n";
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
    });
    phase1_tasks.push_back(task);
  }

  // Create barrier
  TaskBarrier barrier("phase1_complete", graph);
  for (auto &task : phase1_tasks) {
    barrier.add_predecessor(task);
  }

  // Create phase 2 tasks that depend on barrier
  std::cout << "  Creating phase 2 tasks (processing)...\n";
  std::vector<std::shared_ptr<Task>> phase2_tasks;
  for (int i = 0; i < 2; ++i) {
    auto task = graph.add_task(DeviceType::CPU);
    task->set_name("process_" + std::to_string(i));
    task->set_cpu_function([i](TaskContext &) {
      std::cout << "    Processing batch " << i << "\n";
      std::this_thread::sleep_for(std::chrono::milliseconds(30));
    });
    barrier.add_successor(task);
    phase2_tasks.push_back(task);
  }

  std::cout << "  Executing graph with barrier...\n";
  scheduler.execute();

  std::cout << "\n  Barrier demo complete. All phases executed in order.\n";
}

void demo_retry_policies() {
  std::cout << "\n=== Retry Policy Demo ===\n\n";

  // Demonstrate different retry policies
  std::cout << "  1. Fixed Retry Policy:\n";
  auto fixed = RetryPolicyFactory::fixed(3, std::chrono::milliseconds(100));
  for (size_t i = 0; i < 5; ++i) {
    bool should_retry = fixed->should_retry(i, "error");
    auto delay = fixed->get_delay(i);
    std::cout << "     Attempt " << i
              << ": retry=" << (should_retry ? "yes" : "no")
              << ", delay=" << delay.count() << "ms\n";
  }

  std::cout << "\n  2. Exponential Backoff Policy:\n";
  auto exponential =
      RetryPolicyFactory::exponential(5, std::chrono::milliseconds(100));
  for (size_t i = 0; i < 6; ++i) {
    bool should_retry = exponential->should_retry(i, "error");
    auto delay = exponential->get_delay(i);
    std::cout << "     Attempt " << i
              << ": retry=" << (should_retry ? "yes" : "no")
              << ", delay=" << delay.count() << "ms\n";
  }

  std::cout << "\n  3. Jittered Backoff Policy:\n";
  auto jittered =
      RetryPolicyFactory::jittered(5, std::chrono::milliseconds(100));
  for (size_t i = 0; i < 6; ++i) {
    bool should_retry = jittered->should_retry(i, "error");
    auto delay = jittered->get_delay(i);
    std::cout << "     Attempt " << i
              << ": retry=" << (should_retry ? "yes" : "no")
              << ", delay=" << delay.count() << "ms (with jitter)\n";
  }

  std::cout << "\n  4. Conditional Retry (transient errors only):\n";
  auto conditional =
      ConditionalRetryPolicy::transient_errors(RetryPolicyFactory::fixed(3));

  std::vector<std::string> errors = {"Connection timeout", "Invalid input",
                                     "Temporary failure", "Permission denied"};

  for (const auto &error : errors) {
    bool should_retry = conditional->should_retry(0, error);
    std::cout << "     Error: \"" << error << "\" -> "
              << (should_retry ? "RETRY" : "NO RETRY") << "\n";
  }
}

void demo_resource_limiter() {
  std::cout << "\n=== Resource Limiter Demo ===\n\n";

  ResourceLimiter::Limits limits;
  limits.max_concurrent_cpu_tasks = 2;
  limits.max_concurrent_gpu_tasks = 1;
  limits.max_memory_bytes = 1024 * 1024; // 1 MB

  ResourceLimiter limiter(limits);

  std::cout << "  Limits configured:\n";
  std::cout << "    Max CPU tasks: " << limits.max_concurrent_cpu_tasks << "\n";
  std::cout << "    Max GPU tasks: " << limits.max_concurrent_gpu_tasks << "\n";
  std::cout << "    Max memory: " << limits.max_memory_bytes / 1024
            << " KB\n\n";

  // Test CPU slot acquisition
  std::cout << "  Testing CPU slot acquisition:\n";
  bool slot1 = limiter.acquire_cpu_slot();
  bool slot2 = limiter.acquire_cpu_slot();
  bool slot3 = limiter.acquire_cpu_slot(); // Should fail

  std::cout << "    Slot 1: " << (slot1 ? "acquired" : "denied") << "\n";
  std::cout << "    Slot 2: " << (slot2 ? "acquired" : "denied") << "\n";
  std::cout << "    Slot 3: " << (slot3 ? "acquired" : "denied")
            << " (limit reached)\n";

  limiter.release_cpu_slot();
  bool slot4 = limiter.acquire_cpu_slot();
  std::cout << "    After release, Slot 4: " << (slot4 ? "acquired" : "denied")
            << "\n";

  // Test memory allocation
  std::cout << "\n  Testing memory allocation:\n";
  bool alloc1 = limiter.try_allocate_memory(512 * 1024); // 512 KB
  bool alloc2 = limiter.try_allocate_memory(512 * 1024); // 512 KB
  bool alloc3 = limiter.try_allocate_memory(256 * 1024); // Should fail

  std::cout << "    512 KB: " << (alloc1 ? "allocated" : "denied") << "\n";
  std::cout << "    512 KB: " << (alloc2 ? "allocated" : "denied") << "\n";
  std::cout << "    256 KB: " << (alloc3 ? "allocated" : "denied")
            << " (limit reached)\n";

  limiter.free_memory(512 * 1024);
  bool alloc4 = limiter.try_allocate_memory(256 * 1024);
  std::cout << "    After free, 256 KB: " << (alloc4 ? "allocated" : "denied")
            << "\n";

  // Print stats
  auto stats = limiter.get_stats();
  std::cout << "\n  Current stats:\n";
  std::cout << "    CPU tasks: " << stats.current_cpu_tasks << "\n";
  std::cout << "    Memory: " << stats.current_memory / 1024 << " KB\n";
  std::cout << "    Peak memory: " << stats.peak_memory / 1024 << " KB\n";
}

void demo_semaphore() {
  std::cout << "\n=== Semaphore Demo ===\n\n";

  Semaphore sem(3); // Allow 3 concurrent accesses
  std::atomic<int> active_count{0};
  std::atomic<int> max_active{0};

  std::cout << "  Running 10 tasks with semaphore (max 3 concurrent)...\n";

  std::vector<std::thread> threads;
  for (int i = 0; i < 10; ++i) {
    threads.emplace_back([&sem, &active_count, &max_active, i]() {
      SemaphoreGuard guard(sem);

      int current = ++active_count;
      int expected = max_active.load();
      while (current > expected &&
             !max_active.compare_exchange_weak(expected, current)) {
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      --active_count;
    });
  }

  for (auto &t : threads) {
    t.join();
  }

  std::cout << "  Max concurrent tasks observed: " << max_active.load() << "\n";
  std::cout << "  (Should be <= 3)\n";
}

int main() {
  std::cout << "========================================\n";
  std::cout << "   HTS Advanced Features Demo\n";
  std::cout << "========================================\n";

  try {
    demo_event_system();
    demo_task_barrier();
    demo_retry_policies();
    demo_resource_limiter();
    demo_semaphore();

    std::cout << "\n========================================\n";
    std::cout << "   All demos completed successfully!\n";
    std::cout << "========================================\n";

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
