#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>  // For potential exceptions if needed
#include <vector>  // Alternative storage

namespace toolbox::base
{

template<typename T>
class object_pool_t;  // Forward declaration

/**
 * @brief 自定义删除器,用于将对象返回到 object_pool_t/Custom deleter for
 * std::unique_ptr to return objects to an object_pool_t
 * @tparam T 池管理的对象类型/The type of object managed by the pool
 *
 * @example
 * @code
 * object_pool_t<MyClass> pool;
 * {
 *   auto obj = pool.acquire(); // obj 使用 PoolDeleter 作为删除器/obj uses
 * PoolDeleter } // obj 超出作用域时自动返回池中/obj automatically returns to
 * pool when out of scope
 * @endcode
 */
template<typename T>
class PoolDeleter
{
  object_pool_t<T>* pool_ =
      nullptr;  // 指向对象池的非拥有指针/Non-owning pointer to the pool

public:
  /**
   * @brief 构造 PoolDeleter/Construct a PoolDeleter
   * @param pool 指向用于释放对象的对象池的指针/Pointer to the object pool to
   * release objects back to
   */
  explicit PoolDeleter(object_pool_t<T>* pool = nullptr)
      : pool_(pool)
  {
  }

  /**
   * @brief std::unique_ptr 调用此函数代替 delete/The function called by
   * std::unique_ptr instead of delete
   * @param ptr 被管理对象的原始指针/Raw pointer to the object being managed
   */
  void operator()(T* ptr) const
  {
    if (pool_ && ptr) {
      pool_->release(ptr);  // 告诉池回收对象/Tell the pool to take it back
    } else {
      // 如果没有关联的池,执行默认删除/If no pool is associated, perform default
      // delete 处理在池外创建新对象或池过早销毁的情况/This handles cases where
      // acquire might create a new object outside the pool's direct management
      // or if the pool is destroyed early
      delete ptr;
    }
  }
};

/**
 * @brief 线程安全的对象池模板/A thread-safe object pool template
 *
 * 管理可重用对象以减少分配开销。使用带自定义删除器的 std::unique_ptr 通过 RAII
 * 管理对象生命周期/ Manages a pool of reusable objects to reduce allocation
 * overhead. Uses RAII via std::unique_ptr with custom deleter to manage object
 * lifetimes.
 *
 * @tparam T 要池化的对象类型,必须可默认构造/The type of object to pool. Must be
 * default-constructible
 *
 * @example
 * @code
 * // 创建对象池/Create object pool
 * object_pool_t<std::string> pool(5, [](std::string& s){ s.clear(); });
 *
 * // 获取对象/Acquire object
 * {
 *   auto str1 = pool.acquire();
 *   *str1 = "Hello";
 *   auto str2 = pool.acquire();
 *   *str2 = "World";
 * } // 对象自动返回池中/Objects automatically return to pool
 *
 * // 带重置函数的使用示例/Example with reset function
 * object_pool_t<std::vector<int>> vec_pool(2,
 *   [](std::vector<int>& v){ v.clear(); });
 * auto vec = vec_pool.acquire();
 * vec->push_back(1);
 * // vec 返回池时会调用 clear()/clear() will be called when vec returns to pool
 * @endcode
 */
template<typename T>
class object_pool_t
{
public:
  /**
   * @brief 管理池化对象的 unique_ptr 的别名。当超出作用域时自动将对象返回池中/
   * Alias for a unique_ptr managing a pooled object. Automatically returns the
   * object to the pool when it goes out of scope
   */
  using PooledObjectPtr = std::unique_ptr<T, PoolDeleter<T>>;

  /**
   * @brief 构造对象池/Construct an object pool
   * @param initial_size 初始创建的对象数量,默认为0/Number of objects to create
   * initially. Defaults to 0
   * @param resetter 可选的重置函数,在对象返回池前调用以重置状态/Optional
   * function to call on an object to reset its state before returning it to the
   * pool
   */
  explicit object_pool_t(size_t initial_size = 0,
                         std::function<void(T&)> resetter = nullptr)
      : reset_func_(std::move(resetter))
  {
    for (size_t i = 0; i < initial_size; ++i) {
      pool_.push(std::make_unique<T>());
    }
  }

  // 为简单起见删除复制/移动操作/Deleted copy/move operations for simplicity
  object_pool_t(const object_pool_t&) = delete;
  object_pool_t& operator=(const object_pool_t&) = delete;
  object_pool_t(object_pool_t&&) = delete;
  object_pool_t& operator=(object_pool_t&&) = delete;

  /**
   * @brief 从池中获取一个对象/Acquire an object from the pool
   *
   * 如果池为空则创建新对象。返回的 unique_ptr 在析构时会自动将对象释放回池中/
   * If the pool is empty, a new object is created. The returned unique_ptr will
   * automatically release the object back to the pool upon destruction.
   *
   * @return 管理获取对象的 std::unique_ptr/A std::unique_ptr managing the
   * acquired object
   */
  PooledObjectPtr acquire()
  {
    std::unique_ptr<T> obj_ptr = nullptr;  // 用于临时所有权的 unique_ptr/Use
                                           // unique_ptr for temporary ownership

    {  // 锁作用域/Lock scope
      std::lock_guard<std::mutex> lock(mutex_);
      if (!pool_.empty()) {
        obj_ptr = std::move(pool_.front());  // 从池中取出/Take from pool
        pool_.pop();
      }
    }  // 在可能创建新对象前解锁互斥量/Unlock mutex before potentially creating
       // a new object

    if (!obj_ptr) {  // 如果池为空/If pool was empty
      obj_ptr = std::make_unique<T>();  // 创建新对象/Create a new one
    }

    // 将对象包装在带自定义删除器的 unique_ptr 中返回/Return the object wrapped
    // in a unique_ptr with our custom deleter
    return PooledObjectPtr(obj_ptr.release(), PoolDeleter<T>(this));
  }

  /**
   * @brief 将对象释放回池中/Release an object back to the pool
   *
   * 当 unique_ptr 超出作用域时由 PoolDeleter
   * 自动调用。如果提供了重置函数则重置对象状态/ Called automatically by the
   * PoolDeleter when the unique_ptr goes out of scope. Resets the object's
   * state if a reset function was provided.
   *
   * @param ptr 要释放的对象的原始指针,所有权由池接管/Raw pointer to the object
   * to release. Ownership is taken by the pool
   */
  void release(T* ptr)
  {
    if (!ptr)
      return;

    // 如果有重置函数则重置对象状态/Reset object state if a reset function is
    // available
    if (reset_func_) {
      try {
        reset_func_(*ptr);
      } catch (const std::exception& e) {
        // 记录错误?不要让重置器的异常破坏池/Log error? Don't let exceptions
        // from resetter break the pool
        // 考虑使用不使用此池的机制进行日志记录/Consider logging using a
        // mechanism that doesn't use this pool
        fprintf(stderr, "[ObjectPool] Exception during reset: %s\n", e.what());
        // 决定是继续池化对象还是删除它/Decide whether to still pool the object
        // or delete it 如果重置严重失败,删除可能更安全/Deleting might be safer
        // if reset failed critically
        delete ptr;
        return;
      } catch (...) {
        fprintf(stderr, "[ObjectPool] Unknown exception during reset.\n");
        delete ptr;
        return;
      }
    }

    std::unique_ptr<T> obj_ptr(ptr);  // 重新包装原始指针以管理所有权/Re-wrap
                                      // raw pointer to manage ownership
    {  // 锁作用域/Lock scope
      std::lock_guard<std::mutex> lock(mutex_);
      pool_.push(std::move(obj_ptr));  // 将 unique_ptr 添加回池中/Add
                                       // unique_ptr back to the pool
    }
  }

private:
  friend class PoolDeleter<T>;  // 允许删除器调用 release/Allow deleter to call
                                // release

  std::queue<std::unique_ptr<T>> pool_;  // 对象池队列/Object pool queue
  mutable std::mutex mutex_;  // 用于线程安全的互斥量/Mutex for thread safety
  std::function<void(T&)> reset_func_;  // 对象重置函数/Object reset function
};

}  // namespace toolbox::base
