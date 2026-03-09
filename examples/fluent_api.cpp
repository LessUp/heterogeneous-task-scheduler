/**
 * @file fluent_api.cpp
 * @brief Demonstrates the fluent builder API for task creation
 */

#include <hts/heterogeneous_task_scheduler.hpp>
#include <iostream>

int main() {
  std::cout << "=== Fluent API Example ===" << std::endl << std::endl;

  hts::Scheduler scheduler;
  hts::TaskBuilder builder(scheduler.graph());

  int result = 0;

  // Create tasks using fluent API
  auto init = builder.name("Initialize")
                  .device(hts::DeviceType::CPU)
                  .priority(hts::TaskPriority::High)
                  .cpu([&](hts::TaskContext &ctx) {
                    std::cout << "Initializing..." << std::endl;
                    result = 10;
                  })
                  .build();

  auto compute = builder.name("Compute")
                     .device(hts::DeviceType::CPU)
                     .after(init)
                     .cpu([&](hts::TaskContext &ctx) {
                       std::cout << "Computing..." << std::endl;
                       result *= 2;
                     })
                     .build();

  auto finalize = builder.name("Finalize")
                      .device(hts::DeviceType::CPU)
                      .priority(hts::TaskPriority::High)
                      .after(compute)
                      .cpu([&](hts::TaskContext &ctx) {
                        std::cout << "Finalizing..." << std::endl;
                        result += 5;
                      })
                      .build();

  std::cout << "Created " << scheduler.graph().size() << " tasks" << std::endl;
  std::cout << "Executing..." << std::endl << std::endl;

  scheduler.execute();

  std::cout << std::endl;
  std::cout << "Result: " << result << " (expected: 25)" << std::endl;
  std::cout << "Test: " << (result == 25 ? "PASS" : "FAIL") << std::endl;

  return (result == 25) ? 0 : 1;
}
