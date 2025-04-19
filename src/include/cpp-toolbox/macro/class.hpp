#pragma once

/**
 * @brief 禁用类的拷贝操作 / Disable copy operations for a class
 * @param ClassType 类名 / Class name
 *
 * @code{.cpp}
 * class MyClass {
 *   CPP_TOOLBOX_DISABLE_COPY(MyClass)
 * public:
 *   MyClass() = default;
 * };
 * @endcode
 */
#define CPP_TOOLBOX_DISABLE_COPY(ClassType) \
  ClassType(const ClassType&) = delete; \
  ClassType& operator=(const ClassType&) = delete;

/**
 * @brief 禁用类的移动操作 / Disable move operations for a class
 * @param ClassType 类名 / Class name
 *
 * @code{.cpp}
 * class MyClass {
 *   CPP_TOOLBOX_DISABLE_MOVE(MyClass)
 * public:
 *   MyClass() = default;
 * };
 * @endcode
 */
#define CPP_TOOLBOX_DISABLE_MOVE(ClassType) \
  ClassType(ClassType&&) = delete; \
  ClassType& operator=(ClassType&&) = delete;

/**
 * @brief 禁用类的拷贝和移动操作 / Disable both copy and move operations
 * @param ClassType 类名 / Class name
 *
 * @code{.cpp}
 * class MyClass {
 *   CPP_TOOLBOX_DISABLE_COPY_AND_MOVE(MyClass)
 * public:
 *   MyClass() = default;
 * };
 * @endcode
 */
#define CPP_TOOLBOX_DISABLE_COPY_AND_MOVE(ClassType) \
  CPP_TOOLBOX_DISABLE_COPY(ClassType) \
  CPP_TOOLBOX_DISABLE_MOVE(ClassType)

/**
 * @brief 默认构造函数宏 / Default constructor macro
 * @param ClassType 类名 / Class name
 *
 * @code{.cpp}
 * class MyClass {
 * public:
 *   CPP_TOOLBOX_DEFAULT_CONSTRUCTOR(MyClass)
 * };
 * @endcode
 */
#define CPP_TOOLBOX_DEFAULT_CONSTRUCTOR(ClassType) ClassType() = default;

/**
 * @brief 删除拷贝和移动构造函数 / Delete copy and move constructors
 * @param ClassType 类名 / Class name
 *
 * @code{.cpp}
 * class MyClass {
 * public:
 *   CPP_TOOLBOX_DELETE_CONSTRUCTOR(MyClass)
 * };
 * @endcode
 */
#define CPP_TOOLBOX_DELETE_CONSTRUCTOR(ClassType) \
  ClassType(const ClassType&) = delete; \
  ClassType(ClassType&&) = delete;

/**
 * @brief 单例模式宏 / Singleton pattern macro
 * @param ClassType 类名 / Class name
 *
 * @code{.cpp}
 * class MySingleton {
 *   CPP_TOOLBOX_SINGLETON(MySingleton)
 * public:
 *   void doSomething() {}
 * };
 *
 * // 使用单例 / Using singleton
 * MySingleton::instance().doSomething();
 * @endcode
 */
#define CPP_TOOLBOX_SINGLETON(ClassType) \
public: \
  static ClassType& instance() \
  { \
    static ClassType instance; \
    return instance; \
  } \
\
private: \
  ClassType() = default; \
  CPP_TOOLBOX_DISABLE_COPY(ClassType) \
  CPP_TOOLBOX_DISABLE_MOVE(ClassType)