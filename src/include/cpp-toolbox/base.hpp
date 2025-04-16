#pragma once

#include "cpp-toolbox/base/env.hpp"
#include "cpp-toolbox/base/object_pool.hpp"
#include "cpp-toolbox/base/thread_pool.hpp"
#include "cpp-toolbox/base/thread_pool_singleton.hpp"

/**
 * @namespace toolbox::base
 * @brief 提供基础 C++ 工具和组件。 Provides fundamental C++ utilities and
 * components.
 *
 * @details
 * 该命名空间包含了一系列用于 C++
 * 开发的核心基础功能，旨在提高开发效率和代码质量。 主要包括：
 * - **线程管理 (Thread Management):** 提供易于使用的线程池 (`ThreadPool`,
 * `ThreadPoolSingleton`)，用于高效管理并发任务。
 * - **资源管理 (Resource Management):** 提供对象池 (`ObjectPool`)
 * 和可能的内存池 (`MemoryPool`) 实现，以优化资源的分配和回收。
 * - **环境交互 (Environment Interaction):**
 * 可能包含与运行时环境交互的工具，例如环境变量的读取 (`env.hpp`)。
 *
 * 这些组件是构建更复杂应用程序的基础模块。
 *
 * This namespace contains a collection of core fundamental functionalities for
 * C++ development, aiming to improve development efficiency and code quality.
 * Key components include:
 * - **Thread Management:** Provides easy-to-use thread pools (`ThreadPool`,
 * `ThreadPoolSingleton`) for efficient management of concurrent tasks.
 * - **Resource Management:** Offers implementations for object pools
 * (`ObjectPool`) and potentially memory pools (`MemoryPool`) to optimize
 * resource allocation and recycling.
 * - **Environment Interaction:** May include utilities for interacting with the
 * runtime environment, such as reading environment variables (`env.hpp`).
 *
 * These components serve as foundational building blocks for constructing more
 * complex applications.
 *
 * @section base_examples 示例 (Examples)
 *
 * @subsection base_example_thread_pool 线程池使用示例 (ThreadPool Usage
 * Example)
 * @code{.cpp}
 * #include <cpp-toolbox/base/thread_pool.hpp>
 * #include <iostream>
 * #include <vector>
 * #include <future>
 * #include <chrono>
 * #include <numeric>
 *
 * int main() {
 *     // 创建一个拥有 4 个工作线程的线程池
 *     // Create a thread pool with 4 worker threads
 *     toolbox::base::ThreadPool pool(4);
 *
 *     // 准备提交的任务
 *     // Prepare tasks to submit
 *     std::vector<std::future<int>> results;
 *
 *     for (int i = 0; i < 8; ++i) {
 *         results.emplace_back(
 *             pool.enqueue([i] { // 提交 Lambda 函数作为任务 (Submit a lambda
 * function as a task) std::cout << "Processing task " << i << " on thread " <<
 * std::this_thread::get_id() << std::endl;
 *                 std::this_thread::sleep_for(std::chrono::milliseconds(100));
 * // 模拟工作 (Simulate work) std::cout << "Finished task " << i << std::endl;
 *                 return i * i; // 返回结果 (Return a result)
 *             })
 *         );
 *     }
 *
 *     // 等待所有任务完成并获取结果
 *     // Wait for all tasks to complete and retrieve results
 *     int sum = 0;
 *     for (auto && result : results) {
 *         try {
 *             sum += result.get(); // 获取任务的返回值 (Get the return value of
 * the task) } catch (const std::exception& e) { std::cerr << "Exception caught:
 * " << e.what() << std::endl;
 *         }
 *     }
 *
 *     std::cout << "All tasks finished. Sum of squares: " << sum << std::endl;
 *
 *     // 线程池会在析构时自动停止并加入所有线程
 *     // The thread pool will automatically stop and join all threads upon
 *     // destruction
     return 0;
 * }
 * @endcode
 *
 * @subsection base_example_object_pool 对象池使用示例 (ObjectPool Usage
 * Example)
 * @code{.cpp}
 * #include <cpp-toolbox/base/object_pool.hpp>
 * #include <iostream>
 * #include <vector>
 * #include <memory> // for std::shared_ptr
 *
 * struct MyObject {
 *     int id;
 *     MyObject() : id(-1) { std::cout << "MyObject Default constructing." <<
 * std::endl; } MyObject(int i) : id(i) { std::cout << "MyObject constructing
 * with id " << id << "." << std::endl; } ~MyObject() { std::cout << "MyObject
 * destructing with id " << id << "." << std::endl; }
 *
 *     // 对象池可能会调用 reset 来重置状态 (Object pool might call reset to
 *     // reset state)
 *     void reset() {
 *        id = -1;
 *        std::cout << "MyObject resetting." << sstd::endl;
 *     }
 * };
 *
 * int main() {
 *     // 创建一个 MyObject 对象池，初始大小为 2，最大可扩展到 10
 *     // Create an ObjectPool for MyObject, initial size 2, max size 10
 *     toolbox::base::ObjectPool<MyObject> pool(2, 10);
 *
 *     std::vector<std::shared_ptr<MyObject>> objects;
 *
 *     std::cout << "Acquiring objects..." << std::endl;
 *     for (int i = 0; i < 5; ++i) {
 *         // 从池中获取对象 (Acquire an object from the pool)
 *         // 使用 acquire 获取带有关联释放器的智能指针 (Use acquire to get a
 * smart pointer with associated releaser)
 *         // 或者如果 ObjectPool 提供构造函数参数传递，则使用 allocate
 *         // Or use allocate if the ObjectPool supports constructor argument
 * forwarding
 *         // auto obj = pool.allocate(i); // 假设 allocate 可以传递构造函数参数
 *         auto obj = pool.acquire(); // 使用 acquire 获取对象，然后手动设置
 *         if (obj) {
 *            obj->id = i; // 手动设置 ID (Manually set ID)
 *            std::cout << "Acquired object with id: " << obj->id << std::endl;
 *            objects.push_back(obj);
 *         } else {
 *            std::cerr << "Failed to acquire object " << i << " (Pool likely
 * reached max size)" << std::endl;
 *            break;
 *         }
 *     }
 *
 *     std::cout << "\nUsing objects..." << std::endl;
 *     for (const auto& obj : objects) {
 *         std::cout << "Using object with id: " << obj->id << std::endl;
 *     }
 *
 *     std::cout << "\nReleasing objects..." << std::endl;
 *     objects.clear(); // 当 shared_ptr 引用计数归零时，对象会自动释放回池中
 *                      // When the shared_ptr reference count drops to zero,
 * the object is automatically released back to the pool
 *
 *     std::cout << "\nAcquiring again..." << std::endl;
 *     // 再次获取对象，应该会复用之前释放的对象
 *     // Acquire objects again, should reuse previously released objects
 *      auto reused_obj1 = pool.acquire();
 *      if(reused_obj1) std::cout << "Acquired reused object with id: " <<
 * reused_obj1->id << std::endl;
 *      // ID 应该是被 reset 后的值 (ID should be the reset value)
 *
 *      auto reused_obj2 = pool.acquire();
 *      if(reused_obj2) std::cout << "Acquired reused object with id: " <<
 * reused_obj2->id << std::endl;
 *
 *      std::cout << "\nExiting..." << std::endl;
 *      // 对象池析构时，会销毁其管理的所有对象
 *      // When the object pool is destructed, it destroys all the objects it
 *      // manages
 *      return 0;
 * }
 * @endcode
 *
 */
namespace toolbox::base
{

}  // namespace toolbox::base
