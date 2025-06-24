# Asthra CMake Usage Guide

## Overview

The Asthra project uses CMake as its primary build system. This guide covers how to use CMake effectively for building, testing, and developing Asthra.

## Quick Start

### Prerequisites

- **CMake 3.20+**: Modern CMake with good C support
- **C Compiler**: GCC, Clang, or MSVC
- **LLVM 15.0+**: Required for LLVM backend (now the default)
- **json-c**: Optional system library (will download if not found)

### Installing LLVM

#### macOS
```bash
brew install llvm
```

#### Ubuntu/Debian
```bash
sudo apt install llvm-dev
```

#### Fedora
```bash
sudo dnf install llvm-devel
```

### Basic Build

```bash
# Clone and enter project
git clone https://github.com/asthra-lang/asthra.git
cd asthra

# Build with CMake
cmake -B build
cmake --build build -j$(nproc)

# Or use the traditional approach:
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)
```

## Build Configurations

### Release Build (Default)
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### Debug Build
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
```

### Release with Debug Info
```bash
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build -j$(nproc)
```

## Advanced Features

### Sanitizer Builds

#### AddressSanitizer
```bash
cmake -B build -DENABLE_ASAN=ON
cmake --build build --target asthra-asan
./build/bin/asthra-asan --help
```

#### ThreadSanitizer
```bash
cmake -B build -DENABLE_TSAN=ON
cmake --build build --target asthra-tsan
```

#### UndefinedBehaviorSanitizer
```bash
cmake -B build -DENABLE_UBSAN=ON
cmake --build build --target asthra-ubsan
```

#### Combined Sanitizers
```bash
cmake -B build -DENABLE_ASAN=ON -DENABLE_UBSAN=ON
cmake --build build --target asthra-asan-ubsan
```

### Code Coverage

```bash
# Configure with coverage
cmake -B build -DENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target asthra-coverage

# Run tests to generate coverage data
ctest --test-dir build

# Generate coverage report
cmake --build build --target coverage

# View report
open build/coverage-html/index.html  # macOS
xdg-open build/coverage-html/index.html  # Linux
```

### Third-Party Dependencies

#### Use System Libraries
```bash
cmake -B build -DUSE_SYSTEM_JSON_C=ON
```

#### Use Bundled Libraries
```bash
cmake -B build -DUSE_SYSTEM_JSON_C=OFF
```

## Testing

### Building Test Executables

Before running tests, ensure test executables are built:
```bash
# Build all test executables
cmake --build build --target build-tests

# Build specific category tests
cmake --build build --target semantic_expression_validation_test
cmake --build build --target parser_test_import_aliases
```

### Run All Tests
```bash
# Using CTest (recommended)
ctest --test-dir build --output-on-failure

# Using CMake targets
cmake --build build --target test
```

### Quick Tests
```bash
cmake --build build --target test-quick
```

### Category-Specific Tests
```bash
# Using CTest with labels (recommended)
ctest --test-dir build -L parser --output-on-failure
ctest --test-dir build -L semantic --output-on-failure
ctest --test-dir build -L codegen --output-on-failure

# Using CMake targets (if available)
cmake --build build --target test-lexer
cmake --build build --target test-parser
cmake --build build --target test-semantic
```

### Using CTest Directly

CTest is CMake's test runner and provides powerful options:

#### Basic Usage
```bash
# Run all tests
ctest --test-dir build

# Run with detailed output
ctest --test-dir build --output-on-failure

# Run specific test by name
ctest --test-dir build -R parser_test_import_aliases

# Run tests matching a pattern
ctest --test-dir build -R "parser.*import"

# Run tests by label
ctest --test-dir build -L parser
```

#### Advanced CTest Options
```bash
# Run tests in parallel (4 jobs)
ctest --test-dir build -j4

# Run tests with verbose output
ctest --test-dir build -V
# or extra verbose
ctest --test-dir build -VV

# Stop on first failure
ctest --test-dir build --stop-on-failure

# Retry failed tests up to 3 times
ctest --test-dir build --repeat until-pass:3

# Run tests with timeout (seconds)
ctest --test-dir build --timeout 30

# Exclude tests by pattern
ctest --test-dir build -E "slow|performance"

# Run only previously failed tests
ctest --test-dir build --rerun-failed
```

#### Test Discovery and Listing
```bash
# List all available tests
ctest --test-dir build -N

# List tests with labels
ctest --test-dir build -N -L parser

# Show test labels
ctest --test-dir build --print-labels
```

#### Test Output and Reporting
```bash
# Generate JUnit XML report
ctest --test-dir build --output-junit test-results.xml

# Save test output log
ctest --test-dir build --output-log test.log

# Run tests with custom configuration
ctest --test-dir build -C Debug
ctest --test-dir build -C Release
```

### Verbose Testing
```bash
# Using CMake target (if available)
cmake --build build --target test-verbose
# or using CTest directly
ctest --test-dir build -V --output-on-failure
```

### Parallel Testing
```bash
# Using CMake target (if available)
cmake --build build --target test-comprehensive
# or using CTest directly (recommended)
ctest --test-dir build -j$(nproc) --output-on-failure
```

### Test Selection Examples
```bash
# Run all parser tests except import tests
ctest --test-dir build -L parser -E import

# Run only failed tests from last run
ctest --test-dir build --rerun-failed --output-on-failure

# Run tests in a specific order
ctest --test-dir build -I 1,10  # Run tests 1 through 10
ctest --test-dir build -I 1,10,2  # Run every 2nd test from 1 to 10
```

### Memory Testing with Valgrind
```bash
cmake -B build -DENABLE_TEST_VALGRIND=ON
cmake --build build --target test-valgrind
# or using CTest directly
ctest --test-dir build -T memcheck
```

### Test Debugging
```bash
# Run a single test with gdb
ctest --test-dir build -R parser_test_import_aliases --verbose
gdb ./build/bin/parser_test_import_aliases

# Run test with environment variables
CTEST_OUTPUT_ON_FAILURE=1 ctest --test-dir build

# Get test command line
ctest --test-dir build -N -V -R parser_test_import_aliases
```

### CTest Quick Reference

#### Essential Commands
| Command | Description |
|---------|-------------|
| `ctest --test-dir build` | Run all tests |
| `ctest --test-dir build -N` | List tests without running |
| `ctest --test-dir build -L <label>` | Run tests with specific label |
| `ctest --test-dir build -R <regex>` | Run tests matching regex |
| `ctest --test-dir build -E <regex>` | Exclude tests matching regex |
| `ctest --test-dir build -V` | Verbose output |
| `ctest --test-dir build -VV` | Extra verbose output |
| `ctest --test-dir build --output-on-failure` | Show output only for failed tests |
| `ctest --test-dir build -j<N>` | Run N tests in parallel |
| `ctest --test-dir build --rerun-failed` | Rerun only failed tests |
| `ctest --test-dir build --stop-on-failure` | Stop on first failure |
| `ctest --test-dir build --timeout <sec>` | Set test timeout |

#### Test Labels in Asthra
- `parser` - Parser tests
- `lexer` - Lexer tests  
- `semantic` - Semantic analysis tests
- `codegen` - Code generation tests
- `runtime` - Runtime tests
- `ffi` - Foreign Function Interface tests
- `optimization` - Optimization tests
- `integration` - Integration tests

#### Common Test Patterns
```bash
# Run all parser tests
ctest --test-dir build -L parser

# Run specific test suite
ctest --test-dir build -R "parser_test_import"

# Run all tests except slow ones
ctest --test-dir build -E "performance|slow"

# Debug a failing test
ctest --test-dir build -R failing_test -VV --output-on-failure

# Run tests with custom timeout
ctest --test-dir build --timeout 120 -j4
```

## Build Targets

### Core Targets
- `asthra` - Main compiler executable
- `asthra_compiler` - Compiler library
- `asthra_runtime` - Runtime library
- `asthra_stdlib` - Standard library

### Development Tools
- `ast-visualizer` - AST visualization tool
- `grammar-validator` - Grammar validation tool
- `symbol-inspector` - Symbol table inspector
- `performance-profiler` - Performance profiling tool

### Sanitizer Variants
- `asthra-asan` - AddressSanitizer build
- `asthra-tsan` - ThreadSanitizer build
- `asthra-ubsan` - UndefinedBehaviorSanitizer build
- `asthra-msan` - MemorySanitizer build
- `asthra-coverage` - Coverage analysis build

## Installation

### Local Installation
```bash
cmake --build build --target install
```

### Custom Install Prefix
```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build --target install
```

### Package Generation
```bash
cmake --build build --target package
```

## IDE Integration

### VS Code

Create `.vscode/settings.json`:
```json
{
    "cmake.configureOnOpen": true,
    "cmake.buildDirectory": "${workspaceFolder}/build",
    "cmake.generator": "Unix Makefiles",
    "cmake.buildBeforeRun": true,
    "cmake.debugConfig": {
        "name": "Debug Asthra",
        "type": "cppdbg",
        "request": "launch",
        "program": "${workspaceFolder}/build/bin/asthra",
        "args": ["--help"],
        "stopAtEntry": false,
        "cwd": "${workspaceFolder}",
        "environment": [],
        "externalConsole": false,
        "MIMode": "gdb"
    }
}
```

### CLion

CLion has native CMake support:
1. Open the project directory
2. CLion will automatically detect CMakeLists.txt
3. Configure build profiles in Settings → Build, Execution, Deployment → CMake

### Visual Studio (Windows)

1. Open Visual Studio
2. File → Open → CMake...
3. Select the root CMakeLists.txt
4. Visual Studio will configure automatically

## Cross-Platform Builds

### Windows (MSVC)
```cmd
cmake -B build -G "Visual Studio 16 2019"
cmake --build build --config Release
```

### Windows (MinGW)
```bash
cmake -B build -G "MinGW Makefiles"
cmake --build build -j$(nproc)
```

### macOS
```bash
cmake -B build -G "Unix Makefiles"
cmake --build build -j$(sysctl -n hw.ncpu)
```

## Troubleshooting

### Test-Specific Issues

#### Tests Show "Not Run" Status
**Problem**: Tests show as "Not Run" when running `make test-<category>` or `ctest`

**Solution**: Test executables need to be built first:
```bash
# Build all test executables
cmake --build build --target build-tests

# Then run tests
ctest --test-dir build -L parser --output-on-failure
```

#### Individual Test Failures
**Problem**: A specific test is failing

**Solution**: Debug the individual test:
```bash
# Run the specific test with verbose output
ctest --test-dir build -R test_name -VV

# Run the test executable directly
./build/bin/parser_test_import_aliases

# Debug with gdb
gdb ./build/bin/parser_test_import_aliases
```

#### Test Timeout Issues
**Problem**: Tests timeout during execution

**Solution**: Increase timeout or run tests individually:
```bash
# Increase timeout to 60 seconds
ctest --test-dir build --timeout 60

# Run problematic test alone
ctest --test-dir build -R slow_test --timeout 120
```

### Common Issues

#### CMake Version Too Old
```
CMake Error: CMake 3.20 or higher is required
```
**Solution**: Update CMake to version 3.20 or later.

#### json-c Not Found
```
Could NOT find PkgConfig (missing: PKG_CONFIG_EXECUTABLE)
```
**Solution**: Install pkg-config and json-c:
```bash
# macOS
brew install pkg-config json-c

# Ubuntu/Debian
sudo apt-get install pkg-config libjson-c-dev

# CentOS/RHEL
sudo yum install pkgconfig json-c-devel
```

#### Compiler Not Found
```
No CMAKE_C_COMPILER could be found
```
**Solution**: Install a C compiler:
```bash
# Ubuntu/Debian
sudo apt-get install build-essential

# CentOS/RHEL
sudo yum groupinstall "Development Tools"

# macOS
xcode-select --install
```

### Build Failures

#### Clean Build
```bash
rm -rf build
cmake -B build
cmake --build build -j$(nproc)
```

#### Verbose Build
```bash
cmake --build build --verbose
```

#### Debug CMake Configuration
```bash
cmake -B build --debug-output
```

## Performance Optimization

### Link-Time Optimization (LTO)
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
cmake --build build
```

### Profile-Guided Optimization
```bash
# Build with profiling
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-fprofile-generate"
cmake --build build

# Run representative workload
./build/bin/asthra <typical-usage>

# Rebuild with profile data
cmake -B build -DCMAKE_C_FLAGS="-fprofile-use"
cmake --build build
```

## CMake Advantages

| Feature | CMake Benefits |
|---------|----------------|
| **Configuration** | Automatic dependency detection and cross-platform support |
| **Cross-platform** | Excellent support for Windows, macOS, and Linux |
| **IDE Integration** | Native support for VS Code, CLion, Visual Studio |
| **Dependency Management** | Automatic detection and linking of system libraries |
| **Parallel Builds** | Built-in parallel compilation support |
| **Testing Framework** | Integrated CTest with advanced filtering and reporting |
| **Package Generation** | Built-in CPack for creating installers and packages |
| **Maintenance** | Low maintenance overhead with modern CMake practices |

## Contributing

When contributing to the CMake build system:

1. **Test the build system**: Ensure changes work with CMake across platforms
2. **Update documentation**: Keep this guide current
3. **Cross-platform testing**: Test on Windows, macOS, and Linux
4. **Performance validation**: Ensure CMake builds maintain optimal performance

## Advanced Configuration

### Custom Compiler Flags
```bash
cmake -B build -DCMAKE_C_FLAGS="-march=native -O3"
```

### Custom Build Tools
```bash
cmake -B build -DCMAKE_MAKE_PROGRAM=/usr/bin/ninja -G Ninja
```

### Static Analysis Integration
```bash
cmake -B build -DCMAKE_C_CLANG_TIDY=clang-tidy
```

### Memory Debugging
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON -DENABLE_UBSAN=ON
```

## Best Practices

1. **Use out-of-source builds**: Always build in a separate `build/` directory
2. **Specify build type**: Always set `CMAKE_BUILD_TYPE` for optimal performance
3. **Use system libraries**: Prefer system libraries when available with `-DUSE_SYSTEM_*=ON`
4. **Enable warnings**: Use `-DCMAKE_C_FLAGS="-Wall -Wextra"` for better code quality
5. **Test regularly**: Run `ctest --test-dir build` after configuration changes
6. **Clean builds**: Remove `build/` directory when in doubt
7. **Use modern CMake**: Prefer `cmake -B build` over `mkdir build && cd build && cmake ..`

## Getting Help

- **CMake Documentation**: https://cmake.org/documentation/
- **Asthra Issues**: https://github.com/asthra-lang/asthra/issues
- **Build System Questions**: Tag with `build-system` label

The CMake build system provides a modern, maintainable approach to building, testing, and deploying Asthra with excellent cross-platform support and IDE integration. 