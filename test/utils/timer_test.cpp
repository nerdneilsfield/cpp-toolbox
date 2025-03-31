// test/timer/timer_test.cpp
#define CATCH_CONFIG_MAIN  // Provides main() function for Catch2

#include <chrono>
#include <thread>

#include "cpp-toolbox/utils/timer.hpp"  // Adjust path if needed

#include <catch2/catch_approx.hpp>  // For floating-point comparisons
#include <catch2/catch_test_macros.hpp>

using namespace std::chrono_literals;  // For durations like 100ms
using namespace toolbox::utils;

// Helper function for delays
void delay(std::chrono::milliseconds duration)
{
  std::this_thread::sleep_for(duration);
}

TEST_CASE("StopWatchTimer Basic Tests", "[timer][stopwatch]")
{
  StopWatchTimer timer("Basic");

  SECTION("Initial state")
  {
    REQUIRE(timer.elapsed_time_ms() == Catch::Approx(0.0));
    REQUIRE(timer.elapsed_time() == Catch::Approx(0.0));
    REQUIRE(timer.get_name() == "Basic");
  }

  SECTION("Start and Stop")
  {
    timer.start();
    delay(100ms);
    timer.stop();
    REQUIRE(timer.elapsed_time_ms() > 90.0);  // Allow for scheduling variance
    REQUIRE(timer.elapsed_time_ms() < 150.0);  // Upper bound
    REQUIRE(timer.elapsed_time()
            == Catch::Approx(timer.elapsed_time_ms() * 1e-3));

    double first_time = timer.elapsed_time_ms();

    // Stop again without start - should not increase time
    timer.stop();
    REQUIRE(timer.elapsed_time_ms() == Catch::Approx(first_time));

    // Start again and add more time
    timer.start();
    delay(50ms);
    timer.stop();
    REQUIRE(timer.elapsed_time_ms() > first_time + 40.0);
    REQUIRE(timer.elapsed_time_ms() < first_time + 100.0);
  }

  SECTION("Reset")
  {
    timer.start();
    delay(50ms);
    timer.stop();
    REQUIRE(timer.elapsed_time_ms() > 0.0);

    timer.reset();
    REQUIRE(timer.elapsed_time_ms() == Catch::Approx(0.0));
    REQUIRE(timer.elapsed_time() == Catch::Approx(0.0));

    // Time after reset
    timer.start();
    delay(60ms);
    timer.stop();
    REQUIRE(timer.elapsed_time_ms() > 50.0);
    REQUIRE(timer.elapsed_time_ms() < 110.0);
  }

  SECTION("Get time while running")
  {
    timer.start();
    delay(50ms);
    double time1 = timer.elapsed_time_ms();
    REQUIRE(time1 > 40.0);
    REQUIRE(time1 < 100.0);
    delay(50ms);
    double time2 = timer.elapsed_time_ms();
    REQUIRE(time2 > time1 + 40.0);
    REQUIRE(time2 < time1 + 100.0);
    REQUIRE(time2 > 90.0);  // Total should be > 90ms
    timer.stop();
    double final_time = timer.elapsed_time_ms();
    REQUIRE(
        final_time
        == Catch::Approx(time2).margin(10.0));  // Should be close to last read
    REQUIRE(final_time > 90.0);
  }

  SECTION("Set Name")
  {
    timer.set_name("New Name");
    REQUIRE(timer.get_name() == "New Name");
  }
}

TEST_CASE("SimpleTimerArray Tests", "[timer][array]")
{
  SECTION("Constructor with size")
  {
    SimpleTimerArray timers(3);
    REQUIRE(timers.size() == 3);
    // Default names are timer_0, timer_1, timer_2 - checking one is enough
    // Need a way to get the StopWatchTimer object or its name to verify this.
    // Let's add a get_timer_name(index) method for testing, or rely on print
    // output. For now, just check size and basic operation.
    REQUIRE(timers.elapsed_time_ms(0) == Catch::Approx(0.0));
    REQUIRE(timers.elapsed_time_ms(1) == Catch::Approx(0.0));
    REQUIRE(timers.elapsed_time_ms(2) == Catch::Approx(0.0));
  }

  SECTION("Constructor with names")
  {
    std::vector<std::string> names = {"Load", "Process", "Save"};
    SimpleTimerArray timers(names);
    REQUIRE(timers.size() == 3);
    // Again, checking names requires access or print check.
    REQUIRE(timers.elapsed_time_ms(0) == Catch::Approx(0.0));
    REQUIRE(timers.elapsed_time_ms(1) == Catch::Approx(0.0));
    REQUIRE(timers.elapsed_time_ms(2) == Catch::Approx(0.0));
  }

  SECTION("Invalid Constructor Args")
  {
    REQUIRE_THROWS_AS(SimpleTimerArray(0), std::invalid_argument);
    REQUIRE_THROWS_AS(SimpleTimerArray(-1), std::invalid_argument);
    std::vector<std::string> empty_names;
    REQUIRE_THROWS_AS(SimpleTimerArray(empty_names), std::invalid_argument);
  }

  SECTION("Basic Operations and Indexing")
  {
    std::vector<std::string> names = {"A", "B"};
    SimpleTimerArray timers(names);

    timers.start(0);
    delay(50ms);
    timers.stop(0);

    timers.start(1);
    delay(100ms);
    timers.stop(1);

    REQUIRE(timers.elapsed_time_ms(0) > 40.0);
    REQUIRE(timers.elapsed_time_ms(0) < 90.0);
    REQUIRE(timers.elapsed_time_ms(1) > 90.0);
    REQUIRE(timers.elapsed_time_ms(1) < 150.0);

    // Test cumulative
    timers.start(0);
    delay(60ms);
    timers.stop(0);
    REQUIRE(timers.elapsed_time_ms(0) > 100.0);  // 50ms + 60ms roughly
    REQUIRE(timers.elapsed_time_ms(0) < 180.0);

    // Test reset
    timers.reset(1);
    REQUIRE(timers.elapsed_time_ms(1) == Catch::Approx(0.0));

    // Test getters
    REQUIRE(timers.elapsed_time(0)
            == Catch::Approx(timers.elapsed_time_ms(0) * 1e-3));
  }

  SECTION("Index Out of Bounds")
  {
    SimpleTimerArray timers(2);
    REQUIRE_THROWS_AS(timers.start(-1), std::out_of_range);
    REQUIRE_THROWS_AS(timers.start(2), std::out_of_range);
    REQUIRE_THROWS_AS(timers.stop(2), std::out_of_range);
    REQUIRE_THROWS_AS(timers.reset(2), std::out_of_range);
    REQUIRE_THROWS_AS(timers.elapsed_time_ms(2), std::out_of_range);
    REQUIRE_THROWS_AS(timers.elapsed_time(2), std::out_of_range);
    REQUIRE_THROWS_AS(timers.print_stats(2), std::out_of_range);

    // Check valid indices don't throw
    REQUIRE_NOTHROW(timers.start(0));
    REQUIRE_NOTHROW(timers.stop(1));
    REQUIRE_NOTHROW(timers.reset(0));
    REQUIRE_NOTHROW(timers.elapsed_time_ms(1));
    REQUIRE_NOTHROW(timers.elapsed_time(0));
    // print_stats prints to cout, difficult to check here without redirection
    // REQUIRE_NOTHROW(timers.print_stats(0));
    // REQUIRE_NOTHROW(timers.print_all_stats());
  }

  SECTION("Get time while running in array")
  {
    SimpleTimerArray timers(1);
    timers.start(0);
    delay(50ms);
    double time1 = timers.elapsed_time_ms(0);
    REQUIRE(time1 > 40.0);
    REQUIRE(time1 < 100.0);
    delay(50ms);
    double time2 = timers.elapsed_time_ms(0);
    REQUIRE(time2 > time1 + 40.0);
    timers.stop(0);
    double final_time = timers.elapsed_time_ms(0);
    REQUIRE(final_time == Catch::Approx(time2).margin(10.0));
    REQUIRE(final_time > 90.0);
  }
}