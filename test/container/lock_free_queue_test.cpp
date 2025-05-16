#include <atomic>  // For std::atomic_int
#include <cmath>
#include <future>  // For std::async, std::future
#include <limits>
#include <numeric>  // For std::iota
#include <set>  // For checking consumed items
#include <string>
#include <string_view>
#include <thread>  // For std::thread
#include <vector>

#include "cpp-toolbox/container/lock_free_queue.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_contains.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

// Import the namespace for convenience
using namespace toolbox::container;
using Catch::Matchers::Contains;
using Catch::Matchers::Equals;

/**
 * @brief Tests basic single-threaded operations for lock_free_queue_t.
 */
TEST_CASE("LockFreeQueue Single Thread Basic Operations",
          "[container][lock_free_queue]")
{
  lock_free_queue_t<int> queue;

  SECTION("Enqueue and Dequeue Single Element")
  {
    queue.enqueue(10);
    int value;
    REQUIRE(queue.try_dequeue(value));
    REQUIRE(value == 10);
  }

  SECTION("Dequeue from Empty Queue")
  {
    int value;
    REQUIRE_FALSE(queue.try_dequeue(value));  // Should fail
    REQUIRE_FALSE(queue.try_dequeue().has_value());  // Optional version
  }

  SECTION("FIFO Order")
  {
    queue.enqueue(1);
    queue.enqueue(2);
    queue.enqueue(3);

    int v1, v2, v3;
    REQUIRE(queue.try_dequeue(v1));
    REQUIRE(v1 == 1);
    REQUIRE(queue.try_dequeue(v2));
    REQUIRE(v2 == 2);
    REQUIRE(queue.try_dequeue(v3));
    REQUIRE(v3 == 3);
    REQUIRE_FALSE(queue.try_dequeue(v1));  // Queue should be empty now
  }

  SECTION("Optional Dequeue")
  {
    queue.enqueue(42);
    auto opt_val = queue.try_dequeue();
    REQUIRE(opt_val.has_value());
    REQUIRE(opt_val.value() == 42);

    auto empty_opt = queue.try_dequeue();
    REQUIRE_FALSE(empty_opt.has_value());
  }

  // Explicitly clean up retired nodes for the current thread
  lock_free_queue_t<int>::cleanup_this_thread_retired_nodes();
}

/**
 * @brief Tests lock_free_queue_t with std::string elements.
 */
TEST_CASE("LockFreeQueue String Elements", "[container][lock_free_queue]")
{
  lock_free_queue_t<std::string> queue;

  queue.enqueue("hello");
  queue.enqueue("world");

  std::string s1, s2;
  REQUIRE(queue.try_dequeue(s1));
  REQUIRE(s1 == "hello");
  REQUIRE(queue.try_dequeue(s2));
  REQUIRE(s2 == "world");
  REQUIRE_FALSE(queue.try_dequeue(s1));

  lock_free_queue_t<std::string>::cleanup_this_thread_retired_nodes();
}

/**
 * @brief Tests lock_free_queue_t with move-only types like std::unique_ptr.
 */
TEST_CASE("LockFreeQueue Move-Only Types", "[container][lock_free_queue]")
{
  lock_free_queue_t<std::unique_ptr<int>> queue;

  queue.enqueue(std::make_unique<int>(100));
  queue.enqueue(std::make_unique<int>(200));

  std::unique_ptr<int> p1, p2;
  REQUIRE(queue.try_dequeue(p1));
  REQUIRE(p1 != nullptr);
  REQUIRE(*p1 == 100);

  REQUIRE(queue.try_dequeue(p2));
  REQUIRE(p2 != nullptr);
  REQUIRE(*p2 == 200);

  REQUIRE_FALSE(queue.try_dequeue(p1));

  lock_free_queue_t<std::unique_ptr<int>>::cleanup_this_thread_retired_nodes();
}

/**
 * @brief Tests lock_free_queue_t with multiple producer threads and one
 * consumer thread.
 */
TEST_CASE("LockFreeQueue Multi-Producer Single-Consumer (MPSC)",
          "[container][lock_free_queue][multithreaded]")
{
  lock_free_queue_t<int> queue;
  const int num_producers = 4;
  const int items_per_producer = 1000;
  const int total_items = num_producers * items_per_producer;
  std::vector<std::thread> producers;
  std::atomic_int consumed_count(0);
  std::vector<int> consumed_items;
  consumed_items.reserve(total_items);
  std::mutex consumed_mutex;  // Protect consumed_items vector

  // Producer function
  auto producer_func = [&](int start_value)
  {
    for (int i = 0; i < items_per_producer; ++i) {
      queue.enqueue(start_value + i);
    }
    // Clean up retired nodes for this producer thread
    lock_free_queue_t<int>::cleanup_this_thread_retired_nodes();
  };

  // Consumer function
  auto consumer_func = [&]()
  {
    int value;
    int current_consumed = 0;
    while (current_consumed < total_items) {
      if (queue.try_dequeue(value)) {
        {
          std::lock_guard lock(consumed_mutex);
          consumed_items.push_back(value);
        }
        consumed_count++;
        current_consumed++;
      } else {
        // Yield or sleep briefly if queue is empty
        std::this_thread::yield();
      }
    }
    // Clean up retired nodes for this consumer thread
    lock_free_queue_t<int>::cleanup_this_thread_retired_nodes();
  };

  // Start producer threads
  for (int i = 0; i < num_producers; ++i) {
    producers.emplace_back(producer_func, i * items_per_producer);
  }

  // Start consumer thread
  std::thread consumer_thread(consumer_func);

  // Wait for producers to finish
  for (auto& p : producers) {
    p.join();
  }

  // Wait for consumer to finish
  consumer_thread.join();

  // Verification
  REQUIRE(consumed_count.load() == total_items);
  REQUIRE(consumed_items.size() == total_items);

  // Verify all produced items were consumed (order doesn't matter here)
  std::set<int> produced_set;
  for (int i = 0; i < num_producers; ++i) {
    for (int j = 0; j < items_per_producer; ++j) {
      produced_set.insert(i * items_per_producer + j);
    }
  }
  std::set<int> consumed_set(consumed_items.begin(), consumed_items.end());
  REQUIRE(produced_set == consumed_set);
}

/**
 * @brief Tests lock_free_queue_t with one producer thread and multiple consumer
 * threads.
 */
TEST_CASE("LockFreeQueue Single-Producer Multi-Consumer (SPMC)",
          "[container][lock_free_queue][multithreaded]")
{
  lock_free_queue_t<int> queue;
  const int num_consumers = 4;
  const int total_items = 4000;
  std::atomic_int produced_count(0);
  std::atomic_int consumed_count(0);
  std::vector<std::thread> consumers;
  std::vector<std::vector<int>> consumer_results(
      num_consumers);  // Per-consumer results
  std::mutex results_mutex;  // Not strictly needed if using atomic sum

  // Producer function
  auto producer_func = [&]()
  {
    for (int i = 0; i < total_items; ++i) {
      queue.enqueue(i);
      produced_count++;
    }
    // Clean up retired nodes for this producer thread
    lock_free_queue_t<int>::cleanup_this_thread_retired_nodes();
  };

  // Consumer function
  auto consumer_func = [&](int consumer_id)
  {
    int value;
    // Cast consumer_id for indexing
    size_t consumer_idx = static_cast<size_t>(consumer_id);
    while (consumed_count.load()
           < total_items) {  // Check global consumed count
      if (queue.try_dequeue(value)) {
        // Use casted index
        consumer_results[consumer_idx].push_back(value);  // Store locally
        consumed_count++;
      } else {
        // If producer might be finished and queue is empty, break
        if (produced_count.load() == total_items
            && queue.try_dequeue(value) == false)
        {
          // Double check after producer is done
          if (!queue.try_dequeue(value))
            break;  // exit if still empty
          else {  // got one element, process it and continue
            // Use casted index
            consumer_results[consumer_idx].push_back(value);
            consumed_count++;
          }
        } else {
          std::this_thread::yield();  // Yield if queue might still receive
                                      // items
        }
      }
    }
    // Clean up retired nodes for this consumer thread
    lock_free_queue_t<int>::cleanup_this_thread_retired_nodes();
  };

  // Start producer thread
  std::thread producer_thread(producer_func);

  // Start consumer threads
  for (int i = 0; i < num_consumers; ++i) {
    consumers.emplace_back(consumer_func, i);
  }

  // Wait for producer to finish
  producer_thread.join();

  // Wait for consumers to finish
  for (auto& c : consumers) {
    c.join();
  }

  // Verification
  REQUIRE(produced_count.load() == total_items);
  REQUIRE(consumed_count.load() == total_items);

  // Combine results and verify all items were consumed
  std::set<int> consumed_set;
  // Change type to size_t to match results.size()
  size_t total_consumed_locally = 0;
  for (const auto& results : consumer_results) {
    // No conversion needed now
    total_consumed_locally += results.size();
    for (int item : results) {
      consumed_set.insert(item);
    }
  }
  // Cast total_items to size_t for comparison
  REQUIRE(total_consumed_locally
          == static_cast<size_t>(total_items));  // Sanity check
  REQUIRE(consumed_set.size()
          == static_cast<size_t>(total_items));  // Check for duplicates/missing

  std::set<int> produced_set;
  for (int i = 0; i < total_items; ++i) {
    produced_set.insert(i);
  }
  REQUIRE(produced_set == consumed_set);
}

/**
 * @brief Tests lock_free_queue_t with multiple producer and multiple consumer
 * threads (MPMC).
 */
TEST_CASE("LockFreeQueue Multi-Producer Multi-Consumer (MPMC)",
          "[container][lock_free_queue][multithreaded]")
{
  lock_free_queue_t<int> queue;
  const int num_producers = 4;
  const int num_consumers = 4;
  const int items_per_producer = 1000;
  const int total_items = num_producers * items_per_producer;

  std::vector<std::thread> producers;
  std::vector<std::thread> consumers;
  std::atomic_int produced_count(0);
  std::atomic_int consumed_count(0);
  std::vector<std::vector<int>> consumer_results(num_consumers);

  // Producer function
  auto producer_func = [&](int producer_id)
  {
    int start_value = producer_id * items_per_producer;
    for (int i = 0; i < items_per_producer; ++i) {
      queue.enqueue(start_value + i);
      produced_count++;
    }
    lock_free_queue_t<int>::cleanup_this_thread_retired_nodes();
  };

  // Consumer function
  auto consumer_func = [&](int consumer_id)
  {
    int value;
    // Cast consumer_id for indexing
    size_t consumer_idx = static_cast<size_t>(consumer_id);
    while (consumed_count.load() < total_items) {  // Approximate exit condition
      if (queue.try_dequeue(value)) {
        // Use casted index
        consumer_results[consumer_idx].push_back(value);
        consumed_count++;
      } else {
        // Check if producers might be done AND queue seems empty
        if (produced_count.load() == total_items) {
          // Re-check queue one last time to handle race condition
          if (!queue.try_dequeue(value)) {
            break;  // Likely finished
          } else {
            // Got one, process it
            // Use casted index
            consumer_results[consumer_idx].push_back(value);
            consumed_count++;
          }
        } else {
          std::this_thread::yield();  // Still producing, yield
        }
      }
    }
    // Try one final dequeue sweep after loop exit, as count checks are racy
    int v_final;
    while (queue.try_dequeue(v_final)) {
      // Use casted index
      consumer_results[consumer_idx].push_back(v_final);
      consumed_count++;  // Note: This might overshoot total_items slightly, but
                         // helps drain
    }
    lock_free_queue_t<int>::cleanup_this_thread_retired_nodes();
  };

  // Start producer threads
  for (int i = 0; i < num_producers; ++i) {
    producers.emplace_back(producer_func, i);
  }

  // Start consumer threads
  for (int i = 0; i < num_consumers; ++i) {
    consumers.emplace_back(consumer_func, i);
  }

  // Wait for producers to finish
  for (auto& p : producers) {
    p.join();
  }

  // Wait for consumers to finish
  for (auto& c : consumers) {
    c.join();
  }

  // Verification (final count might be slightly off due to race on
  // consumed_count)
  std::set<int> consumed_set;
  // Change type to size_t
  size_t total_consumed_locally = 0;
  for (const auto& results : consumer_results) {
    // No conversion needed
    total_consumed_locally += results.size();
    for (int item : results) {
      consumed_set.insert(item);
    }
  }

  // The most reliable check is the content of the consumed set
  // Cast total_items to size_t for comparison
  REQUIRE(total_consumed_locally
          == static_cast<size_t>(total_items));  // Check exact count collected
  REQUIRE(consumed_set.size()
          == static_cast<size_t>(total_items));  // Check for duplicates/missing

  std::set<int> produced_set;
  for (int i = 0; i < num_producers; ++i) {
    for (int j = 0; j < items_per_producer; ++j) {
      produced_set.insert(i * items_per_producer + j);
    }
  }
  REQUIRE(produced_set == consumed_set);
}
