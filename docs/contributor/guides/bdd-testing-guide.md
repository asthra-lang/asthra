# BDD Testing Guide for Asthra

This guide provides comprehensive instructions for writing and running Behavior-Driven Development (BDD) tests in the Asthra programming language project. BDD tests help ensure the compiler behaves correctly from a user's perspective by describing functionality in terms of scenarios and expected outcomes.

## Table of Contents

1. [Introduction to BDD in Asthra](#introduction-to-bdd-in-asthra)
2. [Getting Started](#getting-started)
3. [Writing BDD Tests](#writing-bdd-tests)
4. [Running BDD Tests](#running-bdd-tests)
5. [Test Organization](#test-organization)
6. [Debugging BDD Tests](#debugging-bdd-tests)
7. [CI/CD Integration](#cicd-integration)
8. [Troubleshooting](#troubleshooting)

## Introduction to BDD in Asthra

Asthra's BDD testing framework combines the expressiveness of Gherkin feature files with the efficiency of C-based step definitions. This dual approach allows for:

- **Readable test scenarios** that serve as living documentation
- **Fast execution** through native C implementations
- **Comprehensive coverage** of compiler features and behaviors
- **CI/CD integration** for automated testing

The framework supports both Cucumber (when available) and a custom C-based BDD implementation, ensuring tests can run on all supported platforms.

## Getting Started

### Prerequisites

Before writing BDD tests, ensure you have:

1. **Build Tools**
   - CMake 3.20 or higher
   - C17-compatible compiler (GCC 11+, Clang 14+, or MSVC 2019+)
   - Make or Ninja build system

2. **Optional Dependencies**
   - Ruby and Cucumber (for Gherkin feature files)

### Setting Up Your Environment

1. **Clone the repository**
   ```bash
   git clone https://github.com/asthra-lang/asthra.git
   cd asthra
   ```

2. **Enable BDD tests in your build**
   ```bash
   cmake -B build -DBUILD_BDD_TESTS=ON
   cmake --build build
   ```

3. **Verify installation**
   ```bash
   cmake --build build --target bdd_tests
   cmake --build build --target run_bdd_tests
   ```

## Writing BDD Tests

Asthra supports two approaches to writing BDD tests:

### C-Based BDD Tests

C-based tests use our custom BDD support library and are the primary testing approach.

#### Basic Structure

```c
#include "bdd_support.h"
#include "compiler.h"
#include <string.h>

int main(void) {
    bdd_init("Compiler Basic Functionality");
    
    bdd_scenario("Compile a simple program");
    
    // Given
    bdd_given("a valid Asthra source file");
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> void {\n"
        "    return ();\n"
        "}\n";
    
    // When
    bdd_when("the compiler processes the file");
    CompilerResult result = compile_string(source);
    
    // Then
    bdd_then("compilation should succeed");
    BDD_ASSERT_TRUE(result.success);
    BDD_ASSERT_NULL(result.error);
    
    return bdd_report();
}
```

#### Step-by-Step Guide

1. **Include necessary headers**
   ```c
   #include "bdd_support.h"    // BDD framework
   #include "compiler.h"       // Component under test
   ```

2. **Initialize the BDD framework**
   ```c
   bdd_init("Feature Name");
   ```

3. **Define scenarios**
   ```c
   bdd_scenario("Clear description of what you're testing");
   ```

4. **Write Given-When-Then steps**
   ```c
   bdd_given("initial state or precondition");
   // Set up test data
   
   bdd_when("action is performed");
   // Execute the code under test
   
   bdd_then("expected outcome");
   // Assert expectations
   ```

5. **Use appropriate assertions**
   ```c
   BDD_ASSERT_TRUE(condition);
   BDD_ASSERT_FALSE(condition);
   BDD_ASSERT_EQ(expected, actual);
   BDD_ASSERT_NEQ(expected, actual);
   BDD_ASSERT_NULL(pointer);
   BDD_ASSERT_NOT_NULL(pointer);
   BDD_ASSERT_STR_EQ(expected_str, actual_str);
   BDD_ASSERT_STR_CONTAINS(haystack, needle);
   ```

6. **Report results**
   ```c
   return bdd_report();
   ```

#### Complex Example with Multiple Scenarios

```c
#include "bdd_support.h"
#include "parser.h"
#include "semantic.h"

int main(void) {
    bdd_init("Expression Type Inference");
    
    // Scenario 1: Integer literals
    bdd_scenario("Integer literal type inference");
    bdd_given("an integer literal expression");
    const char* int_source = "42";
    
    bdd_when("parsing and analyzing the expression");
    ParseResult parse = parse_expression(int_source);
    BDD_ASSERT_TRUE(parse.success);
    
    SemanticResult sem = analyze_expression(parse.ast);
    
    bdd_then("the type should be inferred as int");
    BDD_ASSERT_TRUE(sem.success);
    BDD_ASSERT_STR_EQ(sem.type->name, "int");
    
    // Scenario 2: Binary expressions
    bdd_scenario("Binary expression type inference");
    bdd_given("a binary addition expression");
    const char* binary_source = "10 + 20";
    
    bdd_when("parsing and analyzing the expression");
    parse = parse_expression(binary_source);
    BDD_ASSERT_TRUE(parse.success);
    
    sem = analyze_expression(parse.ast);
    
    bdd_then("the type should be inferred from operands");
    BDD_ASSERT_TRUE(sem.success);
    BDD_ASSERT_STR_EQ(sem.type->name, "int");
    
    // Scenario 3: Type mismatch
    bdd_scenario("Type mismatch in binary expression");
    bdd_given("a binary expression with mismatched types");
    const char* mismatch_source = "10 + \"string\"";
    
    bdd_when("parsing and analyzing the expression");
    parse = parse_expression(mismatch_source);
    BDD_ASSERT_TRUE(parse.success);
    
    sem = analyze_expression(parse.ast);
    
    bdd_then("semantic analysis should fail with type error");
    BDD_ASSERT_FALSE(sem.success);
    BDD_ASSERT_NOT_NULL(sem.error);
    BDD_ASSERT_STR_CONTAINS(sem.error, "type mismatch");
    
    return bdd_report();
}
```

### Gherkin Feature Files (Optional)

When Cucumber is available, you can write feature files in Gherkin syntax:

```gherkin
Feature: Function Compilation
  As a developer
  I want to compile functions with various signatures
  So that I can build modular programs

  Background:
    Given the Asthra compiler is initialized

  Scenario: Compile a simple void function
    Given the following Asthra source:
      """
      package test;
      
      pub fn greet(none) -> void {
          return ();
      }
      """
    When I compile the source
    Then compilation should succeed
    And the function "greet" should be defined
    And the function should have return type "void"

  Scenario: Compile a function with parameters
    Given the following Asthra source:
      """
      package test;
      
      pub fn add(a: int, b: int) -> int {
          return a + b;
      }
      """
    When I compile the source
    Then compilation should succeed
    And the function "add" should accept 2 parameters
    And parameter 0 should have type "int"
    And parameter 1 should have type "int"

  Scenario Outline: Invalid function definitions
    Given the following Asthra source:
      """
      package test;
      <source>
      """
    When I compile the source
    Then compilation should fail
    And the error should contain "<error>"

    Examples:
      | source | error |
      | fn missing_return_type() {} | return type |
      | pub fn (none) -> void {} | function name |
      | pub fn duplicate(a: int, a: int) -> void {} | duplicate parameter |
```

## Running BDD Tests

### Build All BDD Tests

```bash
cmake --build build --target bdd_tests
```

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

# Acceptance tests only
cmake --build build --target bdd_acceptance
```

### Run Individual Test Executables

```bash
# Run a specific test
./build/bdd/bin/test_parser_expressions

# Run with verbose output
./build/bdd/bin/test_parser_expressions -v

# Run specific scenarios
./build/bdd/bin/test_parser_expressions -s "Binary expression"

# Running specific BDD feature files
./build/bdd/bin/bdd_unit_compiler_basic        # Run compiler basic functionality tests
./build/bdd/bin/bdd_unit_function_calls        # Run function call tests
./build/bdd/bin/bdd_unit_if_conditions         # Run if condition tests
./build/bdd/bin/bdd_unit_bitwise_operators     # Run bitwise operator tests
./build/bdd/bin/bdd_integration_cli            # Run CLI integration tests
./build/bdd/bin/bdd_integration_ffi_integration # Run FFI integration tests
```

### BDD Test Output Interpretation

- ✓ = Test passed
- ✗ = Test failed
- Each scenario shows step-by-step execution with assertions
- Summary shows: Passed/Failed/Total counts at the end

Example output:
```
Feature: Basic Compiler Functionality
  Scenario: Compile and run a simple Hello World program
    Given the Asthra compiler is available
      ✓ exists should be true
    When I compile the file
    Then the compilation should succeed
      ✓ compilation_exit_code should equal 0
```

### Generate Test Reports

```bash
# Generate HTML report
cmake --build build --target bdd-report

# View report
open build/bdd/reports/bdd-report.html
```

## Test Organization

### Directory Structure

```
bdd/
├── features/              # Gherkin feature files (*.feature)
│   ├── compiler/         # Compiler features
│   ├── parser/           # Parser features
│   └── semantic/         # Semantic analysis features
├── steps/                # Step definitions (C implementations)
│   ├── unit/            # Unit test steps
│   │   ├── parser/      # Parser unit tests
│   │   ├── semantic/    # Semantic unit tests
│   │   └── codegen/     # Code generation unit tests
│   ├── integration/     # Integration test steps
│   ├── acceptance/      # Acceptance test steps
│   ├── scenario/        # Scenario test steps
│   └── feature/         # Feature test steps
├── support/             # Shared test support code
├── fixtures/            # Test data and examples
└── CMakeLists.txt       # BDD test configuration

Test executables are built to: build/bdd/bin/bdd_*
Scenarios marked with @wip are skipped during test runs.
```

### Test Categories

1. **Unit Tests** (`steps/unit/`)
   - Test individual components in isolation
   - Fast execution, focused scope
   - Example: Testing parser for specific syntax elements

2. **Integration Tests** (`steps/integration/`)
   - Test interaction between components
   - Example: Parser + Semantic Analysis integration

3. **Acceptance Tests** (`steps/acceptance/`)
   - End-to-end compiler functionality
   - Example: Complete compilation of valid programs

4. **Scenario Tests** (`steps/scenario/`)
   - Complex use cases and edge cases
   - Example: Error recovery scenarios

5. **Feature Tests** (`steps/feature/`)
   - Complete language feature validation
   - Example: Full pattern matching implementation

### Naming Conventions

- Test files: `test_<component>_<aspect>.c`
  - Example: `test_parser_expressions.c`
- Feature files: `<component>_<feature>.feature`
  - Example: `parser_expressions.feature`
- Test functions: Descriptive scenario names
  - Example: "Parse binary arithmetic expressions"

### Test Artifacts and Debugging Environment

To preserve test artifacts for debugging:
```bash
export BDD_KEEP_ARTIFACTS=1  # Preserve test artifacts in bdd-temp/
```

Clean test execution pattern:
```c
// Always call cleanup before reporting
common_cleanup();
return bdd_report();
```

## Debugging BDD Tests

### Enable Debug Output

```c
// In your test file
bdd_set_debug(true);

// Or via environment variable
export BDD_DEBUG=1
```

### Common Debugging Techniques

1. **Add debug assertions**
   ```c
   bdd_when("parsing expression");
   ParseResult result = parse_expression(source);
   
   // Debug output
   if (!result.success) {
       printf("Parse error: %s\n", result.error);
       printf("At position: %d\n", result.error_pos);
   }
   
   BDD_ASSERT_TRUE(result.success);
   ```

2. **Use step-through debugging**
   ```bash
   # Build with debug symbols
   cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_BDD_TESTS=ON
   cmake --build build
   
   # Debug with gdb/lldb
   gdb ./build/bdd/bin/test_parser_expressions
   ```

3. **Isolate failing scenarios**
   ```c
   // Temporarily skip other scenarios
   if (0) {
       bdd_scenario("Scenario to skip");
       // ...
   }
   
   // Focus on failing scenario
   bdd_scenario("Failing scenario");
   // ...
   ```

### Memory Debugging

```bash
# Run with AddressSanitizer
cmake -B build -DSANITIZER=Address -DBUILD_BDD_TESTS=ON
cmake --build build
./build/bdd/bin/test_parser_expressions

# Run with Valgrind (Linux)
valgrind --leak-check=full ./build/bdd/bin/test_parser_expressions
```

## CI/CD Integration

BDD tests are automatically run in CI through GitHub Actions:

### Workflow Configuration

The `.github/workflows/bdd.yml` workflow:
- Runs on every pull request and push to main
- Tests on Ubuntu 22.04 and macOS 14
- Installs dependencies automatically
- Generates and uploads test reports

### Adding New Tests to CI

1. Place your test in the appropriate directory
2. Update `bdd/CMakeLists.txt` to include your test
3. Ensure your test follows the naming conventions
4. The CI will automatically pick up and run your test

### Viewing CI Results

1. Go to the Actions tab in GitHub
2. Click on the BDD workflow run
3. Download artifacts for detailed test reports
4. Check the logs for specific failures

## Troubleshooting

### Common Issues and Solutions

#### 1. BDD Tests Not Found

**Problem**: CMake doesn't find BDD tests
```
No tests were found!!!
```

**Solution**: Ensure BDD tests are enabled
```bash
cmake -B build -DBUILD_BDD_TESTS=ON
cmake --build build --target bdd_tests
```

#### 2. Cucumber Not Available

**Problem**: Cucumber features not running
```
Cucumber not found, skipping feature tests
```

**Solution**: Install Ruby and Cucumber
```bash
# Ubuntu/Debian
sudo apt-get install ruby-full
sudo gem install cucumber

# macOS
brew install ruby
gem install cucumber
```

#### 3. Compilation Errors in Tests

**Problem**: Test won't compile
```
error: implicit declaration of function 'compile_string'
```

**Solution**: Include necessary headers and link libraries
```c
#include "compiler.h"  // Add missing include
```

Update `CMakeLists.txt`:
```cmake
target_link_libraries(test_name
    asthra_compiler
    bdd_support
)
```

#### 4. Test Failures

**Problem**: Tests fail unexpectedly
```
Assertion failed: result.success
```

**Solution**: Debug the test
1. Enable debug output: `export BDD_DEBUG=1`
2. Add debug prints before assertions
3. Check test fixtures for validity
4. Ensure proper test data setup

#### 5. Memory Leaks

**Problem**: Valgrind reports memory leaks
```
definitely lost: 1,024 bytes in 1 blocks
```

**Solution**: Ensure proper cleanup
```c
// Free allocated resources
free(result.data);
ast_node_free(parse.ast);
```

### Platform-Specific Issues

#### Linux
- Ensure GCC 11+ or Clang 14+ is installed
- Install development headers: `sudo apt-get install build-essential`

#### macOS
- Use Homebrew for dependencies
- Ensure Xcode Command Line Tools are installed: `xcode-select --install`

#### Windows
- BDD tests currently not supported on Windows
- Use WSL2 for Linux-based testing

### Getting Help

If you encounter issues not covered here:

1. Check existing issues: https://github.com/asthra-lang/asthra/issues
2. Ask in discussions: https://github.com/asthra-lang/asthra/discussions
3. File a bug report with:
   - Platform and compiler version
   - Complete error message
   - Minimal test case reproducing the issue

## Best Practices

1. **Keep tests focused**: Each scenario should test one behavior
2. **Use descriptive names**: Scenario names should clearly indicate what's being tested
3. **Follow Asthra patterns**: Respect expression-oriented design in test code
4. **Clean up resources**: Always free allocated memory and close files
5. **Test both success and failure**: Include negative test cases
6. **Maintain test fixtures**: Keep test data organized and up-to-date

## Conclusion

BDD testing in Asthra provides a powerful way to ensure compiler correctness while maintaining readable, maintainable tests. By following this guide, you can contribute high-quality tests that help maintain Asthra's reliability and correctness.

Remember that tests are not just about finding bugs—they're about documenting expected behavior and preventing regressions. Well-written BDD tests serve as living documentation for the Asthra compiler.