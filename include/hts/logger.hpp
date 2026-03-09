#pragma once

#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>

namespace hts {

/// Log level enumeration
enum class LogLevel { Debug = 0, Info = 1, Warning = 2, Error = 3, None = 4 };

/// Simple thread-safe logger
class Logger {
public:
  using LogCallback = std::function<void(LogLevel, const std::string &)>;

  /// Get singleton instance
  static Logger &instance() {
    static Logger logger;
    return logger;
  }

  /// Set minimum log level
  void set_level(LogLevel level) { level_ = level; }

  /// Get current log level
  LogLevel level() const { return level_; }

  /// Set custom log callback
  void set_callback(LogCallback callback) { callback_ = std::move(callback); }

  /// Log a message
  void log(LogLevel level, const std::string &message) {
    if (level < level_)
      return;

    std::lock_guard<std::mutex> lock(mutex_);

    if (callback_) {
      callback_(level, message);
    } else {
      auto now = std::chrono::system_clock::now();
      auto time = std::chrono::system_clock::to_time_t(now);

      std::tm tm_buf{};
#ifdef _WIN32
      localtime_s(&tm_buf, &time);
#else
      localtime_r(&time, &tm_buf);
#endif

      std::ostream &out = (level >= LogLevel::Warning) ? std::cerr : std::cout;
      out << "[" << std::put_time(&tm_buf, "%H:%M:%S") << "] "
          << level_string(level) << ": " << message << std::endl;
    }
  }

  /// Log with format
  template <typename... Args>
  void log(LogLevel level, const char *fmt, Args &&...args) {
    if (level < level_)
      return;

    std::ostringstream oss;
    format_impl(oss, fmt, std::forward<Args>(args)...);
    log(level, oss.str());
  }

private:
  Logger() = default;

  static const char *level_string(LogLevel level) {
    switch (level) {
    case LogLevel::Debug:
      return "DEBUG";
    case LogLevel::Info:
      return "INFO";
    case LogLevel::Warning:
      return "WARN";
    case LogLevel::Error:
      return "ERROR";
    default:
      return "UNKNOWN";
    }
  }

  void format_impl(std::ostringstream &oss, const char *fmt) { oss << fmt; }

  template <typename T, typename... Args>
  void format_impl(std::ostringstream &oss, const char *fmt, T &&val,
                   Args &&...args) {
    while (*fmt) {
      if (*fmt == '{' && *(fmt + 1) == '}') {
        oss << std::forward<T>(val);
        format_impl(oss, fmt + 2, std::forward<Args>(args)...);
        return;
      }
      oss << *fmt++;
    }
  }

  LogLevel level_ = LogLevel::Info;
  LogCallback callback_;
  std::mutex mutex_;
};

// Convenience macros
#define HTS_LOG_DEBUG(msg, ...)                                                \
  hts::Logger::instance().log(hts::LogLevel::Debug, msg, ##__VA_ARGS__)
#define HTS_LOG_INFO(msg, ...)                                                 \
  hts::Logger::instance().log(hts::LogLevel::Info, msg, ##__VA_ARGS__)
#define HTS_LOG_WARN(msg, ...)                                                 \
  hts::Logger::instance().log(hts::LogLevel::Warning, msg, ##__VA_ARGS__)
#define HTS_LOG_ERROR(msg, ...)                                                \
  hts::Logger::instance().log(hts::LogLevel::Error, msg, ##__VA_ARGS__)

} // namespace hts
