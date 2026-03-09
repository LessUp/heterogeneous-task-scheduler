/**
 * @file parallel_pipeline.cpp
 * @brief Parallel pipeline processing example
 *
 * Demonstrates a data processing pipeline with parallel stages.
 */

#include <chrono>
#include <cmath>
#include <hts/heterogeneous_task_scheduler.hpp>
#include <iostream>
#include <thread>
#include <vector>

// Simulate some work
void simulate_work(int ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

int main() {
  std::cout << "=== Parallel Pipeline Example ===" << std::endl;

  hts::SchedulerConfig config;
  config.cpu_thread_count = 8;
  hts::Scheduler scheduler(config);

  // Pipeline stages:
  // Stage 1: Load data (1 task)
  // Stage 2: Process chunks in parallel (4 tasks)
  // Stage 3: Aggregate results (1 task)

  std::vector<double> data(1000, 1.0);
  std::vector<double> results(4, 0.0);
  double final_result = 0.0;

  // Stage 1: Load data
  auto load_task = scheduler.graph().add_task(hts::DeviceType::CPU);
  load_task->set_name("Load Data");
  load_task->set_cpu_function([&](hts::TaskContext &ctx) {
    std::cout << "[Load] Loading data..." << std::endl;
    simulate_work(50);
    for (size_t i = 0; i < data.size(); ++i) {
      data[i] = static_cast<double>(i + 1);
    }
    std::cout << "[Load] Data loaded" << std::endl;
  });

  // Stage 2: Process chunks in parallel
  std::vector<std::shared_ptr<hts::Task>> process_tasks;
  for (int chunk = 0; chunk < 4; ++chunk) {
    auto task = scheduler.graph().add_task(hts::DeviceType::CPU);
    task->set_name("Process Chunk " + std::to_string(chunk));

    task->set_cpu_function([&, chunk](hts::TaskContext &ctx) {
      size_t start = chunk * 250;
      size_t end = start + 250;

      std::cout << "[Process " << chunk << "] Processing range [" << start
                << ", " << end << ")" << std::endl;

      simulate_work(100); // Simulate processing

      double sum = 0.0;
      for (size_t i = start; i < end; ++i) {
        sum += std::sqrt(data[i]);
      }
      results[chunk] = sum;

      std::cout << "[Process " << chunk << "] Result: " << sum << std::endl;
    });

    // Depends on load task
    scheduler.graph().add_dependency(load_task->id(), task->id());
    process_tasks.push_back(task);
  }

  // Stage 3: Aggregate results
  auto aggregate_task = scheduler.graph().add_task(hts::DeviceType::CPU);
  aggregate_task->set_name("Aggregate");
  aggregate_task->set_priority(hts::TaskPriority::High);
  aggregate_task->set_cpu_function([&](hts::TaskContext &ctx) {
    std::cout << "[Aggregate] Combining results..." << std::endl;
    simulate_work(30);

    for (double r : results) {
      final_result += r;
    }

    std::cout << "[Aggregate] Final result: " << final_result << std::endl;
  });

  // Aggregate depends on all process tasks
  for (auto &task : process_tasks) {
    scheduler.graph().add_dependency(task->id(), aggregate_task->id());
  }

  std::cout << std::endl;
  std::cout << "Pipeline structure:" << std::endl;
  std::cout << "  Load -> [Process0, Process1, Process2, Process3] -> Aggregate"
            << std::endl;
  std::cout << std::endl;

  auto start = std::chrono::high_resolution_clock::now();
  scheduler.execute();
  auto end = std::chrono::high_resolution_clock::now();

  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  std::cout << std::endl;
  std::cout << "=== Results ===" << std::endl;
  std::cout << "Final result: " << final_result << std::endl;
  std::cout << "Wall time: " << duration.count() << " ms" << std::endl;
  std::cout << "(Sequential would be ~480ms, parallel should be ~250ms)"
            << std::endl;

  // Generate timeline
  std::cout << std::endl;
  std::cout << "=== Timeline JSON ===" << std::endl;
  std::cout << scheduler.generate_timeline_json() << std::endl;

  return 0;
}
