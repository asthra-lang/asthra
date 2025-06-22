# Running BDD Tests Locally

This guide explains how to run Behavior-Driven Development (BDD) tests locally for the Asthra project.

## Prerequisites

- CMake 3.10 or higher
- C compiler with C17 support
- CGreen testing framework (version 1.6.3 or higher)

## Quick Start

To run all BDD tests locally:

```bash
# 1. Configure build with BDD tests enabled
cmake -B build -DBUILD_BDD_TESTS=ON

# 2. Build BDD test executables
cmake --build build --target bdd_tests

# 3. Run all BDD tests
cmake --build build --target run_bdd_tests
```

## Available BDD Test Targets

### Run All BDD Tests
```bash
cmake --build build --target run_bdd_tests
```

### Run Specific Test Categories

```bash
# Unit tests only
cmake --build build --target bdd_unit

# Integration tests only
cmake --build build --target bdd_integration

# Cucumber-style features (or C-based equivalents)
cmake --build build --target bdd_cucumber
```

### Alternative: Using CTest

You can also run BDD tests through CTest:

```bash
# Run all BDD tests
ctest --test-dir build -L bdd

# Run with verbose output
ctest --test-dir build -L bdd --verbose

# Run in parallel for faster execution
ctest --test-dir build -L bdd -j8
```

## Generate Test Reports

To generate a JUnit-format test report:

```bash
cmake --build build --target bdd-report
```

The report will be generated in the build directory and can be used for CI/CD integration or test result visualization.

## Troubleshooting

### BDD Tests Not Found

If BDD test targets are not available:

1. Ensure you configured the build with BDD tests enabled:
   ```bash
   cmake -B build -DBUILD_BDD_TESTS=ON
   ```

2. Clean and rebuild if necessary:
   ```bash
   cmake --build build --target clean
   cmake -B build -DBUILD_BDD_TESTS=ON
   cmake --build build --target bdd_tests
   ```

### Test Failures

For detailed output on test failures:

```bash
# Use CTest with output on failure
ctest --test-dir build -L bdd --output-on-failure

# Or run specific test category with verbose output
cmake --build build --target bdd_unit -- VERBOSE=1
```

### Missing Dependencies

If CGreen is not found:

1. On macOS with Homebrew:
   ```bash
   brew install cgreen
   ```

2. On Ubuntu/Debian:
   ```bash
   sudo apt-get install libcgreen1-dev
   ```

3. On other systems, build from source:
   ```bash
   git clone https://github.com/cgreen-devs/cgreen.git
   cd cgreen
   cmake -B build
   cmake --build build
   sudo cmake --build build --target install
   ```

## Best Practices

1. **Always build test executables first**: Before running tests, ensure all test executables are built:
   ```bash
   cmake --build build --target bdd_tests
   ```

2. **Run tests frequently**: Run BDD tests after making changes to ensure functionality remains intact.

3. **Use parallel execution**: For faster test runs, use parallel execution:
   ```bash
   ctest --test-dir build -L bdd -j8
   ```

4. **Check test reports**: Generate and review test reports regularly to track test coverage and identify patterns in failures.

## Integration with Development Workflow

When developing new features:

1. Write BDD tests first (following TDD/BDD principles)
2. Configure and build with BDD tests enabled
3. Run tests to see them fail
4. Implement the feature
5. Run tests again to verify they pass
6. Generate reports for documentation

## Related Documentation

- [BDD Testing Guide](./bdd-testing-guide.md) - Comprehensive guide to BDD testing in Asthra
- [Test Build Strategy](./test-build-strategy.md) - General test building and execution strategies
- [CI/CD Integration](../workflows/ci-cd-integration.md) - How BDD tests integrate with CI/CD