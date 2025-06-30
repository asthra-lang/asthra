# BDD Testing Infrastructure for Asthra

This directory contains the Behavior-Driven Development (BDD) testing infrastructure for the Asthra programming language. BDD tests ensure the compiler behaves correctly from a user's perspective by describing functionality in terms of scenarios and expected outcomes.

## Quick Start

```bash
# Clone and build with BDD tests enabled
git clone https://github.com/asthra-lang/asthra.git
cd asthra
cmake -B build -DBUILD_BDD_TESTS=ON
cmake --build build

# Run all BDD tests
cmake --build build --target run_bdd_tests

# Run specific test category
cmake --build build --target bdd_unit -j8
```

## Directory Structure

```
bdd/
├── CMakeLists.txt        # Build configuration for BDD tests
├── README.md             # This file
├── features/             # Cucumber/Gherkin feature files
│   ├── compiler/         # End-to-end compiler features
│   ├── parser/           # Parser-specific features
│   └── semantic/         # Semantic analysis features
├── steps/                # Step definition implementations
│   ├── unit/            # Unit test step definitions
│   │   ├── parser/      # Parser unit tests
│   │   ├── semantic/    # Semantic analysis unit tests
│   │   └── codegen/     # Code generation unit tests
│   ├── integration/     # Integration test steps
│   ├── acceptance/      # Acceptance test steps
│   ├── scenario/        # Complex scenario tests
│   └── feature/         # Complete feature tests
├── support/             # BDD framework and utilities
│   ├── bdd_support.h    # Core BDD API
│   ├── bdd_support.c    # BDD implementation
│   └── test_helpers.h   # Common test utilities
├── fixtures/            # Test data and example programs
│   ├── valid/           # Valid Asthra programs
│   ├── invalid/         # Programs with intentional errors
│   └── examples/        # Complex test scenarios
└── bin/                 # Compiled test executables (generated)
```

## Building and Running BDD Tests

### Prerequisites

- CMake 3.20+
- C17-compatible compiler (GCC 11+, Clang 14+)
- (Optional) Ruby and Cucumber for Gherkin features

### Enable BDD Tests

```bash
# Configure with BDD tests enabled
cmake -B build -DBUILD_BDD_TESTS=ON

# Build the compiler and BDD test infrastructure
cmake --build build

# Build test executables (required after clean)
cmake --build build --target bdd_tests
```

### Available CMake Targets

| Target | Description |
|--------|-------------|
| `bdd_tests` | Build all BDD test executables |
| `run_bdd_tests` | Execute the full BDD test suite |
| `bdd_cucumber` | Run Cucumber features (requires Ruby/Cucumber) |
| `bdd_unit` | Run unit-level BDD tests |
| `bdd_integration` | Run integration BDD tests |
| `bdd_acceptance` | Run acceptance BDD tests |
| `bdd-report` | Generate HTML test report |
| `bdd-clean` | Clean BDD test outputs |

### Running Tests

```bash
# Run all BDD tests
cmake --build build --target run_bdd_tests

# Run specific test categories
cmake --build build --target bdd_unit        # Unit tests only
cmake --build build --target bdd_integration # Integration tests
cmake --build build --target bdd_acceptance  # Acceptance tests

# Run tests in parallel for faster execution
cmake --build build --target bdd_unit -j8

# Run with verbose output
ctest --test-dir build -V -L bdd

# Run specific test executable
./build/bdd/bin/test_parser_expressions

# Generate HTML report
cmake --build build --target bdd-report
open build/bdd/reports/bdd-report.html
```

## Writing BDD Tests

### C-Based BDD Tests (Primary Approach)

Our BDD framework provides a simple, expressive API for writing tests in C:

```c
#include "bdd_support.h"
#include "compiler.h"

int main(void) {
    bdd_init("Variable Declaration");
    
    bdd_scenario("Declare and initialize an integer variable");
    
    bdd_given("a variable declaration with initialization");
    const char* source = 
        "package test;
"
        "pub fn main(none) -> void {
"
        "    let x: int = 42;
"
        "    return ();
"
        "}
";
    
    bdd_when("compiling the source");
    CompilerResult result = compile_string(source);
    
    bdd_then("compilation succeeds");
    BDD_ASSERT_TRUE(result.success);
    BDD_ASSERT_NULL(result.error);
    
    bdd_and("the variable is properly typed");
    Symbol* var = find_symbol(result.symbols, "x");
    BDD_ASSERT_NOT_NULL(var);
    BDD_ASSERT_STR_EQ(var->type->name, "int");
    
    return bdd_report();
}
```

#### Available Assertions

```c
BDD_ASSERT_TRUE(condition)              // Assert condition is true
BDD_ASSERT_FALSE(condition)             // Assert condition is false
BDD_ASSERT_EQ(expected, actual)         // Assert values are equal
BDD_ASSERT_NEQ(expected, actual)        // Assert values are not equal
BDD_ASSERT_NULL(pointer)                // Assert pointer is NULL
BDD_ASSERT_NOT_NULL(pointer)            // Assert pointer is not NULL
BDD_ASSERT_STR_EQ(expected, actual)     // Assert strings are equal
BDD_ASSERT_STR_CONTAINS(str, substr)    // Assert string contains substring
BDD_ASSERT_GT(a, b)                     // Assert a > b
BDD_ASSERT_LT(a, b)                     // Assert a < b
BDD_ASSERT_GTE(a, b)                    // Assert a >= b
BDD_ASSERT_LTE(a, b)                    // Assert a <= b
```

### Cucumber Features (Optional)

When Cucumber is available, you can write feature files in Gherkin:

```gherkin
Feature: Pattern Matching
  As an Asthra developer
  I want to use pattern matching
  So that I can write expressive conditional logic

  Scenario: Match on enum variants
    Given the following Asthra source:
      '''
      package example;
      
      enum Result {
          Ok(value: int),
          Err(message: string)
      }
      
      pub fn handle_result(r: Result) -> string {
          match r {
              Ok(v) => return "Success: " + int_to_string(v);
              Err(msg) => return "Error: " + msg;
          }
      }
      '''
    When I compile the source
    Then compilation should succeed
    And the match expression should be exhaustive
```

## Test Categories

### Unit Tests (`steps/unit/`)
Fast, focused tests of individual components:
- Parser: Expression parsing, statement parsing, declarations
- Semantic: Type checking, inference, symbol resolution
- Codegen: Instruction generation, register allocation

### Integration Tests (`steps/integration/`)
Tests of component interactions:
- Parser + Semantic analysis
- Semantic + Code generation
- End-to-end compilation

### Acceptance Tests (`steps/acceptance/`)
High-level feature validation:
- Complete program compilation
- Language feature compliance
- Error reporting quality

### Scenario Tests (`steps/scenario/`)
Complex use cases and edge cases:
- Error recovery
- Performance scenarios
- Platform-specific behavior

### Feature Tests (`steps/feature/`)
Complete language feature validation:
- Pattern matching
- Memory safety
- FFI integration

## Best Practices

1. **Keep tests focused** - One scenario per behavior
2. **Use descriptive names** - Scenarios should clearly indicate what's tested
3. **Test both success and failure** - Include negative test cases
4. **Follow Asthra patterns** - Respect expression-oriented design
5. **Clean up resources** - Free allocated memory, close files
6. **Maintain independence** - Tests shouldn't depend on each other

Example of a well-structured test:

```c
bdd_scenario("Type inference for if-else expressions");

bdd_given("an if-else expression with consistent branch types");
const char* source = 
    "package test;
"
    "pub fn choose(condition: bool) -> int {
"
    "    let result = if condition { 42 } else { 0 };
"
    "    return result;
"
    "}
";

bdd_when("analyzing the expression");
SemanticResult result = analyze_source(source);

bdd_then("the if-else expression type is inferred correctly");
BDD_ASSERT_TRUE(result.success);
Expression* if_expr = find_expression(result.ast, EXPR_IF_ELSE);
BDD_ASSERT_NOT_NULL(if_expr);
BDD_ASSERT_STR_EQ(if_expr->type->name, "int");
```

## Platform Support

The BDD infrastructure is tested and supported on:

- **Linux**: Ubuntu 22.04+, Debian 11+, Fedora 36+
- **macOS**: macOS 14+ (Apple Silicon and Intel)
- **Windows**: Not currently supported (use WSL2)

Platform-specific configurations are handled automatically by CMake.

## Troubleshooting

### Common Issues

1. **BDD tests not found**
   ```bash
   # Ensure BDD tests are enabled
   cmake -B build -DBUILD_BDD_TESTS=ON
   cmake --build build --target bdd_tests
   ```

2. **Test executables missing**
   ```bash
   # Build test executables first
   cmake --build build --target bdd_tests
   ```

3. **Cucumber not available**
   ```bash
   # Install Ruby and Cucumber (optional)
   gem install cucumber
   ```

4. **Memory leaks detected**
   ```bash
   # Run with AddressSanitizer
   cmake -B build -DSANITIZER=Address -DBUILD_BDD_TESTS=ON
   ```

## Contributing to Asthra BDD Tests

Thank you for your interest in contributing to Asthra's BDD test suite! This guide will help you write effective BDD tests that maintain our quality standards and integrate seamlessly with the existing test infrastructure.

### Getting Started

#### Prerequisites

Before contributing BDD tests, ensure you:

1. Have a working development environment with BDD tests enabled
2. Understand Asthra's design principles and expression-oriented nature

#### Setting Up

```bash
# Fork and clone the repository
git clone https://github.com/YOUR_USERNAME/asthra.git
cd asthra

# Create a feature branch
git checkout -b test/your-feature-name

# Enable BDD tests
cmake -B build -DBUILD_BDD_TESTS=ON
cmake --build build
```

### Test Organization

#### Choosing the Right Category

Place your test in the appropriate directory based on its scope:

```
bdd/steps/
├── unit/          # Isolated component tests
├── integration/   # Multi-component interaction tests
├── acceptance/    # End-to-end feature validation
├── scenario/      # Complex use cases
└── feature/       # Complete language feature tests
```

#### Decision Tree

1. **Does your test focus on a single component in isolation?**
   - Yes → `unit/` (further organized by component)
   - No → Continue to 2

2. **Does your test verify interaction between 2-3 components?**
   - Yes → `integration/`
   - No → Continue to 3

3. **Does your test validate a complete user-facing feature?**
   - Yes → `acceptance/` or `feature/`
   - No → Continue to 4

4. **Does your test cover edge cases or complex scenarios?**
   - Yes → `scenario/`
   - No → Reconsider test scope

#### Creating New Test Files

When creating a new test file:

1. Use the naming pattern: `test_<component>_<aspect>.c`
2. Place in the appropriate subdirectory
3. Add to `CMakeLists.txt` in the same directory

Example structure for a new parser test:
```
bdd/steps/unit/parser/
├── CMakeLists.txt
├── test_parser_expressions.c      # Existing
├── test_parser_statements.c       # Existing
└── test_parser_your_feature.c     # Your new test
```

### Writing Your First BDD Test

#### Step-by-Step Guide

1. **Create your test file**
   ```bash
   touch bdd/steps/unit/parser/test_parser_loops.c
   ```

2. **Write the test structure**
   ```c
   #include "bdd_support.h"
   #include "parser.h"
   #include <string.h>

   int main(void) {
       bdd_init("Loop Parsing");
       
       // Your scenarios go here
       
       return bdd_report();
   }
   ```

3. **Add scenarios following Given-When-Then**
   ```c
   bdd_scenario("Parse a for loop with range");
   
   bdd_given("a for loop iterating over a range");
   const char* source = 
       "package test;
"
       "pub fn count(none) -> void {
"
       "    for i in 0..10 {
"
       "        print(i);
"
       "    }
"
       "    return ();
"
       "}
";
   
   bdd_when("parsing the source");
   ParseResult result = parse_string(source);
   
   bdd_then("parsing succeeds");
   BDD_ASSERT_TRUE(result.success);
   BDD_ASSERT_NULL(result.error);
   
   bdd_and("the for loop AST is correctly structured");
   ASTNode* for_node = find_node_by_type(result.ast, AST_FOR_LOOP);
   BDD_ASSERT_NOT_NULL(for_node);
   BDD_ASSERT_STR_EQ(for_node->data.for_loop.iterator, "i");
   ```

4. **Update CMakeLists.txt**
   ```cmake
   # In bdd/steps/unit/parser/CMakeLists.txt
   add_bdd_test(
       NAME test_parser_loops
       SOURCES test_parser_loops.c
       LIBRARIES asthra_parser bdd_support
   )
   ```

5. **Build and run your test**
   ```bash
   cmake --build build --target test_parser_loops
   ./build/bdd/bin/test_parser_loops
   ```

### Naming Conventions

#### Test Files
- Pattern: `test_<component>_<aspect>.c`
- Examples:
  - `test_parser_expressions.c`
  - `test_semantic_types.c`
  - `test_codegen_functions.c`

#### Scenario Names
- Use clear, descriptive names that indicate the behavior being tested
- Start with an action verb when possible
- Avoid technical jargon in scenario names

✅ Good:
```c
bdd_scenario("Detect type mismatch in function arguments");
bdd_scenario("Generate optimized code for constant expressions");
```

❌ Bad:
```c
bdd_scenario("Test case 1");
bdd_scenario("AST node validation");
```

#### Test Data Variables
- Use descriptive names that indicate the test case
- Prefix with the type of test data

```c
const char* valid_function = "...";
const char* invalid_syntax = "...";
const char* edge_case_nested_loops = "...";
```

### Code Style Guidelines

#### General C Style
- Follow the project's C coding standards
- Use 4 spaces for indentation (no tabs)
- Keep lines under 100 characters
- Use meaningful variable names

#### BDD-Specific Style

1. **Scenario Structure**
   ```c
   bdd_scenario("Clear description");
   
   bdd_given("precondition");
   // Setup code
   
   bdd_when("action");
   // Action code
   
   bdd_then("expected outcome");
   // Assertions
   
   bdd_and("additional outcome");  // Optional
   // More assertions
   ```

2. **Source Code Formatting**
   ```c
   // Always format Asthra source code clearly
   const char* source = 
       "package test;
"
       "
"  // Use blank lines for readability
       "pub fn example(x: int) -> int {
"
       "    // Function body
"
       "    return x * 2;
"
       "}
";
   ```

3. **Assertion Messages**
   ```c
   // Provide context for complex assertions
   if (!result.success) {
       printf("Parse failed at line %d: %s
", 
              result.error_line, result.error);
   }
   BDD_ASSERT_TRUE(result.success);
   ```

### Memory Management
- Always free allocated resources
- Use cleanup labels for error paths
- Consider using helper functions for common cleanup

```c
bdd_scenario("Test with proper cleanup");
bdd_given("allocated resources");
Parser* parser = parser_create();
AST* ast = NULL;

bdd_when("parsing");
ast = parser_parse(parser, source);

bdd_then("verify and cleanup");
BDD_ASSERT_NOT_NULL(ast);
// ... more assertions ...

// Cleanup
ast_free(ast);
parser_destroy(parser);
```

### Test Data and Fixtures

#### Using Existing Fixtures

Check `bdd/fixtures/` for existing test data before creating new files:

```c
bdd_given("a complex valid program");
char* source = read_fixture("valid/advanced/pattern_matching.at");
BDD_ASSERT_NOT_NULL(source);

// Use the fixture
// ...

free(source);
```

#### Creating New Fixtures

When your test data is complex or reusable:

1. Create a `.at` file in the appropriate fixture directory
2. Document the fixture's purpose with comments
3. Reference it in your tests

Example fixture (`bdd/fixtures/valid/loops/nested_for.at`):
```
// Test fixture for nested for loops
// Tests: Variable scoping, iterator shadowing, break/continue
package test;

pub fn matrix_sum(matrix: [[int; 3]; 3]) -> int {
    let sum = 0;
    for i in 0..3 {
        for j in 0..3 {
            sum = sum + matrix[i][j];
        }
    }
    return sum;
}
```

#### Fixture Guidelines
- Place in the correct category (`valid/`, `invalid/`, `examples/`)
- Use descriptive filenames
- Include header comments explaining the fixture's purpose
- Keep fixtures focused on specific test scenarios

### Submission Process

#### Before Submitting

1. **Run all tests**
   ```bash
   cmake --build build --target run_bdd_tests
   ```

2. **Check code formatting**
   ```bash
   cmake --build build --target format
   ```

3. **Run static analysis**
   ```bash
   cmake --build build --target analyze
   ```

4. **Update documentation** if you've added new test categories or significant features

#### Creating a Pull Request

1. **Commit your changes**
   ```bash
   git add bdd/steps/unit/parser/test_parser_loops.c
   git add bdd/steps/unit/parser/CMakeLists.txt
   git commit -m "test(parser): add BDD tests for loop parsing"
   ```

2. **Push to your fork**
   ```bash
   git push origin test/your-feature-name
   ```

3. **Create the PR**
   - Title: `test(<component>): <description>`
   - Description: Explain what scenarios your tests cover
   - Link any related issues

#### PR Description Template
```markdown
## Description
Brief description of what your tests cover

## Test Coverage
- [ ] Unit tests for [component/feature]
- [ ] Integration tests for [interaction]
- [ ] Edge cases for [scenario]

## Scenarios Added
1. Scenario name - what it tests
2. Scenario name - what it tests

## Related Issues
Closes #123 (if applicable)
```

### Review Criteria

Your BDD tests will be reviewed for:

#### 1. **Correctness**
- Tests accurately validate the intended behavior
- Assertions are appropriate and sufficient
- No false positives or negatives

#### 2. **Clarity**
- Scenario names clearly describe what's being tested
- Given-When-Then structure is properly used
- Code is easy to understand

#### 3. **Coverage**
- Tests cover both success and failure cases
- Edge cases are considered
- No redundant tests with existing suite

#### 4. **Style Compliance**
- Follows naming conventions
- Adheres to code style guidelines
- Properly organized in directory structure

#### 5. **Performance**
- Tests run efficiently (unit tests < 100ms)
- No unnecessary loops or redundant operations
- Appropriate use of fixtures vs. inline data

#### 6. **Maintenance**
- Tests are independent and isolated
- No hardcoded paths or environment assumptions
- Proper resource cleanup

### Common Review Feedback

#### Issues to Avoid

1. **Missing edge cases**
   ```c
   // ❌ Only tests happy path
   bdd_scenario("Parse array declaration");
   test_array("[1, 2, 3]");
   
   // ✅ Include edge cases
   bdd_scenario("Parse empty array");
   test_array("[]");
   
   bdd_scenario("Parse array with trailing comma");
   test_array("[1, 2, 3,]");
   ```

2. **Unclear test data**
   ```c
   // ❌ Unclear what's being tested
   const char* src = "fn f(){let x=1;return x;}";
   
   // ✅ Formatted for clarity
   const char* source = 
       "fn calculate() {
"
       "    let x = 1;  // Test variable declaration
"
       "    return x;   // Test variable usage
"
       "}
";
   ```

3. **Incomplete assertions**
   ```c
   // ❌ Only checks success
   BDD_ASSERT_TRUE(result.success);
   
   // ✅ Validates the actual behavior
   BDD_ASSERT_TRUE(result.success);
   BDD_ASSERT_EQ(result.value, 42);
   BDD_ASSERT_STR_EQ(result.type, "int");
   ```

### Getting Help

If you need assistance:

1. Check existing tests for examples
2. Ask questions in GitHub Discussions
3. Tag `@asthra-lang/testing` team in your PR for specific feedback

### Thank You!

Your contributions help ensure Asthra remains reliable and well-tested. Every test you write helps prevent bugs and serves as documentation for future contributors. We appreciate your efforts in maintaining our high quality standards!

## BDD @wip Tag Filtering

This document explains how the BDD framework in Asthra supports skipping scenarios marked with the `@wip` (Work In Progress) tag.

### Overview

The BDD framework allows scenarios to be marked as work-in-progress using the `@wip` tag. This allows developers to:
- Keep failing tests in the codebase without breaking CI/CD
- Write scenarios for features not yet implemented
- Gradually implement features while maintaining a green test suite

### Usage

#### Two-Part System

The @wip filtering system requires coordination between two files:

1. **Feature files** (`bdd/features/*.feature`) - Mark scenarios with `@wip`
2. **Step implementation files** (`bdd/steps/unit/*_steps.c`) - Use `BDD_WIP_TEST_CASE()` macro

#### Marking Scenarios as WIP

##### Step 1: In Feature Files

Mark scenarios with the `@wip` tag:

```gherkin
@wip
Scenario: Boolean AND operator
  Given I have code using the && operator
  When I compile it
  Then it should work correctly
```

##### Step 2: In Step Implementation Files

Use `BDD_WIP_TEST_CASE()` instead of `BDD_TEST_CASE()`:

```c
// Define test cases
BddTestCase boolean_operators_test_cases[] = {
    BDD_TEST_CASE(boolean_literals, test_boolean_literals),        // Regular test
    BDD_WIP_TEST_CASE(logical_and, test_logical_and),             // WIP test - will be skipped
    BDD_WIP_TEST_CASE(logical_or, test_logical_or),              // WIP test - will be skipped
    BDD_TEST_CASE(boolean_negation, test_boolean_negation),      // Regular test
};
```

#### Environment Variables

Control tag filtering using environment variables:

1. **Skip @wip scenarios (default)**:
   ```bash
   ./bdd_test  # @wip scenarios are skipped by default
   ```

2. **Run all scenarios including @wip**:
   ```bash
   BDD_SKIP_WIP=0 ./bdd_test
   ```

### Implementation Details

#### How It Works

The BDD framework uses the `bdd_test_framework.c` implementation which:

1. Checks the `is_wip` flag on each `BddTestCase`
2. By default skips tests marked with `BDD_WIP_TEST_CASE()`
3. Respects the `BDD_SKIP_WIP` environment variable

#### Test Case Structure

```c
typedef struct {
    const char* name;
    void (*test_function)(void);
    int is_wip;  // Set to 1 by BDD_WIP_TEST_CASE macro
} BddTestCase;

// Macros for defining test cases
#define BDD_TEST_CASE(name, func) {#name, func, 0}
#define BDD_WIP_TEST_CASE(name, func) {#name, func, 1}
```

### Migration Guide

To mark failing tests as WIP:

1. **Add @wip tag to feature file**:
   ```gherkin
   @wip
   Scenario: My failing scenario
     Given ...
     When ...
     Then ...
   ```

2. **Update step implementation file**:
   ```c
   // Change from:
   BDD_TEST_CASE(my_failing_scenario, test_my_failing_scenario),
   
   // To:
   BDD_WIP_TEST_CASE(my_failing_scenario, test_my_failing_scenario),
   ```

3. **Rebuild and run tests**:
   ```bash
   cmake --build build
   ./run-bdd-tests-local.sh
   ```

### Benefits

1. **Clean test output**: Only see results for implemented features
2. **CI/CD friendly**: Tests pass even with incomplete features  
3. **Developer friendly**: Easy to toggle between all tests and stable tests
4. **Progress tracking**: See how many scenarios are still WIP

### Best Practices

1. **Always keep feature files and step files in sync**:
   - If a scenario has `@wip` in the feature file, use `BDD_WIP_TEST_CASE()` in the step file
   - If a scenario doesn't have `@wip`, use `BDD_TEST_CASE()` in the step file

2. **Mark scenarios as `@wip` when**:
   - The feature is not yet implemented
   - The test is written but the implementation is incomplete
   - The scenario is failing due to known issues

3. **Remove `@wip` tag when**:
   - The feature is fully implemented
   - The test passes consistently
   - Remember to change `BDD_WIP_TEST_CASE()` back to `BDD_TEST_CASE()`

### Example Test Output

With @wip filtering enabled (default):
```
Feature: Const Declarations

  Scenario: Basic const declaration with integer
    Given the Asthra compiler is available
      ✓ bdd_compiler_available() should be true
    ...
    Then the exit code should be 0
      ✓ execution_exit_code should equal expected_exit_code

  Scenario: Const declaration with string [@wip]
    ⏭️  SKIPPED (work in progress)

  Scenario: Const declaration with boolean [@wip]
    ⏭️  SKIPPED (work in progress)

Test Summary for 'Const Declarations':
  Passed: 89
  Failed: 0
  Skipped: 3
  Total:  92
```

### Common Issues

1. **@wip tag not working**: Make sure both the feature file has `@wip` AND the step file uses `BDD_WIP_TEST_CASE()`

2. **Tests still failing**: The framework only skips tests properly registered with `BDD_WIP_TEST_CASE()`. Check that you've updated the step implementation file.

3. **Want to run @wip tests**: Set `BDD_SKIP_WIP=0` environment variable:
   ```bash
   BDD_SKIP_WIP=0 ./run-bdd-tests-local.sh
   ```

## Asthra BDD Test Fixtures

This directory contains comprehensive test fixtures for the Asthra programming language, designed to support Behavior-Driven Development (BDD) testing with Cucumber.

### Directory Structure

```
bdd/fixtures/
├── valid/           # Valid Asthra programs
├── invalid/         # Invalid programs for error testing
├── examples/        # Edge cases and complex scenarios
└── performance/     # Performance and stress test fixtures
```

### Valid Programs (`valid/`)

#### Basic Examples
- **hello_world_basic.at** - Simplest valid Asthra program
- **hello_world_with_variables.at** - Hello world with variable declarations

#### Language Features
- **functions_basic.at** - Function definitions, calls, and visibility
- **structs_basic.at** - Struct definitions, fields, and methods
- **structs_with_generics.at** - Generic struct definitions and usage

#### Control Flow
- **control_flow_if_else.at** - If-else expressions and conditionals
- **control_flow_loops.at** - For loops, iterators, break/continue
- **control_flow_pattern_matching.at** - Match expressions with enums/options

#### Concurrency
- **concurrency_basic.at** - Spawn and spawn_with_handle demonstrations

#### FFI Integration
- **ffi_basic.at** - External C function declarations and usage
- **ffi_struct_interop.at** - C struct compatibility and memory management

### Invalid Programs (`invalid/`)

#### Syntax Errors
- **syntax_missing_package.at** - Missing package declaration
- **syntax_missing_semicolon.at** - Missing semicolons
- **syntax_invalid_function.at** - Function syntax errors
- **syntax_invalid_struct.at** - Struct syntax errors
- **syntax_invalid_control_flow.at** - Control flow syntax errors

#### Semantic Errors
- **type_mismatch.at** - Type system violations
- **semantic_undefined_symbols.at** - Undefined variables/functions/types
- **semantic_mutability_errors.at** - Immutability violations
- **semantic_visibility_errors.at** - Private member access violations

### Edge Cases (`examples/`)

- **edge_case_deeply_nested.at** - Deeply nested structures and control flow
- **edge_case_large_expressions.at** - Very long expressions and identifiers

### Performance Tests (`performance/`)

- **large_program.at** - Many functions, types, and symbols
- **memory_intensive.at** - Memory allocation and data structure operations

### Usage with Cucumber

These fixtures are designed to be used with Cucumber feature files. Example:

```gherkin
Feature: Asthra Compilation

  Scenario: Compile valid hello world program
    Given an Asthra source file "bdd/fixtures/valid/hello_world_basic.at"
    When I compile the program
    Then compilation should succeed
    And no errors should be reported

  Scenario: Detect missing package declaration
    Given an Asthra source file "bdd/fixtures/invalid/syntax_missing_package.at"
    When I compile the program
    Then compilation should fail
    And error should contain "package"
```

### Key Language Features Covered

1. **Package System** - All files demonstrate package declarations
2. **Type System** - Primitives, structs, enums, generics, options, results
3. **Expression-Oriented** - If-else expressions, match expressions
4. **Memory Safety** - Immutability by default, explicit mutability
5. **Concurrency** - Deterministic task spawning
6. **FFI Safety** - Ownership annotations, safe C interop
7. **Visibility** - Explicit public/private modifiers

### Testing Strategy

- **Positive Tests**: Valid programs should compile and run correctly
- **Negative Tests**: Invalid programs should produce specific error messages
- **Edge Cases**: Test parser/compiler limits and corner cases
- **Performance**: Measure compilation time and memory usage

### Contributing

When adding new fixtures:
1. Follow naming convention: `category_description.at`
2. Include descriptive comments explaining the test purpose
3. Ensure fixtures align with Asthra's design principles
4. Update this README with new fixtures

### License

All fixtures are licensed under the same terms as the Asthra project. See the main `LICENSE` file for details.
