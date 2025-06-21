# Development Environment Setup

**Complete guide to setting up your Asthra development environment**

This guide provides comprehensive setup instructions for all platforms and development scenarios. For a quick start, see the &#91;Quick Start Guide&#93;(README.md).

## Table of Contents

1\. &#91;Platform-Specific Setup&#93;(#platform-specific-setup)
2\. &#91;IDE Configuration&#93;(#ide-configuration)
3\. &#91;Development Tools&#93;(#development-tools)
4\. &#91;Build System Deep Dive&#93;(#build-system-deep-dive)
5\. &#91;Environment Variables&#93;(#environment-variables)
6\. &#91;Verification &amp; Testing&#93;(#verification--testing)
7\. &#91;Troubleshooting&#93;(#troubleshooting)

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
# Core development tools
brew install llvm make git

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
echo 'export CC=/opt/homebrew/bin/clang' &gt;&gt; ~/.zshrc
echo 'export CXX=/opt/homebrew/bin/clang++' &gt;&gt; ~/.zshrc
```

### Linux (Ubuntu/Debian)

**Update System:**
```bash
sudo apt update &amp;&amp; sudo apt upgrade -y
```

**Required Packages:**
```bash
# Core development tools
sudo apt install -y \
    clang-12 \
    gcc \
    make \
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
echo 'export CC=clang' &gt;&gt; ~/.bashrc
echo 'export CXX=clang++' &gt;&gt; ~/.bashrc
```

### Linux (CentOS/RHEL/Fedora)

**CentOS/RHEL:**
```bash
# Enable EPEL repository
sudo yum install -y epel-release

# Install development tools
sudo yum groupinstall -y "Development Tools"
sudo yum install -y clang gcc make git pkg-config

# Development utilities
sudo yum install -y cppcheck valgrind gdb htop tree
```

**Fedora:**
```bash
# Install development tools
sudo dnf groupinstall -y "Development Tools"
sudo dnf install -y clang gcc make git pkg-config

# Development utilities
sudo dnf install -y cppcheck valgrind gdb htop tree
```

### Windows

**Option 1: WSL2 (Recommended)**
```bash
# Install WSL2 with Ubuntu
wsl --install -d Ubuntu

# Follow Ubuntu setup instructions above
```

**Option 2: MinGW-w64**
```bash
# Install MSYS2
# Download from: https://www.msys2.org/

# In MSYS2 terminal:
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-clang
pacman -S mingw-w64-x86_64-make
pacman -S git

# Add to PATH: C:\msys64\mingw64\bin
```

**Option 3: Visual Studio**
```bash
# Install Visual Studio 2019+ with C++ support
# Install Git for Windows
# Use Developer Command Prompt for building
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
    "C_Cpp.default.includePath": &#91;
        "${workspaceFolder}/src",
        "${workspaceFolder}/runtime/include",
        "${workspaceFolder}/runtime",
        "/usr/include",
        "/usr/local/include"
    &#93;,
    "C_Cpp.default.defines": &#91;
        "_GNU_SOURCE",
        "ASTHRA_DEBUG=1"
    &#93;,
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
    "tasks": &#91;
        {
            "label": "Build Asthra",
            "type": "shell",
            "command": "make",
            "args": ["-j8"&#93;,
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
            "problemMatcher": &#91;"$gcc"&#93;
        },
        {
            "label": "Clean Build",
            "type": "shell",
            "command": "make",
            "args": &#91;"clean"&#93;,
            "group": "build"
        },
        {
            "label": "Run Tests",
            "type": "shell",
            "command": "make",
            "args": &#91;"test-curated"&#93;,
            "group": "test"
        }
    ]
}
```

Create `.vscode/launch.json`:
```json
{
    "version": "0.2.0",
    "configurations": &#91;
        {
            "name": "Debug Asthra Compiler",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/bin/asthra",
            "args": ["${workspaceFolder}/examples/hello_world.asthra"&#93;,
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": &#91;
                {"name": "ASTHRA_DEBUG", "value": "1"}
            &#93;,
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": &#91;
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            &#93;
        }
    ]
}
```

### CLion

**Project Configuration:**
1\. Open CLion and select "Open Project"
2\. Choose the Asthra root directory
3\. CLion will detect the Makefile automatically

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
let g:ale_linters = {'c': &#91;'clang', 'cppcheck'&#93;}
let g:ale_c_clang_options = '-std=c17 -Wall -Wextra'

" CoC configuration for C
let g:coc_global_extensions = &#91;'coc-clangd'&#93;
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
gprof ./bin/asthra gmon.out &gt; profile.txt
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

### Makefile Structure

**Main Makefile:**
- Entry point for all builds
- Includes modular makefiles from `make/` directory
- Provides high-level targets

**Modular Makefiles (`make/`):**
```
make/
├── platform.mk        # Platform detection
├── compiler.mk        # Compiler configuration
├── paths.mk          # Path definitions
├── rules.mk          # Build rules
├── targets.mk        # Build targets
├── tests.mk          # Test configuration
├── utils.mk          # Utility functions
├── sanitizers.mk     # Sanitizer configuration
└── coverage.mk       # Coverage configuration
```

### Build Targets

**Core Targets:**
```bash
make                   # Build everything
make clean            # Clean build artifacts
make distclean        # Deep clean including dependencies
make install          # Install to system (if supported)
```

**Development Targets:**
```bash
make debug            # Debug build (-g -O0)
make release          # Release build (-O3 -DNDEBUG)
make asan             # AddressSanitizer build
make tsan             # ThreadSanitizer build
make msan             # MemorySanitizer build
make ubsan            # UndefinedBehaviorSanitizer build
```

**Testing Targets:**
```bash
make test             # Run all tests
make test-curated     # Run curated high-reliability tests
make test-lexer       # Run lexer tests
make test-parser      # Run parser tests
make test-semantic    # Run semantic analysis tests
make test-codegen     # Run code generation tests
```

**Analysis Targets:**
```bash
make static-analysis  # Run static analysis tools
make coverage         # Generate coverage report
make benchmark        # Run performance benchmarks
make profile          # Build for profiling
```

### Build Configuration

**Compiler Flags:**
```makefile
# Debug build flags
CFLAGS_DEBUG = -g -O0 -DDEBUG -DASTHRA_DEBUG=1

# Release build flags
CFLAGS_RELEASE = -O3 -DNDEBUG -march=native

# Warning flags
CFLAGS_WARNINGS = -Wall -Wextra -Wpedantic -Werror

# Standard flags
CFLAGS_STD = -std=c17 -D_GNU_SOURCE
```

**Platform-Specific Configuration:**
```makefile
# macOS specific
ifeq ($(PLATFORM),darwin)
    CC = clang
    LDFLAGS += -framework CoreFoundation
endif

# Linux specific
ifeq ($(PLATFORM),linux)
    CC = clang
    LDFLAGS += -lpthread -lm
endif
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
alias asthra-build='make clean &amp;&amp; make -j8'
alias asthra-test='make test-curated'
alias asthra-debug='gdb ./bin/asthra'
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
    make clean &amp;&amp; make -j8 &amp;&amp; make test-curated
}

asthra-full-test() {
    make clean &amp;&amp; make -j8 &amp;&amp; make test-all
}
```

## Verification &amp; Testing

### Build Verification

```bash
# Clean build test
make clean &amp;&amp; make -j8
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
make asan &amp;&amp; ./bin/asthra examples/hello_world.asthra
echo "AddressSanitizer status: $?"

# Performance profiling
make profile &amp;&amp; ./bin/asthra examples/hello_world.asthra
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
make clean &amp;&amp; bear -- make -j8

# Check C++ extension
code --list-extensions | grep ms-vscode.cpptools

# Reset extension
# Ctrl+Shift+P -&gt; "C/C++: Reset IntelliSense Database"
```

**CLion Project Issues:**
```bash
# Clear CLion cache
rm -rf .idea/
# Reopen project in CLion

# Check CMake configuration
# File -&gt; Settings -&gt; Build -&gt; CMake
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
gcc --version
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
- &#91;Troubleshooting Guide&#93;(../troubleshooting/README.md)
- &#91;Build System Reference&#93;(../reference/build-system/README.md)
- &#91;Contributor Handbook&#93;(../HANDBOOK.md)

**Community:**
- GitHub Issues: Report setup problems
- GitHub Discussions: Ask setup questions
- Discord/Slack: Real-time help

**Debugging:**
- Enable verbose output: `export ASTHRA_VERBOSE=1`
- Check build logs: `make clean &amp;&amp; make -j1 2>&amp;1 | tee build.log`
- Run with debug: `export ASTHRA_DEBUG=1`

---

**🎯 Environment Setup Complete!**

*You're now ready to contribute to Asthra with a fully configured development environment. Next: &#91;Make your first contribution&#93;(first-contribution.md)* 