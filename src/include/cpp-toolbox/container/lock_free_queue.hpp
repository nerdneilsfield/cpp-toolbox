#pragma once

#include <algorithm>  // For std::find
#include <atomic>  // For std::atomic
#include <functional>  // Include for std::function
#include <memory>  // For std::unique_ptr, std::allocator
#include <mutex>  // For protecting global HP list access (can be refined)
#include <optional>  // For std::optional (C++17)
#include <thread>  // For std::this_thread::yield
#include <unordered_map>  // For mapping thread IDs
#include <vector>

#include <cpp-toolbox/cpp-toolbox_export.hpp>

#include "cpp-toolbox/macro.hpp"

namespace toolbox::container
{

namespace detail
{

/**
 * @brief 危险指针实现 (Hazard Pointer Implementation)
 *
 * 这是一个简化的危险指针系统。生产环境下需要更健壮的线程管理和更高效的扫描机制。
 * (This is a simplified Hazard Pointer system. A production system would need
 * more robust thread management and potentially more efficient scanning.)
 */

struct HPRec;  // Forward declaration

// 全局危险指针记录列表(每个使用队列的线程一个)/Global list of Hazard Pointer
// Records (one per thread using the queue)
// 警告:使用互斥锁保护这个全局列表会产生一个竞争点,违反了HP注册本身的"纯"无锁特性。更高级的方案使用无锁列表或小心使用线程本地存储。
// (WARNING: Protecting this global list with a mutex adds a contention point,
// violating the "pure" lock-free property for HP registration itself. More
// advanced schemes use lock-free lists or thread-local storage carefully.)
inline std::vector<HPRec*> g_hp_list;
inline std::mutex g_hp_list_mutex;

// 存储(节点指针,删除器函数)对/Store pairs of (node_pointer, deleter_function)
inline thread_local std::vector<std::pair<void*, std::function<void(void*)>>>
    t_retired_list;

// 每个线程的最大危险指针数量(对于M&S队列,通常是2个HP)/Maximum hazard pointers
// per thread (For M&S queue, 2 HPs are typical)
constexpr size_t MAX_HAZARD_POINTERS_PER_THREAD = 2;

// 每N次retire调用扫描一次retired列表/Scan retired list every N retire calls
constexpr size_t RETIRE_SCAN_THRESHOLD = 100;

/**
 * @brief 危险指针记录结构体 (Hazard Pointer Record structure)
 */
struct HPRec
{
  std::atomic<std::thread::id> owner_thread_id;
  std::atomic<void*> hazard_pointers[MAX_HAZARD_POINTERS_PER_THREAD];
  HPRec* next =
      nullptr;  // 用于后续可能的无锁列表/For potential lock-free list later

  /**
   * @brief 构造函数 (Constructor)
   *
   * 初始化一个未被占用的危险指针记录 (Initializes an unowned hazard pointer
   * record)
   */
  HPRec()
      : owner_thread_id(std::thread::id())  // 初始未被占用/Initially unowned
  {
    for (size_t i = 0; i < MAX_HAZARD_POINTERS_PER_THREAD; ++i) {
      hazard_pointers[i].store(nullptr, std::memory_order_relaxed);
    }
  }
};

/**
 * @brief 为当前线程获取一个危险指针记录 (Acquire a hazard pointer record for
 * the current thread)
 *
 * @return HPRec* 返回分配给当前线程的危险指针记录 (Returns the hazard pointer
 * record assigned to the current thread)
 *
 * @example
 * @code
 * HPRec* my_record = acquire_hp_record();
 * // 使用记录...
 * release_hp_record(my_record);
 * @endcode
 */
inline HPRec* acquire_hp_record()
{
  static thread_local HPRec* my_hp_rec = nullptr;
  if (my_hp_rec)
    return my_hp_rec;

  // 首先尝试找到一个未使用的记录/Try to find an unused record first
  {
    std::lock_guard lock(g_hp_list_mutex);
    for (HPRec* rec : g_hp_list) {
      std::thread::id expected = std::thread::id();
      if (rec->owner_thread_id.compare_exchange_strong(
              expected, std::this_thread::get_id()))
      {
        my_hp_rec = rec;
        return my_hp_rec;
      }
    }
  }

  // 如果没有未使用的记录,创建一个新的/If no unused record, create a new one
  HPRec* new_rec = new HPRec();  // 如果线程不干净退出可能会泄露/Leak potential
                                 // if thread exits uncleanly
  new_rec->owner_thread_id.store(std::this_thread::get_id());

  {
    std::lock_guard lock(g_hp_list_mutex);
    g_hp_list.push_back(new_rec);
  }
  my_hp_rec = new_rec;
  return my_hp_rec;
}

/**
 * @brief 释放危险指针记录 (Release a hazard pointer record)
 *
 * 理想情况下在线程退出队列操作时调用。警告:这是简单实现,对线程退出检测不够健壮。
 * (Called ideally when thread exits involvement with queue. WARNING: Simple
 * implementation, not robust for thread exit detection.)
 *
 * @param rec 要释放的危险指针记录 (The hazard pointer record to release)
 */
inline void release_hp_record(HPRec* rec)
{
  if (!rec)
    return;
  rec->owner_thread_id.store(
      std::thread::id());  // 标记为未占用/Mark as unowned
  for (size_t i = 0; i < MAX_HAZARD_POINTERS_PER_THREAD; ++i) {
    rec->hazard_pointers[i].store(nullptr, std::memory_order_relaxed);
  }
  // 注意:这里不从g_hp_list中移除以允许重用。真实系统需要仔细清理。
  // (Note: We don't remove from g_hp_list here to allow reuse. A real system
  // needs careful cleanup.)
}

/**
 * @brief 为当前线程设置一个危险指针 (Set a hazard pointer for the current
 * thread)
 *
 * @param index 危险指针的索引 (Index of the hazard pointer)
 * @param ptr 要保护的指针 (Pointer to protect)
 */
inline void set_hazard_pointer(size_t index, void* ptr)
{
  HPRec* rec = acquire_hp_record();
  if (index < MAX_HAZARD_POINTERS_PER_THREAD) {
    rec->hazard_pointers[index].store(ptr, std::memory_order_release);
  }
}

/**
 * @brief 清除当前线程的一个危险指针 (Clear a hazard pointer for the current
 * thread)
 *
 * @param index 要清除的危险指针索引 (Index of the hazard pointer to clear)
 */
inline void clear_hazard_pointer(size_t index)
{
  set_hazard_pointer(index, nullptr);
}

/**
 * @brief 扫描已退休节点并删除安全的节点 (Scan retired nodes and delete safe
 * ones)
 */
inline void scan_retired_nodes()
{
  // 1. 收集所有线程的活跃危险指针/Collect all active hazard pointers from all
  // threads
  std::vector<void*> active_hps;
  {
    std::lock_guard lock(g_hp_list_mutex);  // 保护迭代/Protects iteration
    for (const HPRec* rec : g_hp_list) {
      // 检查记录是否真的被占用/Check if record is actually owned
      if (rec->owner_thread_id.load(std::memory_order_acquire)
          != std::thread::id())
      {
        for (size_t i = 0; i < MAX_HAZARD_POINTERS_PER_THREAD; ++i) {
          void* hp = rec->hazard_pointers[i].load(std::memory_order_acquire);
          if (hp) {
            active_hps.push_back(hp);
          }
        }
      }
    }
  }

  // 2. 检查线程本地退休列表中的每个节点/Check each node in the thread-local
  // retired list
  // Use erase idiom for safe removal during iteration
  for (auto it = t_retired_list.begin(); it != t_retired_list.end();
       /* no increment here */)
  {
    void* node_to_check = it->first;
    const auto& deleter = it->second;  // 获取删除器/Get the deleter
    bool is_hazardous = false;
    // 检查收集的危险指针/Check against collected HPs
    // Optimization: Sort active_hps first for faster lookup if needed
    // std::sort(active_hps.begin(), active_hps.end());
    // if (std::binary_search(active_hps.begin(), active_hps.end(),
    // node_to_check)) Requires sorting active_hps first
    for (void* active_hp : active_hps) {  // Simple linear scan
      if (node_to_check == active_hp) {
        is_hazardous = true;
        break;
      }
    }

    // 如果在活跃HP中未找到,则可以安全删除/If not found in active HPs, it's safe
    // to delete
    if (!is_hazardous) {
      // 使用存储的删除器安全删除/Safely delete using the stored deleter
      deleter(node_to_check);
      // Erase the current element and get iterator to the next
      it = t_retired_list.erase(it);
    } else {
      ++it;  // 保留到下次扫描, 移动到下一个 / Keep for next scan, move to next
    }
  }
}

/**
 * @brief 退休一个节点(添加到线程本地列表,偶尔触发扫描) (Retire a node - add to
 * thread-local list, trigger scan occasionally)
 *
 * @param node 要退休的节点指针 (Pointer to the node to retire)
 * @param deleter 用于删除节点的函数 (Function to delete the node)
 */
inline void retire_node(void* node, std::function<void(void*)> deleter)
{
  if (!node)
    return;
  t_retired_list.emplace_back(
      node, std::move(deleter));  // 存储节点和删除器/Store node and deleter
  if (t_retired_list.size() >= RETIRE_SCAN_THRESHOLD) {
    scan_retired_nodes();
  }
}

/**
 * @brief 清理当前线程的所有剩余退休节点 (Clean up all remaining retired nodes
 * for the current thread)
 *
 * 在线程退出时调用。理想情况下,剩余节点由全局处理或传递给另一个线程。
 * (Called at thread exit. Ideally, remaining nodes are handled globally or
 * passed to another thread.)
 */
inline void cleanup_retired_nodes()
{
  scan_retired_nodes();  // 尝试最后一次扫描/Try one last scan
}

/**
 * @brief 危险指针设置/清除的RAII助手类 (RAII helper for setting/clearing hazard
 * pointers)
 */
class HazardPointerGuard
{
public:
  /**
   * @brief 构造函数 (Constructor)
   *
   * @param index 危险指针索引 (Hazard pointer index)
   * @param node 要保护的节点 (Node to protect)
   */
  HazardPointerGuard(size_t index, void* node)
      : index_(index)
  {
    detail::set_hazard_pointer(index_, node);
  }

  /**
   * @brief 析构函数 (Destructor)
   */
  ~HazardPointerGuard() { detail::clear_hazard_pointer(index_); }

  HazardPointerGuard(const HazardPointerGuard&) = delete;
  HazardPointerGuard& operator=(const HazardPointerGuard&) = delete;
  HazardPointerGuard(HazardPointerGuard&&) = delete;
  HazardPointerGuard& operator=(HazardPointerGuard&&) = delete;

private:
  size_t index_;
};

}  // namespace detail

/**
 * @brief 使用危险指针实现的MPMC无锁无界队列 (An MPMC lock-free unbounded queue
 * using Hazard Pointers for memory safety)
 *
 * 基于Michael & Scott算法。使用原始指针、原子操作和危险指针进行安全内存回收。
 * (Based on the Michael & Scott algorithm. Uses raw pointers, atomic
 * operations, and Hazard Pointers for safe memory reclamation.)
 *
 * @note 这个实现使用了简化的危险指针方案。生产环境可能需要更健壮和优化的HP库。
 * HP注册部分可能涉及锁,略微影响线程加入/离开时的"纯"无锁特性,但核心队列操作保持无锁。
 * (This implementation uses a simplified Hazard Pointer scheme. Production use
 * might require a more robust and optimized HP library. The HP registration
 * part might involve locks, slightly impacting the "pure" lock-free nature for
 * thread joining/leaving, but core queue operations remain lock-free.)
 *
 * @tparam T 存储在队列中的元素类型。必须可移动和默认构造。
 * (The type of elements stored in the queue. Must be movable and default
 * constructible.)
 *
 * @example
 * @code
 * lock_free_queue_t<int> queue;
 *
 * // 生产者线程 (Producer thread)
 * queue.enqueue(42);
 *
 * // 消费者线程 (Consumer thread)
 * if(auto value = queue.try_dequeue()) {
 *   std::cout << "Dequeued: " << *value << std::endl;
 * }
 *
 * // 清理 (Cleanup)
 * queue.cleanup_this_thread_retired_nodes();
 * @endcode
 */
template<typename T>
class CPP_TOOLBOX_EXPORT lock_free_queue_t
{
private:
  /**
   * @brief 链表的内部节点结构 (Internal node structure for the linked list)
   */
  struct Node
  {
    T data;  // 存储实际数据元素(移动进入)/Stores the actual data element (moved
             // in)
    std::atomic<Node*>
        next;  // 指向下一个节点的原子指针/Atomic pointer to the next node

    /**
     * @brief 数据节点的构造函数 (Constructor for data nodes)
     */
    Node(T&& d)
        : data(std::move(d))
        , next(nullptr)
    {
    }

    /**
     * @brief 初始哑节点的构造函数 (Constructor for the initial dummy node)
     */
    Node()
        : data()
        , next(nullptr)
    {
    }  // 需要T可默认构造以用于哑节点/Requires T to be default constructible for
       // dummy
  };

  // 链表头尾的原子指针/Atomic pointers to the head and tail of the linked list
  std::atomic<Node*> head_;
  std::atomic<Node*> tail_;

  /**
   * @brief 退休队列节点的辅助函数 (Helper function specific to this queue's
   * Node type)
   */
  static void retire_queue_node(Node* node)
  {
    detail::retire_node(static_cast<void*>(node),
                        [](void* n)
                        {
                          // 这个lambda捕获了正确删除Node的方式
                          // (This lambda captures the correct way to delete a
                          // Node)
                          delete static_cast<Node*>(n);
                        });
  }

public:
  /**
   * @brief 构造无锁队列 (Constructs the lock-free queue)
   *
   * 用一个哑节点初始化队列 (Initializes the queue with a dummy node)
   */
  lock_free_queue_t()
  {
    Node* dummy_node = new Node();
    head_.store(dummy_node, std::memory_order_relaxed);
    tail_.store(dummy_node, std::memory_order_relaxed);
  }

  /**
   * @brief 销毁无锁队列 (Destroys the lock-free queue)
   *
   * 删除剩余节点。假设销毁期间无并发访问。尝试清理调用线程退休的节点。
   * (Deletes remaining nodes. Assumes no concurrent access during destruction.
   * Attempts to clean up nodes retired by the calling thread.)
   *
   * @warning 跨所有线程的退休节点的正确分布式清理很复杂,这里没有完全处理。
   * 如果其他线程不干净退出或不调用清理例程,可能会泄露内存。
   * (Proper distributed cleanup of retired nodes across all threads is complex
   * and not fully handled here. May leak memory if other threads exit uncleanly
   * or don't call cleanup routines.)
   */
  ~lock_free_queue_t()
  {
    // 基本清理 - 假设无并发访问/Basic cleanup - assumes no concurrent access
    T ignored_value;
    while (try_dequeue(ignored_value)) {
      // 出队涉及退休节点,让HP系统稍后处理
      // (Dequeue involves retiring nodes, let HP system handle them later)
    };

    // 尝试清理*这个*线程退休的节点
    // (Attempt to clean up nodes retired by *this* thread)
    detail::cleanup_retired_nodes();

    // 删除初始哑节点(如果为空应该是head)
    // (Delete the initial dummy node (should be the head if empty))
    Node* dummy = head_.load(std::memory_order_relaxed);
    if (dummy) {
      delete dummy;  // 如果HP系统未清理可能有潜在问题/Potential issue if HP
                     // system hasn't cleared it
    }
  }

  // 禁用复制和移动以防止节点/指针的所有权问题
  // (Disable copying and moving to prevent ownership issues with
  // nodes/pointers)
  CPP_TOOLBOX_DISABLE_COPY_AND_MOVE(lock_free_queue_t)

  /**
   * @brief 将项目入队 (Enqueues an item into the queue)
   *
   * 对多个生产者线程安全。如果需要,隐式使用危险指针读取尾部
   * (Thread-safe for multiple producers. Uses Hazard Pointers implicitly if
   * needed for tail reads)
   *
   * @param value 要入队的值(将被移动) (The value to enqueue (will be moved))
   *
   * @example
   * @code
   * lock_free_queue_t<std::string> queue;
   * queue.enqueue("Hello");
   * queue.enqueue(std::string("World"));
   * @endcode
   */
  void enqueue(T value)
  {
    Node* new_node = new Node(std::move(value));

    while (true) {
      Node* tail_snapshot = tail_.load(std::memory_order_acquire);
      Node* next_snapshot = tail_snapshot->next.load(std::memory_order_acquire);

      // 重新检查尾部一致性/Re-check tail consistency
      if (tail_snapshot == tail_.load(std::memory_order_acquire)) {
        if (next_snapshot == nullptr) {
          // 尝试链接新节点/Try to link the new node
          if (tail_snapshot->next.compare_exchange_weak(
                  next_snapshot,
                  new_node,
                  std::memory_order_release,
                  std::memory_order_relaxed))
          {
            // 尝试移动尾指针(可选,尽力而为)
            // (Try to swing the tail pointer (optional, best effort))
            tail_.compare_exchange_strong(tail_snapshot,
                                          new_node,
                                          std::memory_order_release,
                                          std::memory_order_relaxed);
            return;  // 入队成功/Enqueue successful
          }
        } else {
          // 尾部已前进,尝试帮助移动尾指针
          // (Tail already advanced, try to help swing the tail pointer)
          tail_.compare_exchange_strong(tail_snapshot,
                                        next_snapshot,
                                        std::memory_order_release,
                                        std::memory_order_relaxed);
        }
      }
    }
  }

  /**
   * @brief 尝试使用危险指针从队列中出队一个项目 (Attempts to dequeue an item
   * from the queue using Hazard Pointers)
   *
   * 对多个消费者线程安全。在潜在退休前使用危险指针保护对head和head->next节点的访问。
   * 非阻塞操作。
   * (Thread-safe for multiple consumers. Uses Hazard Pointers to protect access
   * to head and head->next nodes before potential retirement. Non-blocking
   * operation.)
   *
   * @param[out] result 如果成功则存储出队值的引用 (Reference to store the
   * dequeued value if successful)
   * @return 如果成功出队一个项目则为true,如果队列为空则为false
   * (True if an item was successfully dequeued, false if the queue was empty)
   *
   * @example
   * @code
   * lock_free_queue_t<int> queue;
   * int value;
   * if(queue.try_dequeue(value)) {
   *   std::cout << "Dequeued: " << value << std::endl;
   * } else {
   *   std::cout << "Queue is empty" << std::endl;
   * }
   * @endcode
   */
  bool try_dequeue(T& result)
  {
    [[maybe_unused]] detail::HPRec* hp_rec =
        detail::acquire_hp_record();  // 确保HP记录存在/Ensure HP record exists

    while (true) {
      // 获取危险指针/Acquire Hazard Pointers
      Node* head_snapshot = head_.load(std::memory_order_acquire);
      detail::set_hazard_pointer(0, head_snapshot);
      // 验证设置HP后head未改变/Verify head hasn't changed after setting HP
      if (head_snapshot != head_.load(std::memory_order_acquire)) {
        continue;  // Head改变,重试/Head changed, retry
      }

      Node* next_snapshot = head_snapshot->next.load(std::memory_order_acquire);
      detail::set_hazard_pointer(1, next_snapshot);

      // 设置HP后重新验证head和next指针
      // (Re-verify head and next pointers after setting HPs)
      if (head_snapshot != head_.load(std::memory_order_acquire)) {
        continue;  // Head改变,重试循环将重置HP/Head changed, retry loop will
                   // reset HPs
      }
      Node* current_next = head_snapshot->next.load(std::memory_order_acquire);
      if (next_snapshot != current_next) {
        continue;
      }

      Node* tail_snapshot = tail_.load(std::memory_order_acquire);

      if (head_snapshot == tail_snapshot) {
        // 队列为空或暂时不一致(入队进行中)
        // (Queue is empty or transiently inconsistent (enqueue in progress))
        if (next_snapshot == nullptr) {
          detail::clear_hazard_pointer(
              0);  // 返回前清除HP/Clear HPs before returning
          detail::clear_hazard_pointer(1);
          return false;  // 队列为空/Queue is empty
        }
        // 帮助前进尾部/Help advance tail
        tail_.compare_exchange_strong(tail_snapshot,
                                      next_snapshot,
                                      std::memory_order_release,
                                      std::memory_order_relaxed);
      } else {
        // 队列非空,尝试出队/Queue is not empty, try to dequeue
        if (next_snapshot == nullptr) {
          continue;
        }

        // 尝试前移head指针/Attempt to move the head pointer forward
        if (head_.compare_exchange_weak(head_snapshot,
                                        next_snapshot,
                                        std::memory_order_release,
                                        std::memory_order_relaxed))
        {
          result = std::move(
              next_snapshot->data);  // 数据在新head中/Data is in the NEW head

          // 使用危险指针安全退休旧head节点
          // (Retire the old head node safely using Hazard Pointers)
          retire_queue_node(head_snapshot);  // 退休旧的哑节点/数据节点/Retire
                                             // the old dummy/data node

          // 清除此操作的危险指针/Clear Hazard Pointers for this operation
          detail::clear_hazard_pointer(0);
          detail::clear_hazard_pointer(1);

          return true;  // 出队成功/Dequeue successful
        }
      }
    }
    // 清理HP(如果某种情况下未返回就退出循环,不应该发生)
    // (Cleanup HPs if somehow exited loop without returning (shouldn't happen))
    detail::clear_hazard_pointer(0);
    detail::clear_hazard_pointer(1);
    return false;  // 应该不可达/Should be unreachable
  }

  /**
   * @brief 尝试出队一个项目,将其返回在std::optional中
   * (Attempts to dequeue an item, returning it in an std::optional)
   *
   * 使用基于危险指针的try_dequeue实现
   * (Uses the Hazard Pointer based try_dequeue implementation)
   *
   * @return 如果成功则包含出队值的std::optional<T>,如果队列为空则为std::nullopt
   * (std::optional<T> containing the dequeued value if successful, or
   * std::nullopt if the queue was empty)
   *
   * @example
   * @code
   * lock_free_queue_t<std::string> queue;
   * if(auto value = queue.try_dequeue()) {
   *   std::cout << "Dequeued: " << *value << std::endl;
   * } else {
   *   std::cout << "Queue is empty" << std::endl;
   * }
   * @endcode
   */
  std::optional<T> try_dequeue()
  {
    T result;
    if (try_dequeue(result)) {
      return std::optional<T>(std::move(result));
    } else {
      return std::nullopt;
    }
  }

  /**
   * @brief 清理调用线程的退休节点 (Performs cleanup of retired nodes for the
   * calling thread)
   *
   * 每个使用队列的线程在退出前应该调用此函数,以确保及时回收该线程退休的内存。
   * 它尝试扫描并删除调用线程的退休列表中不再被任何危险指针保护的节点。
   * (This function should ideally be called by each thread that used the queue
   * before the thread exits, to ensure timely reclamation of memory retired by
   * that thread. It attempts to scan and delete any nodes in the calling
   * thread's retired list that are no longer protected by any hazard pointers.)
   *
   * @note 这只清理调用线程的列表。如果其他线程在未清理其列表的情况下退出,
   * 其退休的内存可能要等到本线程(或另一个活跃线程)退休足够多的节点以触发扫描,
   * 并恰好收集到足够的全局危险指针时才能回收。
   * 这个简化版本中未实现更健壮的全局清理机制(如专用清理线程、全局退休列表)。
   * (This only cleans the calling thread's list. Memory retired by other
   * threads that have exited without cleaning their lists might not be
   * reclaimed until this thread (or another active thread) retires enough nodes
   * to trigger a scan that happens to collect enough hazard pointers globally.
   * A more robust global cleanup mechanism (e.g., dedicated cleanup thread,
   * global retired list) is not implemented in this simplified version.)
   *
   * @example
   * @code
   * lock_free_queue_t<int> queue;
   * // ... 使用队列 (use queue) ...
   *
   * // 线程退出前清理
   * // (cleanup before thread exit)
   * queue.cleanup_this_thread_retired_nodes();
   * @endcode
   */
  static void cleanup_this_thread_retired_nodes()
  {
    // 调用detail实现来扫描当前线程的列表
    // (Call the detail implementation which scans the current thread's list)
    detail::cleanup_retired_nodes();
  }
};

}  // namespace toolbox::container
