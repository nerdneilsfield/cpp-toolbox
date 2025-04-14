#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <thread>  // For std::this_thread::sleep_for
#include <vector>

#include "cpp-toolbox/utils/timer.hpp"
// for setprecision

namespace toolbox::utils
{

// --- stop_watch_timer_t Implementation ---
stop_watch_timer_t::stop_watch_timer_t(std::string name)
    : name_(std::move(name))
    ,  // Use std::move for efficiency
    total_duration_(0)
    , running_(false)
{
  // Initialize start_time_ although it's only relevant when running
  start_time_ = std::chrono::high_resolution_clock::now();
}

void stop_watch_timer_t::set_name(const std::string& name)
{
  name_ = name;
}

const std::string& stop_watch_timer_t::get_name() const
{
  return name_;
}

void stop_watch_timer_t::start()
{
  // Avoid restarting if already running, just update start time
  start_time_ = std::chrono::high_resolution_clock::now();
  running_ = true;
}

void stop_watch_timer_t::stop()
{
  if (running_) {
    auto end_time = std::chrono::high_resolution_clock::now();
    total_duration_ += std::chrono::duration_cast<std::chrono::nanoseconds>(
        end_time - start_time_);
    running_ = false;
  }
  // If stop() is called when not running, do nothing.
}

void stop_watch_timer_t::reset()
{
  total_duration_ = std::chrono::nanoseconds(0);
  running_ = false;
  // Optionally reset start_time_? Not strictly necessary until next start()
  // start_time_ = std::chrono::high_resolution_clock::now();
}

auto stop_watch_timer_t::elapsed_time_ms() const -> double
{
  // If the timer is currently running, add the time since the last start()
  std::chrono::nanoseconds current_total = total_duration_;
  if (running_) {
    auto now = std::chrono::high_resolution_clock::now();
    current_total +=
        std::chrono::duration_cast<std::chrono::nanoseconds>(now - start_time_);
  }
  return static_cast<double>(current_total.count()) * 1e-6;
}

auto stop_watch_timer_t::elapsed_time() const -> double
{
  // If the timer is currently running, add the time since the last start()
  std::chrono::nanoseconds current_total = total_duration_;
  if (running_) {
    auto now = std::chrono::high_resolution_clock::now();
    current_total +=
        std::chrono::duration_cast<std::chrono::nanoseconds>(now - start_time_);
  }
  return static_cast<double>(current_total.count()) * 1e-9;
}

void stop_watch_timer_t::print_stats() const
{
  // Use std::fixed and std::setprecision for consistent output format
  std::cout << std::fixed << std::setprecision(4);
  std::cout << "Timer [" << name_ << "]: " << elapsed_time_ms() << " ms ("
            << elapsed_time() << " s)" << std::endl;
}

// --- simple_timer_array_t Implementation ---

simple_timer_array_t::simple_timer_array_t(int size)
{
  if (size <= 0) {
    throw std::invalid_argument("Timer array size must be positive.");
  }
  // Cast size to size_t after checking it's positive
  timers_.reserve(static_cast<size_t>(size));  // Reserve space for efficiency
  for (int i = 0; i < size; ++i) {
    // Use emplace_back to construct in place
    timers_.emplace_back("timer_" + std::to_string(i));
  }
}

simple_timer_array_t::simple_timer_array_t(
    const std::vector<std::string>& names)
{
  if (names.empty()) {
    throw std::invalid_argument("Timer name vector cannot be empty.");
  }
  timers_.reserve(names.size());
  for (const auto& name : names) {
    timers_.emplace_back(name);  // Construct with given name
  }
}

void simple_timer_array_t::check_id(int index) const
{
  // Use size() method of vector, check >= 0 for signed/unsigned comparison
  // safety
  if (index < 0 || static_cast<size_t>(index) >= timers_.size()) {
    throw std::out_of_range("Timer index " + std::to_string(index)
                            + " is out of range (size: "
                            + std::to_string(timers_.size()) + ").");
  }
}

void simple_timer_array_t::start(int index)
{
  check_id(index);
  // Cast index to size_t after check_id confirms it's non-negative
  timers_[static_cast<size_t>(index)].start();
}

void simple_timer_array_t::stop(int index)
{
  check_id(index);
  // Cast index to size_t after check_id confirms it's non-negative
  timers_[static_cast<size_t>(index)].stop();
}

void simple_timer_array_t::reset(int index)
{
  check_id(index);
  // Cast index to size_t after check_id confirms it's non-negative
  timers_[static_cast<size_t>(index)].reset();
}

auto simple_timer_array_t::elapsed_time_ms(int index) const -> double
{
  check_id(index);
  // Cast index to size_t after check_id confirms it's non-negative
  return timers_[static_cast<size_t>(index)].elapsed_time_ms();
}

auto simple_timer_array_t::elapsed_time(int index) const -> double
{
  check_id(index);
  // Cast index to size_t after check_id confirms it's non-negative
  return timers_[static_cast<size_t>(index)].elapsed_time();
}

void simple_timer_array_t::print_stats(int index) const
{
  check_id(index);
  // Cast index to size_t after check_id confirms it's non-negative
  timers_[static_cast<size_t>(index)].print_stats();
}

void simple_timer_array_t::print_all_stats() const
{
  std::cout << "--- All Timer Stats ---" << std::endl;
  for (const auto& timer : timers_) {  // Use range-based for loop
    timer.print_stats();
  }
  std::cout << "-----------------------" << std::endl;
}

auto simple_timer_array_t::size() const -> size_t
{
  return timers_.size();
}

}  // namespace toolbox::utils
