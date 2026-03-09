/**
 * @file simple_dag.cpp
 * @brief Simple DAG execution example
 *
 * Demonstrates basic task graph construction and execution.
 */

#include <hts/heterogeneous_task_scheduler.hpp>
#include <iostream>
#include <numeric>
#include <vector>

int main() {
  std::cout << "=== Simple DAG Example ===" << std::endl;
  std::cout << "HTS Version: " << hts::get_version() << std::endl << std::endl;

  // Create scheduler with default config
  hts::Scheduler scheduler;

  // Shared data
  std::vector<int> data(1000);
  std::iota(data.begin(), data.end(), 1); // 1, 2, 3, ..., 1000

  int sum1 = 0, sum2 = 0, total = 0;

  // Task 1: Sum first half
  auto task1 = scheduler.graph().add_task(hts::DeviceType::CPU);
  task1->set_name("Sum First Half");
  task1->set_cpu_function([&](hts::TaskContext &ctx) {
    for (int i = 0; i < 500; ++i) {
      sum1 += data[i];
    }
    std::cout << "Task 1: Sum of first 500 = " << sum1 << std::endl;
  });

  // Task 2: Sum second half
  auto task2 = scheduler.graph().add_task(hts::DeviceType::CPU);
  task2->set_name("Sum Second Half");
  task2->set_cpu_function([&](hts::TaskContext &ctx) {
    for (int i = 500; i < 1000; ++i) {
      sum2 += data[i];
    }
    std::cout << "Task 2: Sum of last 500 = " << sum2 << std::endl;
  });

  // Task 3: Combine results (depends on task1 and task2)
  auto task3 = scheduler.graph().add_task(hts::DeviceType::CPU);
  task3->set_name("Combine Results");
  task3->set_priority(hts::TaskPriority::High);
  task3->set_cpu_function([&](hts::TaskContext &ctx) {
    total = sum1 + sum2;
    std::cout << "Task 3: Total sum = " << total << std::endl;
  });

  // Add dependencies: task3 depends on task1 and task2
  scheduler.graph().add_dependency(task1->id(), task3->id());
  scheduler.graph().add_dependency(task2->id(), task3->id());

  // Set error callback
  scheduler.set_error_callback([](hts::TaskId id, const std::string &msg) {
    std::cerr << "Error in task " << id << ": " << msg << std::endl;
  });

  std::cout << "Executing task graph..." << std::endl;
  std::cout << "Graph has " << scheduler.graph().size() << " tasks"
            << std::endl;
  std::cout << std::endl;

  // Execute
  scheduler.execute();

  // Verify result
  int expected = (1000 * 1001) / 2; // Sum of 1 to 1000
  std::cout << std::endl;
  std::cout << "Expected: " << expected << std::endl;
  std::cout << "Got: " << total << std::endl;
  std::cout << "Result: " << (total == expected ? "PASS" : "FAIL") << std::endl;

  // Print statistics
  auto stats = scheduler.get_stats();
  std::cout << std::endl;
  std::cout << "=== Statistics ===" << std::endl;
  std::cout << "Total time: " << stats.total_time.count() / 1e6 << " ms"
            << std::endl;

  for (const auto &[id, time] : stats.task_times) {
    std::cout << "Task " << id << ": " << time.count() / 1e3 << " us"
              << std::endl;
  }

  return (total == expected) ? 0 : 1;
}
