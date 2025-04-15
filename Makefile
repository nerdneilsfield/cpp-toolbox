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
PROJECT_URL       ?= "https://github.com/nerdneilsfield/cpp-toolbox"
PROJECT_SOURCE_DIR:= $(PWD)# Revert to project root

# --- Build & Documentation Directories ---
BUILD_DIR         := build
DOCS_BUILD_DIR    := $(BUILD_DIR)/docs_gen
DOXYGEN_XML_DIR   := $(BUILD_DIR)/doxygen-xml
RST_DOCS_DIR      := docs/rst/
DOXIDE_SITE_DIR   := $(BUILD_DIR)/docs-site
MKDOCS_MARKDOWN_DIR := $(DOXIDE_SITE_DIR)/docs
MKDOCS_OUTPUT_DIR := $(DOXIDE_SITE_DIR)/site

# Generated config files
DOXYFILE_CONF       := $(DOCS_BUILD_DIR)/Doxyfile
# DOXYREST_CONF       := $(DOCS_BUILD_DIR)/doxyrest-config.lua # Removed, using static file
DOXIDE_CONF         := doxide.yaml

# --- Helper Targets ---

.PHONY: check-doc-tools
check-doc-tools: ## Check for Doxygen, Doxyrest, Doxide, MkDocs
	@command -v doxygen >/dev/null 2>&1 || { echo >&2 "ERROR: 'doxygen' not found. Please install it."; exit 1; }
	@command -v doxyrest >/dev/null 2>&1 || { echo >&2 "ERROR: 'doxyrest' not found. Please install it (e.g., pip install doxyrest)."; exit 1; }
	@command -v doxide >/dev/null 2>&1 || { echo >&2 "ERROR: 'doxide' not found. Please install it."; exit 1; }
	@command -v pip3 >/dev/null 2>&1 || { echo >&2 "ERROR: 'pip3' not found. Please install Python 3 pip."; exit 1; }
	@pip3 show mkdocs >/dev/null 2>&1 || { echo >&2 "WARNING: 'mkdocs' not found or not installed via pip. Trying to run anyway, but consider 'pip3 install mkdocs mkdocs-material'."; }
	@echo "Documentation tools checked."

# Debug: Print variable value before the rule
$(info --- Debug: DOXYFILE_CONF is [$(DOXYFILE_CONF)] ---)

# Generate Doxyfile for XML output (Echo/Grep Approach)
$(DOXYFILE_CONF): docs/Doxyfile.in Makefile
	@echo "--- Generating Doxyfile for XML output (Echo/Grep Approach) ---"
	@mkdir -p $(DOCS_BUILD_DIR)
	@# Echo the customized/required lines first
	@echo "PROJECT_NAME = $(PROJECT_NAME)" > $(DOXYFILE_CONF)
	@echo "PROJECT_NUMBER = $(PROJECT_VERSION)" >> $(DOXYFILE_CONF)
	@echo "OUTPUT_DIRECTORY = $(DOXYGEN_XML_DIR)" >> $(DOXYFILE_CONF)
	@# Ensure paths are quoted correctly for Doxygen INPUT
	@echo "INPUT = \"$(PROJECT_SOURCE_DIR)/README.md\" \\" >> $(DOXYFILE_CONF)
	@echo "        \"$(PROJECT_SOURCE_DIR)/BUILDING.md\" \\" >> $(DOXYFILE_CONF)
	@echo "        \"$(PROJECT_SOURCE_DIR)/src/include\" \\" >> $(DOXYFILE_CONF)
	@echo "        \"$(PROJECT_SOURCE_DIR)/docs/pages\"" >> $(DOXYFILE_CONF)
	@echo "USE_MDFILE_AS_MAINPAGE = \"$(PROJECT_SOURCE_DIR)/README.md\"" >> $(DOXYFILE_CONF)
	@# Ensure paths are quoted correctly for Doxygen STRIP_FROM_PATH
	@echo "STRIP_FROM_PATH = \"$(PROJECT_SOURCE_DIR)/src/include\" \\" >> $(DOXYFILE_CONF)
	@echo "                  \"$(PROJECT_SOURCE_DIR)\"" >> $(DOXYFILE_CONF)
	@echo "EXAMPLE_PATH = \"$(PROJECT_SOURCE_DIR)/examples\"" >> $(DOXYFILE_CONF)
	@echo "GENERATE_XML = YES" >> $(DOXYFILE_CONF)
	@echo "GENERATE_HTML = NO" >> $(DOXYFILE_CONF)
	@echo "GENERATE_LATEX = NO" >> $(DOXYFILE_CONF)
	@echo "XML_OUTPUT = xml" >> $(DOXYFILE_CONF)
	@# Append the rest of Doxyfile.in, excluding overridden/problematic lines
	@grep -vE '^PROJECT_NAME|^PROJECT_NUMBER|^OUTPUT_DIRECTORY|^INPUT|^USE_MDFILE_AS_MAINPAGE|^STRIP_FROM_PATH|^EXAMPLE_PATH|^GENERATE_XML|^GENERATE_HTML|^GENERATE_LATEX|^XML_OUTPUT|^PREDEFINED' docs/Doxyfile.in >> $(DOXYFILE_CONF)
	@echo "Generated $(abspath $(DOXYFILE_CONF))"
	@echo "--------------------------------------------------------------------"

# Generate basic Doxide configuration file if it doesn't exist
$(DOXIDE_CONF):
	@echo "--- Generating basic Doxide configuration (YAML) ---"
	@printf "title: $(PROJECT_NAME) Documentation\n" > $@
	@printf "description: $(PROJECT_DESC)\n" >> $@
	@printf "output: $(DOXIDE_SITE_DIR)\n" >> $@
	@printf "# IMPORTANT: Specify source directory(ies) below!\n" >> $@
	@printf "# src: src/include # Uncomment and set the correct path to your headers/sources\n" >> $@
	@printf "# base: /\n" >> $@
	@echo "Generated $(abspath $@)"
	@echo "--------------------------------------------------"
	@echo "[1;31mAction Required:[0m Edit '$(DOXIDE_CONF)' and set the 'src:' path(s) correctly!"
	@echo "--------------------------------------------------"

# Create build subdirectories if they don't exist
# $(DOCS_BUILD_DIR): ; @mkdir -p $@ # Removed this rule

.PHONY: docs-configs
docs-configs: $(DOXYFILE_CONF) $(DOXIDE_CONF) ## Generate documentation config files (Doxyfile, Doxide)

.PHONY: docs
docs: check-doc-tools docs-configs ## Generate all documentation (Doxygen XML, RST, Static Site)
	@echo "--- Cleaning previous documentation output ---"
	@rm -rf $(DOXYGEN_XML_DIR) $(RST_DOCS_DIR) $(DOXIDE_SITE_DIR)
	@mkdir -p $(DOXYGEN_XML_DIR) $(RST_DOCS_DIR) $(DOXIDE_SITE_DIR)
	@echo "---------------------------------------------"
	@echo "--- [1/4] Running Doxygen to generate XML ---"
	@doxygen $(DOXYFILE_CONF)
	@echo "Checking for Doxygen XML output..."
	@ls -l $(DOXYGEN_XML_DIR)/xml/index.xml || (echo "ERROR: Doxygen XML output (xml/index.xml) not found!"; exit 1)
	@echo "---------------------------------------------"
	@echo "--- [2/4] Running Doxyrest to generate RST ---"
	@# Explicitly pass XML index file and frame directory, use static config file
	@doxyrest $(DOXYGEN_XML_DIR)/xml/index.xml \
		 -o $(RST_DOCS_DIR)/index.rst \
		 -c docs/doxyrest-config.lua \
		 -F /home/dengqi/.local/share/doxyrest/frame/cfamily \
		 -F /home/dengqi/.local/share/doxyrest/frame/common
	@echo "RST documentation generated in $(abspath $(RST_DOCS_DIR))"
	@echo "---------------------------------------------"
	@echo "--- [3/4] Running Doxide to generate Markdown ---" # Uncomment after configuring doxide.yaml
	@doxide build # Use default config (doxide.yaml) and output from config # Uncomment after configuring doxide.yaml
	@echo "Markdown documentation generated in $(abspath $(MKDOCS_MARKDOWN_DIR))" # Uncomment after configuring doxide.yaml
	@echo "------------------------------------------------" # Uncomment after configuring doxide.yaml
	@echo "--- [4/4] Running MkDocs to build static site ---" # Uncomment after configuring doxide.yaml
	@cd $(DOXIDE_SITE_DIR) && mkdocs build # Uncomment after configuring doxide.yaml
	@echo "Static site generated in $(abspath $(MKDOCS_OUTPUT_DIR))" # Uncomment after configuring doxide.yaml
	@echo "-----------------------------------------------"

.PHONY: serve-docs
serve-docs: ## Serve the generated static site locally
	@echo "Serving documentation site from $(abspath $(MKDOCS_OUTPUT_DIR))"
	@cd $(DOXIDE_SITE_DIR) && mkdocs serve


# --- Cleanup ---

.PHONY: clean-docs
clean-docs: ## Clean generated documentation files and directories
	@echo "Cleaning documentation artifacts..."
	@rm -rf $(DOXYGEN_XML_DIR) $(RST_DOCS_DIR) $(DOXIDE_SITE_DIR)
	@rm -rf $(DOXYFILE_CONF) $(DOXIDE_CONF)
	@echo "Done cleaning documentation."

.PHONY: clean
clean: clean-docs ## Clean build directory and documentation artifacts
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)
	@echo "Done cleaning build directory."


