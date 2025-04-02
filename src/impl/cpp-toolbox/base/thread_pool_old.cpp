#include "cpp-toolbox/base/thread_pool.hpp" // 包含头文件
#include <stdexcept>                      // 用于 std::invalid_argument
#include <iostream>                       // 可选，用于调试输出
#include <chrono>                         // 用于 std::chrono::milliseconds

namespace toolbox::base
{

// 构造函数实现
thread_pool_t::thread_pool_t(size_t threads) : stop_(false) {
    // 如果传入的线程数为0，则尝试获取硬件并发核心数
    size_t num_threads = threads;
    if (num_threads == 0) {
        num_threads = std::thread::hardware_concurrency();
        // 如果硬件并发核心数也无法获取或为0，则至少保证1个线程
        if (num_threads == 0) {
            num_threads = 1;
        }
    }

    if (num_threads == 0) { // 理论上不会执行到这里了，但作为防御
         throw std::invalid_argument("线程池的线程数不能为0");
    }

    // 预留空间并创建工作线程
    workers_.reserve(num_threads);
    for(size_t i = 0; i < num_threads; ++i) {
        workers_.emplace_back(
            [this, i] { // 工作线程执行的 lambda 函数 - Capture thread index 'i' for logging
                std::function<void()> task; // 用于存储从队列取出的任务
                std::cout << "Worker thread " << i << " started." << std::endl;

                while(true) {
                    // 尝试从无锁队列中获取任务
                    bool task_dequeued = this->tasks_.try_dequeue(task);

                    if (task_dequeued) {
                        std::cout << "Worker thread " << i << " dequeued a task." << std::endl;
                        // 执行任务
                        try {
                            std::cout << "Worker thread " << i << " executing task..." << std::endl;
                            task(); // Execute the packaged_task wrapper
                            std::cout << "Worker thread " << i << " finished task." << std::endl;
                        } catch (const std::exception& e) {
                             std::cerr << "Worker thread " << i << " caught exception: " << e.what() << std::endl;
                        } catch (...) {
                             std::cerr << "Worker thread " << i << " caught unknown exception." << std::endl;
                        }
                        // Reset the task function object after execution might help release resources,
                        // though the packaged_task itself is managed by shared_ptr.
                        task = nullptr;
                    } else {
                        // 队列为空，检查是否需要停止
                        if (this->stop_.load(std::memory_order_acquire)) { // Acquire a barrier with stop_
                           std::cout << "Worker thread " << i << " detected stop signal." << std::endl;
                           // 再次检查队列，以防在 stop_ 设置后又有任务入队
                           if (!this->tasks_.try_dequeue(task)) {
                               std::cout << "Worker thread " << i << " stopping. Cleaning up HP." << std::endl;
                               // 清理 Hazard Pointer 资源
                               toolbox::container::lock_free_queue_t<std::function<void()>>::cleanup_this_thread_retired_nodes();
                               return; // 退出 lambda 函数，线程结束
                           } else {
                               std::cout << "Worker thread " << i << " dequeued final task after stop signal." << std::endl;
                               // 如果在停止后仍然取到了任务，则执行它
                               try {
                                   std::cout << "Worker thread " << i << " executing final task..." << std::endl;
                                   task();
                                   std::cout << "Worker thread " << i << " finished final task." << std::endl;
                               } catch (const std::exception& e) {
                                   std::cerr << "Worker thread " << i << " caught exception (final task): " << e.what() << std::endl;
                               } catch (...) {
                                   std::cerr << "Worker thread " << i << " caught unknown exception (final task)." << std::endl;
                               }
                               // 执行完这个任务后，将会在下一次循环检查停止条件并退出
                           }
                        }

                        // 如果不需要停止且队列为空，则 yield 或 sleep
                        // std::cout << "Worker thread " << i << " queue empty, sleeping." << std::endl;
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    }
                }
            }
        );
    }
}

// 析构函数实现
thread_pool_t::~thread_pool_t() {
    // 设置停止标志
    // 使用 release 语义确保停止标志对所有线程可见
    stop_.store(true, std::memory_order_release);

    // 不再需要 notify_all

    // 等待所有工作线程完成其当前任务并退出
    for(std::thread &worker: workers_) {
        if(worker.joinable()) { // 确保线程是可加入的
            worker.join();
        }
    }
    // 注意：这里不再显式清理队列，因为 lock_free_queue 的析构函数会处理
    // 但最好确保所有线程都已退出且调用了 cleanup_this_thread_retired_nodes
}

} // namespace toolbox::base