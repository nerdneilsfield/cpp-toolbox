#pragma once

#include <cstddef>
#include <vector>
#include <array>
#include <type_traits>

namespace toolbox::metrics
{

template<typename Derived, typename ElementType>
class base_metric_t
{
public:
  using element_type = ElementType;
  using result_type = ElementType;

  // Raw pointer interface
  constexpr ElementType distance(const ElementType* a,
                                 const ElementType* b,
                                 std::size_t size) const
  {
    return static_cast<const Derived*>(this)->distance_impl(a, b, size);
  }

  constexpr ElementType squared_distance(const ElementType* a,
                                         const ElementType* b,
                                         std::size_t size) const
  {
    return static_cast<const Derived*>(this)->squared_distance_impl(a, b, size);
  }

  // Container interface for convenience
  template<typename Container>
  constexpr ElementType distance(const Container& a, const Container& b) const
  {
    static_assert(std::is_same_v<typename Container::value_type, ElementType>,
                  "Container must have matching element type");
    return distance(a.data(), b.data(), a.size());
  }

  template<typename Container>
  constexpr ElementType squared_distance(const Container& a, const Container& b) const
  {
    static_assert(std::is_same_v<typename Container::value_type, ElementType>,
                  "Container must have matching element type");
    return squared_distance(a.data(), b.data(), a.size());
  }

};

// Type alias for common use cases
template<typename T>
using metric_ptr = std::unique_ptr<base_metric_t<void, T>>;

}  // namespace toolbox::metrics