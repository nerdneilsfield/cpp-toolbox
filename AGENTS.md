# CPP-Toolbox Agent Guide

## Project Overview

cpp-toolbox is a collection of C++ utilities that provides various commonly used components and algorithm implementations. This guide helps Codex agents understand the project structure and operate correctly within the codebase.

## Development Environment Setup

### Basic Requirements

- C++17 or higher
- CMake 3.14 or higher
- Supported compilers:
  - GCC 9+
  - Clang 10+
  - MSVC 19.2+

### Dev Environment Tips (optional)

- use `cppcheck` to check code quality
- use `clang-tidy` to check code style
- use `clang-format` to format code

```bash
sudo apt-get install -y cppcheck clang-tidy clang-format
```

### Setup Steps

```bash
# After cloning the repository
cd /workspace
cmake -S . -B build --preset build-linux
cmake --build build -j $(nproc)

# Run tests
cd build
ctest --output-on-failure --no-tests=error
```

### Dependency Management

The project uses CMake for dependency management. Main dependencies include:

- Catch2 (for testing) [1](https://github.com/nerdneilsfield/cpp-toolbox/blob/master/deps/Catch2/tests/SelfTest/Baselines/console.sw.approved.txt)
- Other dependencies are automatically fetched during the CMake build process

## Repository Structure

```
/
├── src/include/            # Public headers
├── src/impl/                # Source implementations
├── test/              # Test cases
├── example/           # Example code
├── deps/               # Third-party dependencies
├── docs/               # Documentation
└── cmake/              # CMake modules and configurations
```

## Contribution Guidelines

Please read the CONTRIBUTING.md file before contributing.

### Code Style

- Use C++17 standard features
- Follow the project's formatting guidelines which is based on clang-format `docs/md/CODE_STYLE.md`
- All public APIs must have complete documentation comments, all export function should use `CPP_TOOLBOX_EXPORT` to export

```
#include <cpp-toolbox/cpp-toolbox_export.hpp>

class CPP_TOOLBOX_EXPORT MyClass {
public:
    /**
     * This is a function that does something.
     *
     * @param param The input parameter.
     * @return The result.
     */
    int doSomething(int param);
};

CPP_TOOLBOX_EXPORT int doSomething(int param) {
    // Implementation
}

```

- All comments should follow the [Doxygen](https://www.doxygen.nl/manual/docblocks.html) format, and have bot english and chinese version. And use `@code` to provide code examples.

### Commit Conventions

```
[fix/docs/chore/feat]: Brief description of the change

Detailed description (if necessary)
```

Example:

```
[feat]: Add quicksort implementation

- Implement generic quicksort template
- Add unit tests
- Update documentation
```

- `feat` is for new features
- `fix` is for bug fixes
- `docs` is for documentation changes
- `chore` is for changes that do not affect the codebase (e.g. CI/CD pipeline changes)

## Testing Guidelines

The project uses Catch2 as its testing framework [2](https://github.com/nerdneilsfield/cpp-toolbox/blob/master/deps/Catch2/tests/SelfTest/Baselines/console.sw.multi.approved.txt).

### Running Tests

```bash
cd build
ctest --output-on-failure --no-tests=error
```

### Adding New Tests

1. Create a new test file in the `tests/` directory
2. Write test cases using the Catch2 framework
3. Register the test in the appropriate CMakeLists.txt

Example test:

```cpp
#include <catch2/catch_test_macros.hpp>
#include "your_header.h"

TEST_CASE("Test description", "[module]") {
    // Test code
    REQUIRE(expected_value == actual_value);
}
```

## CI/CD Pipeline

- After code push, CI will run automatically
- CI checks include:
  - Code formatting check
  - Compilation tests
  - Unit tests
  - Static code analysis

## Troubleshooting

### Build Issues

If you encounter build problems:

1. Check if your CMake version meets requirements
2. Ensure all dependencies are correctly installed
3. Clear the build directory and rebuild

### Test Failures

If tests fail:

1. Check test logs for specific failure reasons
2. Verify your environment configuration
3. Look at the TAP output format in test files for detailed information [3](https://github.com/nerdneilsfield/cpp-toolbox/blob/master/deps/Catch2/tests/SelfTest/Baselines/tap.sw.multi.approved.txt)

## PR Submission Guidelines

Follow this format when submitting PRs:

```
[fix/docs/chore/feat]: Change title

- Change point 1
- Change point 2
- Testing method
- Documentation updates
```

All PRs must pass CI checks and receive at least one maintainer review before merging.
