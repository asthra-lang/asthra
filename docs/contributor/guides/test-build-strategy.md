# Asthra Test Build Strategy (CMake)

This document describes the proven approach for building and running Asthra tests using CMake, addressing common build issues that arise from the separation of test building and main compilation.

## The Problem

When fixing test failures in one component, using `rm -rf build && cmake -B build` causes ALL test executables to be deleted but NOT rebuilt automatically. This leads to a cascade of test failures in other components that appear broken but are actually just missing their executables.

## The Solution: Targeted Test Building

### Key Principle
**Never use `rm -rf build` when fixing tests.** Instead, use incremental builds with explicit test building.

### The Proven Approach

1. **For all test work, use:**
   ```bash
   cmake --build build -j8 && cmake --build build --target build-tests
   ```
   Or for faster iteration:
   ```bash
   cmake --build build --target build-tests -j8
   ```

2. **For component-specific testing:**
   ```bash
   # Build specific test executables
   cmake --build build --target semantic_type_checking_test semantic_mutability_test -j8
   
   # Or build all tests then run specific category
   cmake --build build --target build-tests
   ctest --test-dir build -L semantic --output-on-failure
   ```

3. **For quick iteration (using test runner script):**
   ```bash
   ./run-tests.sh core
   ./run-tests.sh cross_cutting
   ./run-tests.sh infrastructure
   ```

## Why This Works

### Architecture Understanding
- The CMake build system **deliberately separates** test building from main compilation
- This design choice optimizes build times during regular development
- Test executables are placed in `${CMAKE_BINARY_DIR}/bin/` (typically `build/bin/`)
- CTest is used for test execution with label-based filtering

### Benefits
1. **No unnecessary cleaning** - Existing test binaries remain available
2. **Faster builds** - Only changed files are recompiled
3. **Parallel building** - Tests can be built concurrently with `-j8`
4. **Selective building** - Build only the tests you need
5. **Modern tooling** - Uses CMake and CTest best practices

## Common Patterns and Fixes

### Pattern 1: Missing Test Executables
**Symptom:**
```
Unable to find executable: /path/to/test_executable
```

**Fix:**
```bash
# Option 1: Build all tests
cmake --build build --target build-tests

# Option 2: Build specific test
cmake --build build --target test_executable_name
```

### Pattern 2: Path Resolution Issues
**Symptom:**
Tests fail because they can't find required files (e.g., `.asthra` files in stdlib tests).

**Fix:**
- Test executables run from `bin/` directory
- Use correct relative paths:
  - `../stdlib/` NOT `../../stdlib/`
  - `../tests/` NOT `../../tests/`

### Pattern 3: CMake Configuration Issues
**Symptom:**
```
CMake Error: The source directory "/path" does not contain a CMakeLists.txt file.
```

**Fix:**
Ensure you're running CMake commands from the project root and using proper build directory:
```bash
# Configure build (only needed once or after major changes)
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Build (incremental)
cmake --build build -j8
```

## Test Categories and Special Cases

### Categories with Custom CMakeLists.txt
These categories have special build rules and dependencies:
- `codegen`
- `concurrency`
- `core`
- `ffi`
- `immutable_by_default`
- `integration`
- `memory`
- `optimization`
- `parser`
- `patterns`
- `performance`
- `runtime`
- `security`
- `semantic`

### Simple Test Categories
These use auto-discovery and standard build rules:
- `ai_annotations`
- `ai_api`
- `ai_linter`
- `basic`
- `fast_check`
- `io`
- `lexer`
- `linker`
- `pipeline`
- `platform`
- `stdlib`
- `struct`
- `types`

## Troubleshooting

### Issue: Tests were passing, now they all fail
**Cause:** You ran `rm -rf build` which deleted all test executables
**Solution:** Run `cmake --build build --target build-tests` to rebuild all test executables

### Issue: CMake can't find libraries
**Common causes:**
1. Missing system dependencies (json-c, pthread)
2. Incorrect CMAKE_PREFIX_PATH
3. Missing CMake configuration

**Solutions:**
```bash
# Install system dependencies (macOS)
brew install json-c pkg-config

# Install system dependencies (Ubuntu)
sudo apt-get install libjson-c-dev pkg-config

# Reconfigure if needed
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DUSE_SYSTEM_JSON_C=ON
```

### Issue: Tests can't find required files
**Cause:** Incorrect relative paths from test executable location
**Solution:** Remember tests run from `bin/` directory, adjust paths accordingly

### Issue: CTest can't find tests
**Cause:** Test executables not built or CTest configuration issues
**Solution:**
```bash
# List available tests
ctest --test-dir build -N

# Build missing tests
cmake --build build --target build-tests

# Verify test labels
ctest --test-dir build --print-labels
```

## Best Practices

1. **Always commit working tests** - Run `cmake --build build --target build-tests && ctest --test-dir build` before committing
2. **Use parallel builds** - Add `-j8` (or appropriate number) for faster builds
3. **Document dependencies** - If a test needs special linking, document it in CMakeLists.txt
4. **Keep tests isolated** - Avoid dependencies between test files when possible
5. **Use the test runner script** - `./run-tests.sh` handles the build and test process automatically
6. **Prefer incremental builds** - Only do clean rebuilds when absolutely necessary

## Quick Reference

```bash
# Build everything including all tests
cmake --build build -j8 && cmake --build build --target build-tests

# Build and run specific test category
cmake --build build --target build-tests && ctest --test-dir build -L semantic --output-on-failure

# Quick test execution using script
./run-tests.sh core
./run-tests.sh semantic

# Build specific test executables
cmake --build build --target semantic_mutability_test parser_test_basic -j8

# Run all tests
cmake --build build --target build-tests && ctest --test-dir build

# Check what tests exist for a category
ctest --test-dir build -N -L semantic

# Build without clean (recommended)
cmake --build build --target build-tests -j8

# Clean build (only when necessary)
rm -rf build && cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target build-tests
```

## CMake vs Make Comparison

| Task | Old Make Command | New CMake Command |
|------|------------------|-------------------|
| Build all | `make -j8` | `cmake --build build -j8` |
| Build tests | `make build-tests` | `cmake --build build --target build-tests` |
| Run tests | `make test-semantic` | `ctest --test-dir build -L semantic` |
| Clean build | `make clean` | `rm -rf build && cmake -B build` |
| Quick iteration | `make test-semantic/fast` | `./run-tests.sh semantic` |

## Using the Test Runner Script

The `./run-tests.sh` script provides a high-level interface that handles CMake building and CTest execution:

```bash
# Test core compilation pipeline (lexer, parser, semantic, codegen, etc.)
./run-tests.sh core

# Test cross-cutting components (FFI, concurrency, memory, etc.)
./run-tests.sh cross_cutting

# Test infrastructure components (framework, AI tools, diagnostics)
./run-tests.sh infrastructure

# Test everything
./run-tests.sh all
```

The script automatically:
- Configures CMake if needed
- Builds compiler and test executables
- Runs tests in hierarchical order
- Stops on first component failure for core tests
- Continues through all components for cross-cutting tests
- Provides colored output and detailed logs

## Conclusion

The key insight is that Asthra's CMake build system is designed for efficiency by not building tests by default. This is a feature, not a bug. By understanding this design and using `cmake --build build --target build-tests` appropriately, you can maintain a smooth development workflow without the cascade failures caused by `rm -rf build`.

The modern CMake approach provides better cross-platform support, IDE integration, and testing capabilities compared to the previous Make-based system.