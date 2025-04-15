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


.PHONY: test
test:  ## Run the tests
	@echo "Running the tests..."
	@cd build && ctest --rerun-failed --output-on-failure
	@echo "Done running the tests."


# --- Documentation Settings ---
# TODO: Update these values if necessary, or implement extraction from CMakeLists.txt
PROJECT_NAME      ?= cpp-toolbox
PROJECT_VERSION   ?= 0.0.1
PROJECT_DESC      ?= "A C++ toolbox project"
PROJECT_URL       ?= "https://github.com/nerdneilsfield/cpp-toolbox" # Example URL

BUILD_DIR         := build
DOCS_DIR          := $(BUILD_DIR)/docs
MCSS_DIR          := $(DOCS_DIR)/.ci
MCSS_SCRIPT       := $(MCSS_DIR)/documentation/doxygen.py
MCSS_ZIP_URL      := https://github.com/friendlyanon/m.css/releases/download/release-1/mcss.zip
MCSS_ZIP_MD5      := 00cd2757ebafb9bcba7f5d399b3bec7f
PROJECT_SOURCE_DIR:= $(PWD) # Assuming Makefile is at the root


# --- Documentation Targets ---

# Ensure m.css is downloaded and extracted
$(MCSS_SCRIPT): check-curl-unzip ## Download and extract m.css if needed
	@echo "--- Setting up m.css ---"
	@if [ ! -f "$@" ]; then \
		mkdir -p $(MCSS_DIR); \
		echo "Downloading m.css from $(MCSS_ZIP_URL)..."; \
		curl -# -L $(MCSS_ZIP_URL) -o $(MCSS_DIR)/mcss.zip; \
		echo "Verifying m.css checksum..."; \
		echo "$(MCSS_ZIP_MD5)  $(MCSS_DIR)/mcss.zip" | md5sum -c --status || (echo "ERROR: MD5 checksum failed for mcss.zip"; exit 1); \
		echo "Extracting m.css..."; \
		unzip -q -o $(MCSS_DIR)/mcss.zip -d $(MCSS_DIR); \
		echo "Cleaning up..."; \
		rm $(MCSS_DIR)/mcss.zip; \
	else \
		echo "m.css script found at $(abspath $@). Skipping setup."; \
	fi
	@echo "------------------------"

# Generate Doxyfile from template
$(DOCS_DIR)/Doxyfile: docs/Doxyfile.in Makefile ## Generate Doxyfile configuration
	@echo "--- Generating Doxyfile ---"
	@mkdir -p $(DOCS_DIR)
	@sed \
		-e 's|@PROJECT_NAME@|$(PROJECT_NAME)|g' \
		-e 's|@PROJECT_VERSION@|$(PROJECT_VERSION)|g' \
		-e 's|@PROJECT_SOURCE_DIR@|$(PROJECT_SOURCE_DIR)|g' \
		-e 's|@DOXYGEN_OUTPUT_DIRECTORY@|$(DOCS_DIR)|g' \
		$< > $@
	@echo "Generated $(abspath $@)"
	@echo "--------------------------"

# Generate conf.py from template
# NOTE: This assumes placeholders in conf.py.in. Adjust if needed.
$(DOCS_DIR)/conf.py: docs/conf.py.in Makefile ## Generate m.css python configuration
	@echo "--- Generating conf.py ---"
	@mkdir -p $(DOCS_DIR)
	@sed \
		-e 's|@PROJECT_NAME@|$(PROJECT_NAME)|g' \
		-e 's|@PROJECT_VERSION@|$(PROJECT_VERSION)|g' \
		-e 's|@PROJECT_DESCRIPTION@|$(PROJECT_DESC)|g' \
		-e 's|@PROJECT_HOMEPAGE_URL@|$(PROJECT_URL)|g' \
		-e 's|@DOXYGEN_OUTPUT_DIRECTORY@|$(DOCS_DIR)|g' \
		$< > $@
	@echo "Generated $(abspath $@)"
	@echo "-------------------------"

.PHONY: docs-deps
docs-deps: $(MCSS_SCRIPT) $(DOCS_DIR)/Doxyfile $(DOCS_DIR)/conf.py ## Prepare documentation dependencies (download m.css, generate configs)

.PHONY: docs
docs: docs-deps ## Generate the documentation using Doxygen and m.css
	@echo "--- Generating Documentation ---"
	@echo "Cleaning previous output..."
	@rm -rf $(DOCS_DIR)/html $(DOCS_DIR)/xml
	@echo "Running m.css generator (Python script)..."
	@echo "MCSS_SCRIPT = $(abspath $(MCSS_SCRIPT))"
	@cd $(DOCS_DIR) && python3 $(abspath $(MCSS_SCRIPT)) conf.py
	@echo "Documentation generated in $(abspath $(DOCS_DIR))/html"
	@echo "------------------------------"


.PHONY: check-curl-unzip
check-curl-unzip: ## Check for curl and unzip commands
	@command -v curl >/dev/null 2>&1 || { echo >&2 "ERROR: 'curl' command not found. Please install curl."; exit 1; }
	@command -v unzip >/dev/null 2>&1 || { echo >&2 "ERROR: 'unzip' command not found. Please install unzip."; exit 1; }
	@command -v python3 >/dev/null 2>&1 || { echo >&2 "ERROR: 'python3' command not found. Please install Python 3."; exit 1; }
	@command -v md5sum >/dev/null 2>&1 || { echo >&2 "ERROR: 'md5sum' command not found. Please install coreutils or equivalent."; exit 1; }


