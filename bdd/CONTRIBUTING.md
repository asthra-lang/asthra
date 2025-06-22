# Contributing to Asthra BDD Tests

Thank you for your interest in contributing to Asthra's BDD test suite! This guide will help you write effective BDD tests that maintain our quality standards and integrate seamlessly with the existing test infrastructure.

## Table of Contents

1. [Getting Started](#getting-started)
2. [Test Organization](#test-organization)
3. [Writing Your First BDD Test](#writing-your-first-bdd-test)
4. [Naming Conventions](#naming-conventions)
5. [Code Style Guidelines](#code-style-guidelines)
6. [Test Data and Fixtures](#test-data-and-fixtures)
7. [Submission Process](#submission-process)
8. [Review Criteria](#review-criteria)

## Getting Started

### Prerequisites

Before contributing BDD tests, ensure you:

1. Have read the [BDD Testing Guide](../docs/contributor/guides/bdd-testing-guide.md)
2. Have read the [BDD Best Practices](../docs/contributor/reference/bdd-best-practices.md)
3. Have a working development environment with BDD tests enabled
4. Understand Asthra's design principles and expression-oriented nature

### Setting Up

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

## Test Organization

### Choosing the Right Category

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

### Creating New Test Files

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

## Writing Your First BDD Test

### Step-by-Step Guide

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
       "package test;\n"
       "pub fn count(none) -> void {\n"
       "    for i in 0..10 {\n"
       "        print(i);\n"
       "    }\n"
       "    return ();\n"
       "}\n";
   
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

## Naming Conventions

### Test Files
- Pattern: `test_<component>_<aspect>.c`
- Examples:
  - `test_parser_expressions.c`
  - `test_semantic_types.c`
  - `test_codegen_functions.c`

### Scenario Names
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

### Test Data Variables
- Use descriptive names that indicate the test case
- Prefix with the type of test data

```c
const char* valid_function = "...";
const char* invalid_syntax = "...";
const char* edge_case_nested_loops = "...";
```

## Code Style Guidelines

### General C Style
- Follow the project's C coding standards
- Use 4 spaces for indentation (no tabs)
- Keep lines under 100 characters
- Use meaningful variable names

### BDD-Specific Style

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
       "package test;\n"
       "\n"  // Use blank lines for readability
       "pub fn example(x: int) -> int {\n"
       "    // Function body\n"
       "    return x * 2;\n"
       "}\n";
   ```

3. **Assertion Messages**
   ```c
   // Provide context for complex assertions
   if (!result.success) {
       printf("Parse failed at line %d: %s\n", 
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

## Test Data and Fixtures

### Using Existing Fixtures

Check `bdd/fixtures/` for existing test data before creating new files:

```c
bdd_given("a complex valid program");
char* source = read_fixture("valid/advanced/pattern_matching.at");
BDD_ASSERT_NOT_NULL(source);

// Use the fixture
// ...

free(source);
```

### Creating New Fixtures

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

### Fixture Guidelines
- Place in the correct category (`valid/`, `invalid/`, `examples/`)
- Use descriptive filenames
- Include header comments explaining the fixture's purpose
- Keep fixtures focused on specific test scenarios

## Submission Process

### Before Submitting

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

### Creating a Pull Request

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

### PR Description Template
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

## Review Criteria

Your BDD tests will be reviewed for:

### 1. **Correctness**
- Tests accurately validate the intended behavior
- Assertions are appropriate and sufficient
- No false positives or negatives

### 2. **Clarity**
- Scenario names clearly describe what's being tested
- Given-When-Then structure is properly used
- Code is easy to understand

### 3. **Coverage**
- Tests cover both success and failure cases
- Edge cases are considered
- No redundant tests with existing suite

### 4. **Style Compliance**
- Follows naming conventions
- Adheres to code style guidelines
- Properly organized in directory structure

### 5. **Performance**
- Tests run efficiently (unit tests < 100ms)
- No unnecessary loops or redundant operations
- Appropriate use of fixtures vs. inline data

### 6. **Maintenance**
- Tests are independent and isolated
- No hardcoded paths or environment assumptions
- Proper resource cleanup

## Common Review Feedback

### Issues to Avoid

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
       "fn calculate() {\n"
       "    let x = 1;  // Test variable declaration\n"
       "    return x;   // Test variable usage\n"
       "}\n";
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

## Getting Help

If you need assistance:

1. Check existing tests for examples
2. Review the documentation linked in this guide
3. Ask questions in GitHub Discussions
4. Tag `@asthra-lang/testing` team in your PR for specific feedback

## Thank You!

Your contributions help ensure Asthra remains reliable and well-tested. Every test you write helps prevent bugs and serves as documentation for future contributors. We appreciate your efforts in maintaining our high quality standards!