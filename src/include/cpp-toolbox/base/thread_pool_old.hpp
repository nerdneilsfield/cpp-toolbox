#pragma once

#ifdef CPP_TOOLBOX_OLD_THREAD_POOL

#include <vector>               // 用于存储工作线程
// #include <queue>             // 不再需要标准队列
#include <thread>               // C++ 线程库
// #include <mutex>             // 不再需要互斥锁
// #include <condition_variable> // 不再需要条件变量
#include <future>               // 用于异步获取任务结果 std::future, std::packaged_task
#include <functional>           // 用于 std::function, std::bind
#include <stdexcept>            // 用于运行时异常
#include <atomic>               // 用于原子布尔标志
#include <type_traits>          // 用于 std::invoke_result_t
#include <utility>              // 用于 std::forward, std::move
#include <memory>               // 用于 std::make_shared
#include <iostream>             // 用于 std::cout, std::cerr

// 包含 lock_free_queue
#include "cpp-toolbox/container/lock_free_queue.hpp"
// 假设你的导出宏定义在这里
#include <cpp-toolbox/cpp-toolbox_export.hpp>
// 假设你的宏定义文件在这里
#include <cpp-toolbox/macro.hpp>

namespace toolbox::base
{

/**
 * @brief 一个使用无锁队列的高性能 C++17 线程池实现。
 *
 * 该线程池允许提交任务并异步获取结果。
 * 它在构造时创建固定数量的工作线程，并在析构时优雅地停止它们。
 * 使用 `toolbox::container::lock_free_queue_t` 作为底层任务队列。
 */
class CPP_TOOLBOX_EXPORT thread_pool_t {
public:
    /**
     * @brief 构造函数，初始化并启动线程池。
     *
     * @param threads 要创建的工作线程数量。默认为硬件并发核心数。
     * 如果硬件并发核心数无法确定或为0，则至少创建1个线程。
     * @throws std::invalid_argument 如果线程数参数为0。
     */
    explicit thread_pool_t(size_t threads = 0);

    /**
     * @brief 析构函数，停止线程池并等待所有工作线程结束。
     *
     * 会等待队列中已被线程取出的任务执行完毕。
     * 析构后，线程池将不再可用。
     */
    ~thread_pool_t();

    /**
     * @brief 提交一个任务到线程池执行。
     *
     * @tparam F 任务函数的类型。
     * @tparam Args 任务函数参数的类型。
     * @param f 任务函数（可以是函数指针、lambda表达式、std::function等可调用对象）。
     * @param args 传递给任务函数的参数。
     * @return 一个 std::future 对象，可用于获取任务的返回值或捕获其抛出的异常。
     * @throws std::runtime_error 如果线程池已经停止，无法提交新任务。
     */
    template<class F, class... Args>
    auto submit(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result_t<F, Args...>>;

    // 删除拷贝构造函数和拷贝赋值运算符，防止线程池被意外拷贝
    CPP_TOOLBOX_DISABLE_COPY(thread_pool_t)
    // 删除移动构造函数和移动赋值运算符，简化生命周期管理
    CPP_TOOLBOX_DISABLE_MOVE(thread_pool_t)

private:
    // 工作线程列表
    std::vector<std::thread> workers_;
    // 无锁任务队列，存储待执行的任务 (类型擦除为 void())
    toolbox::container::lock_free_queue_t<std::function<void()>> tasks_;

    // 不再需要以下成员:
    // std::mutex queue_mutex_;
    // std::condition_variable condition_;

    // 原子标志，指示线程池是否应该停止
    std::atomic<bool> stop_;
};

// --- Template Member Function Implementation ---

template<class F, class... Args>
auto thread_pool_t::submit(F&& f, Args&&... args)
    -> std::future<typename std::invoke_result_t<F, Args...>>
{
    using return_type = typename std::invoke_result_t<F, Args...>;

    // 检查线程池是否已停止
    if(stop_.load(std::memory_order_relaxed)) { // 使用 relaxed 因为后续 enqueue 有自己的同步
        throw std::runtime_error("在已停止的线程池上提交任务");
    }

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();

    // 直接将任务入队到无锁队列
    // 无需锁或条件变量
    tasks_.enqueue([task /* Capture shared_ptr */ ]() {
        // Add logging inside the lambda executed by the worker
        // Using task.get() can provide a somewhat unique identifier for the task instance
        // std::cout << "Worker lambda started for task " << task.get() << std::endl;
        try {
            // Execute the actual packaged_task via the shared pointer
            (*task)();
            // std::cout << "Worker lambda successfully invoked packaged_task " << task.get() << std::endl;
        } catch (const std::future_error& fe) {
            // This might catch errors if the promise is already broken
            std::cerr << "Worker lambda caught future_error for task " << task.get() << ": " << fe.what() << std::endl;
            // Potentially re-throw or handle, but packaged_task should set the future state
        } catch (const std::exception& e) {
            std::cerr << "Worker lambda caught unexpected exception during packaged_task invocation for task " << task.get() << ": " << e.what() << std::endl;
             // If the packaged_task itself fails to set the exception on the future,
             // the future might remain unset, leading to errors later.
             // Setting the promise state here is complex and might interfere with packaged_task.
             // For now, just log.
        } catch (...) {
            std::cerr << "Worker lambda caught unknown exception during packaged_task invocation for task " << task.get() << std::endl;
        }
        // std::cout << "Worker lambda finished for task " << task.get() << std::endl;
    });

    // 无需 notify_one，工作线程会主动检查队列

    return res;
}

} // namespace toolbox::base

#endif // CPP_TOOLBOX_OLD_THREAD_POOL
