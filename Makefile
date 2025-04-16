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


# --- Project Settings (Adjust if needed) ---
PROJECT_NAME      ?= cpp-toolbox
PROJECT_VERSION   ?= 0.0.1
PROJECT_DESC      ?= "A C++ toolbox project"
PROJECT_URL       ?= "https://github.com/nerdneilsfield/cpp-toolbox"
PROJECT_SOURCE_DIR:= $(PWD)

# --- Build & m.css Documentation Directories ---
# --- Build & Doxygen Documentation Directories ---
BUILD_DIR         := $(PWD)/build
DOCS_DIR          := $(BUILD_DIR)/docs# Output dir for Doxygen
# Remove MCSS_DIR, MCSS_SCRIPT, MCSS_ZIP_URL, MCSS_ZIP_MD5

# --- Doxygen Awesome CSS Settings ---
DOXYGEN_AWESOME_DIR := $(DOCS_DIR)/doxygen-awesome-css
DOXYGEN_AWESOME_CSS := $(DOXYGEN_AWESOME_DIR)/doxygen-awesome.css
DOXYGEN_AWESOME_JS_DARK_MODE := $(DOXYGEN_AWESOME_DIR)/doxygen-awesome-darkmode-toggle.js
# Add other JS files if needed later e.g.
# DOXYGEN_AWESOME_JS_FRAGMENT_COPY := $(DOXYGEN_AWESOME_DIR)/doxygen-awesome-fragment-copy-button.js
# DOXYGEN_AWESOME_JS_TABS := $(DOXYGEN_AWESOME_DIR)/doxygen-awesome-tabs.js
# DOXYGEN_AWESOME_JS_INTERACTIVE_TOC := $(DOXYGEN_AWESOME_DIR)/doxygen-awesome-interactive-toc.js

# List only the filenames for HTML_EXTRA_FILES
DOXYGEN_AWESOME_JS_FILES := \
    $(notdir $(DOXYGEN_AWESOME_JS_DARK_MODE)) # Add others like $(notdir $(DOXYGEN_AWESOME_JS_FRAGMENT_COPY))


# Generated config files
DOXYFILE_CONF       := $(DOCS_DIR)/Doxyfile# Config for Doxygen
# Remove CONF_PY_CONF reference

# --- Helper Targets ---

.PHONY: check-curl-unzip
check-curl-unzip: ## Check for curl, unzip, python3, md5sum
	@command -v curl >/dev/null 2>&1 || { echo >&2 "ERROR: 'curl' command not found. Please install curl."; exit 1; }
	@command -v unzip >/dev/null 2>&1 || { echo >&2 "ERROR: 'unzip' command not found. Please install unzip."; exit 1; }
	@command -v python3 >/dev/null 2>&1 || { echo >&2 "ERROR: 'python3' command not found. Please install Python 3."; exit 1; }
	@command -v md5sum >/dev/null 2>&1 || { echo >&2 "ERROR: 'md5sum' command not found. Please install coreutils or equivalent."; exit 1; }
	@echo "Documentation dependencies checked."

# Ensure Doxygen Awesome CSS is downloaded and extracted
$(DOXYGEN_AWESOME_CSS): check-curl-unzip ## Download and extract Doxygen Awesome CSS if needed
	@echo "--- Setting up Doxygen Awesome CSS ---"
	@if [ ! -f "$@" ]; then \
		echo ">>> Downloading and extracting Doxygen Awesome CSS..."; \
		mkdir -p $(DOXYGEN_AWESOME_DIR); \
		curl -# -L https://github.com/jothepro/doxygen-awesome-css/archive/refs/tags/v2.3.4.zip -o $(DOXYGEN_AWESOME_DIR)/doxygen-awesome-css.zip; \
		if [ $$? -ne 0 ]; then echo "ERROR: Download failed."; exit 1; fi; \
		unzip -q -o $(DOXYGEN_AWESOME_DIR)/doxygen-awesome-css.zip -d $(DOXYGEN_AWESOME_DIR); \
		if [ $$? -ne 0 ]; then echo "ERROR: Unzip failed."; exit 1; fi; \
		rm $(DOXYGEN_AWESOME_DIR)/doxygen-awesome-css.zip; \
		mv $(DOXYGEN_AWESOME_DIR)/doxygen-awesome-css-2.3.4/* $(DOXYGEN_AWESOME_DIR); \
		rm -rf $(DOXYGEN_AWESOME_DIR)/doxygen-awesome-css-2.3.4; \
	else \
		echo "Doxygen Awesome CSS found at $(abspath $@). Skipping setup."; \
	fi
	@echo "------------------------------------"


# Generate Doxyfile (Echo/Grep Approach, Overwriting .in settings)
# Generate Doxyfile (Using sed to process Doxyfile.in)
$(DOXYFILE_CONF): docs/Doxyfile.in Makefile $(DOXYGEN_AWESOME_CSS) # Add CSS target as dependency
	@echo "--- Generating Doxyfile for Doxygen Awesome CSS (Echo/Grep Approach) ---"
	@echo "--- Generating Doxyfile for Doxygen Awesome CSS (sed Approach) ---"
	@mkdir -p $(DOCS_DIR)
	@# Use sed to replace placeholders and overwrite specific settings
	@sed \
		-e 's|@PROJECT_NAME@|$(PROJECT_NAME)|g' \
		-e 's|@PROJECT_VERSION@|$(PROJECT_VERSION)|g' \
		-e 's|@DOXYGEN_OUTPUT_DIRECTORY@|$(DOCS_DIR)|g' \
		-e 's|@PROJECT_SOURCE_DIR@|$(PROJECT_SOURCE_DIR)|g' \
		docs/Doxyfile.in > $(DOXYFILE_CONF)
	@echo "Generated $(abspath $(DOXYFILE_CONF))"
	@echo "--------------------------------------------------------------------"


.PHONY: docs-deps
# Remove $(CONF_PY_CONF) dependency, keep $(DOXYFILE_CONF), remove $(MCSS_SCRIPT), add $(DOXYGEN_AWESOME_CSS)
docs-deps: $(DOXYFILE_CONF) $(DOXYGEN_AWESOME_CSS) ## Prepare Doxygen Awesome CSS documentation dependencies

.PHONY: docs
docs: check-curl-unzip docs-deps ## Generate the documentation using Doxygen and Doxygen Awesome CSS
	@echo "--- Generating Doxygen Awesome CSS Documentation ---"
	@echo "Cleaning previous output..."
	@rm -rf $(DOCS_DIR)/html $(DOCS_DIR)/xml
	@echo "Running Doxygen generator..."
	@doxygen $(DOXYFILE_CONF)
	@# Check Doxygen exit code
	@if [ $$? -ne 0 ]; then echo "ERROR: Doxygen generation failed."; exit 1; fi;
	@echo "Documentation generated in $(abspath $(DOCS_DIR))/html"
	@echo "---------------------------------------------------"


# --- Cleanup ---

.PHONY: clean-docs
clean-docs: ## Clean generated Doxygen documentation files and directories
	@echo "Cleaning Doxygen documentation artifacts..."
	@# This also removes the downloaded doxygen-awesome-css dir inside DOCS_DIR
	@rm -rf $(DOCS_DIR)
	@echo "Done cleaning documentation."

.PHONY: serve-docs
serve-docs: ## Serve the documentation
	@echo "Serving the documentation..."
	@cd $(DOCS_DIR)/html && python3 -m http.server 8000
	@echo "Done serving the documentation."

.PHONY: clean
clean: clean-docs ## Clean build directory and documentation artifacts
	@echo "Cleaning build directory..."
	@# Keep build dir clean separate, as Doxygen output is inside build dir
	@rm -rf $(BUILD_DIR)/*
	@# Attempt to remove build dir itself if empty (might fail if other things are in it)
	@-rmdir $(BUILD_DIR) 2>/dev/null || true
	@echo "Done cleaning build directory."


