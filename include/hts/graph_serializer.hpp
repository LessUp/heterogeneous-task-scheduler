#pragma once

#include "hts/task_graph.hpp"
#include "hts/types.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace hts {

/// Serializes task graph structure to JSON format
class GraphSerializer {
  public:
    /// Serialize task graph to JSON string
    static std::string to_json(const TaskGraph &graph) {
        std::ostringstream oss;
        oss << "{\n";
        oss << "  \"version\": \"1.0\",\n";

        const auto &tasks = graph.tasks();
        if (tasks.empty()) {
            oss << "  \"tasks\": [],\n";
        } else {
            oss << "  \"tasks\": [\n";
            size_t i = 0;
            for (const auto &[id, task] : tasks) {
                oss << "    {\n";
                oss << "      \"id\": " << id << ",\n";
                oss << "      \"name\": \"" << escape_string(task->name()) << "\",\n";
                oss << "      \"device\": \"" << device_to_string(task->preferred_device())
                    << "\",\n";
                oss << "      \"priority\": \"" << priority_to_string(task->priority()) << "\",\n";
                oss << "      \"has_cpu\": " << (task->has_cpu_function() ? "true" : "false")
                    << ",\n";
                oss << "      \"has_gpu\": " << (task->has_gpu_function() ? "true" : "false")
                    << "\n";
                oss << "    }";
                if (++i < tasks.size())
                    oss << ",";
                oss << "\n";
            }
            oss << "  ],\n";
        }

        std::vector<std::pair<TaskId, TaskId>> deps;
        for (const auto &[id, task] : tasks) {
            auto successors = graph.get_successors(id);
            for (const auto &succ : successors) {
                deps.push_back({id, succ->id()});
            }
        }

        if (deps.empty()) {
            oss << "  \"dependencies\": []\n";
        } else {
            oss << "  \"dependencies\": [\n";
            for (size_t j = 0; j < deps.size(); ++j) {
                oss << "    {\"from\": " << deps[j].first << ", \"to\": " << deps[j].second << "}";
                if (j < deps.size() - 1)
                    oss << ",";
                oss << "\n";
            }
            oss << "  ]\n";
        }
        oss << "}\n";

        return oss.str();
    }

    /// Save task graph to file
    static bool save_to_file(const TaskGraph &graph, const std::string &filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        file << to_json(graph);
        return true;
    }

    /// Generate DOT format for visualization
    static std::string to_dot(const TaskGraph &graph) {
        std::ostringstream oss;
        oss << "digraph TaskGraph {\n";
        oss << "  rankdir=TB;\n";
        oss << "  node [shape=box, style=filled];\n\n";

        const auto &tasks = graph.tasks();

        // Nodes
        for (const auto &[id, task] : tasks) {
            std::string color = "lightblue";
            if (task->preferred_device() == DeviceType::GPU) {
                color = "lightgreen";
            } else if (task->preferred_device() == DeviceType::Any) {
                color = "lightyellow";
            }

            std::string label = task->name().empty() ? "Task " + std::to_string(id) : task->name();

            oss << "  task" << id << " [label=\"" << label << "\\n"
                << device_to_string(task->preferred_device()) << "\", fillcolor=" << color
                << "];\n";
        }

        oss << "\n";

        // Edges
        for (const auto &[id, task] : tasks) {
            auto successors = graph.get_successors(id);
            for (const auto &succ : successors) {
                oss << "  task" << id << " -> task" << succ->id() << ";\n";
            }
        }

        oss << "}\n";
        return oss.str();
    }

    /// Save DOT file for Graphviz visualization
    static bool save_dot_file(const TaskGraph &graph, const std::string &filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        file << to_dot(graph);
        return true;
    }

  private:
    static std::string escape_string(const std::string &s) {
        std::string result;
        for (char c : s) {
            if (c == '"')
                result += "\\\"";
            else if (c == '\\')
                result += "\\\\";
            else if (c == '\n')
                result += "\\n";
            else
                result += c;
        }
        return result;
    }

    static const char *device_to_string(DeviceType device) {
        switch (device) {
        case DeviceType::CPU:
            return "CPU";
        case DeviceType::GPU:
            return "GPU";
        case DeviceType::Any:
            return "Any";
        default:
            return "Unknown";
        }
    }

    static const char *priority_to_string(TaskPriority priority) {
        switch (priority) {
        case TaskPriority::Low:
            return "Low";
        case TaskPriority::Normal:
            return "Normal";
        case TaskPriority::High:
            return "High";
        case TaskPriority::Critical:
            return "Critical";
        default:
            return "Unknown";
        }
    }
};

} // namespace hts
