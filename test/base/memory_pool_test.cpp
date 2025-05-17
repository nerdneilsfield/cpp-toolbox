#include <atomic>
#include <thread>
#include <vector>

#include "cpp-toolbox/base/memory_pool.hpp"

#include <catch2/catch_test_macros.hpp>

using toolbox::base::memory_pool_t;

TEST_CASE("MemoryPool Basic Operations", "[base][memory_pool]")
{
  memory_pool_t pool(32, 2);
  void* p1 = pool.allocate();
  REQUIRE(p1 != nullptr);
  void* p2 = pool.allocate();
  REQUIRE(p2 != nullptr);
  REQUIRE(pool.free_blocks() == 0);

  pool.deallocate(p1);
  pool.deallocate(p2);
  REQUIRE(pool.free_blocks() == 2);
}

TEST_CASE("MemoryPool Allocates When Empty", "[base][memory_pool]")
{
  memory_pool_t pool(16, 1);
  void* p1 = pool.allocate();
  void* p2 = pool.allocate();  // triggers new allocation
  REQUIRE(p1 != nullptr);
  REQUIRE(p2 != nullptr);
  REQUIRE(pool.free_blocks() == 0);
  pool.deallocate(p1);
  pool.deallocate(p2);
  REQUIRE(pool.free_blocks() == 2);
}

TEST_CASE("MemoryPool Thread Safety", "[base][memory_pool][multithreaded]")
{
  memory_pool_t pool(64);
  const int iterations = 500;
  const int num_threads = 4;
  std::atomic_int ops {0};
  std::vector<std::thread> threads;

  auto work = [&]()
  {
    for (int i = 0; i < iterations; ++i) {
      void* p = pool.allocate();
      if (p) {
        std::this_thread::yield();
        pool.deallocate(p);
        ++ops;
      }
    }
  };

  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back(work);
  }
  for (auto& t : threads) {
    t.join();
  }

  REQUIRE(ops.load() == iterations * num_threads);
}

TEST_CASE("MemoryPool Shrinks When Exceeding Cache", "[base][memory_pool]")
{
  memory_pool_t pool(8, 0, 2, 3);  // growth=3, max cache=2

  void* blocks[5];
  for (int i = 0; i < 5; ++i) {
    blocks[i] = pool.allocate();
  }

  // 1 block should remain cached after allocations (5 allocated from two
  // batches of 3)
  REQUIRE(pool.free_blocks() == 1);

  for (int i = 0; i < 5; ++i) {
    pool.deallocate(blocks[i]);
  }

  // Pool should shrink back to max_cached_blocks
  REQUIRE(pool.free_blocks() == 2);

  pool.release_unused();
  REQUIRE(pool.free_blocks() == 0);
}