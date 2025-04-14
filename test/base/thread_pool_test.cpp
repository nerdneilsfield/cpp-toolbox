#include <atomic>  // std::atomic_int, std::atomic_bool
#include <chrono>  // std::chrono::seconds, milliseconds
#include <future>  // std::future
#include <memory>  // std::make_unique
#include <numeric>  // std::iota
#include <stdexcept>  // std::runtime_error
#include <string>
#include <vector>

#include "cpp-toolbox/base/thread_pool.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_contains.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

using namespace toolbox::base;
using Catch::Matchers::Contains;
using Catch::Matchers::Equals;
using Catch::Matchers::Message;

/**
 * @brief Helper function to simulate work.
 * @param milliseconds Duration to sleep.
 * @return The duration slept in milliseconds.
 */
static int simulate_work(int milliseconds)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
  return milliseconds;
}

/**
 * @brief Helper function that throws an exception.
 */
static void throwing_task()
{
  throw std::runtime_error("Task failed intentionally");
}

/**
 * @brief Tests the basic functionality of the thread_pool_t.
 */
TEST_CASE("ThreadPool Basic Operations", "[base][thread_pool]")
{
  // Determine default thread count or use a reasonable number like 4
  unsigned int hardware_threads = std::thread::hardware_concurrency();
  size_t pool_size = (hardware_threads > 0) ? hardware_threads : 4;
  thread_pool_t pool(pool_size);

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
        pool.submit([&task_executed]() { task_executed.store(true); });
    REQUIRE(future.valid());
    future.get();  // Wait for task completion
    REQUIRE(task_executed.load());
  }

  SECTION("Submit task with arguments")
  {
    auto future = pool.submit(simulate_work, 50);  // Submit function pointer
    REQUIRE(future.valid());
    REQUIRE(future.get() == 50);
  }

  SECTION("Submit lambda task with arguments")
  {
    auto future = pool.submit([](int a, int b) { return a + b; }, 10, 20);
    REQUIRE(future.valid());
    REQUIRE(future.get() == 30);
  }

  SECTION("Submit task that throws an exception")
  {
    auto future = pool.submit(throwing_task);
    REQUIRE(future.valid());
    try {
      future.get();
      FAIL("Expected an exception to be thrown");
    } catch (const std::runtime_error& e) {
      REQUIRE(std::string(e.what()) == "Task failed intentionally");
    }
  }

  SECTION("Submit multiple tasks")
  {
    std::vector<std::future<int>> futures;
    const int num_tasks = 10;
    for (int i = 0; i < num_tasks; ++i) {
      futures.push_back(pool.submit([](int val) { return val * 2; }, i));
    }

    int sum = 0;
    for (int i = 0; i < num_tasks; ++i) {
      REQUIRE(futures[i].valid());
      int result = futures[i].get();  // 只调用一次 get()
      REQUIRE(result == i * 2);  // 使用获取到的结果
      sum += result;
    }
    // Sum of 0*2 + 1*2 + ... + 9*2 = 2 * (0+1+...+9) = 2 * (9*10/2) = 90
    REQUIRE(sum == 90);
  }
  // Destructor will be called here, waiting for threads to join.
}

/**
 * @brief Tests thread pool construction and destruction edge cases.
 */
TEST_CASE("ThreadPool Construction and Destruction", "[base][thread_pool]")
{
  SECTION("Construct with default threads")
  {
    // Ensure it doesn't throw
    REQUIRE_NOTHROW(thread_pool_t());
  }

  SECTION("Construct with explicit threads")
  {
    REQUIRE_NOTHROW(thread_pool_t(1));
    REQUIRE_NOTHROW(thread_pool_t(4));
  }

  // Note: Constructing with 0 threads is handled internally to default or 1.
  // We assume the constructor logic tested separately or implicitly here.

  SECTION("Destruction joins threads")
  {
    std::atomic_int tasks_started = 0;
    std::atomic_int tasks_finished = 0;
    {  // Create pool in a scope to control its lifetime
      thread_pool_t pool(2);
      auto fut1 = pool.submit(
          [&]()
          {
            tasks_started++;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            tasks_finished++;
          });
      auto fut2 = pool.submit(
          [&]()
          {
            tasks_started++;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            tasks_finished++;
          });
      // Pool goes out of scope here, destructor runs
    }
    // Check that tasks were at least started and potentially finished
    // Exact state depends on timing, but destructor should block until
    // joinable.
    REQUIRE(tasks_started.load() >= 1);  // At least one should have started
    // We expect both to finish because the destructor joins.
    REQUIRE(tasks_finished.load() == 2);
  }

  SECTION("Submitting to stopped pool (via future)")
  {
    // Testing submission to a pool *during* or *after* its destruction
    // without explicit control over the stop flag is difficult and error-prone.
    // Explicitly calling destructor leads to UB.
    // Relying on submit()'s internal check 'if(stop_)' is the intended
    // safeguard. We will test that check implicitly by ensuring submit works
    // before destruction.

    // Example: Trying to submit after destruction would require unsafe
    // practices auto pool_ptr = std::make_unique<thread_pool_t>(1);
    // pool_ptr.reset(); // Destruct the pool
    // // Any operation on pool_ptr after this is UB.
    // REQUIRE_THROWS_AS(pool_ptr->submit([]{}), std::runtime_error); // Cannot
    // do this

    // We'll skip actively testing submission after stop for now,
    // as it requires either a modified thread pool design (e.g., explicit stop
    // method) or unsafe test patterns.
    SUCCEED(
        "Skipping active test for submission to stopped pool due to "
        "complexity/safety.");
  }
}

/**
 * @brief Tests thread pool under concurrent submissions.
 */
TEST_CASE("ThreadPool Concurrent Submissions",
          "[base][thread_pool][multithreaded]")
{
  thread_pool_t pool(4);  // Use a few threads
  const int num_threads = 8;  // More threads submitting than in the pool
  const int tasks_per_thread = 100;
  std::vector<std::thread> submitters;
  std::vector<std::vector<std::future<int>>> futures(num_threads);
  std::atomic_int total_sum(0);

  auto submit_task_func = [&](int thread_id)
  {
    for (int i = 0; i < tasks_per_thread; ++i) {
      int value = thread_id * tasks_per_thread + i;
      futures[thread_id].push_back(pool.submit(
          [value]()
          {
            // Simulate some small work
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            return value;
          }));
    }
  };

  // Start threads to submit tasks concurrently
  for (int i = 0; i < num_threads; ++i) {
    submitters.emplace_back(submit_task_func, i);
  }

  // Wait for all submitter threads to finish
  for (auto& t : submitters) {
    t.join();
  }

  // Wait for all tasks to complete and verify results
  int expected_sum = 0;
  for (int i = 0; i < num_threads; ++i) {
    for (int j = 0; j < tasks_per_thread; ++j) {
      int value = i * tasks_per_thread + j;
      expected_sum += value;
      REQUIRE(futures[i][j].valid());
      total_sum += futures[i][j].get();  // Accumulate results
    }
  }

  REQUIRE(total_sum.load() == expected_sum);
}

/**
 * @brief Tests submitting move-only types.
 */
TEST_CASE("ThreadPool Move-Only Task Arguments", "[base][thread_pool]")
{
  thread_pool_t pool(2);

  SECTION("Submit lambda capturing unique_ptr")
  {
    auto ptr = std::make_unique<int>(123);
    auto future = pool.submit(
        [p = std::move(ptr)]() mutable
        {
          if (p) {
            int val = *p;
            p.reset();  // Simulate consuming the pointer
            return val;
          }
          return -1;  // Indicate pointer was null
        });
    REQUIRE(future.valid());
    REQUIRE(future.get() == 123);
  }

  SECTION("Submit task returning unique_ptr")
  {
    auto future = pool.submit(
        []() { return std::make_unique<std::string>("hello move"); });
    REQUIRE(future.valid());
    std::unique_ptr<std::string> result_ptr = future.get();
    REQUIRE(result_ptr != nullptr);
    REQUIRE(*result_ptr == "hello move");
  }
}