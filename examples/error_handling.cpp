/**
 * @file error_handling.cpp
 * @brief Error handling and failure propagation example
 *
 * Demonstrates how errors are handled and propagated through the task graph.
 */

#include <hts/heterogeneous_task_scheduler.hpp>
#include <iostream>
#include <stdexcept>

int main() {
  std::cout << "=== Error Handling Example ===" << std::endl << std::endl;

  hts::Scheduler scheduler;

  // Track which tasks executed
  bool task1_ran = false;
  bool task2_ran = false;
  bool task3_ran = false;
  bool task4_ran = false;

  // Task 1: Will succeed
  auto task1 = scheduler.graph().add_task(hts::DeviceType::CPU);
  task1->set_name("Task 1 (Success)");
  task1->set_cpu_function([&](hts::TaskContext &ctx) {
    std::cout << "Task 1: Running..." << std::endl;
    task1_ran = true;
    std::cout << "Task 1: Completed successfully" << std::endl;
  });

  // Task 2: Will fail
  auto task2 = scheduler.graph().add_task(hts::DeviceType::CPU);
  task2->set_name("Task 2 (Failure)");
  task2->set_cpu_function([&](hts::TaskContext &ctx) {
    std::cout << "Task 2: Running..." << std::endl;
    task2_ran = true;
    throw std::runtime_error("Simulated failure in Task 2");
  });

  // Task 3: Depends on Task 2 (will be blocked)
  auto task3 = scheduler.graph().add_task(hts::DeviceType::CPU);
  task3->set_name("Task 3 (Blocked)");
  task3->set_cpu_function([&](hts::TaskContext &ctx) {
    std::cout << "Task 3: Running..." << std::endl;
    task3_ran = true;
    std::cout << "Task 3: Completed" << std::endl;
  });

  // Task 4: Independent (will succeed)
  auto task4 = scheduler.graph().add_task(hts::DeviceType::CPU);
  task4->set_name("Task 4 (Independent)");
  task4->set_cpu_function([&](hts::TaskContext &ctx) {
    std::cout << "Task 4: Running..." << std::endl;
    task4_ran = true;
    std::cout << "Task 4: Completed successfully" << std::endl;
  });

  // Dependencies:
  // Task 1 -> Task 2 -> Task 3
  // Task 4 is independent
  scheduler.graph().add_dependency(task1->id(), task2->id());
  scheduler.graph().add_dependency(task2->id(), task3->id());

  // Set error callback
  std::vector<std::pair<hts::TaskId, std::string>> errors;
  scheduler.set_error_callback([&](hts::TaskId id, const std::string &msg) {
    errors.push_back({id, msg});
    std::cout << "ERROR CALLBACK: Task " << id << " failed: " << msg
              << std::endl;
  });

  std::cout << "Graph structure:" << std::endl;
  std::cout << "  Task1 -> Task2 -> Task3" << std::endl;
  std::cout << "  Task4 (independent)" << std::endl;
  std::cout << std::endl;

  // Execute
  scheduler.execute();

  std::cout << std::endl;
  std::cout << "=== Results ===" << std::endl;
  std::cout << "Task 1 ran: " << (task1_ran ? "yes" : "no") << std::endl;
  std::cout << "Task 2 ran: " << (task2_ran ? "yes" : "no") << std::endl;
  std::cout << "Task 3 ran: " << (task3_ran ? "yes" : "no")
            << " (should be blocked)" << std::endl;
  std::cout << "Task 4 ran: " << (task4_ran ? "yes" : "no") << std::endl;

  std::cout << std::endl;
  std::cout << "Task states:" << std::endl;
  std::cout << "  Task 1: " << static_cast<int>(task1->state())
            << " (Completed=4)" << std::endl;
  std::cout << "  Task 2: " << static_cast<int>(task2->state()) << " (Failed=5)"
            << std::endl;
  std::cout << "  Task 3: " << static_cast<int>(task3->state())
            << " (Blocked=6)" << std::endl;
  std::cout << "  Task 4: " << static_cast<int>(task4->state())
            << " (Completed=4)" << std::endl;

  std::cout << std::endl;
  std::cout << "Errors received: " << errors.size() << std::endl;
  for (const auto &[id, msg] : errors) {
    std::cout << "  Task " << id << ": " << msg << std::endl;
  }

  // Verify expected behavior
  bool success = task1_ran && task2_ran && !task3_ran && task4_ran;
  std::cout << std::endl;
  std::cout << "Test: " << (success ? "PASS" : "FAIL") << std::endl;

  return success ? 0 : 1;
}
