/**
 * @file profiling.cpp
 * @brief Demonstrates the performance profiler
 */

#include <chrono>
#include <hts/heterogeneous_task_scheduler.hpp>
#include <iostream>
#include <thread>

void simulate_work(int ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

int main() {
  std::cout << "=== Profiling Example ===" << std::endl << std::endl;

  hts::Scheduler scheduler;

  // Enable profiling
  scheduler.set_profiling(true);

  // Create a mix of tasks with different durations
  auto fast1 = scheduler.graph().add_task(hts::DeviceType::CPU);
  fast1->set_name("FastTask1");
  fast1->set_cpu_function([](hts::TaskContext &ctx) { simulate_work(10); });

  auto fast2 = scheduler.graph().add_task(hts::DeviceType::CPU);
  fast2->set_name("FastTask2");
  fast2->set_cpu_function([](hts::TaskContext &ctx) { simulate_work(15); });

  auto medium = scheduler.graph().add_task(hts::DeviceType::CPU);
  medium->set_name("MediumTask");
  medium->set_cpu_function([](hts::TaskContext &ctx) { simulate_work(50); });

  auto slow = scheduler.graph().add_task(hts::DeviceType::CPU);
  slow->set_name("SlowTask");
  slow->set_cpu_function([](hts::TaskContext &ctx) { simulate_work(100); });

  auto final_task = scheduler.graph().add_task(hts::DeviceType::CPU);
  final_task->set_name("FinalTask");
  final_task->set_priority(hts::TaskPriority::High);
  final_task->set_cpu_function(
      [](hts::TaskContext &ctx) { simulate_work(20); });

  // Dependencies: fast1, fast2 -> medium -> final
  //               slow -> final
  scheduler.graph().add_dependency(fast1->id(), medium->id());
  scheduler.graph().add_dependency(fast2->id(), medium->id());
  scheduler.graph().add_dependency(medium->id(), final_task->id());
  scheduler.graph().add_dependency(slow->id(), final_task->id());

  std::cout << "Executing " << scheduler.graph().size() << " tasks..."
            << std::endl;
  std::cout << std::endl;

  // Execute
  scheduler.execute();

  // Print profiler report
  std::cout << scheduler.profiler().generate_report() << std::endl;

  // Access summary programmatically
  auto summary = scheduler.profiler().generate_summary();

  std::cout << "=== Programmatic Access ===" << std::endl;
  std::cout << "Completed: " << summary.completed_tasks << "/"
            << summary.total_tasks << std::endl;
  std::cout << "Parallelism: " << summary.parallelism << "x" << std::endl;

  return 0;
}
