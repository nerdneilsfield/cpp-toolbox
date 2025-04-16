
# C++ 命名风格规范 / C++ Identifier Naming Style Guide

---

## 总述 / Overview

本项目采用统一的 C++ 命名风格，旨在提升代码一致性、可读性和可维护性。所有命名风格均由 clang-tidy 的 `readability-identifier-naming` 规则自动检查和约束。  
This project uses a unified C++ identifier naming style to improve code consistency, readability, and maintainability. All naming conventions are enforced by clang-tidy's `readability-identifier-naming` rules.

---

## 1. 类型命名 / Type Naming

| 类型 / Type      | 风格 / Style    | 示例 / Example      |
|------------------|-----------------|---------------------|
| 类 / Class       | lower_case      | `class my_class {};` |
| 结构体 / Struct  | lower_case      | `struct my_struct {};` |
| 联合体 / Union   | lower_case      | `union my_union { int a; };` |
| 枚举 / Enum      | lower_case      | `enum my_enum { ... };` |
| 类型别名 / Type Alias (using/typedef) | lower_case | `using my_alias = int;`<br>`typedef int my_typedef;` |

---

## 2. 枚举常量 / Enum Constants

| 类型 / Type           | 风格 / Style | 示例 / Example                 |
|-----------------------|--------------|--------------------------------|
| 枚举常量 / Enum Value | lower_case   | `enum my_enum { value_one };`  |
| Scoped 枚举常量 / Scoped Enum Value | lower_case | `enum class my_enum { value_one };` |

---

## 3. 成员变量与成员函数 / Members

| 类型 / Type                | 风格 / Style      | 示例 / Example                  |
|----------------------------|-------------------|---------------------------------|
| 公有成员变量 / Public Member      | lower_case      | `int my_member;`                |
| 保护成员变量 / Protected Member   | m_+ lower_case | `int m_my_member;`              |
| 私有成员变量 / Private Member     | m_+ lower_case | `int m_my_member;`              |
| 成员函数 / Member Function        | lower_case      | `void my_method();`             |
| 虚函数 / Virtual Method           | lower_case      | `virtual void my_virtual();`    |

---

## 4. 常量 / Constants

| 类型 / Type               | 风格 / Style | 示例 / Example                         |
|---------------------------|--------------|----------------------------------------|
| 全局常量 / Global Constant| lower_case   | `const int my_global_constant = 1;`    |
| 类常量 / Class Constant   | lower_case   | `static const int my_class_constant = 2;` |
| 成员常量 / Member Constant| lower_case   | `const int my_member_constant = 3;`    |
| 静态常量 / Static Constant| lower_case   | `static const int my_static_constant = 4;` |
| 局部常量 / Local Constant | lower_case   | `const int my_local_constant = 5;`     |

---

## 5. 变量 / Variables

| 类型 / Type                 | 风格 / Style | 示例 / Example                      |
|-----------------------------|--------------|-------------------------------------|
| 全局变量 / Global Variable  | lower_case   | `int my_global_variable = 0;`       |
| 局部变量 / Local Variable   | lower_case   | `int my_local_variable = 0;`        |
| 静态变量 / Static Variable  | lower_case   | `static int my_static_variable = 0;`|
| 普通变量 / Variable         | lower_case   | `int my_variable = 0;`              |

---

## 6. 函数与参数 / Functions & Parameters

| 类型 / Type                  | 风格 / Style | 示例 / Example                          |
|------------------------------|--------------|-----------------------------------------|
| 函数 / Function              | lower_case   | `void my_function();`                   |
| 全局函数 / Global Function   | lower_case   | `void my_global_function();`            |
| constexpr 函数 / Constexpr Function | lower_case | `constexpr int my_constexpr_function() { return 0; }` |
| 参数 / Parameter             | lower_case   | `void foo(int my_param)`                |
| 指针参数 / Pointer Parameter | lower_case   | `void foo(int* my_ptr_param)`           |
| 常量参数 / Constant Parameter| lower_case   | `void foo(const int my_const_param)`    |

---

## 7. 模板参数 / Template Parameters

| 类型 / Type                  | 风格 / Style | 示例 / Example                         |
|------------------------------|--------------|----------------------------------------|
| 模板类型参数 / Template Type Parameter | CamelCase | `template<typename MyType>`            |
| 模板值参数 / Template Value Parameter  | CamelCase | `template<int MyValue>`                |
| 模板模板参数 / Template Template Parameter | CamelCase | `template<template<typename> class MyTemplate>` |

---

## 8. 命名空间 / Namespace

| 类型 / Type                  | 风格 / Style | 示例 / Example              |
|------------------------------|--------------|-----------------------------|
| 命名空间 / Namespace         | lower_case   | `namespace my_namespace {}` |
| 内联命名空间 / Inline Namespace | lower_case | `inline namespace my_inline_namespace {}` |

---

## 9. 宏定义 / Macros

| 类型 / Type                  | 风格 / Style | 示例 / Example         |
|------------------------------|--------------|------------------------|
| 宏定义 / Macro Definition    | UPPER_CASE   | `#define MY_MACRO 100` |

---

## 10. 代码风格示例 / Style Example

```cpp
// 类与结构体 / Class and Struct
class my_class {
public:
    int my_public_member;                // 公有成员变量 / Public member variable
    void my_public_method();             // 公有成员函数 / Public member function

protected:
    int m_my_protected_member;           // 保护成员变量 / Protected member variable
    void m_my_protected_method();        // 保护成员函数 / Protected member function

private:
    int m_my_private_member;             // 私有成员变量 / Private member variable
    void m_my_private_method();          // 私有成员函数 / Private member function
};

struct my_struct {
    int my_member;                       // 结构体成员变量 / Struct member variable
};

// 枚举 / Enum
enum my_enum {
    value_one,                           // 枚举常量 / Enum constant
    value_two
};

enum class my_enum_class {
    value_one,                           // Scoped 枚举常量 / Scoped enum constant
    value_two
};

// 模板 / Template
template<typename MyType, int MyValue>
class my_template_class {};              // 模板类 / Template class

// 函数与参数 / Function and Parameters
void my_function(int my_param,           // 普通参数 / Normal parameter
                 float* my_ptr_param,    // 指针参数 / Pointer parameter
                 const int my_const_param) // 常量参数 / Constant parameter
{}

// 宏 / Macro
#define MY_MACRO 100                     // 宏定义 / Macro definition

// 命名空间 / Namespace
namespace my_namespace {
    // 命名空间内容 / Namespace content
}

```

---

## 11. 备注 / Notes

- 所有命名风格均由 clang-tidy 自动检查，提交代码前请确保无风格警告。  
  *All naming styles are automatically checked by clang-tidy. Please ensure there are no style warnings before submitting code.*

- 模板参数统一采用 CamelCase，宏定义统一采用 UPPER_CASE，其余均为 lower_case。  
  *Template parameters use CamelCase, macro definitions use UPPER_CASE, and all other identifiers use lower_case.*

- 如需特殊风格，请在项目内另行约定并补充说明。  
  *If special naming styles are required, please specify and document them separately within the project.*

---

**如有疑问或建议，请联系维护者。**  
If you have any questions or suggestions, please contact the maintainer.
