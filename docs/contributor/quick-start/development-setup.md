# Development Environment Setup

**Complete guide to setting up your Asthra development environment**

This guide provides comprehensive setup instructions for all platforms and development scenarios. For a quick start, see the [Quick Start Guide](README.md).

## Table of Contents

1. [Platform-Specific Setup](#platform-specific-setup)
2. [IDE Configuration](#ide-configuration)
3. [Development Tools](#development-tools)
4. [Build System Deep Dive](#build-system-deep-dive)
5. [Environment Variables](#environment-variables)
6. [Verification & Testing](#verification--testing)
7. [Troubleshooting](#troubleshooting)

## Platform-Specific Setup

### macOS (Primary Development Platform)

**Prerequisites:**
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

**Required Tools:**
```bash
# Core development tools (LLVM is required)
brew install llvm make git cmake

# Development utilities
brew install cppcheck valgrind pkg-config

# Optional but recommended
brew install gdb htop tree jq
```

**Compiler Configuration:**
```bash
# Set Clang as default compiler
export CC=/opt/homebrew/bin/clang      # Apple Silicon
export CC=/usr/local/bin/clang         # Intel Mac
export CXX=/opt/homebrew/bin/clang++   # Apple Silicon
export CXX=/usr/local/bin/clang++      # Intel Mac

# Add to your shell profile (~/.zshrc or ~/.bash_profile)
echo 'export CC=/opt/homebrew/bin/clang' >> ~/.zshrc
echo 'export CXX=/opt/homebrew/bin/clang++' >> ~/.zshrc
```

### Linux (Ubuntu/Debian)

**Update System:**
```bash
sudo apt update && sudo apt upgrade -y
```

**Required Packages:**
```bash
# Core development tools (LLVM is required)
sudo apt install -y \
    llvm-dev \
    clang-12 \
    make \
    cmake \
    git \
    build-essential \
    pkg-config

# Development utilities
sudo apt install -y \
    cppcheck \
    valgrind \
    gdb \
    htop \
    tree \
    jq
```

**Compiler Configuration:**
```bash
# Set Clang as default
sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-12 100
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-12 100

# Set environment variables
export CC=clang
export CXX=clang++

# Add to ~/.bashrc or ~/.zshrc
echo 'export CC=clang' >> ~/.bashrc
echo 'export CXX=clang++' >> ~/.bashrc
```

### Linux (CentOS/RHEL/Fedora)

**CentOS/RHEL:**
```bash
# Enable EPEL repository
sudo yum install -y epel-release

# Install development tools (LLVM is required)
sudo yum groupinstall -y "Development Tools"
sudo yum install -y llvm-devel clang make cmake git pkg-config

# Development utilities
sudo yum install -y cppcheck valgrind gdb htop tree
```

**Fedora:**
```bash
# Install development tools (LLVM is required)
sudo dnf groupinstall -y "Development Tools"
sudo dnf install -y llvm-devel clang make cmake git pkg-config

# Development utilities
sudo dnf install -y cppcheck valgrind gdb htop tree
```


## IDE Configuration

### Visual Studio Code (Recommended)

**Install Extensions:**
```bash
# Install VS Code extensions
code --install-extension ms-vscode.cpptools
code --install-extension ms-vscode.makefile-tools
code --install-extension ms-vscode.cmake-tools
code --install-extension ms-vscode.hexeditor
```

**Configuration Files:**

Create `.vscode/settings.json`:
```json
{
    "C_Cpp.default.cStandard": "c17",
    "C_Cpp.default.compilerPath": "/usr/bin/clang",
    "C_Cpp.default.includePath": [
        "${workspaceFolder}/src",
        "${workspaceFolder}/runtime/include",
        "${workspaceFolder}/runtime",
        "/usr/include",
        "/usr/local/include"
    ],
    "C_Cpp.default.defines": [
        "_GNU_SOURCE",
        "ASTHRA_DEBUG=1"
    ],
    "C_Cpp.default.compileCommands": "${workspaceFolder}/build/compile_commands.json",
    "files.associations": {
        "*.h": "c",
        "*.c": "c",
        "*.asthra": "rust"
    },
    "editor.tabSize": 4,
    "editor.insertSpaces": true,
    "editor.detectIndentation": false,
    "makefile.extensionOutputFolder": "./.vscode"
}
```

Create `.vscode/tasks.json`:
```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Build Asthra",
            "type": "shell",
            "command": "make",
            "args": ["-j8"],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false,
                "panel": "shared"
            },
            "problemMatcher": ["$clang"]
        },
        {
            "label": "Clean Build",
            "type": "shell",
            "command": "make",
            "args": ["clean"],
            "group": "build"
        },
        {
            "label": "Run Tests",
            "type": "shell",
            "command": "make",
            "args": ["test-curated"],
            "group": "test"
        }
    ]
}
```

Create `.vscode/launch.json`:
```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug Asthra Compiler",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/bin/asthra",
            "args": ["${workspaceFolder}/examples/hello_world.asthra"],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [
                {"name": "ASTHRA_DEBUG", "value": "1"}
            ],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ]
        }
    ]
}
```

### CLion

**Project Configuration:**
1. Open CLion and select "Open Project"
2. Choose the Asthra root directory
3. CLion will detect the Makefile automatically

**CMake Configuration (Optional):**
Create `CMakeLists.txt` for better IDE support:
```cmake
cmake_minimum_required(VERSION 3.16)
project(asthra C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add include directories
include_directories(src)
include_directories(runtime/include)
include_directories(runtime)

# Add source files (for IDE navigation)
file(GLOB_RECURSE SOURCES "src/*.c" "src/*.h")
add_executable(asthra ${SOURCES})

# Add compile definitions
target_compile_definitions(asthra PRIVATE _GNU_SOURCE ASTHRA_DEBUG=1)
```

### Vim/Neovim

**Essential Plugins:**
```vim
" Add to your .vimrc or init.vim
Plug 'neoclide/coc.nvim', {'branch': 'release'}
Plug 'dense-analysis/ale'
Plug 'preservim/nerdtree'
Plug 'junegunn/fzf.vim'
```

**Configuration:**
```vim
" C development settings
autocmd FileType c setlocal tabstop=4 shiftwidth=4 expandtab
autocmd FileType c setlocal cindent

" ALE linter configuration
let g:ale_linters = {'c': ['clang', 'cppcheck']}
let g:ale_c_clang_options = '-std=c17 -Wall -Wextra'

" CoC configuration for C
let g:coc_global_extensions = ['coc-clangd']
```

### Emacs

**Configuration:**
```elisp
;; Add to your .emacs or init.el
(require 'cc-mode)

;; C mode configuration
(setq c-default-style "linux"
      c-basic-offset 4)

;; Enable company mode for completion
(add-hook 'c-mode-hook 'company-mode)

;; Enable flycheck for syntax checking
(add-hook 'c-mode-hook 'flycheck-mode)

;; LSP mode for better C support
(require 'lsp-mode)
(add-hook 'c-mode-hook #'lsp)
```

## Development Tools

### Static Analysis Tools

**Clang Static Analyzer:**
```bash
# Run static analysis
scan-build make

# View results
scan-view /tmp/scan-build-*
```

**Cppcheck:**
```bash
# Run cppcheck on source
cppcheck --enable=all --std=c17 src/

# Generate XML report
cppcheck --enable=all --xml --xml-version=2 src/ 2> cppcheck.xml
```

**Clang-Tidy:**
```bash
# Run clang-tidy
clang-tidy src/**/*.c -- -std=c17 -Isrc -Iruntime/include

# With compilation database
clang-tidy -p build src/**/*.c
```

### Memory Debugging Tools

**Valgrind (Linux/macOS):**
```bash
# Build with debug symbols
make debug

# Run with Valgrind
valgrind --tool=memcheck --leak-check=full ./bin/asthra test.asthra

# Detailed analysis
valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
         --track-origins=yes ./bin/asthra test.asthra
```

**AddressSanitizer:**
```bash
# Build with AddressSanitizer
make asan

# Run normally - ASan will detect issues automatically
./bin/asthra test.asthra
```

**ThreadSanitizer:**
```bash
# Build with ThreadSanitizer
make tsan

# Run to detect race conditions
./bin/asthra test.asthra
```

### Performance Profiling

**GProf:**
```bash
# Build with profiling
make profile

# Run to generate profile data
./bin/asthra large_program.asthra

# Generate profile report
gprof ./bin/asthra gmon.out > profile.txt
```

**Perf (Linux):**
```bash
# Record performance data
perf record ./bin/asthra large_program.asthra

# View performance report
perf report
```

**Instruments (macOS):**
```bash
# Profile with Instruments
instruments -t "Time Profiler" ./bin/asthra large_program.asthra
```

## Build System Deep Dive

### CMake Structure

**Root CMakeLists.txt:**
- Entry point for all builds
- Includes CMake modules from `cmake/` directory
- Provides modern cross-platform configuration

**CMake Modules (`cmake/`):**
```
cmake/
├── Platform.cmake      # Platform detection
├── Compiler.cmake      # Compiler configuration
├── ThirdParty.cmake    # Dependency management
├── BuildTypes.cmake    # Build type definitions
├── Testing.cmake       # Test configuration
├── Sanitizers.cmake    # Sanitizer configuration
├── Coverage.cmake      # Coverage configuration
├── DevTools.cmake      # Development tools
└── Help.cmake          # Help system
```

### Build Targets

**Core Targets:**
```bash
cmake -B build                              # Configure build
cmake --build build                         # Build everything
rm -rf build && cmake -B build              # Clean and reconfigure
cmake --build build --target install        # Install to system
```

**Development Targets:**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug     # Debug build (-g -O0)
cmake -B build -DCMAKE_BUILD_TYPE=Release   # Release build (-O3 -DNDEBUG)
cmake -B build -DENABLE_ASAN=ON             # AddressSanitizer build
cmake -B build -DENABLE_TSAN=ON             # ThreadSanitizer build
cmake -B build -DENABLE_MSAN=ON             # MemorySanitizer build
cmake -B build -DENABLE_UBSAN=ON            # UndefinedBehaviorSanitizer build
```

**Testing Targets:**
```bash
ctest --test-dir build                       # Run all tests
cmake --build build --target test-quick     # Run curated high-reliability tests
ctest --test-dir build -L lexer              # Run lexer tests
ctest --test-dir build -L parser             # Run parser tests
ctest --test-dir build -L semantic           # Run semantic analysis tests
ctest --test-dir build -L codegen            # Run code generation tests
```

**Analysis Targets:**
```bash
cmake --build build --target analyze        # Run static analysis tools
cmake --build build --target coverage       # Generate coverage report
cmake --build build --target benchmark      # Run performance benchmarks
cmake -B build -DENABLE_COVERAGE=ON         # Build for profiling
```

### Build Configuration

**CMake Build Types:**
```cmake
# Debug build configuration
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -DDEBUG -DASTHRA_DEBUG=1")

# Release build configuration
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")

# Release with debug info
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")

# Warning flags (applied to all builds)
set(CMAKE_C_FLAGS "-std=c17 -Wall -Wextra -Wpedantic")
```

**Platform-Specific Configuration:**
```cmake
# macOS specific
if(APPLE)
    find_library(CORE_FOUNDATION CoreFoundation)
    target_link_libraries(asthra ${CORE_FOUNDATION})
endif()

# Linux specific
if(UNIX AND NOT APPLE)
    target_link_libraries(asthra pthread m)
endif()
```

## Environment Variables

### Compiler Configuration

```bash
# Compiler selection
export CC=clang                    # C compiler
export CXX=clang++                 # C++ compiler (for tools)
export AR=ar                       # Archiver
export RANLIB=ranlib              # Archive indexer

# Compiler flags
export CFLAGS="-std=c17 -Wall"    # Additional C flags
export LDFLAGS="-L/usr/local/lib" # Additional linker flags
export CPPFLAGS="-I/usr/local/include" # Preprocessor flags
```

### Asthra-Specific Variables

```bash
# Debug options
export ASTHRA_DEBUG=1              # Enable debug output
export ASTHRA_VERBOSE=1            # Enable verbose output
export ASTHRA_TRACE=1              # Enable execution tracing

# Test configuration
export ASTHRA_TEST_TIMEOUT=30      # Test timeout in seconds
export ASTHRA_TEST_PARALLEL=8      # Parallel test execution
export ASTHRA_TEST_VERBOSE=1       # Verbose test output

# Build configuration
export ASTHRA_BUILD_TYPE=debug     # Build type (debug/release)
export ASTHRA_ENABLE_ASAN=1        # Enable AddressSanitizer
export ASTHRA_ENABLE_COVERAGE=1    # Enable coverage collection
```

### Third-Party Integration

```bash
# JSON-C library
export JSON_C_SYSTEM=1             # Use system json-c library
export JSON_C_CFLAGS="$(pkg-config --cflags json-c)"
export JSON_C_LIBS="$(pkg-config --libs json-c)"

# Other libraries
export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH"
```

### Shell Configuration

**Bash (~/.bashrc):**
```bash
# Asthra development environment
export ASTHRA_ROOT="$HOME/asthra"
export PATH="$ASTHRA_ROOT/bin:$PATH"

# Compiler configuration
export CC=clang
export CXX=clang++

# Development options
export ASTHRA_DEBUG=1
export ASTHRA_TEST_PARALLEL=8

# Aliases
alias asthra-build='rm -rf build && cmake -B build && cmake --build build -j8'
alias asthra-test='cmake --build build --target test-quick'
alias asthra-debug='gdb ./build/bin/asthra'
```

**Zsh (~/.zshrc):**
```zsh
# Asthra development environment
export ASTHRA_ROOT="$HOME/asthra"
path=("$ASTHRA_ROOT/bin" $path)

# Compiler configuration
export CC=clang
export CXX=clang++

# Development options
export ASTHRA_DEBUG=1
export ASTHRA_TEST_PARALLEL=8

# Functions
asthra-quick-test() {
    rm -rf build && cmake -B build && cmake --build build -j8 && cmake --build build --target test-quick
}

asthra-full-test() {
    rm -rf build && cmake -B build && cmake --build build -j8 && ctest --test-dir build
}
```

## Verification & Testing

### Build Verification

```bash
# Clean build test
make clean && make -j8
echo "Build status: $?"

# Verify binary
ls -la bin/asthra
file bin/asthra
./bin/asthra --version
```

### Test Suite Verification

```bash
# Quick test suite
make test-curated
echo "Curated tests status: $?"

# Component tests
make test-lexer test-parser test-semantic test-codegen
echo "Component tests status: $?"

# Full test suite (takes longer)
make test-all
echo "Full test suite status: $?"
```

### Development Tools Verification

```bash
# Static analysis
make static-analysis
echo "Static analysis status: $?"

# Memory checking
make asan && ./bin/asthra examples/hello_world.asthra
echo "AddressSanitizer status: $?"

# Performance profiling
make profile && ./bin/asthra examples/hello_world.asthra
echo "Profiling status: $?"
```

### IDE Integration Verification

**VS Code:**
```bash
# Test IntelliSense
# Open a .c file and verify:
# - Syntax highlighting works
# - Auto-completion works
# - Error squiggles appear for syntax errors
# - Go to definition works (Ctrl+Click)

# Test build integration
# Press Ctrl+Shift+P, type "Tasks: Run Task", select "Build Asthra"
```

**CLion:**
```bash
# Test project loading
# Verify:
# - Project loads without errors
# - Source files are indexed
# - Build configuration is detected
# - Debugger configuration works
```

## Troubleshooting

### Common Build Issues

**Compiler Not Found:**
```bash
# Check compiler installation
which clang
clang --version

# Install if missing (macOS)
brew install llvm

# Install if missing (Linux)
sudo apt install clang-12
```

**Missing Headers:**
```bash
# Check include paths
echo | clang -E -Wp,-v -

# Install development headers (Linux)
sudo apt install libc6-dev

# Install Xcode tools (macOS)
xcode-select --install
```

**Link Errors:**
```bash
# Check library paths
ldconfig -p | grep json-c

# Install missing libraries
sudo apt install libjson-c-dev  # Linux
brew install json-c             # macOS
```

### IDE Issues

**VS Code IntelliSense Not Working:**
```bash
# Regenerate compile commands
make clean && bear -- make -j8

# Check C++ extension
code --list-extensions | grep ms-vscode.cpptools

# Reset extension
# Ctrl+Shift+P -> "C/C++: Reset IntelliSense Database"
```

**CLion Project Issues:**
```bash
# Clear CLion cache
rm -rf .idea/
# Reopen project in CLion

# Check CMake configuration
# File -> Settings -> Build -> CMake
```

### Test Failures

**Environment Issues:**
```bash
# Check environment variables
env | grep ASTHRA
env | grep CC

# Reset environment
unset ASTHRA_DEBUG ASTHRA_VERBOSE
export CC=clang
```

**Platform-Specific Issues:**
```bash
# macOS: Check Xcode tools
xcode-select -p

# Linux: Check build tools
dpkg -l | grep build-essential

# Windows: Check MinGW installation
clang --version
```

### Performance Issues

**Slow Builds:**
```bash
# Use parallel builds
make -j$(nproc)

# Check disk space
df -h

# Check memory usage
free -h
```

**Slow Tests:**
```bash
# Run subset of tests
make test-curated

# Increase test timeout
export ASTHRA_TEST_TIMEOUT=60

# Reduce parallel tests
export ASTHRA_TEST_PARALLEL=4
```

## Getting Help

**Documentation:**
- [Troubleshooting Guide](../troubleshooting/README.md)
- [Build System Reference](../reference/build-system/README.md)
- [Contributor Handbook](../HANDBOOK.md)

**Community:**
- GitHub Issues: Report setup problems
- GitHub Discussions: Ask setup questions
- Discord/Slack: Real-time help

**Debugging:**
- Enable verbose output: `export ASTHRA_VERBOSE=1`
- Check build logs: `make clean && make -j1 2>&1 | tee build.log`
- Run with debug: `export ASTHRA_DEBUG=1`

---

**🎯 Environment Setup Complete!**

*You're now ready to contribute to Asthra with a fully configured development environment. Next: [Make your first contribution](first-contribution.md)* 