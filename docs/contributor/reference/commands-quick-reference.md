# Asthra Commands Quick Reference

This is a comprehensive reference for all commands used in Asthra development. For the most commonly used commands, see the "Essential Commands" section in CLAUDE.md.

## Build Commands

### Basic Build Operations
```bash
cmake -B build && cmake --build build          # Build compiler with optimizations (default)
cmake --build build --target clean             # Clean all build artifacts
cmake -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build    # Build with debug symbols
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build  # Build optimized release
```

### Platform-Specific Builds
```bash
# macOS Universal Binary
cmake -B build -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"

# Linux with specific compiler
CC=gcc-11 CXX=g++-11 cmake -B build

# Windows with MSVC
cmake -B build -G "Visual Studio 17 2022"
```

## Testing Commands

### Core Testing
```bash
ctest --test-dir build                          # Run all tests
ctest --test-dir build --verbose               # Comprehensive test suite with reporting
cmake --build build --target build-tests       # Build all test executables (required after clean)
```

### Component-Specific Testing
```bash
# Run tests for specific component (component must exist under tests/)
ctest --test-dir build -L <component-name>     # Example: ctest -L semantic, ctest -L parser

# Sub-category testing (available for some components)
ctest --test-dir build -R "codegen.*function"  # Run only function call tests
ctest --test-dir build -R "codegen.*control"   # Run only control flow tests
ctest --test-dir build -R "parser.*expression" # Run only expression parser tests
ctest --test-dir build -R "parser.*statement"  # Run only statement parser tests
ctest --test-dir build -R "semantic.*types"    # Run only type checking tests
ctest --test-dir build -R "semantic.*symbols"  # Run only symbol resolution tests
```

### Parallel Testing
```bash
ctest --test-dir build -j8                     # Run tests in parallel with 8 threads
ctest --test-dir build -L <component> -j8      # Component tests in parallel
ctest --test-dir build --parallel 16           # Use 16 parallel jobs
```

### Test Output and Reporting
```bash
ctest --test-dir build --output-on-failure     # View test failures with output
ctest --test-dir build --verbose               # Verbose output for all tests
ctest --test-dir build --rerun-failed          # Re-run only failed tests
ctest --test-dir build --output-junit report.xml # Generate JUnit XML report
```

## BDD Testing Commands

### Build and Run BDD Tests
```bash
cmake -B build -DBUILD_BDD_TESTS=ON            # Enable BDD tests in build configuration
cmake --build build --target bdd_tests         # Build all BDD test executables
cmake --build build --target run_bdd_tests     # Run full BDD test suite
cmake --build build --target bdd_unit          # Run C BDD unit tests only
cmake --build build --target bdd_integration   # Run BDD integration tests only
cmake --build build --target bdd_cucumber      # Run Cucumber features (if available)
cmake --build build --target bdd-report        # Generate BDD test report (JUnit format)
ctest --test-dir build -L bdd                  # Run BDD tests using CTest
```

### Running Individual BDD Tests
```bash
./build/bdd/bin/bdd_unit_compiler_basic        # Run compiler basic functionality tests
./build/bdd/bin/bdd_unit_function_calls        # Run function call tests
./build/bdd/bin/bdd_unit_if_conditions         # Run if condition tests
./build/bdd/bin/bdd_unit_bitwise_operators     # Run bitwise operator tests
./build/bdd/bin/bdd_integration_cli            # Run CLI integration tests
./build/bdd/bin/bdd_integration_ffi_integration # Run FFI integration tests
```

### BDD Environment Variables
```bash
export BDD_DEBUG=1                             # Enable debug output for BDD tests
export BDD_KEEP_ARTIFACTS=1                    # Preserve test artifacts in bdd-temp/
export BDD_TIMEOUT=60                          # Set test timeout in seconds
```

## Coverage and Analysis

### Code Coverage
```bash
cmake --build build --target coverage          # Generate coverage reports
cmake --build build --target coverage-html     # HTML coverage reports
cmake --build build --target coverage-summary  # Summary coverage report

# View coverage
open build/coverage/index.html                 # macOS
xdg-open build/coverage/index.html            # Linux
```

### Static Analysis
```bash
cmake --build build --target analyze           # Run static analysis with Clang
cmake --build build --target cppcheck         # Run cppcheck (if available)
cmake --build build --target pvs-studio       # Run PVS-Studio (if licensed)
```

### Code Formatting
```bash
cmake --build build --target format            # Format all source files
cmake --build build --target format-check      # Check formatting without modifying
```

## Performance and Benchmarking

```bash
cmake --build build --target benchmark         # Run optimized benchmarks
cmake --build build --target perf-test        # Run performance regression tests
cmake --build build --target profile          # Build with profiling enabled
```

## Sanitizer Builds

```bash
# AddressSanitizer (memory errors, leaks)
cmake -B build -DSANITIZER=Address
cmake --build build && ./build/asthra_test

# ThreadSanitizer (data races)
cmake -B build -DSANITIZER=Thread
cmake --build build && ./build/asthra_test

# UndefinedBehaviorSanitizer
cmake -B build -DSANITIZER=Undefined
cmake --build build && ./build/asthra_test

# MemorySanitizer (uninitialized memory)
cmake -B build -DSANITIZER=Memory
cmake --build build && ./build/asthra_test
```

## Build Tool Commands (Ampu)

```bash
cd ampu && cargo build                         # Build the Ampu build tool
cd ampu && cargo test                          # Run Ampu unit tests
./ampu/run_e2e_tests.sh                       # End-to-end tests
cargo test --test language_tests              # Language feature tests
cargo run -- build                            # Build project using Ampu
cargo run -- test                             # Run tests using Ampu
cargo run -- clean                            # Clean using Ampu
```

## Documentation Generation

```bash
cmake --build build --target docs              # Generate documentation
cmake --build build --target doxygen          # Generate Doxygen docs
cmake --build build --target man              # Generate man pages
```

## Installation Commands

```bash
cmake --build build --target install          # Install to system
cmake --install build --prefix ~/asthra       # Install to custom location
cmake --build build --target package          # Create distribution package
```

## Development Utilities

```bash
# Generate compile_commands.json for IDE support
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build with verbose output
cmake --build build --verbose

# Clean and rebuild everything
cmake --build build --target clean && cmake --build build

# Check dependencies
cmake --build build --target deps-check

# Update git submodules
git submodule update --init --recursive
```

## Debugging Commands

```bash
# Build with debug symbols
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Run with gdb
gdb ./build/asthra

# Run with lldb
lldb ./build/asthra

# Generate core dumps
ulimit -c unlimited
./build/asthra_test

# Analyze core dump
gdb ./build/asthra core
```

## CI/CD Commands

```bash
# Run CI locally
act -j build                                  # Using GitHub Actions locally

# Pre-commit checks
./scripts/pre-commit.sh                       # Run all pre-commit checks

# Release build
./scripts/release.sh --version 1.0.0         # Create release build
```

## Tips and Tricks

### Incremental Builds
Always prefer incremental builds over clean builds:
```bash
cmake --build build                           # Incremental build (fast)
# vs
cmake --build build --target clean && cmake --build build  # Full rebuild (slow)
```

### Parallel Builds
Use parallel jobs to speed up compilation:
```bash
cmake --build build -j$(nproc)               # Linux: use all cores
cmake --build build -j$(sysctl -n hw.ncpu)   # macOS: use all cores
cmake --build build -j8                      # Manual: use 8 cores
```

### Build-Test Workflow
The recommended workflow for making changes:
```bash
# 1. Make your changes
# 2. Build incrementally
cmake --build build -j8

# 3. Build test executables if needed
cmake --build build --target build-tests

# 4. Run relevant tests
ctest --test-dir build -L semantic -j8       # Fast component testing

# 5. Run full test suite before committing
ctest --test-dir build
```

## See Also

- `docs/contributor/guides/test-build-strategy.md` - Critical test building strategies
- `docs/contributor/guides/bdd-testing-guide.md` - Comprehensive BDD testing guide
- `docs/contributor/guides/development-guide.md` - General development guidelines
- `CLAUDE.md` - Essential commands for AI-assisted development