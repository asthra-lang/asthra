# BDD Testing Best Practices for Asthra

This document outlines the best practices and implementation patterns for writing Behavior-Driven Development (BDD) tests in the Asthra compiler project.

## Table of Contents
1. [Overview](#overview)
2. [Directory Structure](#directory-structure)
3. [Feature Files](#feature-files)
4. [Step Definitions](#step-definitions)
5. [Test Organization](#test-organization)
6. [Common Patterns](#common-patterns)
7. [Build System Integration](#build-system-integration)
8. [Debugging and Maintenance](#debugging-and-maintenance)

## Overview

Asthra uses a custom BDD framework that follows Gherkin-style syntax but implements tests in C. This approach ensures:
- Tests run close to the metal with minimal overhead
- Easy integration with the C-based compiler infrastructure
- Consistent testing patterns across the project
- Clear separation between test specification (features) and implementation (steps)

## Directory Structure

```
bdd/
├── features/           # Gherkin feature files
├── steps/             # Step definition implementations
│   ├── unit/          # Unit test step definitions
│   └── integration/   # Integration test step definitions
├── support/           # BDD framework support code
├── fixtures/          # Test data and example files
└── bdd-temp/         # Temporary test artifacts (gitignored)
```

### Key Principles:
- **Feature files** describe behavior in human-readable format
- **Step definitions** implement the test logic in C
- **Support files** provide the BDD framework infrastructure
- **Fixtures** contain reusable test data

## Feature Files

### Naming Convention
- Use descriptive names: `<component>_<functionality>.feature`
- Examples: `boolean_operators.feature`, `compiler_basic.feature`

### Structure
```gherkin
Feature: <Feature Name>
  As a <role>
  I want <goal>
  So that <benefit>

  Background:
    Given <common setup steps>

  Scenario: <Scenario description>
    Given <initial context>
    When <action is performed>
    Then <expected outcome>

  @wip
  Scenario: <Work in progress scenario>
    Given <steps for future implementation>
```

### Best Practices:
1. **Use clear, descriptive scenario names** that explain what is being tested
2. **Keep scenarios focused** - test one behavior per scenario
3. **Use Background** for common setup steps
4. **Tag scenarios appropriately**:
   - `@wip` - Work in progress, not yet implemented
   - `@slow` - Tests that take longer to run
   - `@integration` - Integration tests
5. **Write scenarios from the user's perspective**

## Step Definitions

### File Organization
```c
// Include necessary headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "bdd_support.h"

// External function declarations from common_steps.c
extern void given_asthra_compiler_available(void);
extern void given_file_with_content(const char* filename, const char* content);
// ... other external functions

// Test scenario implementations
void test_scenario_name(void) {
    bdd_scenario("Scenario description from feature file");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    // Test code\n"
        "    return 0;\n"
        "}\n";
    
    given_file_with_content("test.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_exit_code_is(0);
}

// Main test runner
int main(void) {
    bdd_init("Feature name");
    
    // Run scenarios NOT marked with @wip
    test_scenario_name();
    
    // @wip scenarios - disabled until fully implemented
    // test_wip_scenario();  // @wip
    
    // Cleanup
    common_cleanup();
    
    return bdd_report();
}
```

### Best Practices:
1. **Reuse common step functions** from `common_steps.c`
2. **Keep test data inline** as string literals for clarity
3. **Follow consistent naming**: `test_<scenario_name>()`
4. **Comment out @wip scenarios** with clear annotations
5. **Always call cleanup** before returning

## Test Organization

### Unit Tests
Located in `steps/unit/`, these test individual components:
- Parser functionality
- Semantic analysis
- Code generation
- Type checking

### Integration Tests
Located in `steps/integration/`, these test:
- End-to-end compilation
- FFI integration
- CLI behavior
- Multi-component interactions

### Common Steps
Shared functionality should be placed in:
- `steps/common_steps.c` - General utilities
- `steps/unit/common_steps.c` - Unit test utilities
- `steps/integration/common_steps.c` - Integration utilities

## Common Patterns

### 1. Given-When-Then Pattern
```c
// Given - Set up the context
given_asthra_compiler_available();
given_file_with_content("test.asthra", source);

// When - Perform the action
when_compile_file();
when_run_executable();

// Then - Assert the outcome
then_compilation_should_succeed();
then_output_contains("expected output");
then_exit_code_is(0);
```

### 2. Error Testing Pattern
```c
void test_error_scenario(void) {
    bdd_scenario("Error scenario description");
    
    given_asthra_compiler_available();
    
    const char* invalid_source = 
        "package test;\n"
        "// Invalid code that should fail\n";
    
    given_file_with_content("error.asthra", invalid_source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("expected error message");
}
```

### 3. Multiple Assertions Pattern
```c
// Test multiple outputs
then_output_contains("First expected line");
then_output_contains("Second expected line");
then_output_contains("Third expected line");
```

### 4. Temporary File Management
- Files are created in `bdd-temp/` directory
- Use `BDD_KEEP_ARTIFACTS=1` to preserve files for debugging
- Cleanup is automatic unless explicitly disabled

## Build System Integration

### CMakeLists.txt Registration
```cmake
# Register BDD tests
add_bdd_test(test_name unit)           # For unit tests
add_bdd_test(test_name integration)    # For integration tests
```

### Build Commands
```bash
# Build all BDD tests
cmake --build build --target bdd_tests

# Run specific BDD test category
cmake --build build --target bdd_unit
cmake --build build --target bdd_integration

# Run all BDD tests
cmake --build build --target run_bdd_tests
```

## Debugging and Maintenance

### Debugging Failed Tests
1. **Preserve artifacts**: `export BDD_KEEP_ARTIFACTS=1`
2. **Run individual test**: `./build/bdd/bin/bdd_unit_<test_name>`
3. **Check generated files**: Look in `bdd-temp/` directory
4. **Add debug output**: Use `printf()` in step definitions

### Test Maintenance
1. **Keep features and steps in sync** - Update both when behavior changes
2. **Remove obsolete @wip tags** when features are implemented
3. **Refactor common patterns** into shared step functions
4. **Update test data** to reflect current language syntax

### Performance Considerations
1. **Clean up large test files** in cleanup functions
2. **Use parallel test execution**: `ctest -j8`
3. **Mark slow tests** with appropriate tags
4. **Minimize file I/O** in unit tests

## BDD Framework API

### Core Functions
```c
// Framework initialization
void bdd_init(const char* feature_name);

// Scenario definition
void bdd_scenario(const char* scenario_name);

// Step definitions
void bdd_given(const char* condition);
void bdd_when(const char* action);
void bdd_then(const char* expectation);

// Assertions
void bdd_assert(int condition, const char* message);

// Reporting
int bdd_report(void);
```

### Assertion Macros
```c
BDD_ASSERT_TRUE(expr)              // Assert expression is true
BDD_ASSERT_FALSE(expr)             // Assert expression is false
BDD_ASSERT_EQ(a, b)                // Assert a equals b
BDD_ASSERT_NE(a, b)                // Assert a not equals b
BDD_ASSERT_NULL(ptr)               // Assert pointer is NULL
BDD_ASSERT_NOT_NULL(ptr)           // Assert pointer is not NULL
```

## Example: Complete BDD Test

### Feature File (features/example.feature)
```gherkin
Feature: Example Feature
  As a developer
  I want to demonstrate BDD best practices
  So that others can write effective tests

  Background:
    Given the Asthra compiler is available

  Scenario: Successful compilation
    Given I have a valid Asthra source file
    When I compile the file
    Then the compilation should succeed
    And an executable should be created

  @wip
  Scenario: Advanced feature not yet implemented
    Given I have a file using advanced features
    When I compile the file
    Then it should handle the features correctly
```

### Step Definition (steps/unit/example_steps.c)
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bdd_support.h"

// External functions
extern void given_asthra_compiler_available(void);
extern void given_file_with_content(const char* filename, const char* content);
extern void when_compile_file(void);
extern void then_compilation_should_succeed(void);
extern void then_executable_created(void);
extern void common_cleanup(void);

// Test scenario: Successful compilation
void test_successful_compilation(void) {
    bdd_scenario("Successful compilation");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package example;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    return 0;\n"
        "}\n";
    
    given_file_with_content("example.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
}

// Main test runner
int main(void) {
    bdd_init("Example Feature");
    
    // Run implemented scenarios
    test_successful_compilation();
    
    // @wip scenarios - disabled until implemented
    // test_advanced_feature();  // @wip
    
    // Cleanup
    common_cleanup();
    
    return bdd_report();
}
```

## Summary

Following these best practices ensures:
- **Consistent** test implementation across the project
- **Maintainable** test suites that evolve with the compiler
- **Clear** separation between test specification and implementation
- **Efficient** test execution and debugging
- **Reliable** test results that catch regressions early

Remember: Good BDD tests tell a story about how the system should behave, making them valuable documentation as well as verification tools.