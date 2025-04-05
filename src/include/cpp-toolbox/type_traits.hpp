// type_traits.hpp
#pragma once
#include <cstddef>
#include <ostream>
#include <type_traits>

namespace toolbox
{

namespace traits
{

// Version detection
#if __cplusplus >= 202002L
#  define TOOLBOX_CPP20
#elif __cplusplus >= 201703L
#  define TOOLBOX_CPP17
#elif __cplusplus >= 201402L
#  define TOOLBOX_CPP14
#elif __cplusplus >= 201103L
#  define TOOLBOX_CPP11
#endif

// Basic type traits utilities
namespace detail
{
/**
 * @brief Empty type for SFINAE
 * @tparam ... Variable number of template parameters
 */
template<typename...>
using void_t = void;

/**
 * @brief Check if type has a type member
 * @tparam T Type to check
 * @tparam void SFINAE parameter
 */
template<typename T, typename = void>
struct has_type_impl : std::false_type
{
};

/**
 * @brief Check if type has a type member (specialization)
 * @tparam T Type to check
 */
template<typename T>
struct has_type_impl<T, void_t<typename T::type>> : std::true_type
{
};

/**
 * @brief Storage traits for types
 * @tparam T Type to analyze
 *
 * Provides information about storage characteristics of a type:
 * - Whether it may be heap allocated
 * - Whether it must be heap allocated
 * - Whether it may be stack allocated
 *
 * Example:
 * @code
 * class MyClass {};
 * bool mayBeHeap = storage_traits<MyClass>::may_be_heap_allocated;
 * bool mustBeHeap = storage_traits<MyClass>::must_be_heap_allocated;
 * bool mayBeStack = storage_traits<MyClass>::may_be_stack_allocated;
 * @endcode
 */
template<typename T>
struct storage_traits
{
  // Check if type may be heap allocated
  static constexpr bool may_be_heap_allocated = !std::is_array_v<T>
      && (sizeof(T) > 1024 ||  // Large objects may be heap allocated
          std::has_virtual_destructor_v<T>
          ||  // Virtual destructor implies dynamic allocation
          std::is_polymorphic_v<T>);  // Polymorphic types usually on heap

  // Check if type must be heap allocated
  static constexpr bool must_be_heap_allocated =
      std::is_abstract_v<T>;  // Abstract classes must be heap allocated

  // Check if type may be stack allocated
  static constexpr bool may_be_stack_allocated =
      !must_be_heap_allocated && std::is_object_v<T> && !std::is_abstract_v<T>;
};
}  // namespace detail

/**
 * @brief Type identity trait
 * @tparam T Type to identify
 *
 * Example:
 * @code
 * using type = type_identity<int>::type; // type is int
 * @endcode
 */
template<typename T>
struct type_identity
{
  using type = T;
};

/**
 * @brief Remove reference from type
 * @tparam T Type to remove reference from
 *
 * Example:
 * @code
 * using type = remove_reference<int&>::type; // type is int
 * @endcode
 */
template<typename T>
struct remove_reference
{
  using type = std::remove_reference_t<T>;
};

// Detection of specific members or methods
#ifdef TOOLBOX_CPP17
/**
 * @brief Check if type has toString method (C++17 version)
 * @tparam T Type to check
 * @tparam void SFINAE parameter
 *
 * Example:
 * @code
 * class A { std::string toString() { return "A"; } };
 * static_assert(has_toString<A>::value, "A has toString");
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
 * @brief Check if type has toString method (pre-C++17 version)
 * @tparam T Type to check
 *
 * Example:
 * @code
 * class A { std::string toString() { return "A"; } };
 * static_assert(has_toString<A>::value, "A has toString");
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
 * @brief Remove all qualifiers from type
 * @tparam T Type to remove qualifiers from
 *
 * Example:
 * @code
 * using type = remove_all_qualifiers<const int&>::type; // type is int
 * @endcode
 */
template<typename T>
struct remove_all_qualifiers
{
  using type =
      typename std::remove_cv<typename std::remove_reference<T>::type>::type;
};

// C++17 and above helper aliases
#ifdef TOOLBOX_CPP17
/**
 * @brief Helper alias to remove all qualifiers
 * @tparam T Type to remove qualifiers from
 */
template<typename T>
using remove_all_qualifiers_t = typename remove_all_qualifiers<T>::type;
#endif

// Callable detection
#ifdef TOOLBOX_CPP20
/**
 * @brief Check if type is callable (C++20 concept)
 * @tparam T Type to check
 *
 * Example:
 * @code
 * auto lambda = []() { return 42; };
 * static_assert(Callable<decltype(lambda)>);
 * @endcode
 */
template<typename T>
concept Callable = requires(T t) { &T::operator(); };
#else
/**
 * @brief Check if type is callable
 * @tparam T Type to check
 * @tparam void SFINAE parameter
 *
 * Example:
 * @code
 * auto lambda = []() { return 42; };
 * static_assert(is_callable<decltype(lambda)>::value);
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
 * @brief Type list container
 * @tparam Ts Parameter pack of types
 *
 * Example:
 * @code
 * using list = type_list<int, float, double>;
 * static_assert(list::size == 3);
 * @endcode
 */
template<typename... Ts>
struct type_list
{
  static constexpr size_t size = sizeof...(Ts);
};

/**
 * @brief Function traits base template
 * @tparam T Function type
 */
template<typename T>
struct function_traits;

/**
 * @brief Function traits for regular functions
 * @tparam R Return type
 * @tparam Args Function parameter types
 *
 * Example:
 * @code
 * int func(float, double);
 * using traits = function_traits<decltype(func)>;
 * static_assert(std::is_same_v<traits::return_type, int>);
 * static_assert(traits::arity == 2);
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
 * @brief Function traits for member functions
 * @tparam C Class type
 * @tparam R Return type
 * @tparam Args Function parameter types
 *
 * Example:
 * @code
 * struct A {
 *   int method(float, double);
 * };
 * using traits = function_traits<decltype(&A::method)>;
 * static_assert(std::is_same_v<traits::class_type, A>);
 * @endcode
 */
template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...)> : function_traits<R(Args...)>
{
  using class_type = C;
};

// SFINAE utilities
#ifdef TOOLBOX_CPP20
/**
 * @brief Check if type has size member (C++20 concept)
 * @tparam T Type to check
 *
 * Example:
 * @code
 * std::vector<int> v;
 * static_assert(HasSize<decltype(v)>);
 * @endcode
 */
template<typename T>
concept HasSize = requires(T t) {
  {
    t.size()
  } -> std::convertible_to<std::size_t>;
};

/**
 * @brief Check if type is printable (C++20 concept)
 * @tparam T Type to check
 *
 * Example:
 * @code
 * static_assert(Printable<int>);
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
 * @brief Check if type has size member
 * @tparam T Type to check
 * @tparam void SFINAE parameter
 *
 * Example:
 * @code
 * std::vector<int> v;
 * static_assert(has_size<decltype(v)>::value);
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
 * @brief Check if type is printable
 * @tparam T Type to check
 *
 * Example:
 * @code
 * static_assert(is_printable<int>::value);
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
 * @brief Type-safe enum wrapper
 * @tparam EnumType Enum type to wrap
 *
 * Example:
 * @code
 * enum class Color { Red, Green, Blue };
 * enum_wrapper<Color> wrapped(Color::Red);
 * auto value = wrapped.to_underlying(); // Get underlying value
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

// Compile-time type name
#ifdef TOOLBOX_CPP17
/**
 * @brief Get type name at compile time
 * @tparam T Type to get name of
 * @return std::string_view containing type name
 *
 * Example:
 * @code
 * constexpr auto name = type_name<int>();
 * std::cout << name; // Prints "int"
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
 * @brief Check if type must be heap allocated
 * @tparam T Type to check
 *
 * Example:
 * @code
 * class Abstract { virtual void foo() = 0; };
 * static_assert(is_heap_allocated<Abstract>::value);
 * @endcode
 */
template<typename T>
struct is_heap_allocated
{
  static constexpr bool value =
      detail::storage_traits<T>::must_be_heap_allocated;
};

/**
 * @brief Check if type can be stack allocated
 * @tparam T Type to check
 *
 * Example:
 * @code
 * struct Simple {};
 * static_assert(is_stack_allocated<Simple>::value);
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
 * @brief Check if type is const-qualified
 * @tparam T Type to check
 *
 * Example:
 * @code
 * static_assert(is_const<const int>::value);
 * static_assert(!is_const<int>::value);
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
 * @brief Check if type is volatile-qualified
 * @tparam T Type to check
 *
 * Example:
 * @code
 * static_assert(is_volatile<volatile int>::value);
 * static_assert(!is_volatile<int>::value);
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
 * @brief Check if type is const-volatile qualified
 * @tparam T Type to check
 *
 * Example:
 * @code
 * static_assert(is_const_volatile<const volatile int>::value);
 * static_assert(!is_const_volatile<const int>::value);
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
 * @brief Check if type is a reference
 * @tparam T Type to check
 *
 * Example:
 * @code
 * static_assert(is_reference<int&>::value);
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
 * @brief Check if type is an array
 * @tparam T Type to check
 *
 * Example:
 * @code
 * static_assert(is_array<int[]>::value);
 * static_assert(!is_array<int>::value);
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
 * @brief Check if type is a function
 * @tparam T Type to check
 *
 * Example:
 * @code
 * void func();
 * static_assert(is_function<decltype(func)>::value);
 * static_assert(!is_function<int>::value);
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
 * @brief Check if type is a member pointer
 * @tparam T Type to check
 *
 * Example:
 * @code
 * struct S { int m; };
 * static_assert(is_member_pointer<int S::*>::value);
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
 * @brief Check if type is a pointer
 * @tparam T Type to check
 *
 * Example:
 * @code
 * static_assert(is_pointer<int*>::value);
 * static_assert(!is_pointer<int>::value);
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
 * @brief Check if type is std::nullptr_t
 * @tparam T Type to check
 *
 * Example:
 * @code
 * static_assert(is_null_pointer<std::nullptr_t>::value);
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

}  // namespace traits
}  // namespace toolbox
