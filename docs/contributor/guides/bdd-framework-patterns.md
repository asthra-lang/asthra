# BDD Framework Usage Patterns Guide

This guide documents the new BDD framework patterns introduced to streamline BDD test development in the Asthra programming language project. The new framework provides a declarative, reusable approach that reduces code duplication by 68-75% while maintaining full compatibility with existing tests.

## Table of Contents

1. [Framework Overview](#framework-overview)
2. [Declarative Test Pattern](#declarative-test-pattern)
3. [BDD Utilities API](#bdd-utilities-api)
4. [Work-in-Progress (@wip) Handling](#work-in-progress-wip-handling)
5. [Test Suite Execution](#test-suite-execution)
6. [Pattern Examples](#pattern-examples)
7. [Migration from Legacy Pattern](#migration-from-legacy-pattern)
8. [Best Practices](#best-practices)
9. [Troubleshooting](#troubleshooting)

## Framework Overview

The new BDD framework introduces three core components:

### Core Components

1. **bdd_test_framework.c/h** - Declarative test execution engine
2. **bdd_utilities.c/h** - Reusable utility functions for common operations
3. **bdd_support.h** - BDD assertion and step definition macros

### Key Benefits

- **68-75% code reduction** across all BDD test files
- **Consistent output formatting** across all tests
- **Proper @wip scenario handling** for CI/CD environments
- **Reusable utility functions** eliminating code duplication
- **Declarative test definition** improving maintainability

## Declarative Test Pattern

The new framework uses a declarative approach where tests are defined as metadata arrays rather than imperative code.

### Basic Structure

```c
#include "bdd_test_framework.h"
#include "bdd_utilities.h"

// Step function implementations
void test_basic_functionality(void) {
    bdd_given("a simple test condition");
    
    bdd_when("performing the test action");
    
    bdd_then("the expected result occurs");
    bdd_assert(1 == 1, "Basic assertion should pass");
}

void test_wip_feature(void) {
    bdd_given("a work-in-progress feature");
    
    bdd_when("testing unimplemented functionality");
    
    bdd_then("proper WIP handling occurs");
    bdd_skip_scenario("Feature not implemented yet");
}

// Declarative test case definitions
BddTestCase test_cases[] = {
    BDD_TEST_CASE(basic_functionality, test_basic_functionality),
    BDD_WIP_TEST_CASE(wip_feature, test_wip_feature),
};

// Main function using framework
int main(void) {
    return bdd_run_test_suite(
        "Feature Name",
        test_cases,
        sizeof(test_cases) / sizeof(test_cases[0]),
        bdd_cleanup_temp_files
    );
}
```

### Macro Definitions

```c
// Regular test case (always runs)
#define BDD_TEST_CASE(name, func) {#name, func, 0}

// Work-in-progress test case (skipped in CI when BDD_SKIP_WIP=1)
#define BDD_WIP_TEST_CASE(name, func) {#name, func, 1}
```

## BDD Utilities API

The BDD utilities provide reusable functions for common testing operations:

### Command Execution

```c
// Execute shell command and return output
char* bdd_execute_command(const char* command, int* exit_code);

// Example usage
int exit_code;
char* output = bdd_execute_command("echo 'test'", &exit_code);
bdd_assert_output_contains(output, "test");
bdd_cleanup_string(&output);
```

### File Operations

```c
// Read file contents into string
char* bdd_read_file_contents(const char* filename);

// Write content to file
int bdd_write_file_contents(const char* filename, const char* content);

// Create temporary source file
void bdd_create_temp_source_file(const char* filename, const char* content);

// Example usage
bdd_create_temp_source_file("test.asthra", 
    "package main;\n"
    "pub fn main(none) -> void { return (); }\n");
```

### Compiler Utilities

```c
// Find Asthra compiler executable
const char* bdd_find_asthra_compiler(void);

// Check if compiler is available
int bdd_compiler_available(void);

// Compile source file
int bdd_compile_source_file(const char* source_file, 
                           const char* output_file, 
                           const char* flags);

// Example usage
if (bdd_compiler_available()) {
    const char* compiler = bdd_find_asthra_compiler();
    int result = bdd_compile_source_file("test.asthra", "test", "-O2");
    bdd_assert(result == 0, "Compilation should succeed");
}
```

### Output Validation

```c
// Check if output contains expected text
int bdd_output_contains(const char* output, const char* expected);

// Assert output contains text (with automatic failure)
void bdd_assert_output_contains(const char* output, const char* expected);

// Assert output does not contain text
void bdd_assert_output_not_contains(const char* output, const char* unexpected);

// Example usage
char* output = bdd_execute_command("./test", &exit_code);
bdd_assert_output_contains(output, "Hello World");
bdd_assert_output_not_contains(output, "ERROR");
```

### Resource Management

```c
// Clean up single string pointer
void bdd_cleanup_string(char** str);

// Clean up array of strings
void bdd_cleanup_strings(char** strings, int count);

// Clean up temporary files
void bdd_cleanup_temp_files(void);

// Example usage
char* result1 = bdd_execute_command("command1", &code1);
char* result2 = bdd_execute_command("command2", &code2);

char* results[] = {result1, result2};
bdd_cleanup_strings(results, 2);
```

## Work-in-Progress (@wip) Handling

The framework provides sophisticated @wip handling for different environments:

### Environment Variable Control

```bash
# Development environment (run all tests including @wip)
unset BDD_SKIP_WIP
./test_executable

# CI environment (skip @wip tests)
export BDD_SKIP_WIP=1
./test_executable
```

### @wip Test Definition

```c
// Define @wip test cases
BddTestCase test_cases[] = {
    BDD_TEST_CASE(implemented_feature, test_implemented),
    BDD_WIP_TEST_CASE(future_feature, test_future),      // Will be skipped in CI
    BDD_WIP_TEST_CASE(experimental, test_experimental),  // Will be skipped in CI
};
```

### Expected Output

**Development Environment (BDD_SKIP_WIP unset):**
```
Feature: Test Suite

  Scenario: implemented_feature
    ✓ Test completed

  Scenario: future_feature [@wip]
    ✓ Test completed

  Scenario: experimental [@wip]
    ✓ Test completed

Test Summary for 'Test Suite':
  Passed: 3
  Skipped: 0
  Failed: 0
  Total: 3
```

**CI Environment (BDD_SKIP_WIP=1):**
```
Feature: Test Suite

  Scenario: implemented_feature
    ✓ Test completed

  Scenario: future_feature [@wip]
    ⏭️ SKIPPED: Work in progress

  Scenario: experimental [@wip]
    ⏭️ SKIPPED: Work in progress

Test Summary for 'Test Suite':
  Passed: 1
  Skipped: 2
  Failed: 0
  Total: 3
```

## Test Suite Execution

### Framework Function Signature

```c
int bdd_run_test_suite(const char* feature_name,
                      BddTestCase* test_cases,
                      int test_count,
                      void (*cleanup_function)(void));
```

### Parameters

- **feature_name**: Descriptive name for the test suite
- **test_cases**: Array of BddTestCase structures
- **test_count**: Number of test cases in the array
- **cleanup_function**: Optional cleanup function (can be NULL)

### Return Value

Returns the number of failed tests (0 for success).

### Standard Usage Pattern

```c
int main(void) {
    // Define test cases
    BddTestCase test_cases[] = {
        BDD_TEST_CASE(scenario1, test_function1),
        BDD_TEST_CASE(scenario2, test_function2),
        BDD_WIP_TEST_CASE(scenario3, test_function3),
    };
    
    // Calculate array size
    int test_count = sizeof(test_cases) / sizeof(test_cases[0]);
    
    // Run test suite with cleanup
    return bdd_run_test_suite("Feature Description", 
                              test_cases, 
                              test_count, 
                              bdd_cleanup_temp_files);
}
```

## Pattern Examples

### Example 1: Compiler Basic Functionality

```c
#include "bdd_test_framework.h"
#include "bdd_utilities.h"

void test_simple_compilation(void) {
    bdd_given("a simple Asthra program");
    const char* source = 
        "package main;\n"
        "pub fn main(none) -> void {\n"
        "    return ();\n"
        "}\n";
    
    bdd_create_temp_source_file("simple.asthra", source);
    
    bdd_when("compiling the program");
    const char* compiler = bdd_find_asthra_compiler();
    char compile_cmd[512];
    snprintf(compile_cmd, sizeof(compile_cmd), "%s simple.asthra -o simple", compiler);
    
    int exit_code;
    char* output = bdd_execute_command(compile_cmd, &exit_code);
    
    bdd_then("compilation should succeed");
    bdd_assert(exit_code == 0, "Compilation should succeed");
    bdd_assert_output_not_contains(output, "error");
    
    bdd_cleanup_string(&output);
}

void test_syntax_error(void) {
    bdd_given("an Asthra program with syntax error");
    const char* source = 
        "package main;\n"
        "pub fn main(none) -> void\n"  // Missing opening brace
        "    return ();\n"
        "}\n";
    
    bdd_create_temp_source_file("syntax_error.asthra", source);
    
    bdd_when("compiling the program");
    const char* compiler = bdd_find_asthra_compiler();
    char compile_cmd[512];
    snprintf(compile_cmd, sizeof(compile_cmd), "%s syntax_error.asthra -o syntax_error", compiler);
    
    int exit_code;
    char* output = bdd_execute_command(compile_cmd, &exit_code);
    
    bdd_then("compilation should fail with syntax error");
    bdd_assert(exit_code != 0, "Compilation should fail");
    bdd_assert_output_contains(output, "syntax error");
    
    bdd_cleanup_string(&output);
}

BddTestCase compiler_basic_test_cases[] = {
    BDD_TEST_CASE(simple_compilation, test_simple_compilation),
    BDD_TEST_CASE(syntax_error, test_syntax_error),
};

int main(void) {
    return bdd_run_test_suite(
        "Compiler Basic Functionality",
        compiler_basic_test_cases,
        sizeof(compiler_basic_test_cases) / sizeof(compiler_basic_test_cases[0]),
        bdd_cleanup_temp_files
    );
}
```

### Example 2: Type System Tests

```c
#include "bdd_test_framework.h"
#include "bdd_utilities.h"

void test_primitive_types(void) {
    bdd_given("primitive type declarations");
    const char* source = 
        "package main;\n"
        "pub fn main(none) -> void {\n"
        "    let x: int = 42;\n"
        "    let y: bool = true;\n"
        "    let z: str = \"hello\";\n"
        "    return ();\n"
        "}\n";
    
    bdd_create_temp_source_file("types.asthra", source);
    
    bdd_when("compiling with type checking");
    int result = bdd_compile_source_file("types.asthra", "types", "--check-types");
    
    bdd_then("type checking should succeed");
    bdd_assert(result == 0, "Type checking should pass");
}

void test_type_mismatch(void) {
    bdd_given("a type mismatch in assignment");
    const char* source = 
        "package main;\n"
        "pub fn main(none) -> void {\n"
        "    let x: int = \"string\";  // Type mismatch\n"
        "    return ();\n"
        "}\n";
    
    bdd_create_temp_source_file("type_error.asthra", source);
    
    bdd_when("compiling with type checking");
    int exit_code;
    char* output = bdd_execute_command("asthra type_error.asthra --check-types", &exit_code);
    
    bdd_then("type checking should fail");
    bdd_assert(exit_code != 0, "Type checking should fail");
    bdd_assert_output_contains(output, "type mismatch");
    
    bdd_cleanup_string(&output);
}

void test_generic_types(void) {
    bdd_given("generic type usage");
    const char* source = 
        "package main;\n"
        "type Vec[T] = { data: [T]; length: int; };\n"
        "pub fn main(none) -> void {\n"
        "    let v: Vec[int] = Vec[int] { data: [1, 2, 3], length: 3 };\n"
        "    return ();\n"
        "}\n";
    
    bdd_create_temp_source_file("generics.asthra", source);
    
    bdd_when("compiling generic types");
    int result = bdd_compile_source_file("generics.asthra", "generics", "--enable-generics");
    
    bdd_then("generic compilation should succeed");
    // Mark as WIP since generics are not fully implemented
    bdd_skip_scenario("Generic types not fully implemented yet");
}

BddTestCase type_system_test_cases[] = {
    BDD_TEST_CASE(primitive_types, test_primitive_types),
    BDD_TEST_CASE(type_mismatch, test_type_mismatch),
    BDD_WIP_TEST_CASE(generic_types, test_generic_types),  // Will be skipped in CI
};

int main(void) {
    return bdd_run_test_suite(
        "Type System",
        type_system_test_cases,
        sizeof(type_system_test_cases) / sizeof(type_system_test_cases[0]),
        bdd_cleanup_temp_files
    );
}
```

## Migration from Legacy Pattern

### Before (Legacy Pattern)

```c
// OLD PATTERN - Lots of boilerplate code
#include "bdd_support.h"
#include "common_steps.h"

static void test_function_call_basic(void) {
    bdd_scenario("Call a function with no parameters");
    
    bdd_given("a function declaration");
    const char* source = 
        "package main;\n"
        "pub fn greet(none) -> void { return (); }\n"
        "pub fn main(none) -> void { greet(); return (); }\n";
    
    bdd_when("compiling and running the program");
    int compilation_exit_code = compile_and_run_source(source);
    
    bdd_then("the program should execute successfully");
    bdd_assert(compilation_exit_code == 0, "Compilation should succeed");
}

static void test_function_call_with_params(void) {
    // Similar boilerplate repeated...
}

int main(void) {
    bdd_init("Function Calls");
    
    if (!bdd_should_skip_wip()) {
        test_function_call_basic();
        test_function_call_with_params();
    }
    
    common_cleanup();
    return bdd_report();
}
```

### After (New Framework Pattern)

```c
// NEW PATTERN - Declarative and concise
#include "bdd_test_framework.h"
#include "bdd_utilities.h"

void test_function_call_basic(void) {
    bdd_given("a function declaration");
    const char* source = 
        "package main;\n"
        "pub fn greet(none) -> void { return (); }\n"
        "pub fn main(none) -> void { greet(); return (); }\n";
    
    bdd_create_temp_source_file("test.asthra", source);
    
    bdd_when("compiling and running the program");
    int result = bdd_compile_source_file("test.asthra", "test", NULL);
    
    bdd_then("the program should execute successfully");
    bdd_assert(result == 0, "Compilation should succeed");
}

void test_function_call_with_params(void) {
    // Test implementation...
}

BddTestCase function_calls_test_cases[] = {
    BDD_TEST_CASE(function_call_basic, test_function_call_basic),
    BDD_TEST_CASE(function_call_with_params, test_function_call_with_params),
};

int main(void) {
    return bdd_run_test_suite(
        "Function Calls",
        function_calls_test_cases,
        sizeof(function_calls_test_cases) / sizeof(function_calls_test_cases[0]),
        bdd_cleanup_temp_files
    );
}
```

### Migration Benefits

- **Line count reduction**: From ~380 lines to ~120 lines (68% reduction)
- **No manual @wip handling**: Framework handles it automatically
- **Consistent cleanup**: Built-in resource management
- **Reusable utilities**: No more duplicated helper functions
- **Better maintainability**: Declarative test definitions

## Best Practices

### 1. Test Function Organization

```c
// Good: Clear, focused test functions
void test_specific_scenario(void) {
    bdd_given("specific initial condition");
    // Setup code
    
    bdd_when("specific action occurs");
    // Action code
    
    bdd_then("specific expected outcome");
    // Assertion code
}

// Avoid: Overly complex test functions
void test_everything_at_once(void) {
    // Testing multiple unrelated scenarios in one function
}
```

### 2. Resource Management

```c
// Good: Always clean up resources
void test_with_resources(void) {
    char* output = bdd_execute_command("some_command", &exit_code);
    
    // Use the output
    bdd_assert_output_contains(output, "expected");
    
    // Clean up before returning
    bdd_cleanup_string(&output);
}

// Good: Use framework cleanup in main
int main(void) {
    return bdd_run_test_suite(
        "Feature",
        test_cases,
        test_count,
        bdd_cleanup_temp_files  // Framework handles cleanup
    );
}
```

### 3. @wip Test Usage

```c
// Good: Use @wip for incomplete features
BddTestCase test_cases[] = {
    BDD_TEST_CASE(implemented_feature, test_implemented),
    BDD_WIP_TEST_CASE(future_feature, test_future),
};

// Good: Skip within function for partial implementation
void test_partial_feature(void) {
    bdd_given("a partially implemented feature");
    
    bdd_when("testing implemented part");
    // Test what works
    
    bdd_then("implemented functionality works");
    bdd_assert(/* working functionality */, "Should work");
    
    // Skip unimplemented part
    if (/* unimplemented part */) {
        bdd_skip_scenario("Advanced features not implemented yet");
        return;
    }
}
```

### 4. Error Message Quality

```c
// Good: Descriptive error messages
bdd_assert(result == 0, "Compilation should succeed for basic program");
bdd_assert_output_contains(output, "Hello", "Program output should contain greeting");

// Avoid: Generic error messages
bdd_assert(result == 0, "Test failed");
```

### 5. Test Case Naming

```c
// Good: Descriptive scenario names
BddTestCase test_cases[] = {
    BDD_TEST_CASE(compile_simple_program, test_compile_simple_program),
    BDD_TEST_CASE(handle_syntax_error, test_handle_syntax_error),
    BDD_TEST_CASE(type_inference_basic, test_type_inference_basic),
};

// Avoid: Vague names
BddTestCase test_cases[] = {
    BDD_TEST_CASE(test1, test_function1),
    BDD_TEST_CASE(test2, test_function2),
};
```

## Troubleshooting

### Common Issues and Solutions

#### 1. Test Case Array Size Calculation

**Problem**: Incorrect test count calculation
```c
// Wrong: Hard-coded count
return bdd_run_test_suite("Feature", test_cases, 5, cleanup);
```

**Solution**: Use sizeof calculation
```c
// Correct: Dynamic count calculation
int test_count = sizeof(test_cases) / sizeof(test_cases[0]);
return bdd_run_test_suite("Feature", test_cases, test_count, cleanup);
```

#### 2. Memory Leaks from Utilities

**Problem**: Not cleaning up allocated strings
```c
char* output = bdd_execute_command("command", &exit_code);
// Missing cleanup - memory leak
```

**Solution**: Always clean up resources
```c
char* output = bdd_execute_command("command", &exit_code);
bdd_assert_output_contains(output, "expected");
bdd_cleanup_string(&output);  // Properly clean up
```

#### 3. @wip Tests Not Skipping in CI

**Problem**: @wip tests running in CI environment

**Solution**: Ensure BDD_SKIP_WIP is set in CI configuration
```yaml
# In GitHub Actions or similar CI
env:
  BDD_SKIP_WIP: 1
```

#### 4. Test Framework Linking Issues

**Problem**: Undefined symbols during linking
```
undefined reference to 'bdd_run_test_suite'
```

**Solution**: Ensure proper linking in CMakeLists.txt
```cmake
target_link_libraries(your_test
    bdd_test_framework
    bdd_utilities
)
```

#### 5. Inconsistent Test Output

**Problem**: Tests showing different output formats

**Solution**: Ensure using new framework consistently
```c
// Don't mix old and new patterns
// Use bdd_run_test_suite for all tests
return bdd_run_test_suite(...);
```

### Debug Mode

Enable debug output for troubleshooting:

```bash
export BDD_DEBUG=1
./your_test_executable
```

This will show additional information about:
- Test case execution order
- @wip scenario detection
- Resource cleanup operations
- Framework internal state

## Framework Architecture

### Component Dependencies

```
bdd_test_framework.h
    ├── bdd_support.h (BDD assertions and macros)
    └── bdd_utilities.h (Utility functions)
        ├── Standard C libraries
        └── System utilities (file operations, command execution)
```

### Test Execution Flow

1. **Initialization**: Framework parses test case array
2. **Environment Check**: Checks BDD_SKIP_WIP environment variable
3. **Test Iteration**: For each test case:
   - Check if @wip and should skip
   - Execute test function if not skipped
   - Report results
4. **Cleanup**: Execute cleanup function if provided
5. **Summary**: Generate and display test summary
6. **Exit**: Return failure count for CI integration

### Memory Management

The framework automatically manages:
- Test case metadata storage
- Result tracking and reporting
- Cleanup function execution
- Environment variable handling

Users are responsible for:
- Cleaning up strings from utilities (use `bdd_cleanup_string`)
- Managing test-specific resources
- Proper test function implementation

## Conclusion

The new BDD framework patterns provide significant improvements in code maintainability, consistency, and functionality. By adopting these patterns, developers can:

- Write more concise and maintainable BDD tests
- Ensure consistent behavior across all test suites
- Properly handle work-in-progress scenarios
- Reduce code duplication through reusable utilities
- Improve CI/CD integration with standardized output

The framework maintains backward compatibility while providing a clear migration path from legacy patterns. All new BDD tests should use these patterns, and existing tests should be migrated as development capacity allows.

For detailed migration instructions, see the [BDD Migration Guide](bdd-migration-guide.md).
For basic BDD testing concepts, see the [BDD Testing Guide](bdd-testing-guide.md).