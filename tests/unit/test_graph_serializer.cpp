#include "hts/graph_serializer.hpp"
#include <gtest/gtest.h>

using namespace hts;

TEST(GraphSerializerTest, EmptyGraph) {
    TaskGraph graph;

    std::string json = GraphSerializer::to_json(graph);

    EXPECT_NE(json.find("\"tasks\": []"), std::string::npos);
    EXPECT_NE(json.find("\"dependencies\": []"), std::string::npos);
}

TEST(GraphSerializerTest, SingleTask) {
    TaskGraph graph;
    auto task = graph.add_task(DeviceType::CPU);
    task->set_name("TestTask");

    std::string json = GraphSerializer::to_json(graph);

    EXPECT_NE(json.find("\"name\": \"TestTask\""), std::string::npos);
    EXPECT_NE(json.find("\"device\": \"CPU\""), std::string::npos);
}

TEST(GraphSerializerTest, WithDependencies) {
    TaskGraph graph;
    auto t1 = graph.add_task();
    auto t2 = graph.add_task();
    graph.add_dependency(t1->id(), t2->id());

    std::string json = GraphSerializer::to_json(graph);

    EXPECT_NE(json.find("\"from\""), std::string::npos);
    EXPECT_NE(json.find("\"to\""), std::string::npos);
}

TEST(GraphSerializerTest, DotFormat) {
    TaskGraph graph;
    auto t1 = graph.add_task(DeviceType::CPU);
    t1->set_name("Task1");
    auto t2 = graph.add_task(DeviceType::GPU);
    t2->set_name("Task2");
    graph.add_dependency(t1->id(), t2->id());

    std::string dot = GraphSerializer::to_dot(graph);

    EXPECT_NE(dot.find("digraph TaskGraph"), std::string::npos);
    EXPECT_NE(dot.find("Task1"), std::string::npos);
    EXPECT_NE(dot.find("Task2"), std::string::npos);
    EXPECT_NE(dot.find("->"), std::string::npos);
}

TEST(GraphSerializerTest, DeviceColors) {
    TaskGraph graph;
    auto cpu = graph.add_task(DeviceType::CPU);
    auto gpu = graph.add_task(DeviceType::GPU);
    auto any = graph.add_task(DeviceType::Any);

    std::string dot = GraphSerializer::to_dot(graph);

    EXPECT_NE(dot.find("lightblue"), std::string::npos);   // CPU
    EXPECT_NE(dot.find("lightgreen"), std::string::npos);  // GPU
    EXPECT_NE(dot.find("lightyellow"), std::string::npos); // Any
}

TEST(GraphSerializerTest, EscapeStrings) {
    TaskGraph graph;
    auto task = graph.add_task();
    task->set_name("Task with \"quotes\" and\nnewline");

    std::string json = GraphSerializer::to_json(graph);

    // Should have escaped quotes
    EXPECT_NE(json.find("\\\""), std::string::npos);
}

TEST(GraphSerializerTest, Priority) {
    TaskGraph graph;
    auto task = graph.add_task();
    task->set_priority(TaskPriority::Critical);

    std::string json = GraphSerializer::to_json(graph);

    EXPECT_NE(json.find("\"priority\": \"Critical\""), std::string::npos);
}
