#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <utility>

#include "cpp-toolbox/logger/thread_logger.hpp"

#include "cpp-toolbox/macro.hpp"

namespace toolbox::logger
{

// Initialize static members for the singleton
std::atomic<thread_logger_t*> thread_logger_t::instance_ptr_ = nullptr;
std::mutex thread_logger_t::instance_mutex_;
std::atomic<bool> thread_logger_t::shutdown_called_ = false;

auto thread_logger_t::instance() -> thread_logger_t&
{
  // Double-checked locking pattern for thread-safe initialization
  thread_logger_t* ptr = instance_ptr_.load(std::memory_order_acquire);
  if (ptr == nullptr) {  // First check (no lock)
    std::lock_guard<std::mutex> lock(instance_mutex_);
    ptr = instance_ptr_.load(
        std::memory_order_relaxed);  // Second check (with lock)
    if (ptr == nullptr) {
      // fprintf(stderr,
      //         "[Logger Diag] Creating new logger instance (dynamic "
      //         "allocation)...\n");
      ptr = new thread_logger_t();  // Allocate instance
      instance_ptr_.store(ptr, std::memory_order_release);
      // Optional: Register an atexit handler to call shutdown? Risky due to
      // order. std::atexit([](){ thread_logger_t::shutdown(); });
    }
  }
  // fprintf(stderr, "[Logger Diag] Returning logger instance reference.\n"); //
  // Can be noisy
  return *ptr;
}

thread_logger_t::thread_logger_t()
    : level_(Level::INFO)  // Explicitly initialize default level
    , stringstream_pool_(10,  // Initial pool size (e.g., 10)
                         [](std::stringstream& ss) {  // Resetter lambda
                           ss.str("");  // Clear content
                           ss.clear();  // Clear error flags
                         })
{
  // fprintf(stderr,
  //         "[Logger Diag] thread_logger_t constructor starting (dynamic "
  //         "instance).\n");
  // Don't call start() immediately, let instance() handle it once?
  // Or call start() here is fine as constructor is called only once.
  start();
  // fprintf(stderr, "[Logger Diag] thread_logger_t constructor finished.\n");
}

void thread_logger_t::start()
{
  // fprintf(stderr, "[Logger Diag] start() called.\n");
  running_ = true;
  worker_ = std::thread(&thread_logger_t::processLogs, this);
  // fprintf(stderr, "[Logger Diag] Worker thread started.\n");
}

void thread_logger_t::stop()
{
  // fprintf(stderr, "[Logger Diag] stop() called.\n");
  if (running_) {
    running_ = false;
    // fprintf(stderr, "[Logger Diag] running_ set to false.\n");
    if (worker_.joinable()) {
      // fprintf(stderr, "[Logger Diag] Joining worker thread...\n");
      worker_.join();
      // fprintf(stderr, "[Logger Diag] Worker thread joined.\n");
    } else {
      // fprintf(stderr, "[Logger Diag] Worker thread was not joinable.\n");
    }
  } else {
    // fprintf(stderr, "[Logger Diag] stop() called but already stopped.\n");
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
    case Level::ERR:
      return "[ERROR]";
    case Level::CRITICAL:
      return "[CRITICAL]";
    default:
      // Handle unexpected level values
      return "[UNKNOWN]";
  }
}

void thread_logger_t::enqueue(Level level, std::string message)
{
  queue_.enqueue(std::make_pair(level, std::move(message)));
}

void thread_logger_t::processLogs()
{
  // fprintf(stderr,
  //         "[Logger Diag] processLogs() worker thread started execution.\n");
  constexpr size_t TIME_STR_BUFFER_SIZE = 20;
  const auto wait_timeout = std::chrono::milliseconds(100);

  while (running_) {
    std::pair<Level, std::string> log_entry_value;

    // Wait for an item with timeout
    if (queue_.wait_dequeue_timed(log_entry_value, wait_timeout)) {
      // If dequeue successful, process the log entry
      auto& [level, message] = log_entry_value;  // Use structured binding

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
        case Level::ERR:
          level_str = "\033[31m[ERROR]\033[0m";
          break;
        case Level::CRITICAL:
          level_str = "\033[1;31m[CRITICAL]\033[0m";
          break;
      }

      fprintf(stderr,
              "%s %s %s\n",
              time_str.data(),
              level_str.c_str(),
              message.c_str());
    } else {
      // wait_dequeue_timed returned false (timeout)
      if (!running_) {
        // fprintf(stderr,
        //         "[Logger Diag] processLogs() timeout occurred and running is
        //         " "false, breaking loop.\n");
        break;
      }
    }
    // Check running flag at the end of each iteration too
    if (!running_) {
      // fprintf(stderr,
      //         "[Logger Diag] processLogs() running is false at end of loop, "
      //         "breaking.\n");
      break;
    }
  }
  // fprintf(stderr,
  //         "[Logger Diag] processLogs() worker thread loop finished. Exiting "
  //         "thread function.\n");
}

thread_logger_t::thread_format_logger_t::thread_format_logger_t(
    thread_logger_t& logger, Level level)
    : logger_(logger)
    , level_(level)
{
}

thread_logger_t::thread_stream_logger_t::thread_stream_logger_t(
    thread_logger_t& logger,
    Level level,
    toolbox::base::object_pool_t<std::stringstream>& pool)
    : logger_(logger)
    , level_(level)
    , ss_ptr_(pool.acquire())  // Acquire stringstream from pool
{
  // Constructor body is now simpler
}

thread_logger_t::thread_stream_logger_t::~thread_stream_logger_t()
{
  // Only enqueue if level is sufficient
  if (level_ >= logger_.level() && ss_ptr_) {  // Check ss_ptr_ validity
    std::string msg = ss_ptr_->str();
    if (!msg.empty()) {  // Avoid enqueuing empty messages
      logger_.enqueue(level_, std::move(msg));
    }
  }
  // ss_ptr_ destructor automatically calls PoolDeleter, which calls
  // pool.release() No explicit release needed here.
}

auto thread_logger_t::thread_stream_logger_t::red(const std::string& text)
    -> thread_stream_logger_t&
{
  if (ss_ptr_)
    *ss_ptr_ << "\033[31m" << text << "\033[0m";
  return *this;
}

auto thread_logger_t::thread_stream_logger_t::green(const std::string& text)
    -> thread_stream_logger_t&
{
  if (ss_ptr_)
    *ss_ptr_ << "\033[32m" << text << "\033[0m";
  return *this;
}

auto thread_logger_t::thread_stream_logger_t::yellow(const std::string& text)
    -> thread_stream_logger_t&
{
  if (ss_ptr_)
    *ss_ptr_ << "\033[33m" << text << "\033[0m";
  return *this;
}

auto thread_logger_t::thread_stream_logger_t::bold(const std::string& text)
    -> thread_stream_logger_t&
{
  if (ss_ptr_)
    *ss_ptr_ << "\033[1m" << text << "\033[0m";
  return *this;
}

// Implementation of the static shutdown method
void thread_logger_t::shutdown()
{
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  // Use atomic flag to ensure shutdown logic runs only once
  bool already_called =
      shutdown_called_.exchange(true, std::memory_order_acq_rel);
  if (already_called) {
    // fprintf(stderr, "[Logger Diag] shutdown() called more than once. Ignoring
    // subsequent calls.\n");
    return;
  }

  // fprintf(stderr, "[Logger Diag] shutdown() called for the first time.\n");

  // Get the instance pointer safely
  thread_logger_t* instance = instance_ptr_.load(std::memory_order_acquire);

  if (instance != nullptr) {
    // fprintf(stderr, "[Logger Diag] Instance exists, calling stop()...\n");
    instance->stop();  // Call the instance's stop method
    // Optional: Consider deleting the dynamically allocated instance here?
    // delete instance;
    // instance_ptr_.store(nullptr, std::memory_order_release);
    // Deleting here might be problematic if other static objects try to log
    // during exit. Leaking the singleton might be safer in complex shutdown
    // scenarios.
  } else {
    // fprintf(stderr, "[Logger Diag] shutdown() called but instance was never
    // created or already destroyed.\n");
  }
  // fprintf(stderr, "[Logger Diag] shutdown() finished.\n");
}

}  // namespace toolbox::logger
