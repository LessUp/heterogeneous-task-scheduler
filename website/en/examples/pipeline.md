# Pipeline Example

This example demonstrates a more complex data processing pipeline with error handling, retry logic, and production-ready patterns.

## Overview

We'll build a ML inference pipeline:

```
Data Ingestion (CPU)
    ├──► Feature Engineering (CPU)
    │       ├──► Model A: Image Classification (GPU)
    │       └──► Model B: Text Classification (GPU)
    └──► Data Validation (CPU)
            └──► Results Aggregation (CPU)
                    └──► Save to Database (CPU)
```

## Complete Code

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>

using namespace hts;

// ============================================================================
// Task Functions
// ============================================================================

struct PipelineData {
    std::vector<float> raw_data;
    std::vector<float> features;
    std::string image_result;
    std::string text_result;
    bool validated = false;
    std::string final_output;
};

// Shared data pointer
auto pipeline_data = std::make_shared<PipelineData>();

void ingest_data(TaskContext& ctx) {
    std::cout << "[1/7] Ingesting data from source..." << std::endl;
    
    // Simulate data loading
    pipeline_data->raw_data.resize(10000);
    for (size_t i = 0; i < pipeline_data->raw_data.size(); ++i) {
        pipeline_data->raw_data[i] = static_cast<float>(i) / 10000.0f;
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    std::cout << "  ✓ Loaded " << pipeline_data->raw_data.size() << " data points" << std::endl;
}

void engineer_features(TaskContext& ctx) {
    std::cout << "[2/7] Engineering features..." << std::endl;
    
    // Simulate feature extraction
    pipeline_data->features.resize(1000);
    for (size_t i = 0; i < pipeline_data->features.size(); ++i) {
        pipeline_data->features[i] = pipeline_data->raw_data[i * 10] * 2.0f + 1.0f;
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << "  ✓ Extracted " << pipeline_data->features.size() << " features" << std::endl;
}

void validate_data(TaskContext& ctx) {
    std::cout << "[2/7b] Validating data quality..." << std::endl;
    
    // Simulate validation
    bool is_valid = true;
    for (const auto& val : pipeline_data->raw_data) {
        if (std::isnan(val) || std::isinf(val)) {
            is_valid = false;
            break;
        }
    }
    
    pipeline_data->validated = is_valid;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    if (is_valid) {
        std::cout << "  ✓ Data validation passed" << std::endl;
    } else {
        std::cout << "  ✗ Data validation failed" << std::endl;
        throw std::runtime_error("Invalid data detected");
    }
}

void run_image_model(TaskContext& ctx, cudaStream_t stream) {
    std::cout << "[3/7a] Running image classification model on GPU..." << std::endl;
    
    // Simulate GPU kernel execution
    // In real code:
    // float* d_features;
    // cudaMallocAsync(&d_features, features.size() * sizeof(float), stream);
    // cudaMemcpyAsync(d_features, features.data(), ..., stream);
    // image_classifier<<<blocks, threads, 0, stream>>>(d_features, d_output);
    // cudaMemcpyAsync(output.data(), d_output, ..., stream);
    // cudaFreeAsync(d_features, stream);
    
    cudaStreamSynchronize(stream);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    pipeline_data->image_result = "Image: Cat (confidence: 0.95)";
    std::cout << "  ✓ " << pipeline_data->image_result << std::endl;
}

void run_text_model(TaskContext& ctx, cudaStream_t stream) {
    std::cout << "[3/7b] Running text classification model on GPU..." << std::endl;
    
    // Simulate GPU kernel execution
    cudaStreamSynchronize(stream);
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    
    pipeline_data->text_result = "Text: Positive sentiment (confidence: 0.87)";
    std::cout << "  ✓ " << pipeline_data->text_result << std::endl;
}

void aggregate_results(TaskContext& ctx) {
    std::cout << "[5/7] Aggregating results..." << std::endl;
    
    // Combine results from both models
    pipeline_data->final_output = "Pipeline Results:\n";
    pipeline_data->final_output += "  " + pipeline_data->image_result + "\n";
    pipeline_data->final_output += "  " + pipeline_data->text_result + "\n";
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "  ✓ Results aggregated" << std::endl;
}

void save_to_database(TaskContext& ctx) {
    std::cout << "[6/7] Saving results to database..." << std::endl;
    
    // Simulate database write
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << "  ✓ Results saved successfully" << std::endl;
}

void cleanup_resources(TaskContext& ctx) {
    std::cout << "[7/7] Cleaning up resources..." << std::endl;
    
    // Cleanup any allocated resources
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::cout << "  ✓ Cleanup complete" << std::endl;
}

// ============================================================================
// Error Handler
// ============================================================================

void task_failure_handler(Task* failed_task) {
    std::cerr << "\n✗ Task failed: " << failed_task->get_name() << std::endl;
    std::cerr << "  Error: " << failed_task->get_error_message() << std::endl;
    std::cerr << "  Retries: " << failed_task->get_retry_count() << std::endl;
    
    // Log to monitoring system
    // metrics::increment("pipeline.task_failures", {{"task", failed_task->get_name()}});
}

// ============================================================================
// Main Pipeline
// ============================================================================

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "ML Inference Pipeline Example" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // Step 1: Create task graph and builder
    TaskGraph graph;
    TaskBuilder builder(graph);
    
    // Step 2: Create all tasks with configuration
    
    // Data ingestion
    auto ingest_task = builder
        .create_task("DataIngestion")
        .device(DeviceType::CPU)
        .cpu_func(ingest_data)
        .priority(10)
        .tag("io")
        .build();
    
    // Feature engineering
    auto feature_task = builder
        .create_task("FeatureEngineering")
        .device(DeviceType::CPU)
        .cpu_func(engineer_features)
        .priority(8)
        .tag("cpu-bound")
        .build();
    
    // Data validation (runs in parallel with feature engineering)
    auto validate_task = builder
        .create_task("DataValidation")
        .device(DeviceType::CPU)
        .cpu_func(validate_data)
        .priority(9)
        .build();
    
    // GPU models (run in parallel)
    auto image_model_task = builder
        .create_task("ImageClassification")
        .device(DeviceType::GPU)
        .gpu_func(run_image_model)
        .priority(15)
        .memory(512 * 1024 * 1024)  // 512 MB
        .retry_policy(RetryPolicy{
            .max_retries = 2,
            .backoff_ms = 200,
            .backoff_multiplier = 2.0f
        })
        .build();
    
    auto text_model_task = builder
        .create_task("TextClassification")
        .device(DeviceType::GPU)
        .gpu_func(run_text_model)
        .priority(15)
        .memory(256 * 1024 * 1024)  // 256 MB
        .retry_policy(RetryPolicy{
            .max_retries = 2,
            .backoff_ms = 200,
            .backoff_multiplier = 2.0f
        })
        .build();
    
    // Results aggregation
    auto aggregate_task = builder
        .create_task("ResultsAggregation")
        .device(DeviceType::CPU)
        .cpu_func(aggregate_results)
        .priority(8)
        .build();
    
    // Database save
    auto save_task = builder
        .create_task("SaveToDatabase")
        .device(DeviceType::CPU)
        .cpu_func(save_to_database)
        .priority(10)
        .tag("io")
        .build();
    
    // Cleanup task (always runs)
    auto cleanup_task = builder
        .create_task("CleanupResources")
        .device(DeviceType::CPU)
        .cpu_func(cleanup_resources)
        .priority(5)
        .build();
    
    // Step 3: Register failure handlers for all tasks
    for (auto* task : {ingest_task, feature_task, validate_task, 
                       image_model_task, text_model_task, 
                       aggregate_task, save_task, cleanup_task}) {
        task->on_failure(task_failure_handler);
    }
    
    // Step 4: Set up dependencies (create the pipeline DAG)
    
    // Linear path: Ingest → Feature Engineering → GPU Models → Aggregate → Save → Cleanup
    graph.add_dependency(ingest_task->id(), feature_task->id());
    graph.add_dependency(feature_task->id(), image_model_task->id());
    graph.add_dependency(feature_task->id(), text_model_task->id());
    graph.add_dependency(image_model_task->id(), aggregate_task->id());
    graph.add_dependency(text_model_task->id(), aggregate_task->id());
    graph.add_dependency(aggregate_task->id(), save_task->id());
    graph.add_dependency(save_task->id(), cleanup_task->id());
    
    // Parallel validation: runs after ingest, independent of feature engineering
    graph.add_dependency(ingest_task->id(), validate_task->id());
    graph.add_dependency(validate_task->id(), aggregate_task->id());
    
    /*
        Pipeline DAG:
        
        DataIngestion ──► FeatureEngineering ──┐
              │                                │
              └► DataValidation ───────────────┼► ResultsAggregation ──► SaveToDatabase ──► Cleanup
                                               │
              ┌────────────────────────────────┘
              │
              ├► ImageClassification (GPU) ────┘
              │
              └► TextClassification (GPU) ─────┘
    */
    
    // Step 5: Validate the graph
    Error err = graph.validate();
    if (!err.ok()) {
        std::cerr << "\n✗ Graph validation failed: " << err.message() << std::endl;
        return 1;
    }
    
    std::cout << "Pipeline Configuration:" << std::endl;
    std::cout << "  Tasks: " << graph.num_tasks() << std::endl;
    std::cout << "  Dependencies: " << graph.num_dependencies() << std::endl;
    
    auto critical_path = graph.get_critical_path();
    std::cout << "  Critical path: " << critical_path.size() << " tasks" << std::endl;
    
    // Step 6: Configure scheduler
    Scheduler scheduler;
    
    SchedulerConfig config;
    config.cpu_thread_count = 8;
    config.gpu_stream_count = 4;
    config.enable_profiling = true;
    config.stop_on_first_error = false;  // Continue even if some tasks fail
    scheduler.configure(config);
    
    // Use load-based policy for balanced execution
    scheduler.set_policy(std::make_unique<LoadBasedPolicy>());
    
    // Step 7: Initialize and execute
    err = scheduler.init(&graph);
    if (!err.ok()) {
        std::cerr << "\n✗ Scheduler initialization failed: " << err.message() << std::endl;
        return 1;
    }
    
    std::cout << "\nExecuting pipeline..." << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    scheduler.execute();
    scheduler.wait_for_completion();
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time
    );
    
    // Step 8: Print results
    std::cout << "\n----------------------------------------" << std::endl;
    std::cout << "Pipeline Execution Complete!" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    std::cout << "Timing:" << std::endl;
    std::cout << "  Total time: " << duration.count() << " ms" << std::endl;
    
    const auto& stats = scheduler.get_stats();
    std::cout << "\nStatistics:" << std::endl;
    std::cout << "  Tasks scheduled: " << stats.tasks_scheduled << std::endl;
    std::cout << "  CPU tasks: " << stats.cpu_tasks << std::endl;
    std::cout << "  GPU tasks: " << stats.gpu_tasks << std::endl;
    std::cout << "  Failed tasks: " << stats.failed_tasks << std::endl;
    std::cout << "  Parallelism factor: " << stats.parallelism_factor << "x" << std::endl;
    std::cout << "  Average wait time: " << stats.avg_wait_time_ms << " ms" << std::endl;
    std::cout << "  Average execution time: " << stats.avg_execution_time_ms << " ms" << std::endl;
    
    // Print final results
    std::cout << "\nFinal Results:" << std::endl;
    std::cout << pipeline_data->final_output << std::endl;
    
    // Print error statistics if any
    const auto& error_stats = scheduler.get_error_stats();
    if (error_stats.total_failures > 0) {
        std::cout << "\nError Statistics:" << std::endl;
        std::cout << "  Total failures: " << error_stats.total_failures << std::endl;
        std::cout << "  Successful retries: " << error_stats.successful_retries << std::endl;
        std::cout << "  Permanent failures: " << error_stats.permanent_failures << std::endl;
    }
    
    return 0;
}
```

## Expected Output

```
========================================
ML Inference Pipeline Example
========================================

Pipeline Configuration:
  Tasks: 8
  Dependencies: 9
  Critical path: 6 tasks

Executing pipeline...
----------------------------------------
[1/7] Ingesting data from source...
  ✓ Loaded 10000 data points
[2/7b] Validating data quality...
  ✓ Data validation passed
[2/7] Engineering features...
  ✓ Extracted 1000 features
[3/7a] Running image classification model on GPU...
[3/7b] Running text classification model on GPU...
  ✓ Text: Positive sentiment (confidence: 0.87)
  ✓ Image: Cat (confidence: 0.95)
[5/7] Aggregating results...
  ✓ Results aggregated
[6/7] Saving results to database...
  ✓ Results saved successfully
[7/7] Cleaning up resources...
  ✓ Cleanup complete

----------------------------------------
Pipeline Execution Complete!
========================================

Timing:
  Total time: 1250 ms

Statistics:
  Tasks scheduled: 8
  CPU tasks: 6
  GPU tasks: 2
  Failed tasks: 0
  Parallelism factor: 1.8x
  Average wait time: 125 ms
  Average execution time: 169 ms

Final Results:
Pipeline Results:
  Image: Cat (confidence: 0.95)
  Text: Positive sentiment (confidence: 0.87)
```

## Key Features Demonstrated

### 1. Complex DAG Structure

Multiple parallel branches that merge:

```cpp
// Two GPU models run in parallel
graph.add_dependency(feature_task->id(), image_model_task->id());
graph.add_dependency(feature_task->id(), text_model_task->id());

// Both must complete before aggregation
graph.add_dependency(image_model_task->id(), aggregate_task->id());
graph.add_dependency(text_model_task->id(), aggregate_task->id());
```

### 2. Retry Policies

GPU tasks have automatic retry:

```cpp
.retry_policy(RetryPolicy{
    .max_retries = 2,
    .backoff_ms = 200,
    .backoff_multiplier = 2.0f
})
```

### 3. Failure Handlers

All tasks register failure callbacks:

```cpp
task->on_failure(task_failure_handler);
```

### 4. Load Balancing

Using `LoadBasedPolicy` to balance CPU and GPU workloads:

```cpp
scheduler.set_policy(std::make_unique<LoadBasedPolicy>());
```

### 5. Production Configuration

```cpp
config.stop_on_first_error = false;  // Continue on errors
config.enable_profiling = true;       // Collect metrics
```

## Building and Running

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./pipeline_example
```

## Next Steps

- [Simple DAG Example](/en/examples/simple-dag) — Basic DAG example
- [Error Handling Guide](/en/guide/error-handling) — Error handling patterns
- [Scheduling Guide](/en/guide/scheduling) — Scheduling policies
