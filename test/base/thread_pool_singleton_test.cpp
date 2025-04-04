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
#define CATCH_CONFIG_MAIN  // 让 Catch2 提供 main 函数
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>  // For exception message matching

using namespace toolbox::base;
using Catch::Matchers::ContainsSubstring;  // For checking exception messages

// --- Helper Functions ---

/**
 * @brief 模拟一些计算工作
 * @param milliseconds 模拟工作时间（通过休眠）
 * @return 模拟处理结果（输入值+1）
 */
int simulate_work(int value, int milliseconds)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
  return value + 1;
}

/**
 * @brief 一个会抛出异常的任务函数
 */
void throwing_task()
{
  throw std::runtime_error("Task failed intentionally");
}

// --- Test Cases ---

TEST_CASE("ThreadPoolSingleton Basic Operations",
          "[base][thread_pool_singleton]")
{
  // 获取单例实例
  // 注意：单例在第一次调用 instance() 时初始化并启动。
  // 如果之前的测试已经调用过，这里获取的是同一个实例。
  // 为了隔离测试，理想情况下可能需要一种重置单例状态的方法，但这通常不适用于单例模式。
  // 我们假设每次 TEST_CASE 开始时，可以认为获取到的是一个“可用”的单例。
  auto& pool = thread_pool_singleton_t::instance();

  SECTION("Singleton Instance")
  {
    auto& instance1 = thread_pool_singleton_t::instance();
    auto& instance2 = thread_pool_singleton_t::instance();
    REQUIRE(&instance1 == &instance2);  // 确保每次获取的是同一个实例
  }

  SECTION("Get Thread Count")
  {
    size_t count = pool.get_thread_count();
    REQUIRE(count > 0);  // 线程数应该大于 0
    // 你可以根据需要添加更具体的检查，例如：
    // unsigned int hardware_threads = std::thread::hardware_concurrency();
    // REQUIRE(count == (hardware_threads > 0 ? hardware_threads : 1)); //
    // 假设默认行为
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
    future.get();  // 等待任务完成
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
        pool.get_thread_count() * 2;  // 提交比线程数稍多的任务
    std::vector<std::future<int>> futures;
    futures.reserve(num_tasks);

    for (int i = 0; i < num_tasks; ++i) {
      futures.push_back(pool.submit([](int val) { return val * 2; }, i));
    }

    int sum = 0;
    for (int i = 0; i < num_tasks; ++i) {
      REQUIRE(futures[i].valid());
      int result = futures[i].get();  // 获取结果，会阻塞直到任务完成
      REQUIRE(result == i * 2);
      sum += result;
    }

    // 计算期望的总和: 2 * (0 + 1 + ... + num_tasks-1)
    int expected_sum = 0;
    if (num_tasks > 0) {
      expected_sum = 2 * (num_tasks * (num_tasks - 1) / 2);
    }
    REQUIRE(sum == expected_sum);
  }
}

TEST_CASE("ThreadPoolSingleton Exception Handling",
          "[base][thread_pool_singleton]")
{
  auto& pool = thread_pool_singleton_t::instance();

  SECTION("Submit task that throws")
  {
    auto future = pool.submit(throwing_task);
    REQUIRE(future.valid());
    // 检查 future.get() 是否抛出预期的异常
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

    // 检查结果或异常
    REQUIRE(futures[0].get() == 1);
    REQUIRE_THROWS_AS(futures[1].get(), std::logic_error);
    REQUIRE(futures[2].get() == 3);
  }
}

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
    std::mutex future_mutex;  // 保护 all_futures 的并发访问

    // 提交任务的函数
    auto submit_func = [&](int thread_id)
    {
      std::vector<std::future<void>> local_futures;
      local_futures.reserve(tasks_per_thread);
      for (int i = 0; i < tasks_per_thread; ++i) {
        local_futures.push_back(pool.submit(
            [&counter]()
            {
              // 模拟少量工作并增加计数器
              std::this_thread::sleep_for(std::chrono::microseconds(10));
              counter.fetch_add(1, std::memory_order_relaxed);
            }));
      }
      // 将局部 future 列表合并到全局列表（需要加锁）
      std::lock_guard lock(future_mutex);
      all_futures.insert(all_futures.end(),
                         std::make_move_iterator(local_futures.begin()),
                         std::make_move_iterator(local_futures.end()));
    };

    // 启动提交线程
    submitters.reserve(num_submit_threads);
    for (int i = 0; i < num_submit_threads; ++i) {
      submitters.emplace_back(submit_func, i);
    }

    // 等待所有提交线程完成
    for (auto& t : submitters) {
      t.join();
    }

    // 确保收集到了所有 future
    REQUIRE(all_futures.size() == total_tasks);

    // 等待所有提交到线程池的任务完成
    size_t completed_count = 0;
    for (auto& fut : all_futures) {
      REQUIRE_NOTHROW(fut.get());  // 检查任务是否正常完成（不抛异常）
      completed_count++;
    }

    REQUIRE(completed_count == total_tasks);  // 再次确认所有任务都已等待

    // 验证最终计数器的值
    REQUIRE(counter.load() == total_tasks);
  }
}

// 注意：测试线程池的优雅停止（析构函数行为）比较困难，因为它通常发生在程序退出时。
// 这些测试主要验证其在运行期间的功能。