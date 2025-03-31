#include <array>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>

#include "cpp-toolbox/logger/thread_logger.hpp"

#include "cpp-toolbox/macro.hpp"

namespace toolbox::logger
{

auto thread_logger_t::instance() -> thread_logger_t&
{
  static thread_logger_t logger;
  return logger;
}

thread_logger_t::thread_logger_t()
{
  start();
}

thread_logger_t::~thread_logger_t()
{
  stop();
}

void thread_logger_t::start()
{
  running_ = true;
  worker_ = std::thread(&thread_logger_t::processLogs, this);
}

void thread_logger_t::stop()
{
  if (running_) {
    running_ = false;
    cv_.notify_one();
    if (worker_.joinable()) {
      worker_.join();
    }
  }
}

auto thread_logger_t::level_to_string(Level level) -> std::string
{
  switch (level) {
    case Level::TRACE:
      return "[TRACE]";
    case Level::DEBUG:
      return "[DEBUG]";
    case Level::INFO:
      return "[INFO]";
    case Level::WARN:
      return "[WARN]";
    case Level::ERROR:
      return "[ERROR]";
    case Level::CRITICAL:
      return "[CRITICAL]";
  }
}

void thread_logger_t::enqueue(Level level, const std::string& message)
{
  {
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.emplace(level, message);
  }
  cv_.notify_one();
}

void thread_logger_t::processLogs()
{
  constexpr size_t TIME_STR_BUFFER_SIZE = 20;

  while (running_) {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait_for(lock,
                 std::chrono::milliseconds(100),
                 [this] { return !queue_.empty() || !running_; });

    while (!queue_.empty()) {
      auto [level, message] = std::move(queue_.front());
      queue_.pop();
      lock.unlock();

      auto now = std::chrono::system_clock::now();
      auto time = std::chrono::system_clock::to_time_t(now);
      std::array<char, TIME_STR_BUFFER_SIZE> time_str {};
      std::tm local_time_info {};
#if defined(CPP_TOOLBOX_PLATFORM_WINDOWS)
      if (localtime_s(&local_time_info, &time) == 0) {
        if (std::strftime(time_str.data(),
                          time_str.size(),
                          "%Y-%m-%d %H:%M:%S",
                          &local_time_info)
            == 0)
        {
          std::snprintf(
              time_str.data(), time_str.size(), "YYYY-MM-DD HH:MM:SS");
        }
      }
#else
      if (localtime_r(&time, &local_time_info) != nullptr) {
        if (std::strftime(time_str.data(),
                          time_str.size(),
                          "%Y-%m-%d %H:%M:%S",
                          &local_time_info)
            == 0)
        {
          std::snprintf(
              time_str.data(), time_str.size(), "YYYY-MM-DD HH:MM:SS");
        }
      }
#endif

      std::string level_str;
      switch (level) {
        case Level::TRACE:
          level_str = "\033[90m[TRACE]\033[0m";
          break;
        case Level::DEBUG:
          level_str = "\033[36m[DEBUG]\033[0m";
          break;
        case Level::INFO:
          level_str = "\033[32m[INFO]\033[0m";
          break;
        case Level::WARN:
          level_str = "\033[33m[WARN]\033[0m";
          break;
        case Level::ERROR:
          level_str = "\033[31m[ERROR]\033[0m";
          break;
        case Level::CRITICAL:
          level_str = "\033[1;31m[CRITICAL]\033[0m";
          break;
      }

      std::cout << time_str.data() << " " << level_str << " " << message
                << "\n";

      lock.lock();
    }
  }
}

thread_logger_t::thread_format_logger_t::thread_format_logger_t(
    thread_logger_t& logger, Level level)
    : logger_(logger)
    , level_(level)
{
}

[[nodiscard]] auto thread_logger_t::thread_format_logger_t::format_message(
    const char* format) -> std::string
{
  char buffer[2048];
  int result = std::snprintf(buffer, sizeof(buffer), "%s", format);
  if (result < 0 || result >= sizeof(buffer)) {
    return "";  // Handle formatting error
  }
  return {buffer};
}

thread_logger_t::thread_stream_logger_t::thread_stream_logger_t(
    thread_logger_t& logger, Level level)
    : logger_(logger)
    , level_(level)
{
}

thread_logger_t::thread_stream_logger_t::~thread_stream_logger_t()
{
  if (level_ < logger_.level())
    return;
  logger_.enqueue(level_, ss_.str());
}

auto thread_logger_t::thread_stream_logger_t::red(const std::string& text)
    -> thread_stream_logger_t&
{
  ss_ << "\033[31m" << text << "\033[0m";
  return *this;
}

auto thread_logger_t::thread_stream_logger_t::green(const std::string& text)
    -> thread_stream_logger_t&
{
  ss_ << "\033[32m" << text << "\033[0m";
  return *this;
}

auto thread_logger_t::thread_stream_logger_t::yellow(const std::string& text)
    -> thread_stream_logger_t&
{
  ss_ << "\033[33m" << text << "\033[0m";
  return *this;
}

auto thread_logger_t::thread_stream_logger_t::bold(const std::string& text)
    -> thread_stream_logger_t&
{
  ss_ << "\033[1m" << text << "\033[0m";
  return *this;
}

}  // namespace toolbox::logger
