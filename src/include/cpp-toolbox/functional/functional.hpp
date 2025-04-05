#pragma once

#include <algorithm>
#include <functional>  // Potentially useful, though not strictly required by compose itself
#include <iostream>
#include <iterator>
#include <memory>  // std::shared_ptr, std::make_shared
#include <mutex>  // std::mutex
#include <numeric>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>  // std::invoke_result_t, std::decay_t
#include <typeinfo>
#include <utility>  // std::forward, std::move
#include <variant>

#include <cpp-toolbox/cpp-toolbox_export.hpp>

namespace toolbox::functional
{

namespace detail
{
/**
 * @brief Helper class for creating overload sets
 * @tparam Fs Types of functions to overload
 *
 * @code{.cpp}
 * // Create overloaded visitor for variant
 * auto visitor = overloaded{
 *   [](int x) { return x * 2; },
 *   [](const std::string& s) { return s + s; }
 * };
 * @endcode
 */
template<class... Fs>
struct overloaded : Fs...
{
  using Fs::operator()...;  // C++17 using pack declaration
};

// C++17 class template argument deduction guide
// Allows writing overloaded{lambda1, lambda2} without explicit template args
template<class... Fs>
overloaded(Fs...) -> overloaded<Fs...>;

/**
 * @brief Type trait to check if T is a std::optional
 *
 * @code{.cpp}
 * static_assert(is_optional<std::optional<int>>::value, "is optional");
 * static_assert(!is_optional<int>::value, "not optional");
 * @endcode
 */
template<typename T>
struct is_optional : std::false_type
{
};

template<typename T>
struct is_optional<std::optional<T>> : std::true_type
{
};

template<typename T>
inline constexpr bool is_optional_v = is_optional<std::decay_t<T>>::value;

/**
 * @brief Type trait to check if type has size() member function
 *
 * @code{.cpp}
 * static_assert(has_size<std::vector<int>>::value, "has size");
 * static_assert(!has_size<int>::value, "no size");
 * @endcode
 */
template<typename T, typename = void>
struct has_size : std::false_type
{
};

template<typename T>
struct has_size<T, std::void_t<decltype(std::declval<T>().size())>>
    : std::true_type
{
};

/**
 * @brief Helper function to get minimum size from multiple containers
 * @tparam Containers Container types
 * @return Minimum size among all containers, or 0 if no containers
 *
 * @code{.cpp}
 * std::vector<int> v1{1,2,3};
 * std::list<int> l1{1,2};
 * auto min_size = get_min_size(v1, l1); // Returns 2
 * @endcode
 */
template<typename... Containers>
auto get_min_size(const Containers&... containers) -> size_t
{
  if constexpr (sizeof...(containers) == 0) {
    return 0;  // Handle empty pack explicitly
  } else {
    return std::min({static_cast<size_t>(containers.size())...});
  }
}

/**
 * @brief Helper function to increment all iterators in a tuple
 *
 * @code{.cpp}
 * auto iters = std::make_tuple(v.begin(), l.begin());
 * increment_iterators(iters,
 * std::index_sequence_for<decltype(v),decltype(l)>{});
 * @endcode
 */
template<typename Tuple, std::size_t... Is>
void increment_iterators(Tuple& iter_tuple, std::index_sequence<Is...>)
{
  (++(std::get<Is>(iter_tuple)), ...);
}

/**
 * @brief Helper function to dereference iterators and create tuple of
 * references
 *
 * @code{.cpp}
 * auto iters = std::make_tuple(v.begin(), l.begin());
 * auto refs = dereference_iterators_as_tuple(iters,
 * std::index_sequence_for<decltype(v),decltype(l)>{});
 * @endcode
 */
template<typename Tuple, std::size_t... Is>
auto dereference_iterators_as_tuple(Tuple& iter_tuple,
                                    std::index_sequence<Is...>)
{
  return std::forward_as_tuple(*(std::get<Is>(iter_tuple))...);
}

/**
 * @brief Internal state for memoization
 *
 * @code{.cpp}
 * MemoizeState<int(int), int, int> state{[](int x) { return x*x; }};
 * auto result = state.original_func(5); // Returns 25
 * @endcode
 */
template<typename Func, typename R, typename... Args>
struct MemoizeState
{
  using KeyType = std::tuple<std::decay_t<Args>...>;
  using ResultType = R;

  Func original_func;
  std::map<KeyType, ResultType> cache;
  std::mutex cache_mutex;

  MemoizeState(Func&& f)
      : original_func(std::forward<Func>(f))
  {
  }
};
}  // namespace detail

/**
 * @brief Composes two functions into a single function
 * @tparam G Type of outer function
 * @tparam F Type of inner function
 * @param g Outer function
 * @param f Inner function
 * @return A function that applies f then g
 *
 * @code{.cpp}
 * // Example of function composition
 * auto add_one = [](int x) { return x + 1; };
 * auto multiply_two = [](int x) { return x * 2; };
 * auto composed = compose(multiply_two, add_one);
 * int result = composed(5); // Returns 12: ((5 + 1) * 2)
 * @endcode
 */
template<typename G, typename F>
CPP_TOOLBOX_EXPORT auto compose(G&& g, F&& f)
{
  return
      [g = std::forward<G>(g), f = std::forward<F>(f)](auto&&... args) mutable
          -> decltype(g(f(std::forward<decltype(args)>(args)...)))
  { return g(f(std::forward<decltype(args)>(args)...)); };
}

/**
 * @brief Composes multiple functions into a single function
 * @tparam F1 Type of first (outermost) function
 * @tparam FRest Types of remaining functions
 * @param f1 First function
 * @param rest Remaining functions
 * @return A function that applies functions from right to left
 *
 * @code{.cpp}
 * // Example of multiple function composition
 * auto add_one = [](int x) { return x + 1; };
 * auto multiply_two = [](int x) { return x * 2; };
 * auto square = [](int x) { return x * x; };
 * auto composed = compose(square, multiply_two, add_one);
 * int result = composed(5); // Returns 144: ((5 + 1) * 2)^2
 * @endcode
 */
template<typename F1, typename... FRest>
CPP_TOOLBOX_EXPORT auto compose(F1&& f1, FRest&&... rest)
{
  if constexpr (sizeof...(FRest) == 0) {
    return [f1_cap =
                std::forward<F1>(f1)](auto&&... args) mutable -> decltype(auto)
    { return f1_cap(std::forward<decltype(args)>(args)...); };
  } else {
    auto composed_rest = compose(std::forward<FRest>(rest)...);

    return [f1_cap = std::forward<F1>(f1),
            composed_rest_cap = std::move(composed_rest)](
               auto&&... args) mutable -> decltype(auto)
    {
      return f1_cap(composed_rest_cap(std::forward<decltype(args)>(args)...));
    };
  }
}

/**
 * @brief Empty compose function that throws an error
 * @throws std::logic_error when called
 *
 * @code{.cpp}
 * try {
 *   auto empty = compose();
 * } catch(const std::logic_error& e) {
 *   // Exception caught
 * }
 * @endcode
 */
inline CPP_TOOLBOX_EXPORT auto compose()
{
  throw std::logic_error("compose called with no functions");
}

/**
 * @brief Binds the first argument of a function
 * @tparam F Function type
 * @tparam Arg1 Type of first argument
 * @param f Function to bind
 * @param arg1 Value to bind as first argument
 * @return Function with first argument bound
 *
 * @code{.cpp}
 * // Example of binding first argument
 * auto divide = [](int x, int y) { return x / y; };
 * auto divide_10_by = bind_first(divide, 10);
 * int result = divide_10_by(2); // Returns 5
 * @endcode
 */
template<typename F, typename Arg1>
CPP_TOOLBOX_EXPORT auto bind_first(F&& f, Arg1&& arg1)
{
  return
      [f = std::forward<F>(f),
       arg1 = std::forward<Arg1>(arg1)](auto&&... rest_args)
          -> decltype(f(arg1, std::forward<decltype(rest_args)>(rest_args)...))
  { return f(arg1, std::forward<decltype(rest_args)>(rest_args)...); };
}

/**
 * @brief Maps a function over an optional value
 * @tparam T Type contained in optional
 * @tparam F Function type
 * @param opt Optional to map over
 * @param f Function to apply
 * @return Optional containing result of f if opt has value, empty optional
 * otherwise
 *
 * @code{.cpp}
 * // Example of mapping over optional
 * std::optional<int> opt(5);
 * auto times_two = [](int x) { return x * 2; };
 * auto result = map(opt, times_two); // Contains 10
 * @endcode
 */
template<typename T, typename F>
CPP_TOOLBOX_EXPORT auto map(const std::optional<T>& opt, F&& f)
    -> std::optional<std::invoke_result_t<F, const T&>>
{
  using result_type = std::invoke_result_t<F, const T&>;
  using result_optional_type = std::optional<result_type>;

  if (opt.has_value()) {
    return result_optional_type(std::invoke(std::forward<F>(f), *opt));
  } else {
    return std::nullopt;
  }
}

/**
 * @brief Maps a function over an optional rvalue
 * @tparam T Type contained in optional
 * @tparam F Function type
 * @param opt Optional rvalue to map over
 * @param f Function to apply
 * @return Optional containing result of f if opt has value, empty optional
 * otherwise
 *
 * @code{.cpp}
 * // Example of mapping over optional rvalue
 * auto result = map(std::optional<int>(5), [](int x) { return x * 2; }); //
 * Contains 10
 * @endcode
 */
template<typename T, typename F>
CPP_TOOLBOX_EXPORT auto map(std::optional<T>&& opt, F&& f)
    -> std::optional<std::invoke_result_t<F, T&&>>
{
  using result_type = std::invoke_result_t<F, T&&>;
  using result_optional_type = std::optional<result_type>;

  if (opt.has_value()) {
    return result_optional_type(
        std::invoke(std::forward<F>(f), std::move(*opt)));
  } else {
    return std::nullopt;
  }
}

/**
 * @brief Flat maps a function over an optional value
 * @tparam T Type contained in optional
 * @tparam F Function type returning optional
 * @param opt Optional to flat map over
 * @param f Function to apply
 * @return Result of f if opt has value, empty optional otherwise
 *
 * @code{.cpp}
 * // Example of flat mapping over optional
 * std::optional<int> opt(5);
 * auto maybe_double = [](int x) -> std::optional<int> {
 *   return x < 10 ? std::optional(x * 2) : std::nullopt;
 * };
 * auto result = flatMap(opt, maybe_double); // Contains 10
 * @endcode
 */
template<typename T, typename F>
CPP_TOOLBOX_EXPORT auto flatMap(const std::optional<T>& opt, F&& f)
    -> std::invoke_result_t<F, const T&>
{
  using result_optional_type = std::invoke_result_t<F, const T&>;

  static_assert(detail::is_optional_v<result_optional_type>,
                "Function passed to flatMap must return a std::optional type.");

  if (opt.has_value()) {
    return std::invoke(std::forward<F>(f), *opt);
  } else {
    return result_optional_type {};
  }
}

/**
 * @brief Flat maps a function over an optional rvalue
 * @tparam T Type contained in optional
 * @tparam F Function type returning optional
 * @param opt Optional rvalue to flat map over
 * @param f Function to apply
 * @return Result of f if opt has value, empty optional otherwise
 *
 * @code{.cpp}
 * // Example of flat mapping over optional rvalue
 * auto result = flatMap(std::optional<int>(5),
 *                      [](int x) -> std::optional<int> {
 *                        return x < 10 ? std::optional(x * 2) : std::nullopt;
 *                      }); // Contains 10
 * @endcode
 */
template<typename T, typename F>
CPP_TOOLBOX_EXPORT auto flatMap(std::optional<T>&& opt, F&& f)
    -> std::invoke_result_t<F, T&&>
{
  using result_optional_type = std::invoke_result_t<F, T&&>;
  static_assert(detail::is_optional_v<result_optional_type>,
                "Function passed to flatMap must return a std::optional type.");

  if (opt.has_value()) {
    return std::invoke(std::forward<F>(f), std::move(*opt));
  } else {
    return result_optional_type {};
  }
}

/**
 * @brief Returns the contained value or a default
 * @tparam T Type contained in optional
 * @tparam U Type of default value
 * @param opt Optional to get value from
 * @param default_value Value to return if optional is empty
 * @return Contained value or default
 *
 * @code{.cpp}
 * // Example of providing default value
 * std::optional<int> opt;
 * int result = orElse(opt, 42); // Returns 42
 * @endcode
 */
template<typename T, typename U>
CPP_TOOLBOX_EXPORT auto orElse(const std::optional<T>& opt, U&& default_value)
    -> T
{
  static_assert(
      std::is_convertible_v<U, T>,
      "Default value type must be convertible to optional's value type T.");

  return opt.value_or(std::forward<U>(default_value));
}

/**
 * @brief Returns the contained value or calls function for default
 * @tparam T Type contained in optional
 * @tparam F Function type returning default value
 * @param opt Optional to get value from
 * @param default_func Function to call for default value
 * @return Contained value or result of default_func
 *
 * @code{.cpp}
 * // Example of providing default via function
 * std::optional<int> opt;
 * auto get_default = []() { return 42; };
 * int result = orElseGet(opt, get_default); // Returns 42
 * @endcode
 */
template<typename T, typename F>
CPP_TOOLBOX_EXPORT auto orElseGet(const std::optional<T>& opt, F&& default_func)
    -> T
{
  static_assert(std::is_invocable_v<F>,
                "Default function must be callable with no arguments.");

  using default_result_type = std::invoke_result_t<F>;
  static_assert(std::is_convertible_v<default_result_type, T>,
                "Default function's return type must be convertible to "
                "optional's value type T.");

  if (opt.has_value()) {
    return *opt;
  } else {
    return static_cast<T>(std::invoke(std::forward<F>(default_func)));
  }
}

/**
 * @brief Filters an optional based on a predicate
 * @tparam T Type contained in optional
 * @tparam P Predicate type
 * @param opt Optional to filter
 * @param p Predicate to apply
 * @return Optional if predicate returns true, empty optional otherwise
 *
 * @code{.cpp}
 * // Example of filtering optional
 * std::optional<int> opt(42);
 * auto is_even = [](int x) { return x % 2 == 0; };
 * auto result = filter(opt, is_even); // Contains 42
 * @endcode
 */
template<typename T, typename P>
CPP_TOOLBOX_EXPORT auto filter(const std::optional<T>& opt, P&& p)
    -> std::optional<T>
{
#if __cpp_lib_is_invocable >= 201703L
  static_assert(
      std::is_invocable_r_v<bool, P, const T&>
          || std::is_convertible_v<std::invoke_result_t<P, const T&>, bool>,
      "Predicate must be callable with const T& and return bool or "
      "bool-convertible.");
#else
  static_assert(
      std::is_convertible_v<std::invoke_result_t<P, const T&>, bool>,
      "Predicate must be callable with const T& and return bool-convertible.");
#endif

  return (opt.has_value() && std::invoke(std::forward<P>(p), *opt))
      ? opt
      : std::nullopt;
}

/**
 * @brief Filters an optional rvalue based on a predicate
 * @tparam T Type contained in optional
 * @tparam P Predicate type
 * @param opt Optional rvalue to filter
 * @param p Predicate to apply
 * @return Optional if predicate returns true, empty optional otherwise
 *
 * @code{.cpp}
 * // Example of filtering optional rvalue
 * auto result = filter(std::optional<int>(42), [](int x) { return x % 2 == 0;
 * }); // Contains 42
 * @endcode
 */
template<typename T, typename P>
CPP_TOOLBOX_EXPORT auto filter(std::optional<T>&& opt, P&& p)
    -> std::optional<T>
{
#if __cpp_lib_is_invocable >= 201703L
  static_assert(
      std::is_invocable_r_v<bool, P, const T&>
          || std::is_convertible_v<std::invoke_result_t<P, const T&>, bool>,
      "Predicate must be callable with const T& and return bool or "
      "bool-convertible.");
#else
  static_assert(
      std::is_convertible_v<std::invoke_result_t<P, const T&>, bool>,
      "Predicate must be callable with const T& and return bool-convertible.");
#endif

  if (opt.has_value() && std::invoke(std::forward<P>(p), *opt)) {
    return std::move(opt);
  } else {
    return std::nullopt;
  }
}

/**
 * @brief Pattern matches on a variant using visitor functions
 * @tparam Ts Types in variant
 * @tparam Fs Visitor function types
 * @param var Variant to match on
 * @param visitors Visitor functions for each type
 * @return Result of matched visitor
 *
 * @code{.cpp}
 * // Example of pattern matching on variant
 * std::variant<int, std::string> v = 42;
 * auto result = match(v,
 *   [](int i) { return i * 2; },
 *   [](const std::string& s) { return s.length(); }
 * ); // Returns 84
 * @endcode
 */
template<typename... Ts, typename... Fs>
CPP_TOOLBOX_EXPORT auto match(const std::variant<Ts...>& var, Fs&&... visitors)
    -> decltype(auto)
{
  static_assert(
      sizeof...(Ts) == sizeof...(Fs),
      "Number of visitors must match the number of types in the variant");
  auto visitor_set = detail::overloaded {std::forward<Fs>(visitors)...};
  return std::visit(visitor_set, var);
}

/**
 * @brief Match variant with visitor functions for non-const lvalue variant
 * @tparam Ts Types in variant
 * @tparam Fs Visitor function types
 * @param var Variant to match on
 * @param visitors Visitor functions for each type
 * @return Result of matched visitor
 *
 * @code{.cpp}
 * // Example of pattern matching on lvalue variant
 * std::variant<int, std::string> v = 42;
 * auto result = match(v,
 *   [](int& i) { i *= 2; return i; },
 *   [](std::string& s) { s += "!"; return s.length(); }
 * ); // Modifies v and returns 84
 * @endcode
 */
template<typename... Ts, typename... Fs>
CPP_TOOLBOX_EXPORT auto match(std::variant<Ts...>& var, Fs&&... visitors)
    -> decltype(auto)
{
  static_assert(
      sizeof...(Ts) == sizeof...(Fs),
      "Number of visitors must match the number of types in the variant");
  auto visitor_set = detail::overloaded {std::forward<Fs>(visitors)...};
  return std::visit(visitor_set, var);
}

/**
 * @brief Match variant with visitor functions for rvalue variant
 * @tparam Ts Types in variant
 * @tparam Fs Visitor function types
 * @param var Variant to match on
 * @param visitors Visitor functions for each type
 * @return Result of matched visitor
 *
 * @code{.cpp}
 * // Example of pattern matching on rvalue variant
 * auto result = match(std::variant<int, std::string>(42),
 *   [](int&& i) { return i * 2; },
 *   [](std::string&& s) { return s + "!"; }
 * ); // Returns 84
 * @endcode
 */
template<typename... Ts, typename... Fs>
CPP_TOOLBOX_EXPORT auto match(std::variant<Ts...>&& var, Fs&&... visitors)
    -> decltype(auto)
{
  static_assert(
      sizeof...(Ts) == sizeof...(Fs),
      "Number of visitors must match the number of types in the variant");
  auto visitor_set = detail::overloaded {std::forward<Fs>(visitors)...};
  return std::visit(visitor_set, std::move(var));
}

/**
 * @brief Apply a function to the value held by a variant and return result in a
 * new variant
 * @tparam ResultVariant The target variant type to return
 * @tparam Ts Types in input variant
 * @tparam F Function type to apply
 * @param var Input variant
 * @param f Function to apply to variant value
 * @return ResultVariant containing f's result
 * @throws std::bad_variant_access if var is valueless_by_exception
 * @throws Any exception thrown by f
 * @throws std::bad_variant_access if f's result cannot construct any
 * ResultVariant type
 *
 * @code{.cpp}
 * // Example of mapping over variant
 * std::variant<int, std::string> v = 42;
 * auto result = map<std::variant<double, size_t>>(v,
 *   [](const auto& x) -> std::variant<double, size_t> {
 *     if constexpr (std::is_same_v<std::decay_t<decltype(x)>, int>) {
 *       return x * 1.5; // int -> double
 *     } else {
 *       return x.length(); // string -> size_t
 *     }
 *   }
 * ); // Returns variant containing 63.0
 * @endcode
 */
template<typename ResultVariant, typename... Ts, typename F>
CPP_TOOLBOX_EXPORT auto map(const std::variant<Ts...>& var, F&& f)
    -> ResultVariant
{
  std::cout << "[Debug map(const variant)] Entered map function.\n";
  return std::visit(
      [&f](const auto& value) -> ResultVariant
      {
        std::cout << "  [Debug map] Inside visit lambda. Visited value type: "
                  << typeid(value).name() << ".\n";
        if constexpr (std::is_void_v<std::invoke_result_t<F, decltype(value)>>)
        {
          std::invoke(f, value);
          static_assert(
              !std::is_void_v<std::invoke_result_t<F, decltype(value)>>,
              "Function used with map cannot return void (or ResultVariant "
              "needs std::monostate).");
        } else {
          auto f_result = std::invoke(f, value);
          std::cout << "    [Debug map] User lambda (f) returned type: "
                    << typeid(f_result).name() << ".\n";
          ResultVariant constructed_result {f_result};
          std::cout << "    [Debug map] Constructed ResultVariant holds index: "
                    << constructed_result.index() << ".\n";
          return constructed_result;
        }
      },
      var);
}

/**
 * @brief Apply a function to the value held by a non-const lvalue variant
 * @tparam ResultVariant The target variant type to return
 * @tparam Ts Types in input variant
 * @tparam F Function type to apply
 * @param var Input variant
 * @param f Function to apply to variant value
 * @return ResultVariant containing f's result
 *
 * @code{.cpp}
 * // Example of mapping over lvalue variant
 * std::variant<int, std::string> v = 42;
 * auto result = map<std::variant<double>>(v,
 *   [](int& x) { x *= 2; return x * 1.5; }
 * ); // Modifies v and returns variant containing 126.0
 * @endcode
 */
template<typename ResultVariant, typename... Ts, typename F>
CPP_TOOLBOX_EXPORT auto map(std::variant<Ts...>& var, F&& f) -> ResultVariant
{
  std::cout << "[Debug map(variant&)] Entered map function.\n";
  return std::visit(
      [&f](auto& value) -> ResultVariant
      {
        std::cout << "  [Debug map(variant&)] Inside visit lambda. Visited "
                     "value type: "
                  << typeid(value).name() << ".\n";
        if constexpr (std::is_void_v<std::invoke_result_t<F, decltype(value)>>)
        {
          std::invoke(f, value);
          static_assert(
              !std::is_void_v<std::invoke_result_t<F, decltype(value)>>,
              "Function used with map cannot return void (or ResultVariant "
              "needs std::monostate).");
        } else {
          auto f_result = std::invoke(f, value);
          std::cout
              << "    [Debug map(variant&)] User lambda (f) returned type: "
              << typeid(f_result).name() << ".\n";
          ResultVariant constructed_result {f_result};
          std::cout << "    [Debug map(variant&)] Constructed ResultVariant "
                       "holds index: "
                    << constructed_result.index() << ".\n";
          return constructed_result;
        }
      },
      var);
}

/**
 * @brief Apply a function to the value held by an rvalue variant
 * @tparam ResultVariant The target variant type to return
 * @tparam Ts Types in input variant
 * @tparam F Function type to apply
 * @param var Input variant
 * @param f Function to apply to variant value
 * @return ResultVariant containing f's result
 *
 * @code{.cpp}
 * // Example of mapping over rvalue variant
 * auto result = map<std::variant<std::string>>(
 *   std::variant<int, std::string>(42),
 *   [](auto&& x) { return std::to_string(x); }
 * ); // Returns variant containing "42"
 * @endcode
 */
template<typename ResultVariant, typename... Ts, typename F>
CPP_TOOLBOX_EXPORT auto map(std::variant<Ts...>&& var, F&& f) -> ResultVariant
{
  return std::visit(
      [&f](auto&& value) -> ResultVariant
      {
        if constexpr (std::is_void_v<std::invoke_result_t<F, decltype(value)>>)
        {
          std::invoke(f, std::forward<decltype(value)>(value));
          static_assert(
              !std::is_void_v<std::invoke_result_t<F, decltype(value)>>,
              "Function used with map cannot return void (or ResultVariant "
              "needs std::monostate).");
        } else {
          return ResultVariant {
              std::invoke(f, std::forward<decltype(value)>(value))};
        }
      },
      std::move(var));
}

/**
 * @brief Apply a function to each element in a container and return results in
 * a new vector
 * @tparam Container Input container type, must provide begin() and end()
 * iterators
 * @tparam Func Function type to apply to elements
 * @param input Input container
 * @param f Function to apply to each element
 * @return Vector containing results of applying f to each element
 *
 * @code{.cpp}
 * // Example usage of map on containers
 * std::vector<int> nums = {1, 2, 3};
 * auto squares = map(nums, [](int x) { return x * x; }); // Returns {1, 4, 9}
 *
 * std::list<std::string> strs = {"a", "bb", "ccc"};
 * auto lengths = map(strs, [](const auto& s) { return s.length(); }); //
 * Returns {1, 2, 3}
 * @endcode
 */
template<typename Container, typename Func>
CPP_TOOLBOX_EXPORT auto map(const Container& input, Func&& f) -> std::vector<
    std::invoke_result_t<Func, typename Container::const_reference>>
{
  using ResultValueType =
      std::invoke_result_t<Func, typename Container::const_reference>;
  std::vector<ResultValueType> result;

  if constexpr (detail::has_size<Container>::value) {
    result.reserve(input.size());
  }

  std::transform(input.cbegin(),
                 input.cend(),
                 std::back_inserter(result),
                 std::forward<Func>(f));

  return result;
}

/**
 * @brief Filter elements from a container that satisfy a predicate
 * @tparam Container Input container type, must provide begin() and end()
 * iterators
 * @tparam Predicate Predicate function type
 * @param input Input container
 * @param p Predicate function returning bool
 * @return Vector containing elements that satisfy predicate p
 *
 * @code{.cpp}
 * // Example usage of filter
 * std::vector<int> nums = {1, 2, 3, 4, 5};
 * auto evens = filter(nums, [](int x) { return x % 2 == 0; }); // Returns {2,
 * 4}
 *
 * std::list<std::string> strs = {"a", "bb", "ccc"};
 * auto long_strs = filter(strs, [](const auto& s) { return s.length() > 1; });
 * // Returns {"bb", "ccc"}
 * @endcode
 */
template<typename Container, typename Predicate>
CPP_TOOLBOX_EXPORT auto filter(const Container& input, Predicate&& p)
    -> std::vector<typename Container::value_type>
{
  using ValueType = typename Container::value_type;
  std::vector<ValueType> result;

  std::copy_if(input.cbegin(),
               input.cend(),
               std::back_inserter(result),
               std::forward<Predicate>(p));

  return result;
}

/**
 * @brief Reduce container elements using a binary operation with initial value
 * @tparam Container Input container type, must provide begin() and end()
 * iterators
 * @tparam T Initial value and result type
 * @tparam BinaryOp Binary operation function type
 * @param input Input container
 * @param identity Initial value (identity element)
 * @param op Binary operation to combine elements
 * @return Result of reduction
 *
 * @code{.cpp}
 * // Example usage of reduce with identity value
 * std::vector<int> nums = {1, 2, 3, 4};
 * auto sum = reduce(nums, 0, std::plus<int>()); // Returns 10
 * auto product = reduce(nums, 1, std::multiplies<int>()); // Returns 24
 *
 * std::list<std::string> strs = {"a", "b", "c"};
 * auto concat = reduce(strs, std::string(), std::plus<std::string>()); //
 * Returns "abc"
 * @endcode
 */
template<typename Container, typename T, typename BinaryOp>
CPP_TOOLBOX_EXPORT auto reduce(const Container& input,
                               T identity,
                               BinaryOp&& op) -> T
{
  return std::accumulate(input.cbegin(),
                         input.cend(),
                         std::move(identity),
                         std::forward<BinaryOp>(op));
}

/**
 * @brief Reduce non-empty container elements using a binary operation
 * @tparam Container Input container type, must provide begin() and end()
 * iterators
 * @tparam BinaryOp Binary operation function type
 * @param input Input container (must be non-empty)
 * @param op Binary operation to combine elements
 * @return Result of reduction
 * @throws std::invalid_argument if input container is empty
 *
 * @code{.cpp}
 * // Example usage of reduce without identity value
 * std::vector<int> nums = {1, 2, 3, 4};
 * auto sum = reduce(nums, std::plus<int>()); // Returns 10
 *
 * std::list<std::string> strs = {"a", "b", "c"};
 * auto concat = reduce(strs, std::plus<std::string>()); // Returns "abc"
 *
 * // This will throw:
 * std::vector<int> empty;
 * auto result = reduce(empty, std::plus<int>()); // throws
 * std::invalid_argument
 * @endcode
 */
template<typename Container, typename BinaryOp>
CPP_TOOLBOX_EXPORT auto reduce(const Container& input, BinaryOp&& op) ->
    typename Container::value_type
{
  if (input.empty()) {
    throw std::invalid_argument(
        "reduce called on empty container without an identity value");
  }

  auto it = input.cbegin();
  typename Container::value_type result = *it;
  ++it;

  return std::accumulate(
      it, input.cend(), std::move(result), std::forward<BinaryOp>(op));
}

/**
 * @brief Zip multiple containers into a vector of tuples
 * @tparam Containers Container types, must provide size() and begin()/end()
 * @param containers Input containers
 * @return Vector of tuples containing references to elements from each
 * container
 *
 * @code{.cpp}
 * // Example usage of zip with multiple containers
 * std::vector<int> nums = {1, 2, 3};
 * std::vector<std::string> strs = {"a", "b", "c"};
 * std::array<double> dbls = {1.1, 2.2, 3.3};
 *
 * auto zipped = zip(nums, strs, dbls);
 * // Returns vector of tuples: {
 * //   {1, "a", 1.1},
 * //   {2, "b", 2.2},
 * //   {3, "c", 3.3}
 * // }
 * @endcode
 *
 * @note The length of result is determined by the shortest input container
 */
template<typename... Containers>
CPP_TOOLBOX_EXPORT auto zip(const Containers&... containers) -> std::vector<
    std::tuple<decltype(*std::cbegin(std::declval<const Containers&>()))...>>
{
  using TupleType =
      std::tuple<decltype(*std::cbegin(std::declval<const Containers&>()))...>;
  std::vector<TupleType> result;

  if constexpr (sizeof...(containers) == 0) {
    return result;
  }

  const size_t min_size = detail::get_min_size(containers...);
  if (min_size == 0) {
    return result;
  }

  result.reserve(min_size);

  auto iter_tuple = std::make_tuple(std::cbegin(containers)...);
  auto index_seq = std::index_sequence_for<Containers...> {};

  for (size_t i = 0; i < min_size; ++i) {
    result.emplace_back(
        detail::dereference_iterators_as_tuple(iter_tuple, index_seq));
    detail::increment_iterators(iter_tuple, index_seq);
  }

  return result;
}

/**
 * @brief Zip two sequences into an unordered_map
 * @tparam ContainerKeys Key container type
 * @tparam ContainerValues Value container type
 * @tparam Key Map key type (default: ContainerKeys::value_type)
 * @tparam Value Map value type (default: ContainerValues::value_type)
 * @tparam Hash Key hash function type
 * @tparam KeyEqual Key equality comparison function type
 * @tparam Alloc Map allocator type
 * @param keys Container of keys
 * @param values Container of values
 * @return Unordered map containing key-value pairs
 *
 * @code{.cpp}
 * // Example usage of zip_to_unordered_map
 * std::vector<std::string> keys = {"a", "b", "c"};
 * std::vector<int> values = {1, 2, 3};
 *
 * auto map = zip_to_unordered_map(keys, values);
 * // Returns unordered_map: {
 * //   {"a", 1},
 * //   {"b", 2},
 * //   {"c", 3}
 * // }
 * @endcode
 *
 * @note If keys container has duplicates, only the first occurrence is used
 * @note The number of pairs is determined by the shorter container
 */
template<typename ContainerKeys,
         typename ContainerValues,
         typename Key = typename std::decay_t<ContainerKeys>::value_type,
         typename Value = typename std::decay_t<ContainerValues>::value_type,
         typename Hash = std::hash<Key>,
         typename KeyEqual = std::equal_to<Key>,
         typename Alloc = std::allocator<std::pair<const Key, Value>>>
CPP_TOOLBOX_EXPORT auto zip_to_unordered_map(const ContainerKeys& keys,
                                             const ContainerValues& values)
    -> std::unordered_map<Key, Value, Hash, KeyEqual, Alloc>
{
  using ResultMapType = std::unordered_map<Key, Value, Hash, KeyEqual, Alloc>;
  ResultMapType result;

  auto keys_it = std::cbegin(keys);
  auto keys_end = std::cend(keys);
  auto values_it = std::cbegin(values);
  auto values_end = std::cend(values);

  while (keys_it != keys_end && values_it != values_end) {
    result.emplace(*keys_it, *values_it);
    ++keys_it;
    ++values_it;
  }

  return result;
}

/**
 * @brief Memoized function class that caches function results
 * @tparam Signature Function signature type (e.g., int(int, std::string))
 *
 * @code{.cpp}
 * // Example usage of MemoizedFunction class
 * // Create memoized factorial function
 * MemoizedFunction<int(int)> fact([](int n) {
 *   if (n <= 1) return 1;
 *   return n * fact(n-1);
 * });
 *
 * int result1 = fact(5); // Computes and caches
 * int result2 = fact(5); // Returns cached result
 * @endcode
 *
 * @note Thread-safe implementation
 * @note Function arguments must be copyable and form valid map key
 * @note Return type must be copyable
 */
template<typename Signature>
class MemoizedFunction;

/**
 * @brief Specialization of MemoizedFunction for specific function signature
 * @tparam R Return type
 * @tparam Args Argument types
 */
template<typename R, typename... Args>
class MemoizedFunction<R(Args...)>
{
private:
  using FuncType = std::function<R(Args...)>;
  using KeyType = std::tuple<std::decay_t<Args>...>;
  using ResultType = R;

  FuncType original_func_;
  std::map<KeyType, ResultType> cache_;
  std::mutex cache_mutex_;

public:
  explicit MemoizedFunction(FuncType f)
      : original_func_(std::move(f))
  {
  }

  MemoizedFunction(const MemoizedFunction&) = delete;
  MemoizedFunction& operator=(const MemoizedFunction&) = delete;
  MemoizedFunction(MemoizedFunction&&) = delete;
  MemoizedFunction& operator=(MemoizedFunction&&) = delete;

  auto operator()(Args... args) -> ResultType
  {
    KeyType key = std::make_tuple(std::decay_t<Args>(args)...);

    {
      std::lock_guard<std::mutex> lock(cache_mutex_);
      auto it = cache_.find(key);
      if (it != cache_.end()) {
        return it->second;
      }
    }

    ResultType result = original_func_(std::forward<Args>(args)...);

    {
      std::lock_guard<std::mutex> lock(cache_mutex_);
      auto it = cache_.find(key);
      if (it == cache_.end()) {
        cache_.emplace(std::move(key), result);
        return result;
      } else {
        return it->second;
      }
    }
  }
};

/**
 * @brief Create a memoized function with explicit signature
 * @tparam Signature Function signature type
 * @tparam Func Function object type
 * @param f Function to memoize
 * @return MemoizedFunction object
 *
 * @code{.cpp}
 * // Example usage of memoize with explicit signature
 * auto memoized_add = memoize<int(int,int)>([](int a, int b) { return a + b;
 * }); int result1 = memoized_add(2, 3); // Computes and caches int result2 =
 * memoized_add(2, 3); // Returns cached result
 * @endcode
 */
template<typename Signature, typename Func>
auto memoize(Func&& f)
{
  std::function<Signature> func_wrapper = std::forward<Func>(f);
  return MemoizedFunction<Signature>(std::move(func_wrapper));
}

/**
 * @brief Create a memoized function with explicit return and argument types
 * @tparam R Return type
 * @tparam Args Argument types
 * @tparam Func Function object type
 * @param f Function to memoize
 * @return std::function with memoization
 *
 * @code{.cpp}
 * // Example usage of memoize_explicit
 * auto fib = memoize_explicit<int, int>([&](int n) {
 *   if (n <= 1) return n;
 *   return fib(n-1) + fib(n-2);
 * });
 * int result = fib(10); // Efficiently computes fibonacci
 * @endcode
 *
 * @note Thread-safe implementation using shared state
 * @note Arguments must form valid map key after decay
 * @note Return type must be copyable
 */
template<typename R, typename... Args, typename Func>
auto memoize_explicit(Func&& f) -> std::function<R(Args...)>
{
  std::function<R(Args...)> func = std::forward<Func>(f);

  using StateType = detail::MemoizeState<std::function<R(Args...)>, R, Args...>;
  auto state = std::make_shared<StateType>(std::move(func));

  return [state](Args... args) -> R
  {
    using KeyType = typename StateType::KeyType;
    KeyType key = std::make_tuple(std::decay_t<Args>(args)...);

    {
      std::lock_guard<std::mutex> lock(state->cache_mutex);
      auto it = state->cache.find(key);
      if (it != state->cache.end()) {
        return it->second;
      }
    }

    R result = state->original_func(std::forward<Args>(args)...);

    {
      std::lock_guard<std::mutex> lock(state->cache_mutex);
      auto it = state->cache.find(key);
      if (it == state->cache.end()) {
        state->cache.emplace(std::move(key), result);
        return result;
      } else {
        return it->second;
      }
    }
  };
}

// !!TODO: more intelligent memoization

}  // namespace toolbox::functional
