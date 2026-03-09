/**
 * @file scheduling_policies.cpp
 * @brief Demonstrates different scheduling policies
 */

#include <chrono>
#include <hts/heterogeneous_task_scheduler.hpp>
#include <iostream>
#include <thread>

void run_with_policy(const std::string &name,
                     std::unique_ptr<hts::SchedulingPolicy> policy) {
  std::cout << "--- " << name << " Policy ---" << std::endl;

  hts::Scheduler scheduler;
  scheduler.set_policy(std::move(policy));
  scheduler.set_profiling(true);

  // Create tasks that can run on either CPU or GPU
  for (int i = 0; i < 6; ++i) {
    auto task = scheduler.graph().add_task(hts::DeviceType::Any);
    task->set_name("Task_" + std::to_string(i));

    // Set both CPU and GPU functions
    task->set_cpu_function([i](hts::TaskContext &ctx) {
      std::cout << "  Task " << i << " on CPU" << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
    });

    task->set_gpu_function([i](hts::TaskContext &ctx, cudaStream_t stream) {
      std::cout << "  Task " << i << " on GPU" << std::endl;
      // In real code, this would launch a kernel
    });
  }

  std::cout << "Policy: " << scheduler.policy_name() << std::endl;

  auto start = std::chrono::high_resolution_clock::now();
  scheduler.execute();
  auto end = std::chrono::high_resolution_clock::now();

  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::cout << "Time: " << duration.count() << " ms" << std::endl;

  auto summary = scheduler.profiler().generate_summary();
  std::cout << "CPU utilization: " << summary.cpu_utilization * 100 << "%"
            << std::endl;
  std::cout << "GPU utilization: " << summary.gpu_utilization * 100 << "%"
            << std::endl;
  std::cout << std::endl;
}

int main() {
  std::cout << "=== Scheduling Policies Example ===" << std::endl << std::endl;

  // Test different policies
  run_with_policy("Default", std::make_unique<hts::DefaultSchedulingPolicy>());
  run_with_policy("GPU-First", std::make_unique<hts::GpuFirstPolicy>());
  run_with_policy("CPU-First", std::make_unique<hts::CpuFirstPolicy>());
  run_with_policy("Round-Robin", std::make_unique<hts::RoundRobinPolicy>());

  std::cout << "=== Policy Comparison Complete ===" << std::endl;

  return 0;
}
