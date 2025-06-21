# Development Environment Setup

**Complete guide for setting up your Asthra development environment**

This guide covers everything you need to set up a productive development environment for contributing to Asthra, from basic tools to advanced debugging and profiling setup.

## Overview

- **Purpose**: Set up a complete development environment for Asthra
- **Time**: 30-60 minutes for full setup
- **Prerequisites**: Basic command line knowledge
- **Outcome**: Fully configured development environment ready for contribution

## Platform-Specific Setup

### macOS (Recommended)

#### Essential Tools
```bash
# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install core development tools
brew install llvm clang-format git make

# Set Clang as default compiler
export CC=clang
export CXX=clang++
echo 'export CC=clang' >> ~/.zshrc
echo 'export CXX=clang++' >> ~/.zshrc
```

#### Development Tools
```bash
# Static analysis and linting
brew install cppcheck clang-tidy

# Performance profiling
brew install gperftools

# Memory debugging (limited on macOS)
# AddressSanitizer is built into clang

# Optional: Advanced debugging
brew install gdb lldb
```

#### Third-Party Dependencies
```bash
# JSON-C library (recommended for system integration)
brew install json-c

# Set environment variable for system library usage
export JSON_C_SYSTEM=1
echo 'export JSON_C_SYSTEM=1' >> ~/.zshrc
```

### Linux (Ubuntu/Debian)

#### Essential Tools
```bash
# Update package manager
sudo apt-get update

# Install core development tools
sudo apt-get install -y \
    clang-12 \
    gcc \
    make \
    git \
    build-essential \
    pkg-config

# Set Clang as default
export CC=clang-12
export CXX=clang++-12
echo 'export CC=clang-12' >> ~/.bashrc
echo 'export CXX=clang++-12' >> ~/.bashrc
```

#### Development Tools
```bash
# Static analysis and debugging
sudo apt-get install -y \
    cppcheck \
    clang-tidy-12 \
    valgrind \
    gdb

# Performance profiling
sudo apt-get install -y \
    google-perftools \
    libgoogle-perftools-dev

# Memory debugging
sudo apt-get install -y \
    libc6-dbg
```

#### Third-Party Dependencies
```bash
# JSON-C library
sudo apt-get install -y libjson-c-dev

# Set environment variable
export JSON_C_SYSTEM=1
echo 'export JSON_C_SYSTEM=1' >> ~/.bashrc
```

### Linux (CentOS/RHEL/Fedora)

#### Essential Tools
```bash
# CentOS/RHEL
sudo yum install -y clang gcc make git

# Fedora
sudo dnf install -y clang gcc make git

# Set compiler
export CC=clang
export CXX=clang++
```

#### Development Tools
```bash
# Static analysis and debugging
sudo yum install -y cppcheck valgrind gdb
# or for Fedora:
sudo dnf install -y cppcheck valgrind gdb
```

### Windows (MinGW-w64)

#### Using MSYS2
```bash
# Install MSYS2 from https://www.msys2.org/
# Then in MSYS2 terminal:

# Update package database
pacman -Syu

# Install development tools
pacman -S mingw-w64-x86_64-gcc \
          mingw-w64-x86_64-clang \
          mingw-w64-x86_64-make \
          git

# Add to PATH
export PATH="/mingw64/bin:$PATH"
```

## IDE Configuration

### VS Code (Recommended)

#### Extensions
Install these essential extensions:
- **C/C++** (Microsoft)
- **C/C++ Extension Pack** (Microsoft)
- **Clang-Format** (xaver)
- **GitLens** (GitKraken)
- **Error Lens** (Alexander)

#### Configuration
Create `.vscode/settings.json`:
```json
{
    "C_Cpp.default.cStandard": "c17",
    "C_Cpp.default.compilerPath": "/usr/bin/clang",
    "C_Cpp.default.includePath": [
        "${workspaceFolder}/src",
        "${workspaceFolder}/runtime/include",
        "${workspaceFolder}/third-party/*/include"
    ],
    "C_Cpp.default.defines": [
        "_GNU_SOURCE",
        "ASTHRA_VERSION=\"dev\""
    ],
    "files.associations": {
        "*.h": "c",
        "*.c": "c"
    },
    "editor.formatOnSave": true,
    "C_Cpp.clang_format_style": "file",
    "editor.rulers": [80, 120],
    "files.trimTrailingWhitespace": true,
    "files.insertFinalNewline": true
}
```

#### Tasks Configuration
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
            "problemMatcher": "$gcc"
        },
        {
            "label": "Run Tests",
            "type": "shell",
            "command": "make",
            "args": ["test-curated"],
            "group": "test",
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false,
                "panel": "shared"
            }
        },
        {
            "label": "Clean Build",
            "type": "shell",
            "command": "make",
            "args": ["clean"],
            "group": "build"
        }
    ]
}
```

#### Launch Configuration
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
            "args": ["examples/hello_world.asthra"],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [
                {
                    "name": "ASTHRA_DEBUG",
                    "value": "1"
                }
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

#### Project Setup
1. **Open Project**: File → Open → Select Asthra directory
2. **Configure Toolchain**: File → Settings → Build, Execution, Deployment → Toolchains
   - Set C Compiler: `/usr/bin/clang`
   - Set C++ Compiler: `/usr/bin/clang++`
   - Set Make: `/usr/bin/make`

#### CMake Alternative
If you prefer CMake over Makefiles, create `CMakeLists.txt`:
```cmake
cmake_minimum_required(VERSION 3.16)
project(asthra C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include directories
include_directories(src)
include_directories(runtime/include)

# Add subdirectories
add_subdirectory(src)
add_subdirectory(tests)

# Main executable
add_executable(asthra src/main.c)
target_link_libraries(asthra asthra_core)
```

#### Code Style
Configure CLion to use project code style:
1. **Settings** → **Editor** → **Code Style** → **C/C++**
2. **Import Scheme** → Select `.clang-format` file
3. **Enable** → "Reformat code" and "Optimize imports"

### Vim/Neovim

#### Essential Plugins
```vim
" .vimrc or init.vim
call plug#begin()

" Language support
Plug 'neoclide/coc.nvim', {'branch': 'release'}
Plug 'jackguo380/vim-lsp-cxx-highlight'

" Code formatting
Plug 'rhysd/vim-clang-format'

" Git integration
Plug 'tpope/vim-fugitive'
Plug 'airblade/vim-gitgutter'

" File navigation
Plug 'junegunn/fzf', { 'do': { -> fzf#install() } }
Plug 'junegunn/fzf.vim'

call plug#end()
```

#### CoC Configuration
Create `.vim/coc-settings.json`:
```json
{
    "clangd.path": "/usr/bin/clangd",
    "clangd.arguments": [
        "--compile-commands-dir=build",
        "--header-insertion=never",
        "--clang-tidy"
    ],
    "diagnostic.errorSign": "✗",
    "diagnostic.warningSign": "⚠",
    "diagnostic.infoSign": "ℹ"
}
```

### Emacs

#### Configuration
```elisp
;; .emacs or init.el
(require 'package)
(add-to-list 'package-archives '("melpa" . "https://melpa.org/packages/"))
(package-initialize)

;; Essential packages
(unless (package-installed-p 'use-package)
  (package-refresh-contents)
  (package-install 'use-package))

;; C development
(use-package cc-mode
  :config
  (setq c-default-style "linux"
        c-basic-offset 4))

;; LSP support
(use-package lsp-mode
  :ensure t
  :hook (c-mode . lsp)
  :commands lsp)

;; Company for completion
(use-package company
  :ensure t
  :config
  (global-company-mode))

;; Git integration
(use-package magit
  :ensure t)
```

## Build System Configuration

### Environment Variables

#### Core Variables
```bash
# Compiler selection
export CC=clang
export CXX=clang++

# Build configuration
export ASTHRA_DEBUG=1          # Enable debug output
export ASTHRA_VERBOSE=1        # Verbose compilation
export ASTHRA_PROFILE=1        # Enable profiling

# Test configuration
export ASTHRA_TEST_TIMEOUT=30  # Test timeout in seconds
export ASTHRA_TEST_PARALLEL=8  # Parallel test execution

# Third-party integration
export JSON_C_SYSTEM=1         # Use system JSON-C library
```

#### Development Shortcuts
```bash
# Add to your shell profile (.bashrc, .zshrc, etc.)
alias asthra-build='make clean && make -j$(nproc)'
alias asthra-test='make test-curated'
alias asthra-debug='make clean && make debug'
alias asthra-asan='make clean && make asan'
alias asthra-format='find src tests -name "*.c" -o -name "*.h" | xargs clang-format -i'
```

### Build Variants

#### Debug Build
```bash
make debug
# Features:
# - Debug symbols (-g)
# - No optimization (-O0)
# - Debug assertions enabled
# - Verbose error messages
```

#### Release Build
```bash
make release
# Features:
# - Full optimization (-O3)
# - No debug symbols
# - Assertions disabled
# - Minimal binary size
```

#### AddressSanitizer Build
```bash
make asan
# Features:
# - Memory error detection
# - Buffer overflow detection
# - Use-after-free detection
# - Memory leak detection
```

#### ThreadSanitizer Build
```bash
make tsan
# Features:
# - Race condition detection
# - Thread safety validation
# - Deadlock detection
```

#### Coverage Build
```bash
make coverage
# Features:
# - Code coverage instrumentation
# - Line and branch coverage
# - Coverage report generation
```

## Development Tools Setup

### Static Analysis

#### Clang-Tidy
```bash
# Install clang-tidy
brew install clang-tidy  # macOS
sudo apt-get install clang-tidy  # Linux

# Create .clang-tidy configuration
cat > .clang-tidy << 'EOF'
Checks: >
  clang-diagnostic-*,
  clang-analyzer-*,
  cppcoreguidelines-*,
  modernize-*,
  performance-*,
  portability-*,
  readability-*,
  -readability-magic-numbers,
  -cppcoreguidelines-avoid-magic-numbers

CheckOptions:
  - key: readability-identifier-naming.VariableCase
    value: snake_case
  - key: readability-identifier-naming.FunctionCase
    value: snake_case
  - key: readability-identifier-naming.TypeCase
    value: CamelCase
EOF

# Run static analysis
make static-analysis
```

#### Cppcheck
```bash
# Install cppcheck
brew install cppcheck  # macOS
sudo apt-get install cppcheck  # Linux

# Run cppcheck
cppcheck --enable=all --std=c17 src/
```

### Memory Debugging

#### AddressSanitizer (Recommended)
```bash
# Build with AddressSanitizer
make clean && make asan

# Run tests with memory checking
make test-curated

# Run specific program
./bin/asthra examples/test.asthra
```

#### Valgrind (Linux only)
```bash
# Install Valgrind
sudo apt-get install valgrind

# Build for Valgrind
make clean && make valgrind

# Memory leak detection
valgrind --tool=memcheck --leak-check=full \
         --show-leak-kinds=all --track-origins=yes \
         ./bin/asthra examples/test.asthra

# Cache profiling
valgrind --tool=cachegrind ./bin/asthra examples/test.asthra
```

### Performance Profiling

#### Built-in Profiling
```bash
# Build with profiling
make clean && make profile

# Run with profiling
./bin/asthra examples/large_file.asthra

# Generate profile report
gprof ./bin/asthra gmon.out > profile_report.txt
```

#### Perf (Linux)
```bash
# Install perf
sudo apt-get install linux-tools-generic

# Profile CPU usage
perf record -g ./bin/asthra examples/large_file.asthra
perf report

# Profile memory access
perf record -e cache-misses ./bin/asthra examples/large_file.asthra
```

## Verification & Testing

### Quick Verification
```bash
# Verify build system
make --version
$CC --version

# Test basic compilation
make clean && make -j$(nproc)
echo $?  # Should be 0

# Test compiler functionality
echo 'fn main() -> i32 { return 42; }' > test.asthra
./bin/asthra test.asthra
echo $?  # Should be 0

# Clean up
rm test.asthra test.c test
```

### Environment Health Check
```bash
# Create health check script
cat > scripts/health_check.sh << 'EOF'
#!/bin/bash
set -e

echo "=== Asthra Development Environment Health Check ==="

# Check required tools
echo "Checking required tools..."
command -v $CC >/dev/null 2>&1 || { echo "❌ Compiler $CC not found"; exit 1; }
command -v make >/dev/null 2>&1 || { echo "❌ Make not found"; exit 1; }
command -v git >/dev/null 2>&1 || { echo "❌ Git not found"; exit 1; }
echo "✅ Required tools found"

# Check compiler version
echo "Checking compiler version..."
$CC --version | head -1
echo "✅ Compiler version OK"

# Check build system
echo "Testing build system..."
make clean >/dev/null 2>&1
make -j$(nproc) >/dev/null 2>&1 || { echo "❌ Build failed"; exit 1; }
echo "✅ Build system working"

# Check basic functionality
echo "Testing basic functionality..."
echo 'fn main() -> i32 { return 0; }' > /tmp/health_test.asthra
./bin/asthra /tmp/health_test.asthra >/dev/null 2>&1 || { echo "❌ Compiler test failed"; exit 1; }
rm -f /tmp/health_test.asthra /tmp/health_test.c /tmp/health_test
echo "✅ Basic functionality working"

# Check third-party integration
if [ "$JSON_C_SYSTEM" = "1" ]; then
    echo "Testing third-party integration..."
    pkg-config --exists json-c || { echo "⚠️  JSON-C system library not found"; }
    echo "✅ Third-party integration configured"
fi

echo "🎉 Environment health check passed!"
EOF

chmod +x scripts/health_check.sh
./scripts/health_check.sh
```

## Troubleshooting

### Common Issues

#### Compiler Not Found
```bash
# Issue: clang: command not found
# Solution: Install and configure compiler
brew install llvm  # macOS
sudo apt-get install clang  # Linux

# Set environment variables
export CC=clang
export CXX=clang++
```

#### Build Failures
```bash
# Issue: Make errors or missing dependencies
# Solution: Clean and rebuild
make clean
make -j1  # Single-threaded for better error messages

# Check for missing headers
find /usr/include -name "*.h" | grep json  # Example for JSON-C
```

#### Test Failures
```bash
# Issue: Tests failing unexpectedly
# Solution: Run with verbose output
ASTHRA_TEST_VERBOSE=1 make test-curated

# Run individual test for debugging
make test-lexer-basic
```

#### Memory Issues
```bash
# Issue: Segmentation faults or memory errors
# Solution: Use AddressSanitizer
make clean && make asan
./bin/asthra problematic_file.asthra
```

### Getting Help

- **Documentation**: Check `docs/contributor/` for detailed guides
- **Issues**: Search existing issues on GitHub
- **Discussions**: Use GitHub Discussions for questions
- **Community**: Join our development chat (link in README)

## Success Criteria

Your development environment is ready when:

- [ ] **Compiler**: Clang/GCC installed and configured
- [ ] **Build System**: Make builds successfully
- [ ] **IDE**: Configured with proper syntax highlighting and debugging
- [ ] **Tools**: Static analysis and debugging tools installed
- [ ] **Tests**: Basic test suite passes
- [ ] **Third-Party**: JSON-C integration working (if using system library)
- [ ] **Health Check**: Environment health check script passes

## Next Steps

Once your environment is set up:

1. **[First Contribution Guide](../quick-start/first-contribution.md)** - Make your first contribution
2. **[Adding Features Workflow](../workflows/adding-features.md)** - Learn the feature development process
3. **[Testing Workflows](../workflows/testing-workflows.md)** - Understand our testing practices
4. **[Code Review Process](../workflows/code-review.md)** - Prepare for code reviews

---

**🛠️ Development Environment Ready!**

*Your development environment is now configured for productive Asthra development. Happy coding!* 