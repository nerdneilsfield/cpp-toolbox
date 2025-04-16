#pragma once

#include <algorithm>
#include <functional>  // 可能有用,但compose本身不需要 / Potentially useful, though not strictly required by compose itself
#include <iostream>
#include <iterator>
#include <map>
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

/**
 * @brief 定义一个简单的函子(函数对象),只有一个operator() / Define a simple
 * functor (function object) with a single operator()
 *
 * @param FunctorName 要创建的函子结构体名称 / Name of the functor struct to
 * create
 * @param ReturnType operator()的返回类型 / Return type of operator()
 * @param Params operator()的参数列表(必须包含括号) / Parameter list for
 * operator() (must include parentheses)
 * @param Body 函数体代码(如果ReturnType不是void则必须包含return语句) / Function
 * body code (must include return statement if ReturnType is not void)
 *
 * @code{.cpp}
 * // 定义一个加法函子 / Define a functor that adds two numbers
 * CPP_TOOLBOX_DEFINE_SIMPLE_FUNCTOR(
 *   Adder,           // 函子名称 / FunctorName
 *   int,             // 返回类型 / ReturnType
 *   (int a, int b),  // 参数 / Params
 *   return a + b;    // 函数体 / Body
 * );
 *
 * // 使用函子 / Use the functor
 * Adder add;
 * int sum = add(1, 2); // 返回3 / Returns 3
 *
 * // 定义一个打印函子 / Define a functor that prints a message
 * CPP_TOOLBOX_DEFINE_SIMPLE_FUNCTOR(
 *   Printer,         // 函子名称 / FunctorName
 *   void,           // 返回类型 / ReturnType
 *   (const std::string& msg),  // 参数 / Params
 *   std::cout << msg << std::endl;  // 函数体 / Body
 * );
 * @endcode
 */
#define CPP_TOOLBOX_DEFINE_SIMPLE_FUNCTOR( \
    FunctorName, ReturnType, Params, Body) \
  struct FunctorName \
  { \
    ReturnType operator() Params const \
    { \
      Body \
    } \
  };

namespace toolbox::functional
{

namespace detail
{
/**
 * @brief 用于创建重载集的辅助类 / Helper class for creating overload sets
 * @tparam Fs 要重载的函数类型 / Types of functions to overload
 *
 * @code{.cpp}
 * // 为variant创建重载访问器 / Create overloaded visitor for variant
 * auto visitor = overloaded{
 *   [](int x) { return x * 2; },
 *   [](const std::string& s) { return s + s; }
 * };
 * @endcode
 */
template<class... Fs>
struct overloaded : Fs...
{
  using Fs::operator()...;  // C++17 using包声明 / C++17 using pack declaration
};

// C++17类模板参数推导指南 / C++17 class template argument deduction guide
// 允许写overloaded{lambda1, lambda2}而不需要显式模板参数 / Allows writing
// overloaded{lambda1, lambda2} without explicit template args
template<class... Fs>
overloaded(Fs...) -> overloaded<Fs...>;

/**
 * @brief 检查T是否为std::optional的类型特征 / Type trait to check if T is a
 * std::optional
 *
 * @code{.cpp}
 * static_assert(is_optional<std::optional<int>>::value, "是optional / is
 * optional"); static_assert(!is_optional<int>::value, "不是optional / not
 * optional");
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
 * @brief 检查类型是否有size()成员函数的类型特征 / Type trait to check if type
 * has size() member function
 *
 * @code{.cpp}
 * static_assert(has_size<std::vector<int>>::value, "有size / has size");
 * static_assert(!has_size<int>::value, "没有size / no size");
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
 * @brief 获取多个容器中的最小大小的辅助函数 / Helper function to get minimum
 * size from multiple containers
 * @tparam Containers 容器类型 / Container types
 * @return 所有容器中的最小大小,如果没有容器则返回0 / Minimum size among all
 * containers, or 0 if no containers
 *
 * @code{.cpp}
 * std::vector<int> v1{1,2,3};
 * std::list<int> l1{1,2};
 * auto min_size = get_min_size(v1, l1); // 返回2 / Returns 2
 * @endcode
 */
template<typename... Containers>
auto get_min_size(const Containers&... containers) -> size_t
{
  if constexpr (sizeof...(containers) == 0) {
    return 0;  // 显式处理空包 / Handle empty pack explicitly
  } else {
    return std::min({static_cast<size_t>(containers.size())...});
  }
}

/**
 * @brief 增加元组中所有迭代器的辅助函数 / Helper function to increment all
 * iterators in a tuple
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
 * @brief 解引用迭代器并创建引用元组的辅助函数 / Helper function to dereference
 * iterators and create tuple of references
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
 * @brief 记忆化的内部状态 / Internal state for memoization
 *
 * @code{.cpp}
 * MemoizeState<int(int), int, int> state{[](int x) { return x*x; }};
 * auto result = state.original_func(5); // 返回25 / Returns 25
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
 * @brief 将两个函数组合成一个函数 / Composes two functions into a single
 * function
 * @tparam G 外层函数类型 / Type of outer function
 * @tparam F 内层函数类型 / Type of inner function
 * @param g 外层函数 / Outer function
 * @param f 内层函数 / Inner function
 * @return 先应用f再应用g的函数 / A function that applies f then g
 *
 * @code{.cpp}
 * // 函数组合示例 / Example of function composition
 * auto add_one = [](int x) { return x + 1; };
 * auto multiply_two = [](int x) { return x * 2; };
 * auto composed = compose(multiply_two, add_one);
 * int result = composed(5); // 返回12: ((5 + 1) * 2) / Returns 12: ((5 + 1) *
 * 2)
 * @endcode
 */
template<typename G, typename F>
CPP_TOOLBOX_EXPORT auto compose(G&& g, F&& f);

/**
 * @brief 将多个函数组合成一个函数 / Composes multiple functions into a single
 * function
 * @tparam F1 第一个(最外层)函数类型 / Type of first (outermost) function
 * @tparam FRest 剩余函数类型 / Types of remaining functions
 * @param f1 第一个函数 / First function
 * @param rest 剩余函数 / Remaining functions
 * @return 从右到左应用函数的函数 / A function that applies functions from right
 * to left
 *
 * @code{.cpp}
 * // 多函数组合示例 / Example of multiple function composition
 * auto add_one = [](int x) { return x + 1; };
 * auto multiply_two = [](int x) { return x * 2; };
 * auto square = [](int x) { return x * x; };
 * auto composed = compose(square, multiply_two, add_one);
 * int result = composed(5); // 返回144: ((5 + 1) * 2)^2 / Returns 144: ((5 + 1)
 * * 2)^2
 * @endcode
 */
template<typename F1, typename... FRest>
CPP_TOOLBOX_EXPORT auto compose(F1&& f1, FRest&&... rest);

/**
 * @brief 空的compose函数,会抛出错误 / Empty compose function that throws an
 * error
 * @throws std::logic_error 当被调用时 / when called
 *
 * @code{.cpp}
 * try {
 *   auto empty = compose();
 * } catch(const std::logic_error& e) {
 *   // 异常被捕获 / Exception caught
 * }
 * @endcode
 */
inline CPP_TOOLBOX_EXPORT auto compose();

/**
 * @brief 绑定函数的第一个参数 / Binds the first argument of a function
 * @tparam F 函数类型 / Function type
 * @tparam Arg1 第一个参数的类型 / Type of first argument
 * @param f 要绑定的函数 / Function to bind
 * @param arg1 要绑定为第一个参数的值 / Value to bind as first argument
 * @return 第一个参数被绑定的函数 / Function with first argument bound
 *
 * @code{.cpp}
 * // 绑定第一个参数的示例 / Example of binding first argument
 * auto divide = [](int x, int y) { return x / y; };
 * auto divide_10_by = bind_first(divide, 10);
 * int result = divide_10_by(2); // 返回5 / Returns 5
 * @endcode
 */
template<typename F, typename Arg1>
CPP_TOOLBOX_EXPORT auto bind_first(F&& f, Arg1&& arg1);

/**
 * @brief 在optional值上映射函数 / Maps a function over an optional value
 * @tparam T optional中包含的类型 / Type contained in optional
 * @tparam F 函数类型 / Function type
 * @param opt 要映射的optional / Optional to map over
 * @param f 要应用的函数 / Function to apply
 * @return 如果opt有值则包含f的结果的optional,否则为空optional / Optional
 * containing result of f if opt has value, empty optional otherwise
 *
 * @code{.cpp}
 * // 在optional上映射的示例 / Example of mapping over optional
 * std::optional<int> opt(5);
 * auto times_two = [](int x) { return x * 2; };
 * auto result = map(opt, times_two); // 包含10 / Contains 10
 * @endcode
 */
template<typename T, typename F>
CPP_TOOLBOX_EXPORT auto map(const std::optional<T>& opt, F&& f)
    -> std::optional<std::invoke_result_t<F, const T&>>;

/**
 * @brief 在optional右值上映射函数 / Maps a function over an optional rvalue
 * @tparam T optional中包含的类型 / Type contained in optional
 * @tparam F 函数类型 / Function type
 * @param opt 要映射的optional右值 / Optional rvalue to map over
 * @param f 要应用的函数 / Function to apply
 * @return 如果opt有值则包含f的结果的optional,否则为空optional / Optional
 * containing result of f if opt has value, empty optional otherwise
 *
 * @code{.cpp}
 * // 在optional右值上映射的示例 / Example of mapping over optional rvalue
 * auto result = map(std::optional<int>(5), [](int x) { return x * 2; }); //
 * 包含10 / Contains 10
 * @endcode
 */
template<typename T, typename F>
CPP_TOOLBOX_EXPORT auto map(std::optional<T>&& opt, F&& f)
    -> std::optional<std::invoke_result_t<F, T&&>>;

/**
 * @brief 在optional值上平面映射函数 / Flat maps a function over an optional
 * value
 * @tparam T optional中包含的类型 / Type contained in optional
 * @tparam F 返回optional的函数类型 / Function type returning optional
 * @param opt 要平面映射的optional / Optional to flat map over
 * @param f 要应用的函数 / Function to apply
 * @return 如果opt有值则为f的结果,否则为空optional / Result of f if opt has
 * value, empty optional otherwise
 *
 * @code{.cpp}
 * // 在optional上平面映射的示例 / Example of flat mapping over optional
 * std::optional<int> opt(5);
 * auto maybe_double = [](int x) -> std::optional<int> {
 *   return x < 10 ? std::optional(x * 2) : std::nullopt;
 * };
 * auto result = flatMap(opt, maybe_double); // 包含10 / Contains 10
 * @endcode
 */
template<typename T, typename F>
CPP_TOOLBOX_EXPORT auto flatMap(const std::optional<T>& opt, F&& f)
    -> std::invoke_result_t<F, const T&>;

/**
 * @brief 在optional右值上平面映射函数 / Flat maps a function over an optional
 * rvalue
 * @tparam T optional中包含的类型 / Type contained in optional
 * @tparam F 返回optional的函数类型 / Function type returning optional
 * @param opt 要平面映射的optional右值 / Optional rvalue to flat map over
 * @param f 要应用的函数 / Function to apply
 * @return 如果opt有值则为f的结果,否则为空optional / Result of f if opt has
 * value, empty optional otherwise
 *
 * @code{.cpp}
 * // 在optional右值上平面映射的示例 / Example of flat mapping over optional
 * rvalue auto result = flatMap(std::optional<int>(5),
 *                      [](int x) -> std::optional<int> {
 *                        return x < 10 ? std::optional(x * 2) : std::nullopt;
 *                      }); // 包含10 / Contains 10
 * @endcode
 */
template<typename T, typename F>
CPP_TOOLBOX_EXPORT auto flatMap(std::optional<T>&& opt, F&& f)
    -> std::invoke_result_t<F, T&&>;

/**
 * @brief 返回包含的值或默认值 / Returns the contained value or a default
 * @tparam T optional中包含的类型 / Type contained in optional
 * @tparam U 默认值类型 / Type of default value
 * @param opt 要获取值的optional / Optional to get value from
 * @param default_value 如果optional为空时返回的值 / Value to return if optional
 * is empty
 * @return 包含的值或默认值 / Contained value or default
 *
 * @code{.cpp}
 * // 提供默认值的示例 / Example of providing default value
 * std::optional<int> opt;
 * int result = orElse(opt, 42); // 返回42 / Returns 42
 * @endcode
 */
template<typename T, typename U>
CPP_TOOLBOX_EXPORT auto orElse(const std::optional<T>& opt, U&& default_value)
    -> T;

/**
 * @brief 返回包含的值或调用函数获取默认值 / Returns the contained value or
 * calls function for default
 * @tparam T optional中包含的类型 / Type contained in optional
 * @tparam F 返回默认值的函数类型 / Function type returning default value
 * @param opt 要获取值的optional / Optional to get value from
 * @param default_func 用于获取默认值的函数 / Function to call for default value
 * @return 包含的值或default_func的结果 / Contained value or result of
 * default_func
 *
 * @code{.cpp}
 * // 通过函数提供默认值的示例 / Example of providing default via function
 * std::optional<int> opt;
 * auto get_default = []() { return 42; };
 * int result = orElseGet(opt, get_default); // 返回42 / Returns 42
 * @endcode
 */
template<typename T, typename F>
CPP_TOOLBOX_EXPORT auto orElseGet(const std::optional<T>& opt, F&& default_func)
    -> T;

/**
 * @brief 基于谓词过滤optional / Filters an optional based on a predicate
 * @tparam T optional中包含的类型 / Type contained in optional
 * @tparam P 谓词类型 / Predicate type
 * @param opt 要过滤的optional / Optional to filter
 * @param p 要应用的谓词 / Predicate to apply
 * @return 如果谓词返回true则为optional,否则为空optional / Optional if predicate
 * returns true, empty optional otherwise
 *
 * @code{.cpp}
 * // 过滤optional的示例 / Example of filtering optional
 * std::optional<int> opt(42);
 * auto is_even = [](int x) { return x % 2 == 0; };
 * auto result = filter(opt, is_even); // 包含42 / Contains 42
 * @endcode
 */
template<typename T, typename P>
CPP_TOOLBOX_EXPORT auto filter(const std::optional<T>& opt, P&& p)
    -> std::optional<T>;

/**
 * @brief 基于谓词过滤optional右值 / Filters an optional rvalue based on a
 * predicate
 * @tparam T optional中包含的类型 / Type contained in optional
 * @tparam P 谓词类型 / Predicate type
 * @param opt 要过滤的optional右值 / Optional rvalue to filter
 * @param p 要应用的谓词 / Predicate to apply
 * @return 如果谓词返回true则为optional,否则为空optional / Optional if predicate
 * returns true, empty optional otherwise
 *
 * @code{.cpp}
 * // 过滤optional右值的示例 / Example of filtering optional rvalue
 * auto result = filter(std::optional<int>(42), [](int x) { return x % 2 == 0;
 * }); // 包含42 / Contains 42
 * @endcode
 */
template<typename T, typename P>
CPP_TOOLBOX_EXPORT auto filter(std::optional<T>&& opt, P&& p)
    -> std::optional<T>;

/**
 * @brief 使用访问器函数对variant进行模式匹配 / Pattern matches on a variant
 * using visitor functions
 * @tparam Ts variant中的类型 / Types in variant
 * @tparam Fs 访问器函数类型 / Visitor function types
 * @param var 要匹配的variant / Variant to match on
 * @param visitors 每个类型的访问器函数 / Visitor functions for each type
 * @return 匹配的访问器的结果 / Result of matched visitor
 *
 * @code{.cpp}
 * // variant模式匹配的示例 / Example of pattern matching on variant
 * std::variant<int, std::string> v = 42;
 * auto result = match(v,
 *   [](int i) { return i * 2; },
 *   [](const std::string& s) { return s.length(); }
 * ); // 返回84 / Returns 84
 * @endcode
 */
template<typename... Ts, typename... Fs>
CPP_TOOLBOX_EXPORT auto match(const std::variant<Ts...>& var, Fs&&... visitors)
    -> decltype(auto);

/**
 * @brief 对非const左值variant使用访问器函数进行匹配 / Match variant with
 * visitor functions for non-const lvalue variant
 * @tparam Ts variant中的类型 / Types in variant
 * @tparam Fs 访问器函数类型 / Visitor function types
 * @param var 要匹配的variant / Variant to match on
 * @param visitors 每个类型的访问器函数 / Visitor functions for each type
 * @return 匹配的访问器的结果 / Result of matched visitor
 *
 * @code{.cpp}
 * // 对左值variant进行模式匹配的示例 / Example of pattern matching on lvalue
 * variant std::variant<int, std::string> v = 42; auto result = match(v,
 *   [](int& i) { i *= 2; return i; },
 *   [](std::string& s) { s += "!"; return s.length(); }
 * ); // 修改v并返回84 / Modifies v and returns 84
 * @endcode
 */
template<typename... Ts, typename... Fs>
CPP_TOOLBOX_EXPORT auto match(std::variant<Ts...>& var, Fs&&... visitors)
    -> decltype(auto);

/**
 * @brief 对右值variant使用访问器函数进行匹配 / Match variant with visitor
 * functions for rvalue variant
 * @tparam Ts variant中的类型 / Types in variant
 * @tparam Fs 访问器函数类型 / Visitor function types
 * @param var 要匹配的variant / Variant to match on
 * @param visitors 每个类型的访问器函数 / Visitor functions for each type
 * @return 匹配的访问器的结果 / Result of matched visitor
 *
 * @code{.cpp}
 * // 对右值variant进行模式匹配的示例 / Example of pattern matching on rvalue
 * variant auto result = match(std::variant<int, std::string>(42),
 *   [](int&& i) { return i * 2; },
 *   [](std::string&& s) { return s + "!"; }
 * ); // 返回84 / Returns 84
 * @endcode
 */
template<typename... Ts, typename... Fs>
CPP_TOOLBOX_EXPORT auto match(std::variant<Ts...>&& var, Fs&&... visitors)
    -> decltype(auto);

/**
 * @brief 对variant中保存的值应用函数并在新variant中返回结果 / Apply a function
 * to the value held by a variant and return result in a new variant
 * @tparam ResultVariant 要返回的目标variant类型 / The target variant type to
 * return
 * @tparam Ts 输入variant中的类型 / Types in input variant
 * @tparam F 要应用的函数类型 / Function type to apply
 * @param var 输入variant / Input variant
 * @param f 要应用到variant值的函数 / Function to apply to variant value
 * @return 包含f的结果的ResultVariant / ResultVariant containing f's result
 * @throws std::bad_variant_access 如果var是valueless_by_exception / if var is
 * valueless_by_exception
 * @throws f抛出的任何异常 / Any exception thrown by f
 * @throws std::bad_variant_access 如果f的结果不能构造任何ResultVariant类型 / if
 * f's result cannot construct any ResultVariant type
 *
 * @code{.cpp}
 * // 在variant上映射的示例 / Example of mapping over variant
 * std::variant<int, std::string> v = 42;
 * auto result = map<std::variant<double, size_t>>(v,
 *   [](const auto& x) -> std::variant<double, size_t> {
 *     if constexpr (std::is_same_v<std::decay_t<decltype(x)>, int>) {
 *       return x * 1.5; // int -> double
 *     } else {
 *       return x.length(); // string -> size_t
 *     }
 *   }
 * ); // 返回包含63.0的variant / Returns variant containing 63.0
 * @endcode
 */
template<typename ResultVariant, typename... Ts, typename F>
CPP_TOOLBOX_EXPORT auto map(const std::variant<Ts...>& var, F&& f)
    -> ResultVariant;

/**
 * @brief 对非const左值variant中保存的值应用函数 / Apply a function to the value
 * held by a non-const lvalue variant
 * @tparam ResultVariant 要返回的目标variant类型 / The target variant type to
 * return
 * @tparam Ts 输入variant中的类型 / Types in input variant
 * @tparam F 要应用的函数类型 / Function type to apply
 * @param var 输入variant / Input variant
 * @param f 要应用到variant值的函数 / Function to apply to variant value
 * @return 包含f的结果的ResultVariant / ResultVariant containing f's result
 *
 * @code{.cpp}
 * // 在左值variant上映射的示例 / Example of mapping over lvalue variant
 * std::variant<int, std::string> v = 42;
 * auto result = map<std::variant<double>>(v,
 *   [](int& x) { x *= 2; return x * 1.5; }
 * ); // 修改v并返回包含126.0的variant / Modifies v and returns variant
 * containing 126.0
 * @endcode
 */
template<typename ResultVariant, typename... Ts, typename F>
CPP_TOOLBOX_EXPORT auto map(std::variant<Ts...>& var, F&& f) -> ResultVariant;

/**
 * @brief 对右值variant中保存的值应用函数 / Apply a function to the value held
 * by an rvalue variant
 * @tparam ResultVariant 要返回的目标variant类型 / The target variant type to
 * return
 * @tparam Ts 输入variant中的类型 / Types in input variant
 * @tparam F 要应用的函数类型 / Function type to apply
 * @param var 输入variant / Input variant
 * @param f 要应用到variant值的函数 / Function to apply to variant value
 * @return 包含f的结果的ResultVariant / ResultVariant containing f's result
 *
 * @code{.cpp}
 * // 在右值variant上映射的示例 / Example of mapping over rvalue variant
 * auto result = map<std::variant<std::string>>(
 *   std::variant<int, std::string>(42),
 *   [](auto&& x) { return std::to_string(x); }
 * ); // 返回包含"42"的variant / Returns variant containing "42"
 * @endcode
 */
template<typename ResultVariant, typename... Ts, typename F>
CPP_TOOLBOX_EXPORT auto map(std::variant<Ts...>&& var, F&& f) -> ResultVariant;

/**
 * @brief 对容器中的每个元素应用函数并返回结果向量 / Apply a function to each
 * element in a container and return results in a vector
 * @tparam Container 输入容器类型,必须提供begin()和end()迭代器 / Input container
 * type, must provide begin() and end() iterators
 * @tparam Func 要应用到元素的函数类型 / Function type to apply to elements
 * @param input 输入容器 / Input container
 * @param f 要应用到每个元素的函数 / Function to apply to each element
 * @return 包含对每个元素应用f的结果的向量 / Vector containing results of
 * applying f to each element
 *
 * @code{.cpp}
 * // 基本用法 / Basic usage
 * std::vector<int> nums = {1, 2, 3};
 * auto squares = map(nums, [](int x) { return x * x; });
 * // 返回 {1, 4, 9} / Returns {1, 4, 9}
 *
 * // 字符串长度映射 / String length mapping
 * std::list<std::string> strs = {"a", "bb", "ccc"};
 * auto lengths = map(strs, [](const auto& s) { return s.length(); });
 * // 返回 {1, 2, 3} / Returns {1, 2, 3}
 * @endcode
 */
template<typename Container, typename Func>
CPP_TOOLBOX_EXPORT auto map(const Container& input, Func&& f) -> std::vector<
    std::invoke_result_t<Func, typename Container::const_reference>>;

/**
 * @brief 过滤满足谓词的容器元素 / Filter elements from a container that satisfy
 * a predicate
 * @tparam Container 输入容器类型,必须提供begin()和end()迭代器 / Input container
 * type, must provide begin() and end() iterators
 * @tparam Predicate 谓词函数类型 / Predicate function type
 * @param input 输入容器 / Input container
 * @param p 返回bool的谓词函数 / Predicate function returning bool
 * @return 包含满足谓词p的元素的向量 / Vector containing elements that satisfy
 * predicate p
 *
 * @code{.cpp}
 * // 过滤偶数 / Filter even numbers
 * std::vector<int> nums = {1, 2, 3, 4, 5};
 * auto evens = filter(nums, [](int x) { return x % 2 == 0; });
 * // 返回 {2, 4} / Returns {2, 4}
 *
 * // 过滤长字符串 / Filter long strings
 * std::list<std::string> strs = {"a", "bb", "ccc"};
 * auto long_strs = filter(strs, [](const auto& s) { return s.length() > 1; });
 * // 返回 {"bb", "ccc"} / Returns {"bb", "ccc"}
 * @endcode
 */
template<typename Container, typename Predicate>
CPP_TOOLBOX_EXPORT auto filter(const Container& input, Predicate&& p)
    -> std::vector<typename Container::value_type>;

/**
 * @brief 使用带初始值的二元操作归约容器元素 / Reduce container elements using a
 * binary operation with initial value
 * @tparam Container 输入容器类型,必须提供begin()和end()迭代器 / Input container
 * type, must provide begin() and end() iterators
 * @tparam T 初始值和结果类型 / Initial value and result type
 * @tparam BinaryOp 二元操作函数类型 / Binary operation function type
 * @param input 输入容器 / Input container
 * @param identity 初始值(单位元) / Initial value (identity element)
 * @param op 组合元素的二元操作 / Binary operation to combine elements
 * @return 归约的结果 / Result of reduction
 *
 * @code{.cpp}
 * // 求和 / Sum numbers
 * std::vector<int> nums = {1, 2, 3, 4};
 * auto sum = reduce(nums, 0, std::plus<int>());
 * // 返回 10 / Returns 10
 *
 * // 求积 / Calculate product
 * auto product = reduce(nums, 1, std::multiplies<int>());
 * // 返回 24 / Returns 24
 *
 * // 连接字符串 / Concatenate strings
 * std::list<std::string> strs = {"a", "b", "c"};
 * auto concat = reduce(strs, std::string(), std::plus<std::string>());
 * // 返回 "abc" / Returns "abc"
 * @endcode
 */
template<typename Container, typename T, typename BinaryOp>
CPP_TOOLBOX_EXPORT auto reduce(const Container& input,
                               T identity,
                               BinaryOp&& op) -> T;

/**
 * @brief 使用二元操作归约非空容器元素 / Reduce non-empty container elements
 * using a binary operation
 * @tparam Container 输入容器类型,必须提供begin()和end()迭代器 / Input container
 * type, must provide begin() and end() iterators
 * @tparam BinaryOp 二元操作函数类型 / Binary operation function type
 * @param input 输入容器(必须非空) / Input container (must be non-empty)
 * @param op 组合元素的二元操作 / Binary operation to combine elements
 * @return 归约的结果 / Result of reduction
 * @throws std::invalid_argument 如果输入容器为空 / if input container is empty
 *
 * @code{.cpp}
 * // 求和 / Sum numbers
 * std::vector<int> nums = {1, 2, 3, 4};
 * auto sum = reduce(nums, std::plus<int>());
 * // 返回 10 / Returns 10
 *
 * // 连接字符串 / Concatenate strings
 * std::list<std::string> strs = {"a", "b", "c"};
 * auto concat = reduce(strs, std::plus<std::string>());
 * // 返回 "abc" / Returns "abc"
 *
 * // 空容器会抛出异常 / Empty container throws
 * std::vector<int> empty;
 * auto result = reduce(empty, std::plus<int>());
 * // 抛出 std::invalid_argument / throws std::invalid_argument
 * @endcode
 */
template<typename Container, typename BinaryOp>
CPP_TOOLBOX_EXPORT auto reduce(const Container& input, BinaryOp&& op) ->
    typename Container::value_type;

/**
 * @brief 将多个容器压缩成元组向量 / Zip multiple containers into a vector of
 * tuples
 * @tparam Containers 容器类型,必须提供size()和begin()/end() / Container types,
 * must provide size() and begin()/end()
 * @param containers 输入容器 / Input containers
 * @return 包含每个容器元素引用的元组向量 / Vector of tuples containing
 * references to elements from each container
 *
 * @code{.cpp}
 * // 压缩不同类型的容器 / Zip containers of different types
 * std::vector<int> nums = {1, 2, 3};
 * std::vector<std::string> strs = {"a", "b", "c"};
 * std::array<double> dbls = {1.1, 2.2, 3.3};
 *
 * auto zipped = zip(nums, strs, dbls);
 * // 返回元组向量 / Returns vector of tuples:
 * // {
 * //   {1, "a", 1.1},
 * //   {2, "b", 2.2},
 * //   {3, "c", 3.3}
 * // }
 * @endcode
 *
 * @note 结果长度由最短的输入容器决定 / The length of result is determined by
 * the shortest input container
 */
template<typename... Containers>
CPP_TOOLBOX_EXPORT auto zip(const Containers&... containers) -> std::vector<
    std::tuple<decltype(*std::cbegin(std::declval<const Containers&>()))...>>;

/**
 * @brief 将两个序列压缩成无序映射 / Zip two sequences into an unordered_map
 * @tparam ContainerKeys 键容器类型 / Key container type
 * @tparam ContainerValues 值容器类型 / Value container type
 * @tparam Key 映射键类型(默认:ContainerKeys::value_type) / Map key type
 * (default: ContainerKeys::value_type)
 * @tparam Value 映射值类型(默认:ContainerValues::value_type) / Map value type
 * (default: ContainerValues::value_type)
 * @tparam Hash 键哈希函数类型 / Key hash function type
 * @tparam KeyEqual 键相等比较函数类型 / Key equality comparison function type
 * @tparam Alloc 映射分配器类型 / Map allocator type
 * @param keys 键容器 / Container of keys
 * @param values 值容器 / Container of values
 * @return 包含键值对的无序映射 / Unordered map containing key-value pairs
 *
 * @code{.cpp}
 * // 基本用法 / Basic usage
 * std::vector<std::string> keys = {"a", "b", "c"};
 * std::vector<int> values = {1, 2, 3};
 *
 * auto map = zip_to_unordered_map(keys, values);
 * // 返回无序映射 / Returns unordered_map:
 * // {
 * //   {"a", 1},
 * //   {"b", 2},
 * //   {"c", 3}
 * // }
 * @endcode
 *
 * @note 如果键容器有重复,只使用第一次出现的键 / If keys container has
 * duplicates, only the first occurrence is used
 * @note 键值对数量由较短的容器决定 / The number of pairs is determined by the
 * shorter container
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
    -> std::unordered_map<Key, Value, Hash, KeyEqual, Alloc>;

/**
 * @brief 缓存函数结果的记忆化函数类 / Memoized function class that caches
 * function results
 * @tparam Signature 函数签名类型(如 int(int, std::string)) / Function signature
 * type (e.g., int(int, std::string))
 *
 * @code{.cpp}
 * // 创建记忆化阶乘函数 / Create memoized factorial function
 * MemoizedFunction<int(int)> fact([](int n) {
 *   if (n <= 1) return 1;
 *   return n * fact(n-1);
 * });
 *
 * int result1 = fact(5); // 计算并缓存 / Computes and caches
 * int result2 = fact(5); // 返回缓存结果 / Returns cached result
 * @endcode
 *
 * @note 线程安全实现 / Thread-safe implementation
 * @note 函数参数必须可复制且能形成有效的映射键 / Function arguments must be
 * copyable and form valid map key
 * @note 返回类型必须可复制 / Return type must be copyable
 */
template<typename Signature>
class CPP_TOOLBOX_EXPORT MemoizedFunction;  // 前向声明 / Forward declaration

/**
 * @brief MemoizedFunction对特定函数签名的特化 / Specialization of
 * MemoizedFunction for specific function signature 仅声明,实现在impl中 /
 * Declaration only, definition in impl
 */
template<typename R, typename... Args>
class CPP_TOOLBOX_EXPORT MemoizedFunction<R(Args...)>
{
private:
  using FuncType = std::function<R(Args...)>;
  using KeyType = std::tuple<std::decay_t<Args>...>;
  using ResultType = R;

  // 使用pimpl惯用法或shared_ptr隐藏实现细节 / Use pimpl idiom or shared_ptr to
  // hide implementation details 这避免在头文件中包含<map>和<mutex> / This
  // avoids including <map> and <mutex> in the header
  struct State;  // 前向声明状态结构 / Forward declare the state struct
  std::shared_ptr<State>
      state_;  // 指向实现状态的指针 / Pointer to implementation state

public:
  explicit MemoizedFunction(FuncType f);

  MemoizedFunction(const MemoizedFunction&) = delete;
  MemoizedFunction& operator=(const MemoizedFunction&) = delete;
  // 允许移动 / Allow moving
  MemoizedFunction(MemoizedFunction&&) noexcept = default;
  MemoizedFunction& operator=(MemoizedFunction&&) noexcept = default;

  auto operator()(Args... args) -> ResultType;
};

/**
 * @brief 创建带显式签名的记忆化函数 / Create a memoized function with explicit
 * signature
 * @tparam Signature 函数签名类型 / Function signature type
 * @tparam Func 函数对象类型 / Function object type
 * @param f 要记忆化的函数 / Function to memoize
 * @return MemoizedFunction对象 / MemoizedFunction object
 *
 * @code{.cpp}
 * // 带显式签名的记忆化函数 / Memoized function with explicit signature
 * auto memoized_add = memoize<int(int,int)>([](int a, int b) {
 *   return a + b;
 * });
 * int result1 = memoized_add(2, 3); // 计算并缓存 / Computes and caches
 * int result2 = memoized_add(2, 3); // 返回缓存结果 / Returns cached result
 * @endcode
 */
template<typename Signature, typename Func>
CPP_TOOLBOX_EXPORT auto memoize(Func&& f);

/**
 * @brief 创建带显式返回和参数类型的记忆化函数 / Create a memoized function with
 * explicit return and argument types
 * @tparam R 返回类型 / Return type
 * @tparam Args 参数类型 / Argument types
 * @tparam Func 函数对象类型 / Function object type
 * @param f 要记忆化的函数 / Function to memoize
 * @return 带记忆化的std::function / std::function with memoization
 *
 * @code{.cpp}
 * // 记忆化斐波那契函数 / Memoized fibonacci function
 * auto fib = memoize_explicit<int, int>([&](int n) {
 *   if (n <= 1) return n;
 *   return fib(n-1) + fib(n-2);
 * });
 * int result = fib(10); // 高效计算斐波那契数 / Efficiently computes fibonacci
 * @endcode
 *
 * @note 使用共享状态的线程安全实现 / Thread-safe implementation using shared
 * state
 * @note 参数经过decay后必须能形成有效的映射键 / Arguments must form valid map
 * key after decay
 * @note 返回类型必须可复制 / Return type must be copyable
 */
template<typename R, typename... Args, typename Func>
CPP_TOOLBOX_EXPORT auto memoize_explicit(Func&& f) -> std::function<R(Args...)>;

}  // namespace toolbox::functional

#include <cpp-toolbox/functional/impl/functional_impl.hpp>
