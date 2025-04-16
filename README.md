# cpp-toolbox

[![Stable Docs](https://img.shields.io/badge/docs-stable-blue.svg)](https://cpp-toolbox.dengqi.site/) [![Latest Docs](https://img.shields.io/badge/docs-latest-orange.svg)](https://cpp-toolbox.dengqi.site/)

---

cpp-toolbox is a collection of reusable C++ components and utility functions designed to streamline common programming tasks. It covers areas such as concurrency, containers, file I/O, logging, and more, aiming to provide robust and efficient solutions for C++ developers.

`cpp-toolbox` 是一个可重用的 C++ 组件和实用函数集合，旨在简化常见的编程任务。它涵盖了并发、容器、文件 I/O、日志记录等领域，致力于为 C++ 开发者提供健壮且高效的解决方案。

---

## Features

* **Base Utilities**: Core functionalities like environment variable access.
* **Concurrency**: Tools for concurrent programming, including a thread pool based on a lock-free queue.
* **Containers**: Custom containers like a lock-free queue and helpers for standard containers (`std::string`, `std::string_view`).
* **File I/O**: Utilities for file operations.
* **Functional**: Helpers for functional programming paradigms.
* **Input/Output**: Components for I/O tasks.
* **Logging**: Flexible logging framework.
* **Numeric**: Utilities for numerical operations.
* **Types**: Common type definitions and traits (`type_traits.hpp`).
* **Utilities**: Various helper functions and classes like timers (`timer.hpp`).
* **Macros**: Useful preprocessor macros (`macro.hpp`).

### 特性 (中文)

* **基础工具**: 核心功能，如环境变量访问。
* **并发**: 并发编程工具，包括基于无锁队列的线程池。
* **容器**: 自定义容器（如无锁队列）和标准容器（`std::string`, `std::string_view`）的辅助工具。
* **文件 I/O**: 文件操作实用程序。
* **函数式**: 函数式编程范式的辅助工具。
* **输入/输出**: I/O 任务组件。
* **日志**: 灵活的日志框架。
* **数值**: 数值运算实用程序。
* **类型**: 通用类型定义和特征 (`type_traits.hpp`)。
* **实用程序**: 各种辅助函数和类，如计时器 (`timer.hpp`)。
* **宏**: 实用的预处理器宏 (`macro.hpp`)。

---

## Getting Started

Please refer to the [BUILDING.md](BUILDING.md) document for instructions on how to build and install the library.

### 开始使用 (中文)

请参阅 [BUILDING.md](BUILDING.md) 文档获取关于如何构建和安装本库的说明。

---

## Modules

Here's a breakdown of the available modules (located under `src/include/cpp-toolbox/`):

* [`base/`](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/base): Base classes and functions (e.g., `env.hpp`).
* [`concurrent/`](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/concurrent): Concurrency utilities (e.g., `thread_pool.hpp`).
* [`container/`](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/container): Container classes and helpers (e.g., `lock_free_queue.hpp`, `string.hpp`).
* [`file/`](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/file): File system operations.
* [`functional/`](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/functional): Functional programming tools.
* [`io/`](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/io): Input/Output utilities.
* [`logger/`](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/logger): Logging framework.
* [`numeric/`](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/numeric): Numerical utilities.
* [`types/`](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/types): Type definitions and aliases.
* [`utils/`](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/utils): General utilities (e.g., `timer.hpp`).

Top-level headers like `type_traits.hpp` and `macro.hpp` provide cross-cutting functionalities.

### 模块 (中文)

以下是可用模块的明细（位于 `src/include/cpp-toolbox/` 下）：

* [`base/`](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/base): 基础类和函数 (例如 `env.hpp`).
* [`concurrent/`](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/concurrent): 并发工具 (例如 `thread_pool.hpp`).
* [`container/`](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/container): 容器类和辅助工具 (例如 `lock_free_queue.hpp`, `string.hpp`).
* [`file/`](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/file): 文件系统操作。
* [`functional/`](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/functional): 函数式编程工具。
* [`io/`](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/io): 输入/输出工具。
* [`logger/`](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/logger): 日志框架。
* [`numeric/`](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/numeric): 数值计算工具。
* [`types/`](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/types): 类型定义和别名。
* [`utils/`](https://github.com/nerdneilsfield/cpp-toolbox/tree/master/src/include/cpp-toolbox/utils): 通用工具 (例如 `timer.hpp`).

顶层头文件如 `type_traits.hpp` 和 `macro.hpp` 提供了跨模块的功能。

---

## Contributing

Contributions are welcome! Please read [CONTRIBUTING.md](CONTRIBUTING.md) and the [Code of Conduct](CODE_OF_CONDUCT.md) before submitting pull requests. For development details, see [HACKING.md](HACKING.md).

### 贡献 (中文)

欢迎贡献！在提交拉取请求之前，请阅读 [CONTRIBUTING.md](CONTRIBUTING.md) 和 [行为准则 (Code of Conduct)](CODE_OF_CONDUCT.md)。有关开发细节，请参阅 [HACKING.md](HACKING.md)。

---

## License

Please refer to the license information within the source files or contact the maintainers. (A standard `LICENSE` file was not found in the root directory).

### 许可证 (中文)

请参阅源文件中的许可证信息或联系维护者。（在根目录中未找到标准的 `LICENSE` 文件）。
