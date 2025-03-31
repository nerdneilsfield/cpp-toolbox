# --- OS Detection ---
# Get the kernel name (e.g., Linux, Darwin) and convert to lowercase
UNAME_S := $(shell uname -s | tr '[:upper:]' '[:lower:]')

# Alternative using GNU Make's built-in variable (might be simpler if GNU Make is guaranteed)
# Check if .OSTYPE is defined and non-empty
ifeq ($(strip $(.OSTYPE)),)
    # Fallback if .OSTYPE is not available
    OS_FAMILY := unknown
    ifeq ($(findstring linux,$(UNAME_S)),linux)
        OS_FAMILY := linux
    else ifeq ($(findstring darwin,$(UNAME_S)),darwin)
        OS_FAMILY := macos
    # Check for common Windows environments where uname might exist
    else ifneq (,$(findstring cygwin,$(UNAME_S))$(findstring mingw,$(UNAME_S))$(findstring msys,$(UNAME_S)))
        OS_FAMILY := windows
    # Check standard Windows environment variable as another fallback
    else ifeq ($(OS),Windows_NT)
        OS_FAMILY := windows
    endif
else
    # Use .OSTYPE if available
    OS_FAMILY := unknown
    ifeq ($(findstring linux,$(.OSTYPE)),linux)
        OS_FAMILY := linux
    else ifeq ($(findstring darwin,$(.OSTYPE)),darwin)
        OS_FAMILY := macos
    else ifeq ($(findstring msys,$(.OSTYPE)),msys)
        OS_FAMILY := windows
    else ifeq ($(findstring cygwin,$(.OSTYPE)),cygwin)
        OS_FAMILY := windows
    # Add other .OSTYPE checks if necessary
    endif
endif

.PHONY: help
help:  ## Show this help
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)


.PHONY: check-os
check-os:  ## Check the current OS
	@echo "--- Checking Operating System ---"
	@echo "UNAME_S = $(UNAME_S)"
	@echo ".OSTYPE = $(.OSTYPE)"
	@echo "OS Env Var = $(OS)"
	@echo "Detected OS Family: $(OS_FAMILY)"
	@echo "-------------------------------"

.PHONY: format
format:  ## Format the code
	@echo "Formatting the code..."
	@find src -type f \( -name "*.cpp" -o -name "*.hpp" \) -exec clang-format -i -style=file {} \;
	@find test -type f \( -name "*.cpp" -o -name "*.hpp" \) -exec clang-format -i -style=file {} \;
	@echo "Done formatting the code."

.PHONY: lint
lint:  ## Lint the code
	@echo "Linting the code..."
	@find src -type f \( -name "*.cpp" -o -name "*.hpp" \) -exec clang-tidy {} \;
	@find test -type f \( -name "*.cpp" -o -name "*.hpp" \) -exec clang-tidy {} \;
	@echo "Done linting the code."


.PHONY: dev-linux
dev-linux:  ## Setup the development environment for Linux
	@echo "Setting up the development environment for Linux..."
	@cmake -B build -S . --preset dev-linux
	@echo "Done setting up the development environment for Linux."

.PHONY: dev-macos
dev-macos:  ## Setup the development environment for macOS
	@echo "Setting up the development environment for macOS..."
	@cmake -B build -S . --preset dev-macos
	@echo "Done setting up the development environment for macOS."

.PHONY: dev-windows
dev-windows:  ## Setup the development environment for Windows
	@echo "Setting up the development environment for Windows..."
	@cmake -B build -S . --preset dev-windows
	@echo "Done setting up the development environment for Windows."

.PHONY: dev
dev: check-os  ## Setup the development environment according to the current OS
ifeq ($(OS_FAMILY),linux)
	@echo ">>> Detected Linux. Running dev-linux target..."
	$(MAKE) dev-linux
else ifeq ($(OS_FAMILY),macos)
	@echo ">>> Detected macOS. Running dev-macos target..."
	$(MAKE) dev-macos
else ifeq ($(OS_FAMILY),windows)
	@echo ">>> Detected Windows. Running dev-windows target..."
	$(MAKE) dev-windows
else
	@echo "ERROR: Unsupported OS detected ('$(UNAME_S)' / '.OSTYPE=$(.OSTYPE)' / 'OS=$(OS)'). Cannot determine development target."
	@exit 1
endif


.PHONY: build
build:  ## Build the project
	@echo "Building the project..."
	@cmake --build build -j $(shell nproc)
	@echo "Done building the project."


.PHONY: docs
docs:  ## Generate the documentation
	@echo "Generating the documentation..."
	@echo "Current space is $(PWD)"
	@cmake "-DPROJECT_SOURCE_DIR=$(PWD)" "-DPROJECT_BINARY_DIR=$(PWD)/build" -P cmake/docs-ci.cmake
	@echo "Done generating the documentation."


