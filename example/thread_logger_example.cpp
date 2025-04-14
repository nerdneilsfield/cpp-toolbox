#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "cpp-toolbox/logger/thread_logger.hpp"

void worker_thread(int id)
{
  // Each thread logs messages
  LOG_TRACE_S << "Worker thread " << id << " started.";
  for (int i = 0; i < 5; ++i) {
    LOG_INFO_F("Worker thread {} logging message {}/5", id, i + 1);
    std::this_thread::sleep_for(
        std::chrono::milliseconds(10 * id + 5));  // Simulate work
  }
  LOG_WARN_S << "Worker thread " << id << " finished.";
}

int main()
{
  std::cerr << "[Example Main] Getting logger instance first time...\n";
  auto& logger = toolbox::logger::thread_logger_t::instance();
  std::cerr << "[Example Main] Logger instance ready.\n";

  // Set the logging level (e.g., TRACE to see everything)
  logger.set_level(toolbox::logger::thread_logger_t::Level::TRACE);
  std::cerr << "[Example Main] Log level set to TRACE.\n";

  LOG_CRITICAL_F(
      "This is a critical message from main thread using format {}, {}",
      1,
      "test");
  LOG_ERROR_S << "This is an error message from main thread using stream."
              << " Value: " << 123;
  LOG_INFO_S << "Starting worker threads...";

  std::vector<std::thread> threads;
  const int num_threads = 4;
  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back(worker_thread, i + 1);
  }

  std::cerr << "[Example Main] Waiting for worker threads to join...\n";
  for (auto& t : threads) {
    if (t.joinable()) {
      t.join();
    }
  }

  LOG_INFO_S << "All worker threads finished.";

  // Add a small delay before exiting main.
  // This gives the logger's background thread some time to process
  // any remaining messages before the static logger instance is destroyed.
  std::cerr << "[Example Main] Main thread finished work, sleeping briefly "
               "before exit...\n";
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  std::cerr << "[Example Main] Exiting main function.\n";
  // The static thread_logger_t instance will be destroyed after this point.
  return 0;
}