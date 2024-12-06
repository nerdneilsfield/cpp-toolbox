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

auto ThreadLogger::instance() -> ThreadLogger&
{
  static ThreadLogger logger;
  return logger;
}

ThreadLogger::ThreadLogger()
{
  start();
}

ThreadLogger::~ThreadLogger()
{
  stop();
}

void ThreadLogger::start()
{
  running_ = true;
  worker_ = std::thread(&ThreadLogger::processLogs, this);
}

void ThreadLogger::stop()
{
  if (running_) {
    running_ = false;
    cv_.notify_one();
    if (worker_.joinable()) {
      worker_.join();
    }
  }
}

auto ThreadLogger::level_to_string(Level level) -> std::string
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

void ThreadLogger::enqueue(Level level, const std::string& message)
{
  {
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.emplace(level, message);
  }
  cv_.notify_one();
}

void ThreadLogger::processLogs()
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

ThreadLogger::ThreadFormatLogger::ThreadFormatLogger(ThreadLogger& logger,
                                                     Level level)
    : logger_(logger)
    , level_(level)
{
}

[[nodiscard]] auto ThreadLogger::ThreadFormatLogger::format_message(
    const char* format) -> std::string
{
  char buffer[2048];
  int result = std::snprintf(buffer, sizeof(buffer), "%s", format);
  if (result < 0 || result >= sizeof(buffer)) {
    return "";  // Handle formatting error
  }
  return {buffer};
}

ThreadLogger::ThreadStreamLogger::ThreadStreamLogger(ThreadLogger& logger,
                                                     Level level)
    : logger_(logger)
    , level_(level)
{
}

ThreadLogger::ThreadStreamLogger::~ThreadStreamLogger()
{
  if (level_ < logger_.level())
    return;
  logger_.enqueue(level_, ss_.str());
}

auto ThreadLogger::ThreadStreamLogger::red(const std::string& text)
    -> ThreadStreamLogger&
{
  ss_ << "\033[31m" << text << "\033[0m";
  return *this;
}

auto ThreadLogger::ThreadStreamLogger::green(const std::string& text)
    -> ThreadStreamLogger&
{
  ss_ << "\033[32m" << text << "\033[0m";
  return *this;
}

auto ThreadLogger::ThreadStreamLogger::yellow(const std::string& text)
    -> ThreadStreamLogger&
{
  ss_ << "\033[33m" << text << "\033[0m";
  return *this;
}

auto ThreadLogger::ThreadStreamLogger::bold(const std::string& text)
    -> ThreadStreamLogger&
{
  ss_ << "\033[1m" << text << "\033[0m";
  return *this;
}

}  // namespace toolbox::logger
