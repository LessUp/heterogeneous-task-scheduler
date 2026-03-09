/**
 * @file graph_visualization.cpp
 * @brief Demonstrates task graph serialization and visualization
 */

#include <fstream>
#include <hts/graph_serializer.hpp>
#include <hts/heterogeneous_task_scheduler.hpp>
#include <iostream>

int main() {
  std::cout << "=== Graph Visualization Example ===" << std::endl << std::endl;

  hts::Scheduler scheduler;
  hts::TaskBuilder builder(scheduler.graph());

  // Create a complex task graph
  // Stage 1: Data loading (parallel)
  auto load_a = builder.name("Load_A")
                    .device(hts::DeviceType::CPU)
                    .cpu([](hts::TaskContext &ctx) {})
                    .build();

  auto load_b = builder.name("Load_B")
                    .device(hts::DeviceType::CPU)
                    .cpu([](hts::TaskContext &ctx) {})
                    .build();

  // Stage 2: Preprocessing (parallel, depends on loading)
  auto preprocess_a = builder.name("Preprocess_A")
                          .device(hts::DeviceType::GPU)
                          .after(load_a)
                          .gpu([](hts::TaskContext &ctx, cudaStream_t s) {})
                          .build();

  auto preprocess_b = builder.name("Preprocess_B")
                          .device(hts::DeviceType::GPU)
                          .after(load_b)
                          .gpu([](hts::TaskContext &ctx, cudaStream_t s) {})
                          .build();

  // Stage 3: Computation (depends on both preprocessors)
  auto compute = builder.name("Compute")
                     .device(hts::DeviceType::GPU)
                     .priority(hts::TaskPriority::High)
                     .after(preprocess_a)
                     .after(preprocess_b)
                     .gpu([](hts::TaskContext &ctx, cudaStream_t s) {})
                     .build();

  // Stage 4: Post-processing (parallel)
  auto postprocess_1 = builder.name("Postprocess_1")
                           .device(hts::DeviceType::GPU)
                           .after(compute)
                           .gpu([](hts::TaskContext &ctx, cudaStream_t s) {})
                           .build();

  auto postprocess_2 = builder.name("Postprocess_2")
                           .device(hts::DeviceType::CPU)
                           .after(compute)
                           .cpu([](hts::TaskContext &ctx) {})
                           .build();

  // Stage 5: Save results (depends on all post-processing)
  auto save = builder.name("Save_Results")
                  .device(hts::DeviceType::CPU)
                  .priority(hts::TaskPriority::High)
                  .after(postprocess_1)
                  .after(postprocess_2)
                  .cpu([](hts::TaskContext &ctx) {})
                  .build();

  std::cout << "Created task graph with " << scheduler.graph().size()
            << " tasks" << std::endl;
  std::cout << std::endl;

  // Generate JSON
  std::string json = hts::GraphSerializer::to_json(scheduler.graph());
  std::cout << "=== JSON Format ===" << std::endl;
  std::cout << json << std::endl;

  // Save JSON to file
  if (hts::GraphSerializer::save_to_file(scheduler.graph(),
                                         "task_graph.json")) {
    std::cout << "Saved JSON to task_graph.json" << std::endl;
  }

  // Generate DOT
  std::string dot = hts::GraphSerializer::to_dot(scheduler.graph());
  std::cout << "=== DOT Format (for Graphviz) ===" << std::endl;
  std::cout << dot << std::endl;

  // Save DOT to file
  if (hts::GraphSerializer::save_dot_file(scheduler.graph(),
                                          "task_graph.dot")) {
    std::cout << "Saved DOT to task_graph.dot" << std::endl;
    std::cout << "To visualize: dot -Tpng task_graph.dot -o task_graph.png"
              << std::endl;
  }

  // Execute the graph
  std::cout << std::endl;
  std::cout << "Executing task graph..." << std::endl;
  scheduler.execute();

  std::cout << "Execution complete!" << std::endl;

  // Print timeline
  std::cout << std::endl;
  std::cout << "=== Execution Timeline ===" << std::endl;
  std::cout << scheduler.generate_timeline_json() << std::endl;

  return 0;
}
