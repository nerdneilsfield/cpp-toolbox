#pragma once

#include <cpp-toolbox/functional/functional.hpp>  // Include the main header

// Necessary includes for implementation
#include <functional>  // For std::function in memoize
#include <map>
#include <memory>  // For std::shared_ptr in PIMPL
#include <mutex>
#include <numeric>  // For std::accumulate
#include <optional>
#include <stdexcept>  // For std::logic_error, std::invalid_argument
#include <tuple>  // For std::tuple used in MemoizedFunction and zip
#include <type_traits>  // For various type traits
#include <unordered_map>  // For zip_to_unordered_map
#include <utility>  // For std::forward, std::move
#include <variant>
#include <vector>  // For map/filter container results

namespace toolbox::functional
{

// --- Implementation detail for memoize_explicit ---
namespace detail_impl
{

template<typename R, typename... Args>
struct MemoizeHelperState
{
  using FuncType = std::function<R(Args...)>;
  using KeyType = std::tuple<std::decay_t<Args>...>;
  using ResultType = R;

  FuncType original_func_;
  std::map<KeyType, ResultType> cache_;
  std::mutex cache_mutex_;

  explicit MemoizeHelperState(FuncType f)
      : original_func_(std::move(f))
  {
  }
};

}  // namespace detail_impl

// --- Implementation for MemoizedFunction<R(Args...)> using PIMPL ---

template<typename R, typename... Args>
struct MemoizedFunction<R(Args...)>::State
{
  using FuncType = std::function<R(Args...)>;
  using KeyType = std::tuple<std::decay_t<Args>...>;
  using ResultType = R;

  FuncType original_func_;
  std::map<KeyType, ResultType> cache_;
  std::mutex cache_mutex_;

  explicit State(FuncType f)
      : original_func_(std::move(f))
  {
  }
};

template<typename R, typename... Args>
MemoizedFunction<R(Args...)>::MemoizedFunction(FuncType f)
    : state_(std::make_shared<State>(std::move(f)))
{
}

template<typename R, typename... Args>
auto MemoizedFunction<R(Args...)>::operator()(Args... args) -> ResultType
{
  // Use state_ pointer to access members
  KeyType key = std::make_tuple(std::decay_t<Args>(args)...);

  {
    std::lock_guard<std::mutex> lock(state_->cache_mutex_);
    auto it = state_->cache_.find(key);
    if (it != state_->cache_.end()) {
      return it->second;
    }
  }

  // Call the original function through the state object
  ResultType result = state_->original_func_(std::forward<Args>(args)...);

  {
    std::lock_guard<std::mutex> lock(state_->cache_mutex_);
    // Double-check insertion to handle race condition where another thread
    // calculated it
    auto it = state_->cache_.find(key);
    if (it == state_->cache_.end()) {
      state_->cache_.emplace(std::move(key), result);
      return result;
    } else {
      // Another thread finished first, return its result
      return it->second;
    }
  }
}

// --- Implementations for other functions ---

template<typename G, typename F>
auto compose(G&& g, F&& f)
{
  return
      [g = std::forward<G>(g), f = std::forward<F>(f)](auto&&... args) mutable
          -> decltype(g(f(std::forward<decltype(args)>(args)...)))
  { return g(f(std::forward<decltype(args)>(args)...)); };
}

template<typename F1, typename... FRest>
auto compose(F1&& f1, FRest&&... rest)
{
  if constexpr (sizeof...(FRest) == 0) {
    // Base case: return the single function wrapped in a lambda
    return [f1_cap =
                std::forward<F1>(f1)](auto&&... args) mutable -> decltype(auto)
    { return f1_cap(std::forward<decltype(args)>(args)...); };
  } else {
    // Recursive step: compose the rest of the functions first
    auto composed_rest = compose(std::forward<FRest>(rest)...);
    // Then compose f1 with the result
    return [f1_cap = std::forward<F1>(f1),
            composed_rest_cap = std::move(composed_rest)](
               auto&&... args) mutable -> decltype(auto)
    {
      // Apply composed_rest first, then f1_cap
      return f1_cap(composed_rest_cap(std::forward<decltype(args)>(args)...));
    };
  }
}

inline auto compose()
{
  throw std::logic_error("compose called with no functions");
}

template<typename F, typename Arg1>
auto bind_first(F&& f, Arg1&& arg1)
{
  return [f = std::forward<F>(f),
          arg1 = std::forward<Arg1>(arg1)](
             auto&&... rest_args) mutable  // Ensure mutable if f or arg1 state
                                           // needs change
         -> decltype(f(arg1, std::forward<decltype(rest_args)>(rest_args)...))
  { return f(arg1, std::forward<decltype(rest_args)>(rest_args)...); };
}

template<typename T, typename F>
auto map(const std::optional<T>& opt, F&& f)
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

template<typename T, typename F>
auto map(std::optional<T>&& opt, F&& f)
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

template<typename T, typename F>
auto flatMap(const std::optional<T>& opt, F&& f)
    -> std::invoke_result_t<F, const T&>
{
  using result_optional_type = std::invoke_result_t<F, const T&>;

  static_assert(detail::is_optional_v<result_optional_type>,
                "Function passed to flatMap must return a std::optional type.");

  if (opt.has_value()) {
    return std::invoke(std::forward<F>(f), *opt);
  } else {
    return result_optional_type {};  // Return default-constructed optional
                                     // (nullopt)
  }
}

template<typename T, typename F>
auto flatMap(std::optional<T>&& opt, F&& f) -> std::invoke_result_t<F, T&&>
{
  using result_optional_type = std::invoke_result_t<F, T&&>;
  static_assert(detail::is_optional_v<result_optional_type>,
                "Function passed to flatMap must return a std::optional type.");

  if (opt.has_value()) {
    return std::invoke(std::forward<F>(f), std::move(*opt));
  } else {
    return result_optional_type {};  // Return default-constructed optional
                                     // (nullopt)
  }
}

template<typename T, typename U>
auto orElse(const std::optional<T>& opt, U&& default_value) -> T
{
  static_assert(
      std::is_convertible_v<U, T>,
      "Default value type must be convertible to optional's value type T.");

  return opt.value_or(std::forward<U>(default_value));
}

template<typename T, typename F>
auto orElseGet(const std::optional<T>& opt, F&& default_func) -> T
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

template<typename T, typename P>
auto filter(const std::optional<T>& opt, P&& p) -> std::optional<T>
{
#if __cpp_lib_is_invocable >= 201703L
  static_assert(
      std::is_invocable_r_v<bool, P, const T&>
          || std::is_convertible_v<std::invoke_result_t<P, const T&>, bool>,
      "Predicate must be callable with const T& and return bool or "
      "bool-convertible.");
#else
  // Fallback check for older compilers
  static_assert(
      std::is_convertible_v<std::invoke_result_t<P, const T&>, bool>,
      "Predicate must be callable with const T& and return bool-convertible.");
#endif

  return (opt.has_value() && std::invoke(std::forward<P>(p), *opt))
      ? opt
      : std::nullopt;
}

template<typename T, typename P>
auto filter(std::optional<T>&& opt, P&& p) -> std::optional<T>
{
#if __cpp_lib_is_invocable >= 201703L
  static_assert(
      std::is_invocable_r_v<bool, P, const T&>
          || std::is_convertible_v<std::invoke_result_t<P, const T&>, bool>,
      "Predicate must be callable with const T& and return bool or "
      "bool-convertible.");
#else
  // Fallback check for older compilers
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

template<typename... Ts, typename... Fs>
auto match(const std::variant<Ts...>& var, Fs&&... visitors) -> decltype(auto)
{
  static_assert(
      sizeof...(Ts) == sizeof...(Fs),
      "Number of visitors must match the number of types in the variant");
  auto visitor_set = detail::overloaded {std::forward<Fs>(visitors)...};
  return std::visit(visitor_set, var);
}

template<typename... Ts, typename... Fs>
auto match(std::variant<Ts...>& var, Fs&&... visitors) -> decltype(auto)
{
  static_assert(
      sizeof...(Ts) == sizeof...(Fs),
      "Number of visitors must match the number of types in the variant");
  auto visitor_set = detail::overloaded {std::forward<Fs>(visitors)...};
  return std::visit(visitor_set, var);
}

template<typename... Ts, typename... Fs>
auto match(std::variant<Ts...>&& var, Fs&&... visitors) -> decltype(auto)
{
  static_assert(
      sizeof...(Ts) == sizeof...(Fs),
      "Number of visitors must match the number of types in the variant");
  auto visitor_set = detail::overloaded {std::forward<Fs>(visitors)...};
  return std::visit(visitor_set, std::move(var));
}

template<typename ResultVariant, typename... Ts, typename F>
auto map(const std::variant<Ts...>& var, F&& f) -> ResultVariant
{
  return std::visit(
      [&f](const auto& value) -> ResultVariant
      {
        if constexpr (std::is_void_v<
                          std::invoke_result_t<F, decltype(value)>>) {
          // This path should ideally not be taken if ResultVariant is used
          // correctly
          std::invoke(std::forward<F>(f), value);
          // Cannot construct ResultVariant from void. Check if ResultVariant
          // has monostate? This static_assert might be too strict if monostate
          // is intended.
          static_assert(
              !std::is_void_v<std::invoke_result_t<F, decltype(value)>>,
              "Mapping function returning void requires ResultVariant to "
              "support default construction (e.g., std::monostate).");
          return ResultVariant {};  // Or throw, depending on desired semantics
        } else {
          return ResultVariant {std::invoke(std::forward<F>(f), value)};
        }
      },
      var);
}

template<typename ResultVariant, typename... Ts, typename F>
auto map(std::variant<Ts...>& var, F&& f) -> ResultVariant
{
  return std::visit(
      [&f](auto& value) -> ResultVariant
      {
        if constexpr (std::is_void_v<
                          std::invoke_result_t<F, decltype(value)>>) {
          std::invoke(std::forward<F>(f), value);
          static_assert(
              !std::is_void_v<std::invoke_result_t<F, decltype(value)>>,
              "Mapping function returning void requires ResultVariant to "
              "support default construction (e.g., std::monostate).");
          return ResultVariant {};
        } else {
          return ResultVariant {std::invoke(std::forward<F>(f), value)};
        }
      },
      var);
}

template<typename ResultVariant, typename... Ts, typename F>
auto map(std::variant<Ts...>&& var, F&& f) -> ResultVariant
{
  return std::visit(
      [&f](auto&& value) -> ResultVariant
      {
        if constexpr (std::is_void_v<std::invoke_result_t<
                          F,
                          decltype(std::forward<decltype(value)>(value))>>)
        {
          std::invoke(std::forward<F>(f), std::forward<decltype(value)>(value));
          static_assert(
              !std::is_void_v<std::invoke_result_t<
                  F,
                  decltype(std::forward<decltype(value)>(value))>>,
              "Mapping function returning void requires ResultVariant to "
              "support default construction (e.g., std::monostate).");
          return ResultVariant {};
        } else {
          return ResultVariant {std::invoke(
              std::forward<F>(f), std::forward<decltype(value)>(value))};
        }
      },
      std::move(var));
}

template<typename Container, typename Func>
auto map(const Container& input, Func&& f) -> std::vector<
    std::invoke_result_t<Func, typename Container::const_reference>>
{
  using ResultValueType =
      std::invoke_result_t<Func, typename Container::const_reference>;
  std::vector<ResultValueType> result;

  if constexpr (detail::has_size<Container>::value) {
    result.reserve(input.size());
  }

  std::transform(std::cbegin(input),  // Use std::cbegin for const-correctness
                 std::cend(input),  // Use std::cend for const-correctness
                 std::back_inserter(result),
                 std::forward<Func>(f));

  return result;
}

template<typename Container, typename Predicate>
auto filter(const Container& input, Predicate&& p)
    -> std::vector<typename Container::value_type>
{
  using ValueType = typename Container::value_type;
  std::vector<ValueType> result;

  std::copy_if(std::cbegin(input),  // Use std::cbegin
               std::cend(input),  // Use std::cend
               std::back_inserter(result),
               std::forward<Predicate>(p));

  return result;
}

template<typename Container, typename T, typename BinaryOp>
auto reduce(const Container& input, T identity, BinaryOp&& op) -> T
{
  return std::accumulate(std::cbegin(input),  // Use std::cbegin
                         std::cend(input),  // Use std::cend
                         std::move(identity),
                         std::forward<BinaryOp>(op));
}

template<typename Container, typename BinaryOp>
auto reduce(const Container& input, BinaryOp&& op) ->
    typename Container::value_type
{
  if (std::empty(input)) {  // Use std::empty for check
    throw std::invalid_argument(
        "reduce called on empty container without an identity value");
  }

  auto it = std::cbegin(input);  // Use std::cbegin
  typename Container::value_type result = *it;
  ++it;

  return std::accumulate(it,
                         std::cend(input),  // Use std::cend
                         std::move(result),  // Move the initial value
                         std::forward<BinaryOp>(op));
}

template<typename... Containers>
auto zip(const Containers&... containers) -> std::vector<
    std::tuple<decltype(*std::cbegin(std::declval<const Containers&>()))...>>
{
  using ResultTupleType =
      std::tuple<decltype(*std::cbegin(std::declval<const Containers&>()))...>;
  std::vector<ResultTupleType> result;

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
    // Use std::apply to unpack the tuple for emplace_back
    std::apply([&result](auto&&... its) { result.emplace_back(*its...); },
               iter_tuple);
    detail::increment_iterators(iter_tuple, index_seq);
  }

  return result;
}

template<typename ContainerKeys,
         typename ContainerValues,
         typename Key,
         typename Value,
         typename Hash,
         typename KeyEqual,
         typename Alloc>
auto zip_to_unordered_map(const ContainerKeys& keys,
                          const ContainerValues& values)
    -> std::unordered_map<Key, Value, Hash, KeyEqual, Alloc>
{
  using ResultMapType = std::unordered_map<Key, Value, Hash, KeyEqual, Alloc>;
  ResultMapType result;

  auto keys_it = std::cbegin(keys);
  auto keys_end = std::cend(keys);
  auto values_it = std::cbegin(values);
  auto values_end = std::cend(values);

  // Optionally reserve based on min size if possible
  if constexpr (detail::has_size<ContainerKeys>::value
                && detail::has_size<ContainerValues>::value)
  {
    result.reserve(std::min(keys.size(), values.size()));
  }

  while (keys_it != keys_end && values_it != values_end) {
    // Use try_emplace to avoid overwriting existing keys if that's desired,
    // or use emplace/insert depending on exact requirements for duplicates.
    result.emplace(*keys_it, *values_it);
    ++keys_it;
    ++values_it;
  }

  return result;
}

template<typename Signature, typename Func>
auto memoize(Func&& f)
{
  std::function<Signature> func_wrapper = std::forward<Func>(f);
  // Return the MemoizedFunction wrapper directly
  return MemoizedFunction<Signature>(std::move(func_wrapper));
}

// --- Implementation for memoize_explicit ---
template<typename R, typename... Args, typename Func>
auto memoize_explicit(Func&& f) -> std::function<R(Args...)>
{
  // Use the independent helper state struct defined above
  using HelperStateType = detail_impl::MemoizeHelperState<R, Args...>;
  auto state = std::make_shared<HelperStateType>(
      std::function<R(Args...)>(std::forward<Func>(f)));

  // The returned lambda captures the shared_ptr to the helper state.
  // This lambda is copy-constructible.
  return [state](Args... args) -> R
  {
    using KeyType = typename HelperStateType::KeyType;
    KeyType key = std::make_tuple(std::decay_t<Args>(args)...);

    {
      std::lock_guard<std::mutex> lock(state->cache_mutex_);
      auto it = state->cache_.find(key);
      if (it != state->cache_.end()) {
        return it->second;
      }
    }

    // Call the original function stored in the helper state
    R result = state->original_func_(std::forward<Args>(args)...);

    {
      std::lock_guard<std::mutex> lock(state->cache_mutex_);
      // Double-check insertion in the helper state's cache
      auto it = state->cache_.find(key);
      if (it == state->cache_.end()) {
        state->cache_.emplace(std::move(key), result);
        return result;
      } else {
        return it->second;
      }
    }
  };
}

}  // namespace toolbox::functional
