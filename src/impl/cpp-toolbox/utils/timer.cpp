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

// --- StopWatchTimer Implementation ---
StopWatchTimer::StopWatchTimer(std::string name)
    : name_(std::move(name))
    ,  // Use std::move for efficiency
    total_duration_(0)
    , running_(false)
{
  // Initialize start_time_ although it's only relevant when running
  start_time_ = std::chrono::high_resolution_clock::now();
}

void StopWatchTimer::set_name(const std::string& name)
{
  name_ = name;
}

const std::string& StopWatchTimer::get_name() const
{
  return name_;
}

void StopWatchTimer::start()
{
  // Avoid restarting if already running, just update start time
  start_time_ = std::chrono::high_resolution_clock::now();
  running_ = true;
}

void StopWatchTimer::stop()
{
  if (running_) {
    auto end_time = std::chrono::high_resolution_clock::now();
    total_duration_ += std::chrono::duration_cast<std::chrono::nanoseconds>(
        end_time - start_time_);
    running_ = false;
  }
  // If stop() is called when not running, do nothing.
}

void StopWatchTimer::reset()
{
  total_duration_ = std::chrono::nanoseconds(0);
  running_ = false;
  // Optionally reset start_time_? Not strictly necessary until next start()
  // start_time_ = std::chrono::high_resolution_clock::now();
}

auto StopWatchTimer::elapsed_time_ms() const -> double
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

auto StopWatchTimer::elapsed_time() const -> double
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

void StopWatchTimer::print_stats() const
{
  // Use std::fixed and std::setprecision for consistent output format
  std::cout << std::fixed << std::setprecision(4);
  std::cout << "Timer [" << name_ << "]: " << elapsed_time_ms() << " ms ("
            << elapsed_time() << " s)" << std::endl;
}

// --- SimpleTimerArray Implementation ---

SimpleTimerArray::SimpleTimerArray(int size)
{
  if (size <= 0) {
    throw std::invalid_argument("Timer array size must be positive.");
  }
  timers_.reserve(size);  // Reserve space for efficiency
  for (int i = 0; i < size; ++i) {
    // Use emplace_back to construct in place
    timers_.emplace_back("timer_" + std::to_string(i));
  }
}

SimpleTimerArray::SimpleTimerArray(const std::vector<std::string>& names)
{
  if (names.empty()) {
    throw std::invalid_argument("Timer name vector cannot be empty.");
  }
  timers_.reserve(names.size());
  for (const auto& name : names) {
    timers_.emplace_back(name);  // Construct with given name
  }
}

void SimpleTimerArray::check_id(int index) const
{
  // Use size() method of vector, check >= 0 for signed/unsigned comparison
  // safety
  if (index < 0 || static_cast<size_t>(index) >= timers_.size()) {
    throw std::out_of_range("Timer index " + std::to_string(index)
                            + " is out of range (size: "
                            + std::to_string(timers_.size()) + ").");
  }
}

void SimpleTimerArray::start(int index)
{
  check_id(index);
  timers_[index].start();
}

void SimpleTimerArray::stop(int index)
{
  check_id(index);
  timers_[index].stop();
}

void SimpleTimerArray::reset(int index)
{
  check_id(index);
  timers_[index].reset();
}

auto SimpleTimerArray::elapsed_time_ms(int index) const -> double
{
  check_id(index);
  return timers_[index].elapsed_time_ms();
}

auto SimpleTimerArray::elapsed_time(int index) const -> double
{
  check_id(index);
  return timers_[index].elapsed_time();
}

void SimpleTimerArray::print_stats(int index) const
{
  check_id(index);
  timers_[index].print_stats();
}

void SimpleTimerArray::print_all_stats() const
{
  std::cout << "--- All Timer Stats ---" << std::endl;
  for (const auto& timer : timers_) {  // Use range-based for loop
    timer.print_stats();
  }
  std::cout << "-----------------------" << std::endl;
}

auto SimpleTimerArray::size() const -> size_t
{
  return timers_.size();
}

}  // namespace toolbox::utils
