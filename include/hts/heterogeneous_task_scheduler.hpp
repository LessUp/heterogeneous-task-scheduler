#pragma once

/**
 * @file heterogeneous_task_scheduler.hpp
 * @brief Main header for the Heterogeneous Task Scheduler library
 *
 * This library provides a framework for scheduling and executing tasks
 * across CPU and GPU devices. Users define a DAG of tasks with dependencies,
 * and the scheduler automatically manages execution order, device assignment,
 * and memory allocation.
 *
 * Key Features:
 * - DAG-based task dependency management
 * - Automatic cycle detection
 * - GPU memory pool to avoid frequent cudaMalloc/cudaFree
 * - Asynchronous execution with CUDA streams
 * - Load-based device selection
 * - Performance monitoring and timeline generation
 *
 * Example Usage:
 * @code
 * #include <hts/heterogeneous_task_scheduler.hpp>
 *
 * int main() {
 *     hts::Scheduler scheduler;
 *
 *     // Create tasks
 *     auto task1 = scheduler.graph().add_task(hts::DeviceType::CPU);
 *     auto task2 = scheduler.graph().add_task(hts::DeviceType::GPU);
 *     auto task3 = scheduler.graph().add_task(hts::DeviceType::Any);
 *
 *     // Set task functions
 *     task1->set_cpu_function([](hts::TaskContext& ctx) {
 *         // CPU computation
 *     });
 *
 *     task2->set_gpu_function([](hts::TaskContext& ctx, cudaStream_t stream) {
 *         // GPU kernel launch
 *     });
 *
 *     task3->set_cpu_function([](hts::TaskContext& ctx) {
 *         // Can run on CPU
 *     });
 *     task3->set_gpu_function([](hts::TaskContext& ctx, cudaStream_t stream) {
 *         // Or GPU - scheduler decides based on load
 *     });
 *
 *     // Add dependencies: task2 and task3 depend on task1
 *     scheduler.graph().add_dependency(task1->id(), task2->id());
 *     scheduler.graph().add_dependency(task1->id(), task3->id());
 *
 *     // Execute
 *     scheduler.execute();
 *
 *     // Get statistics
 *     auto stats = scheduler.get_stats();
 *     std::cout << "Total time: " << stats.total_time.count() << " ns\n";
 *
 *     return 0;
 * }
 * @endcode
 */

// Core types and enums
#include "hts/types.hpp"

// Task definition
#include "hts/task.hpp"

// Task execution context
#include "hts/task_context.hpp"

// Task graph (DAG) management
#include "hts/task_graph.hpp"

// Dependency tracking
#include "hts/dependency_manager.hpp"

// GPU memory pool
#include "hts/memory_pool.hpp"

// CUDA stream management
#include "hts/stream_manager.hpp"

// Task execution engine
#include "hts/execution_engine.hpp"

// Main scheduler
#include "hts/scheduler.hpp"

// Fluent task builder
#include "hts/task_builder.hpp"

// Task groups
#include "hts/task_group.hpp"

// Scheduling policies
#include "hts/scheduling_policy.hpp"

// Performance profiler
#include "hts/profiler.hpp"

// Graph serialization
#include "hts/graph_serializer.hpp"

// CUDA utilities
#include "hts/cuda_utils.hpp"

// Logging utilities
#include "hts/logger.hpp"

// Task futures for result retrieval
#include "hts/task_future.hpp"

// Task barriers for synchronization
#include "hts/task_barrier.hpp"

// Retry policies
#include "hts/retry_policy.hpp"

// Event system
#include "hts/event_system.hpp"

// Resource limiting
#include "hts/resource_limiter.hpp"

#include <string>

namespace hts {

/**
 * @brief Library version information
 */
constexpr int VERSION_MAJOR = 1;
constexpr int VERSION_MINOR = 2;
constexpr int VERSION_PATCH = 0;

/**
 * @brief Get library version string
 * @return Version string in format "major.minor.patch"
 */
inline std::string get_version() {
    return std::to_string(VERSION_MAJOR) + "." + std::to_string(VERSION_MINOR) + "." +
           std::to_string(VERSION_PATCH);
}

} // namespace hts
