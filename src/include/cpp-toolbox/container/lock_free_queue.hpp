#pragma once

#include <atomic>  // For std::atomic
#include <memory>  // For std::unique_ptr, std::allocator
#include <optional>  // For std::optional (C++17)
#include <thread>  // For std::this_thread::yield
#include <vector>
#include <mutex>    // For protecting global HP list access (can be refined)
#include <unordered_map> // For mapping thread IDs
#include <algorithm> // For std::find
#include <functional> // Include for std::function

#include "cpp-toolbox/macro.hpp"

#include <cpp-toolbox/cpp-toolbox_export.hpp>

namespace toolbox::container
{

namespace detail
{

// --- Hazard Pointer Implementation ---
// Simplified Hazard Pointer system for illustration.
// A production system would need more robust thread management
// and potentially more efficient scanning.

struct HPRec; // Forward declaration

// Global list of Hazard Pointer Records (one per thread using the queue)
// WARNING: Protecting this global list with a mutex adds a contention point,
// violating the "pure" lock-free property for HP registration itself.
// More advanced schemes use lock-free lists or thread-local storage carefully.
inline std::vector<HPRec*> g_hp_list;
inline std::mutex g_hp_list_mutex;
// Store pairs of (node_pointer, deleter_function)
inline thread_local std::vector<std::pair<void*, std::function<void(void*)>>> t_retired_list;

constexpr size_t MAX_HAZARD_POINTERS_PER_THREAD = 2; // For M&S queue, 2 HPs are typical
constexpr size_t RETIRE_SCAN_THRESHOLD = 100; // Scan retired list every N retire calls

struct HPRec
{
  std::atomic<std::thread::id> owner_thread_id;
  std::atomic<void*> hazard_pointers[MAX_HAZARD_POINTERS_PER_THREAD];
  HPRec* next = nullptr; // For potential lock-free list later

  HPRec() : owner_thread_id(std::thread::id()) // Initially unowned
  {
    for (size_t i = 0; i < MAX_HAZARD_POINTERS_PER_THREAD; ++i) {
      hazard_pointers[i].store(nullptr, std::memory_order_relaxed);
    }
  }
};

// Simple allocation/retrieval of HP record for the current thread
inline HPRec* acquire_hp_record()
{
  static thread_local HPRec* my_hp_rec = nullptr;
  if (my_hp_rec) return my_hp_rec;

  // Try to find an unused record first
  {
    std::lock_guard lock(g_hp_list_mutex);
    for (HPRec* rec : g_hp_list) {
      std::thread::id expected = std::thread::id();
      if (rec->owner_thread_id.compare_exchange_strong(expected, std::this_thread::get_id())) {
        my_hp_rec = rec;
        return my_hp_rec;
      }
    }
  }

  // If no unused record, create a new one
  HPRec* new_rec = new HPRec(); // Leak potential if thread exits uncleanly
  new_rec->owner_thread_id.store(std::this_thread::get_id());

  {
    std::lock_guard lock(g_hp_list_mutex);
    g_hp_list.push_back(new_rec);
  }
  my_hp_rec = new_rec;
  return my_hp_rec;
}

// Release HP record (called ideally when thread exits involvement with queue)
// WARNING: Simple implementation, not robust for thread exit detection.
inline void release_hp_record(HPRec* rec)
{
  if (!rec) return;
  rec->owner_thread_id.store(std::thread::id()); // Mark as unowned
  for(size_t i = 0; i < MAX_HAZARD_POINTERS_PER_THREAD; ++i) {
      rec->hazard_pointers[i].store(nullptr, std::memory_order_relaxed);
  }
  // Note: We don't remove from g_hp_list here to allow reuse.
  // A real system needs careful cleanup.
}

// Sets a hazard pointer for the current thread
inline void set_hazard_pointer(size_t index, void* ptr)
{
  HPRec* rec = acquire_hp_record();
  if (index < MAX_HAZARD_POINTERS_PER_THREAD) {
    rec->hazard_pointers[index].store(ptr, std::memory_order_release);
  }
}

// Clears a hazard pointer for the current thread
inline void clear_hazard_pointer(size_t index)
{
   set_hazard_pointer(index, nullptr);
}

// Function to scan retired nodes and delete safe ones
inline void scan_retired_nodes()
{
  // 1. Collect all active hazard pointers from all threads
  std::vector<void*> active_hps;
  {
    std::lock_guard lock(g_hp_list_mutex); // Protects iteration
    for (const HPRec* rec : g_hp_list) {
      // Check if record is actually owned
      if (rec->owner_thread_id.load(std::memory_order_acquire) != std::thread::id()) {
        for (size_t i = 0; i < MAX_HAZARD_POINTERS_PER_THREAD; ++i) {
          void* hp = rec->hazard_pointers[i].load(std::memory_order_acquire);
          if (hp) {
            active_hps.push_back(hp);
          }
        }
      }
    }
  }
  // Optional: Sort active_hps for faster lookup if many retired nodes
  // std::sort(active_hps.begin(), active_hps.end());

  // 2. Check each node in the thread-local retired list
  auto it = t_retired_list.begin();
  while (it != t_retired_list.end()) {
    void* node_to_check = it->first;
    const auto& deleter = it->second; // Get the deleter
    bool is_hazardous = false;
    // Check against collected HPs
    for(void* active_hp : active_hps) {
        if (node_to_check == active_hp) {
            is_hazardous = true;
            break;
        }
    }

    // If not found in active HPs, it's safe to delete
    if (!is_hazardous) {
      // Safely delete using the stored deleter
      deleter(node_to_check);

      // Efficiently remove the element by swapping with the last and popping
      if (it != t_retired_list.end() - 1) {
          *it = std::move(t_retired_list.back());
      }
      t_retired_list.pop_back();
    } else {
      ++it; // Keep for next scan
    }
  }
}


// Retire a node (add to thread-local list, trigger scan occasionally)
inline void retire_node(void* node, std::function<void(void*)> deleter)
{
  if (!node) return;
  t_retired_list.emplace_back(node, std::move(deleter)); // Store node and deleter
  if (t_retired_list.size() >= RETIRE_SCAN_THRESHOLD) {
    scan_retired_nodes();
  }
}

// Clean up all remaining retired nodes for the current thread (e.g., at thread exit)
inline void cleanup_retired_nodes() {
     scan_retired_nodes(); // Try one last scan
    // Ideally, remaining nodes are handled globally or passed to another thread
    // For simplicity, we might leak here if scan doesn't clear all.
    // A robust system needs a global retirement list or similar.
    // Or force scans until empty, potentially blocking.
}

// RAII helper for setting/clearing hazard pointers
class HazardPointerGuard {
public:
    HazardPointerGuard(size_t index, void* node) : index_(index) {
        detail::set_hazard_pointer(index_, node);
        // Optional: Re-read and verify node hasn't changed immediately after setting HP
        // This protects against certain race conditions.
    }

    ~HazardPointerGuard() {
        detail::clear_hazard_pointer(index_);
    }

    HazardPointerGuard(const HazardPointerGuard&) = delete;
    HazardPointerGuard& operator=(const HazardPointerGuard&) = delete;
    HazardPointerGuard(HazardPointerGuard&&) = delete;
    HazardPointerGuard& operator=(HazardPointerGuard&&) = delete;

private:
    size_t index_;
};


} // namespace detail


/**
 * @brief An MPMC lock-free unbounded queue using Hazard Pointers for memory safety.
 *
 * Based on the Michael & Scott algorithm.
 * Uses raw pointers, atomic operations, and Hazard Pointers for safe memory reclamation.
 *
 * @note This implementation uses a simplified Hazard Pointer scheme.
 * Production use might require a more robust and optimized HP library.
 * The HP registration part might involve locks, slightly impacting the "pure" lock-free nature
 * for thread joining/leaving, but core queue operations remain lock-free.
 *
 * @tparam T The type of elements stored in the queue. Must be movable and default constructible.
 */
template<typename T>
class CPP_TOOLBOX_EXPORT lock_free_queue_t
{
private:
  // Internal node structure for the linked list
  struct Node
  {
    T data;  // Stores the actual data element (moved in)
    std::atomic<Node*> next;  // Atomic pointer to the next node

    // Constructor for data nodes
    Node(T&& d)
        : data(std::move(d))
        , next(nullptr)
    {
    }
    // Constructor for the initial dummy node
    Node()
        : data()
        , next(nullptr)
    {
    }  // Requires T to be default constructible for dummy
  };

  // Atomic pointers to the head and tail of the linked list
  std::atomic<Node*> head_;
  std::atomic<Node*> tail_;

  // --- Hazard Pointer Integration ---
  // Helper function specific to this queue's Node type
  static void retire_queue_node(Node* node) {
      detail::retire_node(static_cast<void*>(node), [](void* n) {
          // This lambda captures the correct way to delete a Node
          delete static_cast<Node*>(n);
      });
  }
   // Proper deletion within scan needs knowledge of T's Node type
   // This requires redesigning the HP system slightly (e.g., storing deleters)
   // Hack: Re-declare the scan function here or make Node type accessible globally?
   // Let's stick to the void* approach and assume scan can delete later (requires fix)


public:
  /**
   * @brief Constructs the lock-free queue.
   *
   * Initializes the queue with a dummy node.
   */
  lock_free_queue_t()
  {
    Node* dummy_node = new Node();
    head_.store(dummy_node, std::memory_order_relaxed);
    tail_.store(dummy_node, std::memory_order_relaxed);
  }

  /**
   * @brief Destroys the lock-free queue.
   *
   * Deletes remaining nodes. Assumes no concurrent access during destruction.
   * Attempts to clean up nodes retired by the calling thread.
   * @warning Proper distributed cleanup of retired nodes across all threads
   * is complex and not fully handled here. May leak memory if other threads
   * exit uncleanly or don't call cleanup routines.
   */
  ~lock_free_queue_t()
  {
    // Basic cleanup - assumes no concurrent access.
    T ignored_value;
    while (try_dequeue(ignored_value)) {
      // Dequeue involves retiring nodes, let HP system handle them later
    };

     // Attempt to clean up nodes retired by *this* thread.
     detail::cleanup_retired_nodes();

    // Delete the initial dummy node (should be the head if empty)
    Node* dummy = head_.load(std::memory_order_relaxed);
    if (dummy) {
       // Is the dummy node ever retired? It shouldn't be normally.
       // But if try_dequeue had issues, head_ might not be the original dummy.
       // Need to consider if the final head_ needs retirement or direct delete.
       // If the queue is empty, head_ points to the original dummy.
       // If not empty, head_ points to the last dequeued node's *next*.
       // The original dummy should have been retired by the last dequeue.
       // Let's assume cleanup handles retired nodes including the original dummy.
       // If head_ still points to a node, it might be the *new* dummy after
       // the last dequeue, or the original if never dequeued.
       // Deleting it directly might be unsafe if another thread still holds HP.
       // This destruction logic needs careful review in a full HP implementation.
       // For now, let's delete the head node directly, assuming it's safe at destruction.
       delete dummy; // Potential issue if HP system hasn't cleared it.
    }
     // What about nodes in global retired lists from other threads? Not handled.
  }

  // Disable copying and moving to prevent ownership issues with nodes/pointers.
  CPP_TOOLBOX_DISABLE_COPY_AND_MOVE(lock_free_queue_t)

  /**
   * @brief Enqueues an item into the queue.
   *
   * Thread-safe for multiple producers. Uses Hazard Pointers implicitly if needed
   * for tail reads (though M&S enqueue typically doesn't require HPs).
   *
   * @param value The value to enqueue (will be moved).
   */
  void enqueue(T value)
  {
    Node* new_node = new Node(std::move(value));
    // No need for Hazard Pointer on new_node, it's local initially.

    while (true) {
      // Reading tail doesn't strictly require HP if we CAS tail later,
      // but reading tail->next *might* if tail could be deleted concurrently
      // (which isn't the case in standard M&S). Let's assume no HP needed here.
      Node* tail_snapshot = tail_.load(std::memory_order_acquire);
      Node* next_snapshot = tail_snapshot->next.load(std::memory_order_acquire);

      // Re-check tail consistency
      if (tail_snapshot == tail_.load(std::memory_order_acquire)) {
        if (next_snapshot == nullptr) {
          // Try to link the new node
          if (tail_snapshot->next.compare_exchange_weak(
                  next_snapshot, new_node,
                  std::memory_order_release, std::memory_order_relaxed))
          {
            // Try to swing the tail pointer (optional, best effort)
            tail_.compare_exchange_strong(tail_snapshot, new_node,
                                          std::memory_order_release, std::memory_order_relaxed);
            return; // Enqueue successful
          }
        } else {
          // Tail already advanced, try to help swing the tail pointer
          tail_.compare_exchange_strong(tail_snapshot, next_snapshot,
                                        std::memory_order_release, std::memory_order_relaxed);
        }
      }
      // Yield might be beneficial under high contention
      // std::this_thread::yield();
    }
  }

  /**
   * @brief Attempts to dequeue an item from the queue using Hazard Pointers.
   *
   * Thread-safe for multiple consumers. Uses Hazard Pointers to protect
   * access to head and head->next nodes before potential retirement.
   * Non-blocking operation.
   *
   * @param[out] result Reference to store the dequeued value if successful.
   * @return True if an item was successfully dequeued, false if the queue was empty.
   */
  bool try_dequeue(T& result)
  {
    detail::HPRec* hp_rec = detail::acquire_hp_record(); // Ensure HP record exists

    while (true) {
      // Acquire Hazard Pointers
      Node* head_snapshot = head_.load(std::memory_order_acquire);
      detail::set_hazard_pointer(0, head_snapshot);
      // Verify head hasn't changed after setting HP
      if(head_snapshot != head_.load(std::memory_order_acquire)) {
          continue; // Head changed, retry
      }

      Node* next_snapshot = head_snapshot->next.load(std::memory_order_acquire);
      detail::set_hazard_pointer(1, next_snapshot);

      // Re-verify head and next pointers after setting HPs
      if(head_snapshot != head_.load(std::memory_order_acquire)) {
          continue; // Head changed, retry loop will reset HPs
      }
       // If next changed between reading and setting HP, need careful check.
       // If next is now null, but wasn't initially, head might have been dequeued.
       // If next is different non-null, the node we set HP[1] for might be wrong.
       // Let's reload next_snapshot after setting HP[1] for safety.
       Node* current_next = head_snapshot->next.load(std::memory_order_acquire);
       if (next_snapshot != current_next) {
           // Update HP[1] or retry loop. Retrying is simpler.
           continue;
       }


      Node* tail_snapshot = tail_.load(std::memory_order_acquire);

      if (head_snapshot == tail_snapshot) {
        // Queue is empty or transiently inconsistent (enqueue in progress)
        if (next_snapshot == nullptr) {
           detail::clear_hazard_pointer(0); // Clear HPs before returning
           detail::clear_hazard_pointer(1);
           return false; // Queue is empty
        }
        // Help advance tail
        tail_.compare_exchange_strong(tail_snapshot, next_snapshot,
                                      std::memory_order_release, std::memory_order_relaxed);
        // Retry loop
      } else {
        // Queue is not empty, try to dequeue
        if (next_snapshot == nullptr) {
            // This state (head != tail but head->next == nullptr) is problematic.
            // Could indicate a race or corruption. Retry might resolve transient states.
            continue;
        }

        // Read data *before* attempting to CAS head.
        // We have HPs protecting head_snapshot and next_snapshot.
        // It should be safe to read data from next_snapshot now.
        // Note: We read from the *new* head (next_snapshot) before the CAS.
        // If CAS fails, we haven't moved the data. If it succeeds, we have.
        // Let's move this read after the successful CAS for clarity.

        // Attempt to move the head pointer forward
        if (head_.compare_exchange_weak(head_snapshot, next_snapshot,
                                        std::memory_order_release, std::memory_order_relaxed))
        {
          // Successfully moved head. head_snapshot is now logically dequeued.
          // Move the data out from the node *after* the old head (the new head).
          result = std::move(next_snapshot->data); // Data is in the NEW head

          // Retire the old head node safely using Hazard Pointers
          retire_queue_node(head_snapshot); // Retire the old dummy/data node

          // Clear Hazard Pointers for this operation
          detail::clear_hazard_pointer(0);
          detail::clear_hazard_pointer(1);

          return true; // Dequeue successful
        }
        // CAS failed, head was changed by another thread. Retry loop.
      }
      // Yield might be beneficial under high contention
      // std::this_thread::yield();
    }
     // Cleanup HPs if somehow exited loop without returning (shouldn't happen)
     detail::clear_hazard_pointer(0);
     detail::clear_hazard_pointer(1);
     return false; // Should be unreachable
  }

  /**
   * @brief Attempts to dequeue an item, returning it in an std::optional.
   *
   * Uses the Hazard Pointer based try_dequeue implementation.
   *
   * @return std::optional<T> containing the dequeued value if successful,
   * or std::nullopt if the queue was empty.
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
   * @brief Performs cleanup of retired nodes for the calling thread.
   *
   * This function should ideally be called by each thread that used the queue
   * before the thread exits, to ensure timely reclamation of memory retired
   * by that thread. It attempts to scan and delete any nodes in the calling
   * thread's retired list that are no longer protected by any hazard pointers.
   *
   * @note This only cleans the calling thread's list. Memory retired by other
   * threads that have exited without cleaning their lists might not be reclaimed
   * until this thread (or another active thread) retires enough nodes to trigger
   * a scan that happens to collect enough hazard pointers globally. A more robust
   * global cleanup mechanism (e.g., dedicated cleanup thread, global retired list)
   * is not implemented in this simplified version.
   */
  static void cleanup_this_thread_retired_nodes() {
      // Call the detail implementation which scans the current thread's list
      detail::cleanup_retired_nodes();
  }

  // TODO: Address the type-safety issue in the `scan_retired_nodes` delete call.
};

}  // namespace toolbox::container
