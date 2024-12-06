// type_traits.hpp
#pragma once
#include <cstddef>
#include <ostream>
#include <type_traits>

namespace toolbox
{

namespace traits
{

// 版本检测
#if __cplusplus >= 202002L
#  define TOOLBOX_CPP20
#elif __cplusplus >= 201703L
#  define TOOLBOX_CPP17
#elif __cplusplus >= 201402L
#  define TOOLBOX_CPP14
#elif __cplusplus >= 201103L
#  define TOOLBOX_CPP11
#endif

// 类型特征基础工具
namespace detail
{
/**
 * @brief 空类型
 * @note 用于 SFINAE
 */
template<typename...>
using void_t = void;

/**
 * @brief 检查类型是否具有 type 成员
 * @note 用于 SFINAE
 */
template<typename T, typename = void>
struct has_type_impl : std::false_type
{
};

/**
 * @brief 检查类型是否具有 type 成员
 * @note 用于 SFINAE
 */
template<typename T>
struct has_type_impl<T, void_t<typename T::type>> : std::true_type
{
};

/**
 * @brief 检查类型是否可能在堆上分配
 * @note 用于 SFINAE
 */
template<typename T>
struct storage_traits
{
  // 检查是否可能在堆上分配
  static constexpr bool may_be_heap_allocated = !std::is_array_v<T>
      && (sizeof(T) > 1024 ||  // 大对象可能在堆上
          std::has_virtual_destructor_v<T> ||  // 虚析构函数通常意味着动态分配
          std::is_polymorphic_v<T>);  // 多态类型通常在堆上

  // 检查是否必须在堆上分配
  static constexpr bool must_be_heap_allocated =
      std::is_abstract_v<T>;  // 抽象类必须在堆上

  // 检查是否可能在栈上分配
  static constexpr bool may_be_stack_allocated =
      !must_be_heap_allocated && std::is_object_v<T> && !std::is_abstract_v<T>;
};
}  // namespace detail

// 基础类型特征
/**
 * @brief 类型标识
 */
template<typename T>
struct type_identity
{
  using type = T;
};

/**
 * @brief 移除引用
 */
template<typename T>
struct remove_reference
{
  using type = std::remove_reference_t<T>;
};

// 检测是否有特定成员或方法
#ifdef TOOLBOX_CPP17
// C++17 版本使用 void_t
template<typename T, typename = void>
struct has_toString : std::false_type
{
};

template<typename T>
struct has_toString<T, std::void_t<decltype(std::declval<T>().toString())>>
    : std::true_type
{
};
#else
// C++11/14 版本
template<typename T>
struct has_toString
{
private:
  template<typename U>
  static auto test(int)
      -> decltype(std::declval<U>().toString(), std::true_type {});

  template<typename>
  static std::false_type test(...);

public:
  static constexpr bool value = decltype(test<T>(0))::value;
};
#endif

/**
 * @brief 移除所有限定符
 */
template<typename T>
struct remove_all_qualifiers
{
  using type =
      typename std::remove_cv<typename std::remove_reference<T>::type>::type;
};

// C++17 及以上版本的简化写法
#ifdef TOOLBOX_CPP17
/**
 * @brief 移除所有限定符
 */
template<typename T>
using remove_all_qualifiers_t = typename remove_all_qualifiers<T>::type;
#endif

// 检测是否可调用
#ifdef TOOLBOX_CPP20
/**
 * @brief 检测是否可调用
 */
template<typename T>
concept Callable = requires(T t) { &T::operator(); };
#else
/**
 * @brief 检测是否可调用
 */
template<typename T, typename = void>
struct is_callable : std::false_type
{
};

/**
 * @brief 检测是否可调用
 */
template<typename T>
struct is_callable<T, std::void_t<decltype(&T::operator())>> : std::true_type
{
};
#endif

// 类型列表操作
/**
 * @brief 类型列表
 */
template<typename... Ts>
struct type_list
{
  static constexpr size_t size = sizeof...(Ts);
};

// 类型萃取工具
/**
 * @brief 函数类型萃取
 */
template<typename T>
struct function_traits;

// 普通函数
/**
 * @brief 普通函数类型萃取
 */
template<typename R, typename... Args>
struct function_traits<R(Args...)>
{
  using return_type = R;
  static constexpr size_t arity = sizeof...(Args);

  template<size_t N>
  using arg_type = typename std::tuple_element<N, std::tuple<Args...>>::type;
};

// 成员函数
/**
 * @brief 成员函数类型萃取
 */
template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...)> : function_traits<R(Args...)>
{
  using class_type = C;
};

// SFINAE 工具
#ifdef TOOLBOX_CPP20
// C++20 使用 concepts
/**
 * @brief 检测是否具有 size 成员
 */
template<typename T>
concept HasSize = requires(T t) {
  {
    t.size()
  } -> std::convertible_to<std::size_t>;
};

/**
 * @brief 检测是否可打印
 */
template<typename T>
concept Printable = requires(T t, std::ostream& os) {
  {
    os << t
  } -> std::same_as<std::ostream&>;
};
#else
// C++17 及以下版本
/**
 * @brief 检测是否具有 size 成员
 */
template<typename T, typename = void>
struct has_size : std::false_type
{
};

/**
 * @brief 检测是否具有 size 成员
 */
template<typename T>
struct has_size<T, std::void_t<decltype(std::declval<T>().size())>>
    : std::true_type
{
};

/**
 * @brief 检测是否可打印
 */
template<typename T>
struct is_printable
{
private:
  template<typename U>
  static auto test(int)
      -> decltype(std::declval<std::ostream&>() << std::declval<U>(),
                  std::true_type {});

  template<typename>
  static std::false_type test(...);

public:
  static constexpr bool value = decltype(test<T>(0))::value;
};
#endif

// 类型安全的枚举包装
/**
 * @brief 类型安全的枚举包装
 */
template<typename EnumType>
class enum_wrapper
{
  static_assert(std::is_enum<EnumType>::value,
                "Template parameter must be an enum type");

public:
  using underlying_type = typename std::underlying_type<EnumType>::type;

  constexpr enum_wrapper(EnumType value)
      : value_(value)
  {
  }

  constexpr operator EnumType() const { return value_; }
  constexpr auto to_underlying() const -> underlying_type
  {
    return static_cast<underlying_type>(value_);
  }

private:
  EnumType value_;
};

// 编译时类型名称
#ifdef TOOLBOX_CPP17
template<typename T>
constexpr std::string_view type_name()
{
#  if defined(__clang__)
  constexpr auto prefix = std::string_view {"[T = "};
  constexpr auto suffix = std::string_view {"]"};
  constexpr auto function = std::string_view {__PRETTY_FUNCTION__};
#  elif defined(__GNUG__)
  constexpr auto prefix = std::string_view {"with T = "};
  constexpr auto suffix = std::string_view {"]"};
  constexpr auto function = std::string_view {__PRETTY_FUNCTION__};
#  elif defined(_MSC_VER)
  constexpr auto prefix = std::string_view {"type_name<"};
  constexpr auto suffix = std::string_view {">(void)"};
  constexpr auto function = std::string_view {__FUNCSIG__};
#  else
#    error Unsupported compiler
#  endif

  const auto start = function.find(prefix) + prefix.size();
  const auto end = function.find(suffix);
  const auto size = end - start;
  return function.substr(start, size);
}
#endif

/**
 * @brief 堆分配检测
 */
template<typename T>
struct is_heap_allocated
{
  static constexpr bool value =
      detail::storage_traits<T>::must_be_heap_allocated;
};

/**
 * @brief 栈分配检测
 */
template<typename T>
struct is_stack_allocated
{
  static constexpr bool value =
      detail::storage_traits<T>::may_be_stack_allocated;
};

#ifdef TOOLBOX_CPP17
template<typename T>
inline constexpr bool is_heap_allocated_v = is_heap_allocated<T>::value;

template<typename T>
inline constexpr bool is_stack_allocated_v = is_stack_allocated<T>::value;
#endif

/**
 * @brief 是否是 const 类型
 */
template<typename T>
struct is_const
{
  static constexpr bool value = std::is_const_v<T>;
};

#ifdef TOOLBOX_CPP17
template<typename T>
inline constexpr bool is_const_v = is_const<T>::value;
#endif

/**
 * @brief 是否是 volatile 类型
 */
template<typename T>
struct is_volatile
{
  static constexpr bool value = std::is_volatile_v<T>;
};

#ifdef TOOLBOX_CPP17
template<typename T>
inline constexpr bool is_volatile_v = is_volatile<T>::value;
#endif

/**
 * @brief 是否是 const volatile 类型
 */
template<typename T>
struct is_const_volatile
{
  static constexpr bool value = is_const_v<T> && is_volatile_v<T>;
};

#ifdef TOOLBOX_CPP17
template<typename T>
inline constexpr bool is_const_volatile_v = is_const_volatile<T>::value;
#endif

/**
 * @brief 是否是引用类型
 */
template<typename T>
struct is_reference
{
  static constexpr bool value = std::is_reference_v<T>;
};

#ifdef TOOLBOX_CPP17
template<typename T>
inline constexpr bool is_reference_v = is_reference<T>::value;
#endif

/**
 * @brief 是否是数组类型
 */
template<typename T>
struct is_array
{
  static constexpr bool value = std::is_array_v<T>;
};

#ifdef TOOLBOX_CPP17
template<typename T>
inline constexpr bool is_array_v = is_array<T>::value;
#endif

/**
 * @brief 是否是函数类型
 */
template<typename T>
struct is_function
{
  static constexpr bool value = std::is_function_v<T>;
};

#ifdef TOOLBOX_CPP17
template<typename T>
inline constexpr bool is_function_v = is_function<T>::value;
#endif

/**
 * @brief 是否是成员指针类型
 */
template<typename T>
struct is_member_pointer
{
  static constexpr bool value = std::is_member_pointer_v<T>;
};

#ifdef TOOLBOX_CPP17
template<typename T>
inline constexpr bool is_member_pointer_v = is_member_pointer<T>::value;
#endif

/**
 * @brief 是否是指针类型
 */
template<typename T>
struct is_pointer
{
  static constexpr bool value = std::is_pointer_v<T>;
};

#ifdef TOOLBOX_CPP17
template<typename T>
inline constexpr bool is_pointer_v = is_pointer<T>::value;
#endif

/**
 * @brief 是否是空类型
 */
template<typename T>
struct is_null_pointer
{
  static constexpr bool value = std::is_null_pointer_v<T>;
};

#ifdef TOOLBOX_CPP17
template<typename T>
inline constexpr bool is_null_pointer_v = is_null_pointer<T>::value;
#endif

}  // namespace traits
}  // namespace toolbox
