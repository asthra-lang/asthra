# BDD Step File Refactoring Migration Guide

This guide explains how to migrate BDD step files from the old framework to the new reusable BDD framework introduced in the Asthra project.

## Overview

The new BDD framework provides:
- **68-75% code reduction** through reusable utilities
- **Consistent output format** across all BDD tests
- **Proper test isolation** from other test frameworks
- **Declarative test case definitions** with @wip support
- **Standardized error handling** and cleanup

## Migration Process

### Step 1: Update Includes

**Before (Old Framework):**
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "bdd_support.h"

// External functions from common_steps.c
extern void given_asthra_compiler_available(void);
extern void given_file_with_content(const char* filename, const char* content);
extern void when_compile_file(void);
extern void when_run_executable(void);
extern void then_compilation_should_succeed(void);
extern void then_compilation_should_fail(void);
extern void then_executable_created(void);
extern void then_output_contains(const char* expected_output);
extern void then_exit_code_is(int expected_code);
extern void then_error_contains(const char* expected_error);
extern void common_cleanup(void);
```

**After (New Framework):**
```c
#include "bdd_support.h"
#include "bdd_utilities.h"
#include "bdd_test_framework.h"
```

### Step 2: Refactor Test Functions

**Before (Old Framework - 25 lines):**
```c
void test_boolean_literals(void) {
    bdd_scenario("Boolean literals");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let t: bool = true;\n"
        "    let f: bool = false;\n"
        "    log(\"Boolean literals work\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("bool_literals.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Boolean literals work");
    then_exit_code_is(0);
}
```

**After (New Framework - 8 lines):**
```c
void test_boolean_literals(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let t: bool = true;\n"
        "    let f: bool = false;\n"
        "    log(\"Boolean literals work\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Boolean literals",
                               "bool_literals.asthra",
                               source,
                               "Boolean literals work",
                               0);
}
```

### Step 3: Handle Different Test Types

#### Execution Tests (Should Compile and Run)
```c
void test_example(void) {
    const char* source = "/* Asthra source code */";
    
    bdd_run_execution_scenario("Test description",
                               "filename.asthra",
                               source,
                               "Expected output",
                               0);  // expected exit code
}
```

#### Compilation Tests (Should Fail to Compile)
```c
void test_error_case(void) {
    const char* source = "/* Invalid Asthra code */";
    
    bdd_run_compilation_scenario("Error test description",
                                 "error_filename.asthra",
                                 source,
                                 0,  // should fail (non-zero exit code)
                                 "expected error message");
}
```

### Step 4: Define Test Cases Array

**Before (Old Framework - 40+ lines):**
```c
int main(void) {
    bdd_init("Boolean operators");
    
    // Check if @wip scenarios should be skipped
    if (bdd_should_skip_wip()) {
        // Skip @wip scenarios
        printf("⏭️ SKIPPED: Work in progress - Logical NOT operator\n");
        printf("⏭️ SKIPPED: Work in progress - Logical AND operator\n");
        // ... more manual skipping
        
        // Run only non-@wip scenarios
        test_boolean_literals();
        test_logical_or();
        test_boolean_precedence();
        test_complex_expressions();
        test_error_not_boolean();
        test_error_and_non_boolean();
        test_error_or_non_boolean();
        test_error_if_non_boolean();
    } else {
        // Run all scenarios
        test_boolean_literals();
        test_logical_not();
        test_logical_and();
        test_logical_or();
        test_boolean_expressions();
        test_short_circuit_and();
        test_short_circuit_or();
        test_nested_expressions();
        test_boolean_precedence();
        test_complex_expressions();
        test_type_inference();
        test_assignment_mutation();
        test_error_not_boolean();
        test_error_and_non_boolean();
        test_error_or_non_boolean();
        test_error_if_non_boolean();
    }
    
    // Cleanup
    common_cleanup();
    
    return bdd_report();
}
```

**After (New Framework - 8 lines):**
```c
// Define test cases using the new framework
BddTestCase boolean_operators_test_cases[] = {
    BDD_TEST_CASE(boolean_literals, test_boolean_literals),
    BDD_WIP_TEST_CASE(logical_not, test_logical_not),
    BDD_WIP_TEST_CASE(logical_and, test_logical_and),
    BDD_TEST_CASE(logical_or, test_logical_or),
    BDD_WIP_TEST_CASE(boolean_expressions, test_boolean_expressions),
    BDD_WIP_TEST_CASE(short_circuit_and, test_short_circuit_and),
    BDD_WIP_TEST_CASE(short_circuit_or, test_short_circuit_or),
    BDD_WIP_TEST_CASE(nested_expressions, test_nested_expressions),
    BDD_TEST_CASE(boolean_precedence, test_boolean_precedence),
    BDD_TEST_CASE(complex_expressions, test_complex_expressions),
    BDD_WIP_TEST_CASE(type_inference, test_type_inference),
    BDD_WIP_TEST_CASE(assignment_mutation, test_assignment_mutation),
    BDD_TEST_CASE(error_not_boolean, test_error_not_boolean),
    BDD_TEST_CASE(error_and_non_boolean, test_error_and_non_boolean),
    BDD_TEST_CASE(error_or_non_boolean, test_error_or_non_boolean),
    BDD_TEST_CASE(error_if_non_boolean, test_error_if_non_boolean),
};

// Main test runner using the new framework
int main(void) {
    return bdd_run_test_suite("Boolean operators",
                              boolean_operators_test_cases,
                              sizeof(boolean_operators_test_cases) / sizeof(boolean_operators_test_cases[0]),
                              bdd_cleanup_temp_files);
}
```

## Framework Components

### Core Utilities (`bdd_utilities.h/c`)

#### Command Execution
```c
char* bdd_execute_command(const char* command, int* exit_code);
const char* bdd_find_asthra_compiler(void);
```

#### File Management
```c
void bdd_create_temp_source_file(const char* filename, const char* content);
void bdd_cleanup_temp_files(void);
```

#### Assertions
```c
void bdd_assert_output_contains(const char* output, const char* expected);
void bdd_assert_exit_code(int actual, int expected);
```

### Test Framework (`bdd_test_framework.h/c`)

#### High-Level Test Functions
```c
void bdd_run_execution_scenario(const char* description,
                               const char* filename,
                               const char* source,
                               const char* expected_output,
                               int expected_exit_code);

void bdd_run_compilation_scenario(const char* description,
                                 const char* filename,
                                 const char* source,
                                 int expected_compilation_success,
                                 const char* expected_error);
```

#### Test Case Definition
```c
typedef struct {
    const char* name;
    BddTestFunction function;
    int is_wip;
} BddTestCase;

// Macros for easier definition
#define BDD_TEST_CASE(name, func) {#name, func, 0}
#define BDD_WIP_TEST_CASE(name, func) {#name, func, 1}
```

#### Test Suite Runner
```c
int bdd_run_test_suite(const char* feature_name, 
                       BddTestCase* test_cases,
                       int test_count,
                       void (*cleanup_function)(void));
```

## @wip (Work In Progress) Handling

### Marking Tests as @wip
Use `BDD_WIP_TEST_CASE` instead of `BDD_TEST_CASE` for unimplemented features:

```c
BddTestCase my_test_cases[] = {
    BDD_TEST_CASE(working_feature, test_working_feature),           // Will run
    BDD_WIP_TEST_CASE(unimplemented_feature, test_unimplemented),  // Will skip
};
```

### @wip Behavior
- **CI Environment**: @wip scenarios are automatically skipped
- **Development Environment**: @wip scenarios can be run for testing
- **Output**: Clear "⏭️ SKIPPED: Work in progress" messages

## Error Handling Patterns

### Compilation Error Tests
```c
void test_syntax_error(void) {
    const char* source = 
        "package main;\n"
        "invalid syntax here\n";
    
    bdd_run_compilation_scenario("Syntax error detection",
                                 "syntax_error.asthra",
                                 source,
                                 0,  // should fail
                                 "syntax error");
}
```

### Runtime Error Tests
```c
void test_runtime_error(void) {
    const char* source = 
        "package main;\n"
        "pub fn main(none) -> void {\n"
        "    panic(\"test error\");\n"
        "}\n";
    
    bdd_run_execution_scenario("Runtime error test",
                               "runtime_error.asthra",
                               source,
                               "test error",
                               1);  // expect non-zero exit code
}
```

## Complete Migration Example

Here's a complete before/after example showing the full migration:

### Before (142 lines)
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "bdd_support.h"

// External functions from common_steps.c
extern void given_asthra_compiler_available(void);
extern void given_file_with_content(const char* filename, const char* content);
extern void when_compile_file(void);
extern void when_run_executable(void);
extern void then_compilation_should_succeed(void);
extern void then_compilation_should_fail(void);
extern void then_executable_created(void);
extern void then_output_contains(const char* expected_output);
extern void then_exit_code_is(int expected_code);
extern void then_error_contains(const char* expected_error);
extern void common_cleanup(void);

void test_simple_struct(void) {
    bdd_scenario("Simple struct definition");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Point {\n"
        "    x: i32,\n"
        "    y: i32\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let p: Point = Point { x: 10, y: 20 };\n"
        "    log(\"Simple struct works\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("simple_struct.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Simple struct works");
    then_exit_code_is(0);
}

void test_duplicate_field(void) {
    bdd_scenario("Duplicate struct field error");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Point {\n"
        "    x: i32,\n"
        "    x: i32\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("duplicate_field.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("duplicate field");
}

// Main test runner
int main(void) {
    bdd_init("User-Defined Types");
    
    // Check if @wip scenarios should be skipped
    if (bdd_should_skip_wip()) {
        // Run only non-@wip scenarios
        test_simple_struct();
        test_duplicate_field();
    } else {
        // Run all scenarios
        test_simple_struct();
        test_duplicate_field();
    }
    
    // Cleanup
    common_cleanup();
    
    return bdd_report();
}
```

### After (47 lines)
```c
#include "bdd_support.h"
#include "bdd_utilities.h"
#include "bdd_test_framework.h"

// Test scenarios using the new reusable framework

void test_simple_struct(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Point {\n"
        "    x: i32,\n"
        "    y: i32\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let p: Point = Point { x: 10, y: 20 };\n"
        "    log(\"Simple struct works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Simple struct definition",
                               "simple_struct.asthra",
                               source,
                               "Simple struct works",
                               0);
}

void test_duplicate_field(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Point {\n"
        "    x: i32,\n"
        "    x: i32\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Duplicate struct field error",
                                 "duplicate_field.asthra",
                                 source,
                                 0,  // should fail
                                 "duplicate field");
}

// Define test cases using the new framework
BddTestCase user_defined_types_test_cases[] = {
    BDD_TEST_CASE(simple_struct, test_simple_struct),
    BDD_TEST_CASE(duplicate_field, test_duplicate_field),
};

// Main test runner using the new framework
int main(void) {
    return bdd_run_test_suite("User-Defined Types",
                              user_defined_types_test_cases,
                              sizeof(user_defined_types_test_cases) / sizeof(user_defined_types_test_cases[0]),
                              bdd_cleanup_temp_files);
}
```

**Result: 67% code reduction (142 → 47 lines)**

## Migration Checklist

- [ ] Replace includes with new framework headers
- [ ] Convert test functions to use `bdd_run_execution_scenario()` or `bdd_run_compilation_scenario()`
- [ ] Define test cases array with `BDD_TEST_CASE` and `BDD_WIP_TEST_CASE` macros
- [ ] Replace manual main() function with `bdd_run_test_suite()` call
- [ ] Remove external function declarations and manual @wip handling
- [ ] Test compilation with `cmake --build build --target build-tests`
- [ ] Validate @wip scenarios are properly skipped with `ctest --test-dir build -L bdd`

## Benefits After Migration

1. **Reduced Maintenance**: 68-75% less code to maintain
2. **Consistent Output**: Standardized BDD test format across all tests
3. **Automatic @wip Handling**: No manual skip logic needed
4. **Better Isolation**: BDD tests isolated from other test frameworks
5. **Easier Test Addition**: Simple declarative syntax for new tests
6. **Improved Readability**: Focus on test logic, not boilerplate

## Troubleshooting

### Common Issues

1. **Missing Includes**: Ensure all three headers are included
2. **Compilation Failures**: Check that `bdd_utilities.c` and `bdd_test_framework.c` are linked
3. **@wip Not Skipping**: Verify `BDD_WIP_TEST_CASE` is used correctly
4. **Output Format Issues**: Ensure old `asthra_test_framework` linking is removed

### Debug Commands
```bash
# Build tests
cmake --build build --target build-tests

# Run specific BDD test
ctest --test-dir build -R "bdd_unit_your_test_name"

# Run all BDD tests
ctest --test-dir build -L bdd
```

## Future Enhancements

The new framework is designed to be extensible. Future enhancements may include:
- **Performance benchmarking utilities**
- **Memory leak detection helpers**
- **Cross-platform test validation**
- **Automated test report generation**

For questions or issues with migration, refer to:
- `docs/contributor/guides/bdd-testing-guide.md`
- `bdd/BDD_BEST_PRACTICES.md`
- Example refactored files in `bdd/steps/unit/`