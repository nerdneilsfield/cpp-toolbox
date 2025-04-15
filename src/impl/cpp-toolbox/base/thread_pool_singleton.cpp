// src/impl/cpp-toolbox/base/thread_pool.cpp
#include <algorithm>  // for std::max
#include <chrono>
#include <iostream>  // 临时用于调试输出，最终可用 logger 替代
#include <stdexcept>
#include <thread>
#include <vector>

#include "cpp-toolbox/base/thread_pool_singleton.hpp"

#include "cpp-toolbox/logger/thread_logger.hpp"  // 引入日志

namespace toolbox::base
{

// --- 构造函数实现 ---
thread_pool_singleton_t::thread_pool_singleton_t()
    : stop_(false)
{
  start_pool();  // 在构造时启动线程池
}

// --- 析构函数 ---
thread_pool_singleton_t::~thread_pool_singleton_t()
{
  stop_pool();  // 在单例析构时停止线程池
}

// --- 启动线程池 ---
void thread_pool_singleton_t::start_pool(size_t threads)
{
  size_t num_threads = threads;
  if (num_threads == 0) {
    num_threads = std::thread::hardware_concurrency();
    num_threads = (num_threads == 0) ? 1 : num_threads;  // 保证至少1个线程
  }

  // LOG_INFO_F("Starting thread pool with {} threads.", num_threads);
  // std::cout << "[INFO] Starting thread pool with " << num_threads << "
  // threads."
  //           << std::endl;  // 临时输出

  stop_ = false;  // 确保启动时 stop_ 标志为 false
  workers_.reserve(num_threads);
  for (size_t i = 0; i < num_threads; ++i) {
    workers_.emplace_back(&thread_pool_singleton_t::worker_loop,
                          this,
                          i);  // 传递成员函数指针和 this
  }
}

// --- 停止线程池 ---
void thread_pool_singleton_t::stop_pool()
{
  if (stop_.exchange(true)) {  // 保证只停止一次
    return;
  }
  // LOG_INFO_S << "Stopping thread pool...";
  // std::cout << "[INFO] Stopping thread pool..." << std::endl;  // 临时输出

  // 等待所有工作线程结束
  for (std::thread& worker : workers_) {
    if (worker.joinable()) {
      worker.join();
    }
  }
  workers_.clear();
  // std::cout << "[INFO] Thread pool stopped." << std::endl;  // 临时输出
  // LOG_INFO_S << "Thread pool stopped.";
}

// --- 工作线程循环 (自适应旋转-让步-休眠) ---
void thread_pool_singleton_t::worker_loop(size_t worker_id)
{
  // LOG_DEBUG_F("Worker thread {} started.", worker_id);
  // std::cout << "[DEBUG] Worker thread " << worker_id << " started."
  //           << std::endl;  // 临时输出

  std::function<void()> task;

  // 自适应等待参数 (需要调整)
  const int spin_iterations = 1000;  // 旋转次数
  const int yield_iterations = 10;  // 让步次数
  const auto min_sleep_duration = std::chrono::microseconds(1);  // 最小休眠时间
  const auto max_sleep_duration =
      std::chrono::microseconds(1000);  // 最大休眠时间 (1ms)
  auto current_sleep_duration = min_sleep_duration;

  while (true) {
    bool task_found = false;

    // 1. 尝试立即获取任务
    if (tasks_.try_dequeue(task)) {
      task_found = true;
    } else {
      // 2. 快速旋转尝试
      for (int i = 0; i < spin_iterations; ++i) {
        if (tasks_.try_dequeue(task)) {
          task_found = true;
          break;
        }
// 可选: CPU 暂停指令 (如 _mm_pause on x86) 减少旋转功耗
#if defined(CPP_TOOLBOX_ARCH_X86) || defined(CPP_TOOLBOX_ARCH_X86_64)
#  ifdef _MSC_VER
        _mm_pause();  // MSVC specific intrinsic for x86/x64
#  else
        __builtin_ia32_pause();  // GCC/Clang intrinsic for x86/x64
#  endif
#elif defined(CPP_TOOLBOX_ARCH_ARM) || defined(CPP_TOOLBOX_ARCH_ARM64)
        // ARM YIELD instruction hint (works on ARMv7+, AArch64)
        asm volatile("yield" ::: "memory");
#else
        // 对于其他未知架构，可以什么都不做，或者退化为
        // std::this_thread::yield() (但效果不同) std::this_thread::yield(); //
        // 效果不同，谨慎使用
#endif
      }
    }

    // 3. 如果旋转未果，尝试让步
    if (!task_found) {
      for (int i = 0; i < yield_iterations; ++i) {
        if (tasks_.try_dequeue(task)) {
          task_found = true;
          break;
        }
        std::this_thread::yield();
      }
    }

    // 4. 如果让步未果，检查停止信号或进入休眠
    if (!task_found) {
      if (stop_.load(std::memory_order_acquire)) {
        // 停止前最后一次检查队列
        if (tasks_.try_dequeue(task)) {
          task_found = true;  // 执行最后一个任务
        } else {
          // LOG_DEBUG_F("Worker thread {} stopping.", worker_id);
          //      std::cout << "[DEBUG] Worker thread " << worker_id << "
          //      stopping." << std::endl; // 临时输出
          return;  // 退出循环
        }
      } else {
        // 进入休眠 (指数退避)
        std::this_thread::sleep_for(current_sleep_duration);
        current_sleep_duration =
            std::min(max_sleep_duration, current_sleep_duration * 2);
        // 继续下一次循环尝试获取任务
        continue;  // 跳过任务执行，直接开始下一次循环的尝试
      }
    }

    // --- 执行任务 ---
    if (task_found && task) {  // 确保 task 有效
      // LOG_TRACE_F("Worker thread {} executing task.", worker_id);
      try {
        task();
      } catch (const std::exception& e) {
        //  std::cerr << "Worker thread " << worker_id << " caught exception: "
        //  << e.what() << std::endl;
        LOG_ERROR_F(
            "Worker thread {} caught exception: {}", worker_id, e.what());
      } catch (...) {
        //  std::cerr << "Worker thread " << worker_id << " caught unknown
        //  exception." << std::endl;
        LOG_ERROR_F("Worker thread {} caught unknown exception.", worker_id);
      }
      task = nullptr;  // 释放 std::function 持有的资源

      // 成功执行任务后，重置休眠时间
      current_sleep_duration = min_sleep_duration;
    } else if (task_found && !task) {
      //      std::cerr << "Worker thread " << worker_id << " dequeued an empty
      //      task wrapper." << std::endl;
      LOG_WARN_F("Worker thread {} dequeued an empty task wrapper.", worker_id);
    }
  }  // end while(true)
}

}  // namespace toolbox::base