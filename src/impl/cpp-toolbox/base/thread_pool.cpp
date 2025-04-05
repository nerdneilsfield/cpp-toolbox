#include <chrono>  // For std::chrono::milliseconds
#include <iostream>  // Optional, for debug output
#include <stdexcept>  // For std::invalid_argument

#include "cpp-toolbox/base/thread_pool.hpp"  // Include header file

namespace toolbox::base
{

/**
 * @brief Constructs a thread pool with specified number of threads
 * @param threads Number of threads to create. If 0, uses hardware concurrency
 * @throws std::invalid_argument if thread count is 0 after all fallbacks
 */
thread_pool_t::thread_pool_t(size_t threads)
    : stop_(false)
{
  // If thread count is 0, try to get hardware concurrency
  size_t num_threads = threads;
  if (num_threads == 0) {
    num_threads = std::thread::hardware_concurrency();
    // If hardware concurrency cannot be determined or is 0, use at least 1
    // thread
    if (num_threads == 0) {
      num_threads = 1;
    }
  }

  if (num_threads == 0) {  // Should never reach here, but defensive programming
    throw std::invalid_argument("Thread pool cannot have 0 threads");
  }

  // Reserve space and create worker threads
  workers_.reserve(num_threads);
  for (size_t i = 0; i < num_threads; ++i) {
    workers_.emplace_back(
           [this, i] { // Worker thread lambda function
                std::unique_ptr<detail::task_base> task_ptr; // Stores dequeued task wrapper
                while(true) {
                    bool task_dequeued = this->tasks_.try_dequeue(task_ptr);

                    if (task_dequeued) {
                        if (task_ptr) { // Check if the unique_ptr is valid
                            try {
                                task_ptr->execute(); // Execute via virtual dispatch
                            } catch (const std::exception& e) {
                                std::cerr << "Worker thread " << i << " caught exception during task execution: " << e.what() << std::endl;
                            } catch (...) {
                                std::cerr << "Worker thread " << i << " caught unknown exception during task execution." << std::endl;
                            }
                        }
                        task_ptr.reset(); // Reset unique_ptr, deleting the task object
                    } else {
                        // Queue is empty, check stop condition or yield/sleep
                        if (this->stop_.load(std::memory_order_acquire)) {
                          // Double check queue
                          if (!this->tasks_.try_dequeue(task_ptr)) {
                              return; // Exit lambda function
                          } else {
                              // Execute final task
                              if (task_ptr) {
                                    try { task_ptr->execute(); } catch (...) { /* Handle */ }
                              }
                              task_ptr.reset();
                          }
                        } else {
                            std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Or yield
                        }
                    }
                }
              }
      );
  }
}

/**
 * @brief Destructor that stops all threads and waits for them to finish
 */
thread_pool_t::~thread_pool_t()
{
  // Set stop flag with release semantics
  stop_.store(true, std::memory_order_release);

  // Wait for all worker threads to finish their current tasks and exit
  for (std::thread& worker : workers_) {
    if (worker.joinable()) {  // Ensure thread is joinable
      worker.join();
    }
  }
  // Note: No explicit queue cleanup needed as concurrent_queue destructor
  // handles it
}

}  // namespace toolbox::base