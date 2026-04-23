#include "hts/logger.hpp"
#include <gtest/gtest.h>
#include <sstream>
#include <vector>

using namespace hts;

TEST(LoggerTest, SetLevel) {
    auto &logger = Logger::instance();

    logger.set_level(LogLevel::Debug);
    EXPECT_EQ(logger.level(), LogLevel::Debug);

    logger.set_level(LogLevel::Error);
    EXPECT_EQ(logger.level(), LogLevel::Error);

    // Reset to default
    logger.set_level(LogLevel::Info);
}

TEST(LoggerTest, CustomCallback) {
    auto &logger = Logger::instance();

    std::vector<std::pair<LogLevel, std::string>> logs;

    logger.set_callback(
        [&](LogLevel level, const std::string &msg) { logs.push_back({level, msg}); });

    logger.set_level(LogLevel::Debug);

    logger.log(LogLevel::Debug, "Debug message");
    logger.log(LogLevel::Info, "Info message");
    logger.log(LogLevel::Warning, "Warning message");
    logger.log(LogLevel::Error, "Error message");

    EXPECT_EQ(logs.size(), 4);
    EXPECT_EQ(logs[0].first, LogLevel::Debug);
    EXPECT_EQ(logs[0].second, "Debug message");
    EXPECT_EQ(logs[3].first, LogLevel::Error);

    // Reset
    logger.set_callback(nullptr);
    logger.set_level(LogLevel::Info);
}

TEST(LoggerTest, LevelFiltering) {
    auto &logger = Logger::instance();

    std::vector<std::string> logs;

    logger.set_callback([&](LogLevel level, const std::string &msg) { logs.push_back(msg); });

    logger.set_level(LogLevel::Warning);

    logger.log(LogLevel::Debug, "Debug");
    logger.log(LogLevel::Info, "Info");
    logger.log(LogLevel::Warning, "Warning");
    logger.log(LogLevel::Error, "Error");

    EXPECT_EQ(logs.size(), 2); // Only Warning and Error
    EXPECT_EQ(logs[0], "Warning");
    EXPECT_EQ(logs[1], "Error");

    // Reset
    logger.set_callback(nullptr);
    logger.set_level(LogLevel::Info);
}

TEST(LoggerTest, FormatString) {
    auto &logger = Logger::instance();

    std::string captured;

    logger.set_callback([&](LogLevel level, const std::string &msg) { captured = msg; });

    logger.log(LogLevel::Info, "Value: {}, Name: {}", 42, "test");

    EXPECT_EQ(captured, "Value: 42, Name: test");

    // Reset
    logger.set_callback(nullptr);
}

TEST(LoggerTest, Macros) {
    auto &logger = Logger::instance();

    std::vector<LogLevel> levels;

    logger.set_callback([&](LogLevel level, const std::string &msg) { levels.push_back(level); });

    logger.set_level(LogLevel::Debug);

    HTS_LOG_DEBUG("debug");
    HTS_LOG_INFO("info");
    HTS_LOG_WARN("warn");
    HTS_LOG_ERROR("error");

    EXPECT_EQ(levels.size(), 4);
    EXPECT_EQ(levels[0], LogLevel::Debug);
    EXPECT_EQ(levels[1], LogLevel::Info);
    EXPECT_EQ(levels[2], LogLevel::Warning);
    EXPECT_EQ(levels[3], LogLevel::Error);

    // Reset
    logger.set_callback(nullptr);
    logger.set_level(LogLevel::Info);
}
