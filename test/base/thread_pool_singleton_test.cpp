// test/base/thread_pool_singleton_test.cpp
#include <atomic>  // std::atomic_int, std::atomic_bool
#include <chrono>  // std::chrono::seconds, milliseconds
#include <future>  // std::future
#include <numeric>  // std::iota
#include <stdexcept>  // std::runtime_error
#include <string>
#include <thread>  // std::thread for concurrency test
#include <vector>

// Include the header for the class under test
#include "cpp-toolbox/base/thread_pool_singleton.hpp"

// Include Catch2 testing framework
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>  // For exception message matching

using namespace toolbox::base;
using Catch::Matchers::ContainsSubstring;  // For checking exception messages

// --- Helper Functions ---

/**
 * @brief Simulates some computational work by sleeping
 * @param value Input value to process
 * @param milliseconds Time to sleep simulating work
 * @return Input value incremented by 1
 *
 * @example
 * int result = simulate_work(100, 5); // Sleeps 5ms and returns 101
 */
static int simulate_work(int value, int milliseconds)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
  return value + 1;
}

/**
 * @brief A task function that intentionally throws an exception
 * @throws std::runtime_error Always throws with message "Task failed
 * intentionally"
 *
 * @example
 * try {
 *   throwing_task();
 * } catch(const std::runtime_error& e) {
 *   // Handle exception
 * }
 */
static void throwing_task()
{
  throw std::runtime_error("Task failed intentionally");
}

// --- Test Cases ---

/**
 * @brief Test case for basic operations of ThreadPoolSingleton
 *
 * Tests the following functionality:
 * - Singleton pattern correctness
 * - Thread count validation
 * - Task submission with return values
 * - Void task submission
 * - Lambda tasks with arguments
 * - Function pointer tasks
 * - Multiple task submission and results verification
 */
TEST_CASE("ThreadPoolSingleton Basic Operations",
          "[base][thread_pool_singleton]")
{
  // Get singleton instance
  // Note: The singleton is initialized and started on first instance() call.
  // If previous tests have called it, we get the same instance.
  // Ideally we'd want a way to reset singleton state between tests,
  // but this typically doesn't work with singleton pattern.
  // We assume each TEST_CASE starts with a "usable" singleton.
  auto& pool = thread_pool_singleton_t::instance();

  SECTION("Singleton Instance")
  {
    auto& instance1 = thread_pool_singleton_t::instance();
    auto& instance2 = thread_pool_singleton_t::instance();
    REQUIRE(&instance1 == &instance2);  // Ensure we get same instance
  }

  SECTION("Get Thread Count")
  {
    size_t count = pool.get_thread_count();
    REQUIRE(count > 0);  // Thread count should be positive
    // Could add more specific checks like:
    // unsigned int hardware_threads = std::thread::hardware_concurrency();
    // REQUIRE(count == (hardware_threads > 0 ? hardware_threads : 1));
  }

  SECTION("Submit simple task returning value")
  {
    auto future = pool.submit([]() { return 42; });
    REQUIRE(future.valid());
    REQUIRE(future.get() == 42);
  }

  SECTION("Submit simple void task")
  {
    std::atomic_bool task_executed = false;
    auto future =
        pool.submit([&task_executed]()
                    { task_executed.store(true, std::memory_order_relaxed); });
    REQUIRE(future.valid());
    future.get();  // Wait for task completion
    REQUIRE(task_executed.load());
  }

  SECTION("Submit task with arguments using lambda")
  {
    auto future = pool.submit([](int a, int b) { return a + b; }, 10, 20);
    REQUIRE(future.valid());
    REQUIRE(future.get() == 30);
  }

  SECTION("Submit task with arguments using function pointer")
  {
    auto future = pool.submit(simulate_work, 100, 5);  // value=100, delay=5ms
    REQUIRE(future.valid());
    REQUIRE(future.get() == 101);  // 100 + 1
  }

  SECTION("Submit multiple tasks and check results")
  {
    const int num_tasks =
        pool.get_thread_count() * 2;  // Submit more tasks than threads
    std::vector<std::future<int>> futures;
    futures.reserve(num_tasks);

    for (int i = 0; i < num_tasks; ++i) {
      futures.push_back(pool.submit([](int val) { return val * 2; }, i));
    }

    int sum = 0;
    for (int i = 0; i < num_tasks; ++i) {
      REQUIRE(futures[i].valid());
      int result = futures[i].get();  // Blocks until task completes
      REQUIRE(result == i * 2);
      sum += result;
    }

    // Calculate expected sum: 2 * (0 + 1 + ... + num_tasks-1)
    int expected_sum = 0;
    if (num_tasks > 0) {
      expected_sum = 2 * (num_tasks * (num_tasks - 1) / 2);
    }
    REQUIRE(sum == expected_sum);
  }
}

/**
 * @brief Test case for exception handling in ThreadPoolSingleton
 *
 * Tests the following functionality:
 * - Tasks that throw exceptions
 * - Multiple tasks with mixed success/failure
 * - Exception propagation through futures
 */
TEST_CASE("ThreadPoolSingleton Exception Handling",
          "[base][thread_pool_singleton]")
{
  auto& pool = thread_pool_singleton_t::instance();

  SECTION("Submit task that throws")
  {
    auto future = pool.submit(throwing_task);
    REQUIRE(future.valid());
    // Check if future.get() throws expected exception
    try {
      future.get();
    } catch (const std::runtime_error& e) {
      REQUIRE(e.what() == std::string("Task failed intentionally"));
    }
  }

  SECTION("Submit multiple tasks, one throws")
  {
    std::vector<std::future<int>> futures;
    futures.push_back(pool.submit([]() { return 1; }));
    futures.push_back(
        pool.submit([]() -> int { throw std::logic_error("Logic error"); }));
    futures.push_back(pool.submit([]() { return 3; }));

    // Check results and exceptions
    REQUIRE(futures[0].get() == 1);
    REQUIRE_THROWS_AS(futures[1].get(), std::logic_error);
    REQUIRE(futures[2].get() == 3);
  }
}

/**
 * @brief Test case for concurrent operations in ThreadPoolSingleton
 *
 * Tests the following functionality:
 * - Multiple threads submitting tasks simultaneously
 * - Task completion verification
 * - Thread safety of submission process
 * - Atomic counter updates from multiple tasks
 */
TEST_CASE("ThreadPoolSingleton Concurrency",
          "[base][thread_pool_singleton][multithreaded]")
{
  auto& pool = thread_pool_singleton_t::instance();

  SECTION("Submit many tasks concurrently from multiple threads")
  {
    const int num_submit_threads = 4;
    const int tasks_per_thread = 250;
    const int total_tasks = num_submit_threads * tasks_per_thread;
    std::atomic_int counter = 0;
    std::vector<std::thread> submitters;
    std::vector<std::future<void>> all_futures;
    std::mutex future_mutex;  // Protects concurrent access to all_futures

    // Task submission function
    auto submit_func = [&](int thread_id)
    {
      std::vector<std::future<void>> local_futures;
      local_futures.reserve(tasks_per_thread);
      for (int i = 0; i < tasks_per_thread; ++i) {
        local_futures.push_back(pool.submit(
            [&counter]()
            {
              // Simulate small amount of work and increment counter
              std::this_thread::sleep_for(std::chrono::microseconds(10));
              counter.fetch_add(1, std::memory_order_relaxed);
            }));
      }
      // Merge local futures into global list (requires lock)
      std::lock_guard lock(future_mutex);
      all_futures.insert(all_futures.end(),
                         std::make_move_iterator(local_futures.begin()),
                         std::make_move_iterator(local_futures.end()));
    };

    // Launch submitter threads
    submitters.reserve(num_submit_threads);
    for (int i = 0; i < num_submit_threads; ++i) {
      submitters.emplace_back(submit_func, i);
    }

    // Wait for all submitter threads to complete
    for (auto& t : submitters) {
      t.join();
    }

    // Ensure we collected all futures
    REQUIRE(all_futures.size() == total_tasks);

    // Wait for all submitted tasks to complete
    size_t completed_count = 0;
    for (auto& fut : all_futures) {
      REQUIRE_NOTHROW(fut.get());  // Check task completed without exceptions
      completed_count++;
    }

    REQUIRE(completed_count == total_tasks);  // Double check all tasks waited

    // Verify final counter value
    REQUIRE(counter.load() == total_tasks);
  }
}

// Note: Testing graceful shutdown (destructor behavior) is difficult
// as it typically happens during program exit.
// These tests mainly verify functionality during runtime.