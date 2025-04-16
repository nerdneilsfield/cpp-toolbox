/**
 * @file type_traits.hpp
 * @brief 类型特征工具集合/Type traits utilities collection
 */
#pragma once
#include <cstddef>
#include <ostream>
#include <type_traits>

namespace toolbox
{

namespace traits
{

// 版本检测/Version detection
#if __cplusplus >= 202002L
#  define TOOLBOX_CPP20
#elif __cplusplus >= 201703L
#  define TOOLBOX_CPP17
#elif __cplusplus >= 201402L
#  define TOOLBOX_CPP14
#elif __cplusplus >= 201103L
#  define TOOLBOX_CPP11
#endif

// 基本类型特征工具/Basic type traits utilities
namespace detail
{
/**
 * @brief 用于SFINAE的空类型/Empty type for SFINAE
 * @tparam ... 可变数量的模板参数/Variable number of template parameters
 */
template<typename...>
using void_t = void;

/**
 * @brief 检查类型是否具有type成员/Check if type has a type member
 * @tparam T 要检查的类型/Type to check
 * @tparam void SFINAE参数/SFINAE parameter
 *
 * @code
 * struct HasType { using type = int; };
 * struct NoType {};
 * static_assert(has_type_impl<HasType>::value);
 * static_assert(!has_type_impl<NoType>::value);
 * @endcode
 */
template<typename T, typename = void>
struct has_type_impl : std::false_type
{
};

/**
 * @brief 检查类型是否具有type成员(特化版本)/Check if type has a type member
 * (specialization)
 * @tparam T 要检查的类型/Type to check
 */
template<typename T>
struct has_type_impl<T, void_t<typename T::type>> : std::true_type
{
};

/**
 * @brief 类型存储特征/Storage traits for types
 * @tparam T 要分析的类型/Type to analyze
 *
 * 提供类型的存储特征信息/Provides information about storage characteristics of
 * a type:
 * - 是否可以堆分配/Whether it may be heap allocated
 * - 是否必须堆分配/Whether it must be heap allocated
 * - 是否可以栈分配/Whether it may be stack allocated
 *
 * @code
 * class MyClass {};
 * class Abstract { virtual void foo() = 0; };
 *
 * // 检查普通类/Check regular class
 * static_assert(storage_traits<MyClass>::may_be_stack_allocated);
 * static_assert(!storage_traits<MyClass>::must_be_heap_allocated);
 *
 * // 检查抽象类/Check abstract class
 * static_assert(storage_traits<Abstract>::must_be_heap_allocated);
 * static_assert(!storage_traits<Abstract>::may_be_stack_allocated);
 * @endcode
 */
template<typename T>
struct storage_traits
{
  // 检查类型是否可以堆分配/Check if type may be heap allocated
  static constexpr bool may_be_heap_allocated = !std::is_array_v<T>
      && (sizeof(T) > 1024
          ||  // 大对象可能需要堆分配/Large objects may be heap allocated
          std::has_virtual_destructor_v<T>
          ||  // 虚析构函数暗示动态分配/Virtual destructor implies dynamic
              // allocation
          std::is_polymorphic_v<T>);  // 多态类型通常在堆上/Polymorphic types
                                      // usually on heap

  // 检查类型是否必须堆分配/Check if type must be heap allocated
  static constexpr bool must_be_heap_allocated =
      std::is_abstract_v<T>;  // 抽象类必须堆分配/Abstract classes must be heap
                              // allocated

  // 检查类型是否可以栈分配/Check if type may be stack allocated
  static constexpr bool may_be_stack_allocated =
      !must_be_heap_allocated && std::is_object_v<T> && !std::is_abstract_v<T>;
};
}  // namespace detail

/**
 * @brief 类型标识特征/Type identity trait
 * @tparam T 要标识的类型/Type to identify
 *
 * @code
 * // 保持类型不变/Keep type unchanged
 * using type1 = type_identity<int>::type;  // type1 is int
 * using type2 = type_identity<std::string>::type;  // type2 is std::string
 * @endcode
 */
template<typename T>
struct type_identity
{
  using type = T;
};

/**
 * @brief 移除类型的引用/Remove reference from type
 * @tparam T 要移除引用的类型/Type to remove reference from
 *
 * @code
 * // 移除左值引用/Remove lvalue reference
 * using type1 = remove_reference<int&>::type;  // type1 is int
 * // 移除右值引用/Remove rvalue reference
 * using type2 = remove_reference<int&&>::type;  // type2 is int
 * @endcode
 */
template<typename T>
struct remove_reference
{
  using type = std::remove_reference_t<T>;
};

// 特定成员或方法的检测/Detection of specific members or methods
#ifdef TOOLBOX_CPP17
/**
 * @brief 检查类型是否有toString方法(C++17版本)/Check if type has toString
 * method (C++17 version)
 * @tparam T 要检查的类型/Type to check
 * @tparam void SFINAE参数/SFINAE parameter
 *
 * @code
 * struct WithToString {
 *   std::string toString() { return "hello"; }
 * };
 * struct WithoutToString {};
 *
 * static_assert(has_toString<WithToString>::value);
 * static_assert(!has_toString<WithoutToString>::value);
 * @endcode
 */
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
/**
 * @brief 检查类型是否有toString方法(C++17之前版本)/Check if type has toString
 * method (pre-C++17 version)
 * @tparam T 要检查的类型/Type to check
 *
 * @code
 * struct WithToString {
 *   std::string toString() { return "hello"; }
 * };
 * struct WithoutToString {};
 *
 * static_assert(has_toString<WithToString>::value);
 * static_assert(!has_toString<WithoutToString>::value);
 * @endcode
 */
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
 * @brief 移除类型的所有限定符/Remove all qualifiers from type
 * @tparam T 要移除限定符的类型/Type to remove qualifiers from
 *
 * @code
 * // 移除const和引用限定符/Remove const and reference qualifiers
 * using type1 = remove_all_qualifiers<const int&>::type;  // type1 is int
 * // 移除volatile和const限定符/Remove volatile and const qualifiers
 * using type2 = remove_all_qualifiers<volatile const double>::type;  // type2
 * is double
 * @endcode
 */
template<typename T>
struct remove_all_qualifiers
{
  using type =
      typename std::remove_cv<typename std::remove_reference<T>::type>::type;
};

// C++17及以上的辅助别名/C++17 and above helper aliases
#ifdef TOOLBOX_CPP17
/**
 * @brief 移除所有限定符的辅助别名/Helper alias to remove all qualifiers
 * @tparam T 要移除限定符的类型/Type to remove qualifiers from
 *
 * @code
 * // 使用别名模板简化语法/Use alias template to simplify syntax
 * using type1 = remove_all_qualifiers_t<const int&>;  // type1 is int
 * using type2 = remove_all_qualifiers_t<volatile const double>;  // type2 is
 * double
 * @endcode
 */
template<typename T>
using remove_all_qualifiers_t = typename remove_all_qualifiers<T>::type;
#endif

// 可调用对象检测/Callable detection
#ifdef TOOLBOX_CPP20
/**
 * @brief 检查类型是否可调用(C++20概念)/Check if type is callable (C++20
 * concept)
 * @tparam T 要检查的类型/Type to check
 *
 * @code
 * // 检查lambda是否可调用/Check if lambda is callable
 * auto lambda = []() { return 42; };
 * static_assert(Callable<decltype(lambda)>);
 *
 * // 检查函数对象是否可调用/Check if function object is callable
 * struct Functor { void operator()() {} };
 * static_assert(Callable<Functor>);
 * @endcode
 */
template<typename T>
concept Callable = requires(T t) { &T::operator(); };
#else
/**
 * @brief 检查类型是否可调用/Check if type is callable
 * @tparam T 要检查的类型/Type to check
 * @tparam void SFINAE参数/SFINAE parameter
 *
 * @code
 * // 检查lambda是否可调用/Check if lambda is callable
 * auto lambda = []() { return 42; };
 * static_assert(is_callable<decltype(lambda)>::value);
 *
 * // 检查函数对象是否可调用/Check if function object is callable
 * struct Functor { void operator()() {} };
 * static_assert(is_callable<Functor>::value);
 * @endcode
 */
template<typename T, typename = void>
struct is_callable : std::false_type
{
};

template<typename T>
struct is_callable<T, std::void_t<decltype(&T::operator())>> : std::true_type
{
};
#endif

/**
 * @brief 类型列表容器/Type list container
 * @tparam Ts 类型参数包/Parameter pack of types
 *
 * @code
 * // 创建类型列表/Create type list
 * using number_types = type_list<int, float, double>;
 * static_assert(number_types::size == 3);
 *
 * // 创建空类型列表/Create empty type list
 * using empty_list = type_list<>;
 * static_assert(empty_list::size == 0);
 * @endcode
 */
template<typename... Ts>
struct type_list
{
  static constexpr size_t size = sizeof...(Ts);
};

/**
 * @brief 函数特征基础模板/Function traits base template
 * @tparam T 函数类型/Function type
 */
template<typename T>
struct function_traits;

/**
 * @brief 普通函数的函数特征/Function traits for regular functions
 * @tparam R 返回类型/Return type
 * @tparam Args 函数参数类型/Function parameter types
 *
 * @code
 * // 分析普通函数/Analyze regular function
 * int func(float x, double y) { return 0; }
 * using traits = function_traits<decltype(func)>;
 * static_assert(std::is_same_v<traits::return_type, int>);
 * static_assert(traits::arity == 2);
 * static_assert(std::is_same_v<traits::arg_type<0>, float>);
 * @endcode
 */
template<typename R, typename... Args>
struct function_traits<R(Args...)>
{
  using return_type = R;
  static constexpr size_t arity = sizeof...(Args);

  template<size_t N>
  using arg_type = typename std::tuple_element<N, std::tuple<Args...>>::type;
};

/**
 * @brief 成员函数的函数特征/Function traits for member functions
 * @tparam C 类类型/Class type
 * @tparam R 返回类型/Return type
 * @tparam Args 函数参数类型/Function parameter types
 *
 * @code
 * // 分析成员函数/Analyze member function
 * struct MyClass {
 *   double method(int x, float y) { return 0.0; }
 * };
 *
 * using traits = function_traits<decltype(&MyClass::method)>;
 * static_assert(std::is_same_v<traits::class_type, MyClass>);
 * static_assert(std::is_same_v<traits::return_type, double>);
 * static_assert(traits::arity == 2);
 * @endcode
 */
template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...)> : function_traits<R(Args...)>
{
  using class_type = C;
};

// SFINAE工具/SFINAE utilities
#ifdef TOOLBOX_CPP20
/**
 * @brief 检查类型是否有size成员(C++20概念)/Check if type has size member (C++20
 * concept)
 * @tparam T 要检查的类型/Type to check
 *
 * @code
 * // 检查容器类型/Check container types
 * static_assert(HasSize<std::vector<int>>);
 * static_assert(HasSize<std::string>);
 * static_assert(!HasSize<int>);
 * @endcode
 */
template<typename T>
concept HasSize = requires(T t) {
  {
    t.size()
  } -> std::convertible_to<std::size_t>;
};

/**
 * @brief 检查类型是否可打印(C++20概念)/Check if type is printable (C++20
 * concept)
 * @tparam T 要检查的类型/Type to check
 *
 * @code
 * struct Printable { friend std::ostream& operator<<(std::ostream& os, const
 * Printable&) { return os; } }; struct NonPrintable {};
 *
 * static_assert(Printable<int>);
 * static_assert(Printable<std::string>);
 * static_assert(Printable<Printable>);
 * static_assert(!Printable<NonPrintable>);
 * @endcode
 */
template<typename T>
concept Printable = requires(T t, std::ostream& os) {
  {
    os << t
  } -> std::same_as<std::ostream&>;
};
#else
/**
 * @brief 检查类型是否有size成员/Check if type has size member
 * @tparam T 要检查的类型/Type to check
 * @tparam void SFINAE参数/SFINAE parameter
 *
 * @code
 * // 检查容器类型/Check container types
 * static_assert(has_size<std::vector<int>>::value);
 * static_assert(has_size<std::string>::value);
 * static_assert(!has_size<int>::value);
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
 * @brief 检查类型是否可打印/Check if type is printable
 * @tparam T 要检查的类型/Type to check
 *
 * @code
 * struct Printable { friend std::ostream& operator<<(std::ostream& os, const
 * Printable&) { return os; } }; struct NonPrintable {};
 *
 * static_assert(is_printable<int>::value);
 * static_assert(is_printable<std::string>::value);
 * static_assert(is_printable<Printable>::value);
 * static_assert(!is_printable<NonPrintable>::value);
 * @endcode
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

/**
 * @brief 类型安全的枚举包装器/Type-safe enum wrapper
 * @tparam EnumType 要包装的枚举类型/Enum type to wrap
 *
 * @code
 * enum class Color { Red, Green, Blue };
 *
 * // 创建和使用包装器/Create and use wrapper
 * enum_wrapper<Color> color(Color::Red);
 * Color raw_color = color;  // 隐式转换/Implicit conversion
 * auto value = color.to_underlying();  // 获取底层值/Get underlying value
 *
 * // 编译期检查/Compile-time check
 * // enum_wrapper<int> error;  // 编译错误：不是枚举类型/Compilation error: not
 * an enum type
 * @endcode
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

// 编译期类型名称/Compile-time type name
#ifdef TOOLBOX_CPP17
/**
 * @brief 在编译期获取类型名称/Get type name at compile time
 * @tparam T 要获取名称的类型/Type to get name of
 * @return 包含类型名称的std::string_view/std::string_view containing type name
 *
 * @code
 * // 获取基本类型名称/Get basic type names
 * constexpr auto int_name = type_name<int>();  // "int"
 * constexpr auto string_name = type_name<std::string>();  // "std::string"
 *
 * // 获取复杂类型名称/Get complex type names
 * template<typename T>
 * class MyTemplate {};
 * constexpr auto template_name = type_name<MyTemplate<int>>();
 * @endcode
 */
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
 * @brief 检查类型是否必须堆分配/Check if type must be heap allocated
 * @tparam T 要检查的类型/Type to check
 *
 * @code
 * class Abstract { virtual void foo() = 0; };
 * class Concrete {};
 *
 * static_assert(is_heap_allocated<Abstract>::value);
 * static_assert(!is_heap_allocated<Concrete>::value);
 * @endcode
 */
template<typename T>
struct is_heap_allocated
{
  static constexpr bool value =
      detail::storage_traits<T>::must_be_heap_allocated;
};

/**
 * @brief 检查类型是否可以栈分配/Check if type can be stack allocated
 * @tparam T 要检查的类型/Type to check
 *
 * @code
 * class Abstract { virtual void foo() = 0; };
 * class Concrete {};
 *
 * static_assert(!is_stack_allocated<Abstract>::value);
 * static_assert(is_stack_allocated<Concrete>::value);
 * @endcode
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
 * @brief 检查类型是否为const限定/Check if type is const-qualified
 * @tparam T 要检查的类型/Type to check
 *
 * @code
 * static_assert(is_const<const int>::value);
 * static_assert(!is_const<int>::value);
 * static_assert(is_const<const std::string>::value);
 * @endcode
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
 * @brief 检查类型是否为volatile限定/Check if type is volatile-qualified
 * @tparam T 要检查的类型/Type to check
 *
 * @code
 * static_assert(is_volatile<volatile int>::value);
 * static_assert(!is_volatile<int>::value);
 * static_assert(is_volatile<volatile double>::value);
 * @endcode
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
 * @brief 检查类型是否同时具有const和volatile限定/Check if type is
 * const-volatile qualified
 * @tparam T 要检查的类型/Type to check
 *
 * @code
 * static_assert(is_const_volatile<const volatile int>::value);
 * static_assert(!is_const_volatile<const int>::value);
 * static_assert(!is_const_volatile<volatile int>::value);
 * @endcode
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
 * @brief 检查类型是否为引用/Check if type is a reference
 * @tparam T 要检查的类型/Type to check
 *
 * @code
 * static_assert(is_reference<int&>::value);
 * static_assert(is_reference<int&&>::value);
 * static_assert(!is_reference<int>::value);
 * @endcode
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
 * @brief 检查类型是否为数组/Check if type is an array
 * @tparam T 要检查的类型/Type to check
 *
 * @code
 * static_assert(is_array<int[]>::value);
 * static_assert(is_array<int[5]>::value);
 * static_assert(!is_array<int*>::value);
 * @endcode
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
 * @brief 检查类型是否为函数/Check if type is a function
 * @tparam T 要检查的类型/Type to check
 *
 * @code
 * void func(int);
 * static_assert(is_function<decltype(func)>::value);
 * static_assert(!is_function<int>::value);
 * static_assert(!is_function<void(*)()>::value);
 * @endcode
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
 * @brief 检查类型是否为成员指针/Check if type is a member pointer
 * @tparam T 要检查的类型/Type to check
 *
 * @code
 * struct S { int m; void f(); };
 * static_assert(is_member_pointer<int S::*>::value);
 * static_assert(is_member_pointer<void (S::*)()>::value);
 * static_assert(!is_member_pointer<int*>::value);
 * @endcode
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
 * @brief 检查类型是否为指针/Check if type is a pointer
 * @tparam T 要检查的类型/Type to check
 *
 * @code
 * static_assert(is_pointer<int*>::value);
 * static_assert(is_pointer<void*>::value);
 * static_assert(!is_pointer<int>::value);
 * static_assert(!is_pointer<int&>::value);
 * @endcode
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
 * @brief 检查类型是否为std::nullptr_t/Check if type is std::nullptr_t
 * @tparam T 要检查的类型/Type to check
 *
 * @code
 * static_assert(is_null_pointer<std::nullptr_t>::value);
 * static_assert(!is_null_pointer<void*>::value);
 * static_assert(!is_null_pointer<int*>::value);
 * @endcode
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

// 1. 基础：void_t 实现（C++17 提供，但习惯写出来更清晰）
template<typename...>
using void_t = void;

// 2. is_iterable trait：只有当 T 有 begin/end 时才是可迭代的
template<typename T, typename = void>
struct is_iterable : std::false_type
{
};

template<typename T>
struct is_iterable<T,
                   void_t<decltype(std::begin(std::declval<T>())),
                          decltype(std::end(std::declval<T>()))>>
    : std::true_type
{
};

template<typename T>
inline constexpr bool is_iterable_v = is_iterable<T>::value;

}  // namespace traits
}  // namespace toolbox
