# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

cpp-toolbox is a comprehensive C++ library providing reusable components for accelerated development, with strong focus on point cloud processing, concurrency, and utility functions.

## Build Commands

### Primary Development Flow

**Configure with CMake preset:**
```bash
# Configure (example for Linux with Clang 21 and TBB)
cmake -S . -B build/build-linux-clang-21-tbb --preset build-linux-clang-21-tbb

# Build
cmake --build build/build-linux-clang-21-tbb -j $(nproc)
```

**Run tests and benchmarks:**
```bash
# Run all tests
build/build-linux-clang-21-tbb/bin/cpp-toolbox_test

# Run tests with specific tags
build/build-linux-clang-21-tbb/bin/cpp-toolbox_test "[knn][correspondence]"

# Run benchmarks
build/build-linux-clang-21-tbb/bin/cpp-toolbox_benchmark --benchmark-samples 10

# Run benchmarks with specific tags
build/build-linux-clang-21-tbb/bin/cpp-toolbox_benchmark "[pcl]" --benchmark-samples 10
```

**Development utilities:**
```bash
# Format code before committing
make format

# Run linters
make lint

# Generate documentation
make docs
```

### Alternative Build Systems

**xmake:**
```bash
xmake
xmake run cpp-toolbox_test
xmake run cpp-toolbox_benchmark
```

## Architecture Overview

The codebase follows a modular architecture with clear separation:

1. **Public API** (`src/include/cpp-toolbox/`): All public headers organized by module
2. **Implementation** (`src/impl/cpp-toolbox/`): Source files for non-template code
3. **Header-only implementations** (`*/impl/`): Template implementations in nested impl directories

### Key Modules

- **base/**: Core utilities including thread pools, memory pools, environment helpers
- **concurrent/**: Parallel algorithms with optional TBB support, lock-free data structures
- **pcl/**: Point cloud processing components (KNN, features, descriptors, filters, correspondence)
- **io/**: File I/O including PCD and KITTI format support, data loaders
- **metrics/**: Metric implementations for evaluation
- **logger/**: Thread-safe logging framework

### Design Patterns

- CRTP for base classes (e.g., `BaseKNN`, `BaseDescriptorExtractor`)
- Header-only templates with explicit instantiation where appropriate
- Factory patterns for extensible components (metrics, correspondence generators)
- Clear separation of interface and implementation

## Testing Strategy

- All tests compiled into single executable using Catch2 framework
- All benchmarks compiled into single executable
- Test structure mirrors source structure
- Test data located in `test/data/`
- Use tags to run specific test groups

## Code Style

- **Follow the code style guide in `docs/md/CODE_STYLE.md`**
- Use clang-format before committing (`make format`)
- Follow existing patterns in neighboring files
- Template implementations go in `impl/` subdirectories
- Use provided type aliases (e.g., `PointCloud`, `KDTree`)

### Key Naming Conventions (from CODE_STYLE.md)
- Classes, structs, enums, functions, variables: `lower_case`
- Protected/private members: `m_` prefix + `lower_case`
- Template parameters: `CamelCase`
- Macros: `UPPER_CASE`
- All naming conventions enforced by clang-tidy

## Common Development Tasks

### Adding New PCL Component
1. Create interface in `src/include/cpp-toolbox/pcl/<category>/`
2. If templated, add implementation in corresponding `impl/` subdirectory
3. Add to module's main header (e.g., `features.hpp`, `descriptors.hpp`)
4. Create unit test in `test/pcl/`
5. Add benchmark if performance-critical

### Running Specific Tests
```bash
# Run tests with specific tag
build/build-linux-clang-21-tbb/bin/cpp-toolbox_test "[knn]"

# Run multiple test tags
build/build-linux-clang-21-tbb/bin/cpp-toolbox_test "[knn][correspondence]"

# List all available test cases
build/build-linux-clang-21-tbb/bin/cpp-toolbox_test --list-tests
```

### Debugging Build Issues
- Check `CMakeUserPresets.json` for correct configuration
- Ensure dependencies are fetched: `cmake --preset=dev`
- For TBB support: install system TBB or let CMake fetch it

## Important Notes

- **Use cpp-toolbox utilities wherever possible** - prefer internal implementations over external libraries
- Thread pool singleton is available via `ThreadPoolSingleton::instance()`
- PCL components are designed to work with PCL library types but provide own implementations
- Memory-mapped file support for efficient large file processing
- Extensive use of concepts and SFINAE for type safety in C++17/20