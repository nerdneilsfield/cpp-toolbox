#pragma once

#include <memory>

namespace toolbox::base::detail
{
// Abstract base class for type erasure
struct task_base
{
  virtual ~task_base() = default;
  virtual void execute() = 0;  // Pure virtual function to execute the task
};

// Template derived class to hold the actual callable object
template<typename F>
struct task_derived : task_base
{
  F func;  // The callable object (lambda, function, etc.)

  // Constructor moves the callable object into the 'func' member
  explicit task_derived(F&& f)
      : func(std::move(f))
  {
  }

  // Override execute to call the stored callable
  void execute() override { func(); }
};
}  // namespace toolbox::base::detail