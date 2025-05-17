#include <cstddef>  ///< 用于 std::size_t/For std::size_t
#include <limits>  ///< 用于 std::numeric_limits/For std::numeric_limits
#include <mutex>  ///< 用于 std::mutex/For std::mutex
#include <queue>  ///< 用于 std::queue/For std::queue
#include <stdexcept>  ///< 用于 std::invalid_argument/For std::invalid_argument

#include <cpp-toolbox/macro.hpp>

namespace toolbox::base
{

/**
 * @brief 固定大小内存池类/Fixed-size memory pool class
 *
 * 提供高效的固定大小内存块分配与回收，支持线程安全和自动扩容/收缩。
 * Provides efficient allocation and deallocation of fixed-size memory blocks,
 * supporting thread safety and automatic expansion/shrinking.
 *
 * @code
 * toolbox::base::memory_pool_t pool(64, 4, 8, 2); //
 * 创建一个块大小为64字节，预分配4块，最大缓存8块，扩容步长2的内存池/Create a
 * pool with 64-byte blocks, 4 preallocated, max 8 cached, growth 2 void* p =
 * pool.allocate(); // 分配内存块/Allocate a block pool.deallocate(p); //
 * 归还内存块/Deallocate a block pool.release_unused(); //
 * 释放所有未用内存/Release all unused memory
 * @endcode
 */
class memory_pool_t
{
public:
  /**
   * @brief 构造内存池/Construct a memory pool
   * @param block_size 每个内存块的字节数，必须大于0/Size of each memory block
   * in bytes, must be > 0
   * @param initial_blocks 预先分配的内存块数量/Number of blocks to preallocate
   * @param max_cached_blocks 最大缓存空闲块数，超出则归还系统/Maximum number of
   * freed blocks to keep cached, exceeding this returns memory to the system
   *        使用 std::numeric_limits<std::size_t>::max() 表示无限制/Use
   * std::numeric_limits<std::size_t>::max() for unlimited
   * @param growth 池耗尽时每次扩容分配的块数/Number of blocks to allocate when
   * the pool runs out
   *
   * 创建一个提供固定大小内存块的池。池耗尽时按 growth
   * 分配新块，归还过多时自动释放多余内存。 Creates a pool that provides
   * fixed-size memory blocks. If the pool is exhausted, additional blocks will
   * be allocated in batches of `growth`. When too many blocks are returned,
   * excess ones are released so the pool can shrink.
   *
   * @code
   * memory_pool_t pool(32, 2); // 创建块大小32字节，预分配2块的内存池/Create a
   * pool with 32-byte blocks, 2 preallocated
   * @endcode
   */
  explicit memory_pool_t(
      std::size_t block_size,
      std::size_t initial_blocks = 0,
      std::size_t max_cached_blocks = std::numeric_limits<std::size_t>::max(),
      std::size_t growth = 1)
      : block_size_(block_size)
      , growth_(growth == 0 ? 1 : growth)
      , max_cached_blocks_(max_cached_blocks)
  {
    if (block_size_ == 0) {
      throw std::invalid_argument("block size must be > 0");
    }

    // 预分配指定数量的内存块/Preallocate the specified number of blocks
    for (std::size_t i = 0; i < initial_blocks; ++i) {
      pool_.push(::operator new(block_size_));
    }
  }

  /**
   * @brief 析构函数，释放所有缓存内存块/Destructor, frees all cached blocks
   */
  ~memory_pool_t() { release_unused(); }

  /**
   * @brief 禁用拷贝构造和赋值/Disable copy constructor and assignment
   */
  CPP_TOOLBOX_DISABLE_COPY(memory_pool_t)
  /**
   * @brief 禁用移动构造和赋值/Disable move constructor and assignment
   */
  CPP_TOOLBOX_DISABLE_MOVE(memory_pool_t)

  /**
   * @brief 分配一个内存块/Allocate a memory block
   * @return 指向内存块的指针/Pointer to a block of size block_size()
   *
   * @code
   * void* p = pool.allocate(); // 分配内存块/Allocate a block
   * @endcode
   */
  void* allocate()
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (pool_.empty()) {
      expand_blocks(growth_);
    }
    void* ptr = pool_.front();
    pool_.pop();
    return ptr;
  }

  /**
   * @brief 归还内存块到池中/Return a block to the pool
   * @param ptr 由 allocate() 获得的指针/Pointer previously obtained from
   * allocate()
   *
   * @code
   * void* p = pool.allocate();
   * pool.deallocate(p); // 归还内存块/Return the block
   * @endcode
   */
  void deallocate(void* ptr)
  {
    if (!ptr) {
      return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (pool_.size() >= max_cached_blocks_) {
      ::operator delete(ptr);
    } else {
      pool_.push(ptr);
    }
  }

  /**
   * @brief 获取每个内存块的大小/Get the size of each memory block
   * @return 内存块字节数/Size of each memory block in bytes
   */
  std::size_t block_size() const noexcept { return block_size_; }

  /**
   * @brief 获取当前池中空闲块数量/Get the number of free blocks currently
   * stored in the pool
   * @return 空闲块数量/Number of free blocks
   */
  std::size_t free_blocks() const
  {
    std::lock_guard<std::mutex> lock(mutex_);
    return pool_.size();
  }

  /**
   * @brief 释放所有缓存的内存块到系统/Release all cached blocks back to the
   * system
   *
   * @code
   * pool.release_unused(); // 释放所有未用内存/Release all unused memory
   * @endcode
   */
  void release_unused()
  {
    std::lock_guard<std::mutex> lock(mutex_);
    // 释放所有缓存块/Release all cached blocks
    while (!pool_.empty()) {
      ::operator delete(pool_.front());
      pool_.pop();
    }
  }

private:
  /**
   * @brief 每个内存块的字节数/Size of each memory block in bytes
   */
  std::size_t block_size_ {};
  /**
   * @brief 池耗尽时每次扩容的块数/Number of blocks to allocate when expanding
   */
  std::size_t growth_ {1};
  /**
   * @brief 最大缓存空闲块数/Maximum number of cached free blocks
   */
  std::size_t max_cached_blocks_ {std::numeric_limits<std::size_t>::max()};
  /**
   * @brief 互斥锁，保证线程安全/Mutex for thread safety
   */
  mutable std::mutex mutex_;
  /**
   * @brief 内存块指针队列/Queue of memory block pointers
   */
  std::queue<void*> pool_;

  /**
   * @brief 扩容内存池，分配指定数量的块/Expand the pool by allocating the
   * specified number of blocks
   * @param count 要分配的块数/Number of blocks to allocate
   */
  void expand_blocks(std::size_t count)
  {
    for (std::size_t i = 0; i < count; ++i) {
      pool_.push(::operator new(block_size_));
    }
  }
};

}  // namespace toolbox::base