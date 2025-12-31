/**
 * @file task_groups.cpp
 * @brief Demonstrates TaskGroup for batch task management
 */

#include <hts/heterogeneous_task_scheduler.hpp>
#include <iostream>
#include <vector>
#include <numeric>

int main() {
    std::cout << "=== Task Groups Example ===" << std::endl << std::endl;
    
    hts::Scheduler scheduler;
    
    // Create data
    std::vector<double> data(1000);
    std::iota(data.begin(), data.end(), 1.0);
    
    std::vector<double> chunk_results(4, 0.0);
    double final_result = 0.0;
    
    // Stage 1: Initialize (single task)
    auto init = scheduler.graph().add_task(hts::DeviceType::CPU);
    init->set_name("Initialize");
    init->set_cpu_function([&](hts::TaskContext& ctx) {
        std::cout << "Initializing data..." << std::endl;
    });
    
    // Stage 2: Process chunks (task group)
    hts::TaskGroup process_group("ProcessChunks", scheduler.graph());
    
    for (int i = 0; i < 4; ++i) {
        auto task = process_group.add_task(hts::DeviceType::CPU);
        task->set_name("Process_" + std::to_string(i));
        task->set_cpu_function([&, i](hts::TaskContext& ctx) {
            size_t start = i * 250;
            size_t end = start + 250;
            
            double sum = 0.0;
            for (size_t j = start; j < end; ++j) {
                sum += data[j];
            }
            chunk_results[i] = sum;
            
            std::cout << "Chunk " << i << " sum: " << sum << std::endl;
        });
    }
    
    // All process tasks depend on init
    process_group.depends_on(init);
    
    // Stage 3: Aggregate (single task)
    auto aggregate = scheduler.graph().add_task(hts::DeviceType::CPU);
    aggregate->set_name("Aggregate");
    aggregate->set_priority(hts::TaskPriority::High);
    aggregate->set_cpu_function([&](hts::TaskContext& ctx) {
        for (double r : chunk_results) {
            final_result += r;
        }
        std::cout << "Final result: " << final_result << std::endl;
    });
    
    // Aggregate depends on all process tasks
    process_group.then(aggregate);
    
    std::cout << "Task group '" << process_group.name() << "' has " 
              << process_group.size() << " tasks" << std::endl;
    std::cout << "Total tasks: " << scheduler.graph().size() << std::endl;
    std::cout << std::endl;
    
    // Execute
    scheduler.execute();
    
    // Verify
    double expected = (1000.0 * 1001.0) / 2.0;
    std::cout << std::endl;
    std::cout << "Expected: " << expected << std::endl;
    std::cout << "Got: " << final_result << std::endl;
    std::cout << "Test: " << (final_result == expected ? "PASS" : "FAIL") << std::endl;
    
    // Check group status
    std::cout << std::endl;
    std::cout << "Group completed: " << process_group.completed_count() 
              << "/" << process_group.size() << std::endl;
    std::cout << "All completed: " << (process_group.all_completed() ? "yes" : "no") << std::endl;
    std::cout << "Any failed: " << (process_group.any_failed() ? "yes" : "no") << std::endl;
    
    return (final_result == expected) ? 0 : 1;
}
