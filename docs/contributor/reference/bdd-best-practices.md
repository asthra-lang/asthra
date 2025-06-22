# BDD Best Practices for Asthra

This reference guide outlines best practices for writing and maintaining BDD tests in the Asthra project. Following these practices ensures consistent, maintainable, and effective tests that serve as both validation and documentation.

## Table of Contents

1. [Core Principles](#core-principles)
2. [Test Design Best Practices](#test-design-best-practices)
3. [Writing Effective Scenarios](#writing-effective-scenarios)
4. [Code Organization](#code-organization)
5. [Assertion Best Practices](#assertion-best-practices)
6. [Performance Considerations](#performance-considerations)
7. [Maintenance Guidelines](#maintenance-guidelines)
8. [Common Anti-Patterns](#common-anti-patterns)
9. [Language-Specific Considerations](#language-specific-considerations)

## Core Principles

### 1. Tests as Living Documentation

BDD tests should serve as executable documentation that clearly communicates system behavior.

✅ **Good Example**:
```c
bdd_scenario("Compiler rejects functions with duplicate parameter names");
bdd_given("a function definition with duplicate parameter names");
const char* source = 
    "package test;\n"
    "pub fn calculate(x: int, x: int) -> int {\n"
    "    return x;\n"
    "}\n";

bdd_when("compiling the source");
CompilerResult result = compile_string(source);

bdd_then("compilation fails with a duplicate parameter error");
BDD_ASSERT_FALSE(result.success);
BDD_ASSERT_STR_CONTAINS(result.error, "duplicate parameter");
BDD_ASSERT_STR_CONTAINS(result.error, "x");
```

❌ **Bad Example**:
```c
// Unclear what's being tested
bdd_scenario("Test case 42");
// No clear given/when/then structure
CompilerResult r = compile_string("pub fn f(x:int,x:int)->int{return x;}");
assert(!r.success);
```

### 2. Single Responsibility

Each test scenario should validate exactly one behavior or requirement.

✅ **Good Example**:
```c
// One scenario per type of expression
bdd_scenario("Parse integer literal expressions");
// ... test integer literals only

bdd_scenario("Parse string literal expressions");
// ... test string literals only

bdd_scenario("Parse binary arithmetic expressions");
// ... test binary expressions only
```

❌ **Bad Example**:
```c
// Testing multiple unrelated things
bdd_scenario("Parse all expression types");
// Tests integers, strings, binary ops, function calls, etc.
```

### 3. Independence

Tests should be completely independent and not rely on execution order or shared state.

✅ **Good Example**:
```c
int main(void) {
    bdd_init("Parser Tests");
    
    // Each scenario sets up its own state
    bdd_scenario("Parse variable declaration");
    ParserState* parser = parser_create();
    // ... test ...
    parser_destroy(parser);
    
    bdd_scenario("Parse function declaration");
    ParserState* parser2 = parser_create();  // New instance
    // ... test ...
    parser_destroy(parser2);
    
    return bdd_report();
}
```

## Test Design Best Practices

### 1. Follow Asthra's Expression-Oriented Design

Since Asthra is expression-oriented, tests must respect this design principle.

✅ **Good Example**:
```c
bdd_given("an if-else expression");
const char* source = 
    "package test;\n"
    "pub fn calculate(x: int) -> int {\n"
    "    let result = if x > 0 { x * 2 } else { 0 };\n"
    "    return result;\n"
    "}\n";

bdd_then("the if-else should have a type");
// Verify expression semantics
```

❌ **Bad Example**:
```c
// Treating if-else as a statement
const char* source = "if (x > 0) { print(x); }";  // Missing package, returns
```

### 2. Use Meaningful Test Data

Test data should be realistic and clearly demonstrate the behavior being tested.

✅ **Good Example**:
```c
bdd_given("a function that calculates fibonacci numbers");
const char* source = 
    "package math;\n"
    "\n"
    "pub fn fibonacci(n: int) -> int {\n"
    "    if n <= 1 {\n"
    "        return n;\n"
    "    } else {\n"
    "        return fibonacci(n - 1) + fibonacci(n - 2);\n"
    "    }\n"
    "}\n";
```

### 3. Test Edge Cases and Error Conditions

Always include tests for boundary conditions and error scenarios.

```c
// Edge case: Empty function body
bdd_scenario("Function with empty body requires explicit return");
const char* source = 
    "package test;\n"
    "pub fn noop(none) -> void {\n"
    "    // Empty function must have explicit return\n"
    "    return ();\n"
    "}\n";

// Error case: Missing return
bdd_scenario("Function without return statement fails compilation");
const char* missing_return = 
    "package test;\n"
    "pub fn broken(none) -> int {\n"
    "    // Missing return statement\n"
    "}\n";
```

## Writing Effective Scenarios

### 1. Use Business Language

Write scenarios in terms that stakeholders can understand.

✅ **Good**:
```c
bdd_scenario("User-defined types can be used as function parameters");
```

❌ **Bad**:
```c
bdd_scenario("AST node type 0x42 with symbol table entry");
```

### 2. Structure with Given-When-Then

Always use the Given-When-Then pattern for clarity:

- **Given**: Establishes context and preconditions
- **When**: Describes the action being taken
- **Then**: Specifies the expected outcome

```c
bdd_given("a module with circular imports");
// Set up source files with circular dependencies

bdd_when("attempting to compile the module");
// Execute compilation

bdd_then("the compiler detects and reports the circular dependency");
// Verify error detection and message
```

### 3. Keep Scenarios Focused

Each scenario should be concise and test one specific aspect.

✅ **Good**:
```c
bdd_scenario("Array index out of bounds is detected at compile time");
bdd_given("an array access with constant out-of-bounds index");
const char* source = 
    "package test;\n"
    "pub fn access_array(none) -> int {\n"
    "    let arr: [int; 5] = [1, 2, 3, 4, 5];\n"
    "    return arr[10];  // Index 10 is out of bounds\n"
    "}\n";
```

## Code Organization

### 1. Group Related Tests

Organize tests by component and functionality:

```
bdd/steps/unit/
├── parser/
│   ├── test_parser_expressions.c
│   ├── test_parser_statements.c
│   └── test_parser_declarations.c
├── semantic/
│   ├── test_semantic_types.c
│   ├── test_semantic_scopes.c
│   └── test_semantic_inference.c
└── codegen/
    ├── test_codegen_functions.c
    └── test_codegen_expressions.c
```

### 2. Share Common Setup

Use helper functions for common test setup:

```c
// In test_helpers.h
static inline CompilerOptions default_test_options(void) {
    return (CompilerOptions){
        .optimization_level = 0,
        .target = TARGET_X86_64,
        .debug_info = true
    };
}

// In tests
bdd_given("default compiler options");
CompilerOptions opts = default_test_options();
```

### 3. Maintain Test Fixtures

Keep test fixtures organized and documented:

```
bdd/fixtures/
├── valid/
│   ├── basic/          # Simple valid programs
│   ├── advanced/       # Complex valid programs
│   └── edge_cases/     # Valid edge cases
├── invalid/
│   ├── syntax/         # Syntax errors
│   ├── semantic/       # Semantic errors
│   └── type_errors/    # Type system errors
└── examples/
    └── real_world/     # Real-world code examples
```

## Assertion Best Practices

### 1. Use Specific Assertions

Choose the most specific assertion for your test:

```c
// Specific assertions provide better error messages
BDD_ASSERT_STR_EQ(result.type->name, "int");      // ✅ Good
BDD_ASSERT_TRUE(strcmp(result.type->name, "int") == 0);  // ❌ Less informative

BDD_ASSERT_STR_CONTAINS(error, "undefined variable");  // ✅ Good
BDD_ASSERT_TRUE(strstr(error, "undefined variable"));  // ❌ Less clear
```

### 2. Provide Context in Assertions

When using complex assertions, add context:

```c
bdd_then("all variables should be properly typed");
for (int i = 0; i < symbol_count; i++) {
    Symbol* sym = symbols[i];
    // Provide context for failures
    if (!sym->type) {
        printf("Symbol '%s' at line %d has no type\n", 
               sym->name, sym->line);
    }
    BDD_ASSERT_NOT_NULL(sym->type);
}
```

### 3. Assert Both Success and Error Details

For error cases, verify both the failure and the error message:

```c
bdd_then("compilation fails with a type mismatch error");
BDD_ASSERT_FALSE(result.success);
BDD_ASSERT_NOT_NULL(result.error);
BDD_ASSERT_STR_CONTAINS(result.error, "type mismatch");
BDD_ASSERT_STR_CONTAINS(result.error, "expected 'int'");
BDD_ASSERT_STR_CONTAINS(result.error, "found 'string'");
```

## Performance Considerations

### 1. Balance Coverage and Speed

- Unit tests should be fast (< 100ms per test)
- Integration tests can be slower but should stay under 1 second
- Use fixtures instead of generating test data in loops

```c
// ✅ Good: Pre-defined test cases
static const char* test_expressions[] = {
    "42",
    "3.14",
    "\"hello\"",
    "true",
    "x + y",
    "func(a, b)"
};

for (size_t i = 0; i < sizeof(test_expressions)/sizeof(char*); i++) {
    // Test each expression
}

// ❌ Bad: Generating test data in loops
for (int i = 0; i < 1000; i++) {
    char expr[100];
    sprintf(expr, "%d + %d", i, i+1);
    // This creates 1000 similar tests
}
```

### 2. Use Test Filtering

Support running subsets of tests for faster iteration:

```c
// Support filtering by environment variable
const char* filter = getenv("BDD_FILTER");
if (filter && !strstr(scenario_name, filter)) {
    return;  // Skip this scenario
}
```

## Maintenance Guidelines

### 1. Keep Tests Up-to-Date

When modifying compiler behavior:
1. Update affected tests first (TDD approach)
2. Ensure tests reflect new behavior
3. Remove obsolete tests
4. Update test documentation

### 2. Refactor Tests

Apply the same quality standards to tests as production code:

```c
// Extract common patterns into helpers
void assert_compilation_fails_with(const char* source, const char* expected_error) {
    CompilerResult result = compile_string(source);
    BDD_ASSERT_FALSE(result.success);
    BDD_ASSERT_NOT_NULL(result.error);
    BDD_ASSERT_STR_CONTAINS(result.error, expected_error);
}
```

### 3. Document Test Intentions

Add comments explaining complex test scenarios:

```c
bdd_scenario("Tail call optimization in mutual recursion");
// This tests that the compiler correctly identifies and optimizes
// tail calls in mutually recursive functions, which is critical
// for functional programming patterns in Asthra
bdd_given("two mutually recursive functions with tail calls");
// ... test implementation ...
```

## Common Anti-Patterns

### 1. Testing Implementation Details

❌ **Bad**: Testing internal compiler structures
```c
// Don't test internal AST node structure
BDD_ASSERT_EQ(ast->node_type, AST_BINARY_OP);
BDD_ASSERT_EQ(ast->data.binary.op_type, OP_ADD);
```

✅ **Good**: Testing observable behavior
```c
// Test the result of compilation
BDD_ASSERT_TRUE(result.success);
BDD_ASSERT_STR_EQ(execute_program(result.binary), "42");
```

### 2. Overly Complex Scenarios

❌ **Bad**: Kitchen sink scenarios
```c
bdd_scenario("Test complete compiler functionality");
// Tests parsing, semantic analysis, optimization, code generation,
// linking, and execution in one massive test
```

✅ **Good**: Focused scenarios
```c
bdd_scenario("Constant folding optimizes arithmetic expressions");
// Tests only constant folding optimization
```

### 3. Brittle Assertions

❌ **Bad**: Exact string matching for errors
```c
BDD_ASSERT_STR_EQ(error, "Error at line 3, column 15: Undefined variable 'x'");
```

✅ **Good**: Flexible error checking
```c
BDD_ASSERT_STR_CONTAINS(error, "undefined variable");
BDD_ASSERT_STR_CONTAINS(error, "'x'");
```

## Language-Specific Considerations

### 1. Respect Asthra's Design Principles

All tests must align with Asthra's 5 core design principles:

```c
// AI-friendly: Clear, predictable test patterns
bdd_scenario("Type inference follows local reasoning");

// Safe FFI: Test ownership annotations
bdd_scenario("FFI calls require explicit ownership transfer");

// Deterministic: Test reproducible behavior
bdd_scenario("Compilation produces identical output for identical input");

// Observable: Test diagnostic quality
bdd_scenario("Type errors include suggestion for fix");

// Pragmatic: Test essential features first
bdd_scenario("Basic arithmetic expressions compile correctly");
```

### 2. Test Asthra-Specific Features

Focus on Asthra's unique aspects:

```c
// Memory safety
bdd_scenario("Unsafe blocks are required for raw pointer operations");

// Expression-oriented
bdd_scenario("Block expressions return the last expression value");

// Pattern matching
bdd_scenario("Match expressions must be exhaustive");

// Ownership system
bdd_scenario("Move semantics prevent use-after-move");
```

### 3. Platform Considerations

Write platform-aware tests when necessary:

```c
#ifdef __APPLE__
bdd_scenario("macOS-specific memory alignment is respected");
#endif

#ifdef __linux__
bdd_scenario("Linux ELF binary format is generated correctly");
#endif
```

## Conclusion

Following these best practices ensures that BDD tests in Asthra remain valuable as both validation tools and living documentation. Remember that good tests are:

- **Clear**: Easy to understand the intent
- **Focused**: Test one thing at a time
- **Reliable**: Consistent results across runs
- **Maintainable**: Easy to update as the system evolves
- **Fast**: Quick feedback for developers

By adhering to these practices, you contribute to a robust test suite that helps maintain Asthra's quality and reliability while serving as excellent documentation for the language's behavior.