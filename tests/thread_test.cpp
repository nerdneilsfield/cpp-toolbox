#include "cpp_toolbox.hpp"

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this
                          // in
#include "catch.hpp"

using namespace cpp_toolbox::thread;

TEST_CASE("TEST REENTRANTRWLockTest") {
  SECTION("Read Lock") {
    int count = 0;
    int thread_init = 0;
    bool flag = true;
    ReentrantRWLock lock;
    auto f = [&]() {
      ReadLockGuard<ReentrantRWLock> lg(lock);
      count++;
      thread_init++;
      while (flag) {
        std::this_thread::yield();
      }
    };
    std::thread t1(f);
    std::thread t2(f);
    while (thread_init != 2) {
      std::this_thread::yield();
    }
    REQUIRE(2 == count);
    flag = false;
    t1.join();
    t2.join();
    {
      ReadLockGuard<ReentrantRWLock> lg1(lock);
      {
        ReadLockGuard<ReentrantRWLock> lg2(lock);
        {
          ReadLockGuard<ReentrantRWLock> lg3(lock);
          { ReadLockGuard<ReentrantRWLock> lg4(lock); }
        }
      }
    }
  }

  SECTION("WRITE LOCK") {
    int count = 0;
    int thread_run = 0;
    bool flag = true;
    ReentrantRWLock lock(false);
    auto f = [&]() {
      thread_run++;
      WriteLockGuard<ReentrantRWLock> lg(lock);
      count++;
      while (flag) {
        std::this_thread::yield();
      }
    };
    std::thread t1(f);
    std::thread t2(f);
    while (thread_run != 2) {
      std::this_thread::yield();
    }
    REQUIRE(1 == count);
    flag = false;
    t1.join();
    t2.join();

    {
      WriteLockGuard<ReentrantRWLock> lg1(lock);
      {
        WriteLockGuard<ReentrantRWLock> lg2(lock);
        { ReadLockGuard<ReentrantRWLock> lg3(lock); }
      }
    }
  }
}

TEST_CASE("TEST BoundedQueue") {
  SECTION("Enqueue") {
    BoundedQueue<int> queue;
    queue.Init(100);
    REQUIRE(0 == queue.Size());
    REQUIRE(queue.Empty());
    for (int i = 1; i <= 100; i++) {
      REQUIRE(queue.Enqueue(i));
      REQUIRE(i == queue.Size());
    }
    REQUIRE(!queue.Enqueue(101));
  }

  SECTION("Dequeue") {
    BoundedQueue<int> queue;
    queue.Init(100);
    for (int i = 1; i <= 100; i++) {
      queue.Enqueue(i);
    }

    int value;

    for (int i = 1; i <= 100; i++) {
      REQUIRE(queue.Dequeue(&value));
      REQUIRE(i == value);
    }
    REQUIRE(!queue.Dequeue(&value));
  }

  SECTION("Concurrency") {
    BoundedQueue<int> queue;
    queue.Init(10);
    std::atomic_int count = {0};
    std::thread threads[48];
    for (int i = 0; i < 48; ++i) {
      if (i % 4 == 0) {
        threads[i] = std::thread([&]() {
          for (int j = 0; j < 10000; ++j) {
            if (queue.Enqueue(j)) {
              count++;
            }
          }
        });
      } else if (i % 4 == 1) {
        threads[i] = std::thread([&]() {
          for (int j = 0; j < 10000; ++j) {
            if (queue.WaitEnqueue(j)) {
              count++;
            }
          }
        });
      } else if (i % 4 == 2) {
        threads[i] = std::thread([&]() {
          for (int j = 0; j < 10000; ++j) {
            int value = 0;
            if (queue.Dequeue(&value)) {
              count--;
            }
          }
        });
      } else {
        threads[i] = std::thread([&]() {
          for (int j = 0; j < 10000; ++j) {
            int value = 0;
            if (queue.WaitDequeue(&value)) {
              count--;
            }
          }
        });
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    queue.BreakAllWait();
    for (int i = 0; i < 48; ++i) {
      threads[i].join();
    }
    REQUIRE(count.load() == queue.Size());
  }

  SECTION("WaitDequeue") {
    BoundedQueue<int> queue;
    queue.Init(100);
    queue.Enqueue(10);
    std::thread t([&]() {
      int value = 0;
      queue.WaitDequeue(&value);
      REQUIRE(10 == value);
      queue.WaitDequeue(&value);
      REQUIRE(100 == value);
    });
    queue.Enqueue(100);
    t.join();
  }

  SECTION("block_wait") {
    BoundedQueue<int> queue;
    queue.Init(100, new BlockWaitStrategy());
    std::thread t([&]() {
      int value = 0;
      queue.WaitDequeue(&value);
      REQUIRE(100 == value);
    });
    queue.Enqueue(100);
    t.join();
  }

  SECTION("yield_wait") {
    BoundedQueue<int> queue;
    queue.Init(100, new YieldWaitStrategy());
    std::thread t([&]() {
      int value = 0;
      queue.WaitDequeue(&value);
      REQUIRE(100 == value);
    });
    queue.Enqueue(100);
    t.join();
  }

  SECTION("spin_wait") {
    BoundedQueue<int> queue;
    queue.Init(100, new BusySpinWaitStrategy());
    std::thread t([&]() {
      int value = 0;
      queue.WaitDequeue(&value);
      REQUIRE(100 == value);
    });
    queue.Enqueue(100);
    t.join();
  }

  SECTION("busy_wait") {
    BoundedQueue<int> queue;
    queue.Init(100, new BusySpinWaitStrategy());
    std::thread t([&]() {
      int value = 0;
      queue.WaitDequeue(&value);
      REQUIRE(100 == value);
    });
    queue.Enqueue(100);
    t.join();
  }

  
}