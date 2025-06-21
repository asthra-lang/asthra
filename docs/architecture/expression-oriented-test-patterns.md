# Expression-Oriented Test Patterns for Asthra

## Overview

Asthra is an expression-oriented language where most constructs return values rather than just performing actions. This document outlines the correct test patterns that respect Asthra's expression-oriented design and architectural requirements.

## Key Principles

### 1. Expression-Oriented Design
- **If-else as expressions**: Control flow constructs return values and have types
- **Explicit returns required**: Void functions must use `return ();` explicitly
- **Unit type support**: `()` represents void/no-value contexts
- **Type inference**: Every expression must have a well-defined type

### 2. Architectural Requirements
- **Semantic analysis before code generation**: All expressions must have type information
- **Phase separation**: Parser → Semantic Analysis → Code Generation (no cross-phase calls)
- **Type information attachment**: Expressions need TypeInfo structures from semantic analysis

## Correct Test Patterns

### 1. Testing Bare Expressions

When testing expression code generation, expressions must be analyzed in a proper context:

```c
// ❌ INCORRECT: Bare expression without context
ASTNode* expr = parse_test_expression_string("a + b");
code_generate_expression(generator, expr, REG_RAX); // FAILS: No type info

// ✅ CORRECT: Expression with semantic context
ASTNode* expr = parse_and_analyze_expression("a + b", analyzer);
code_generate_expression(generator, expr, REG_RAX); // SUCCESS: Has type info
```

### 2. Complete Program Context

Test programs must include all required elements:

```c
// ❌ INCORRECT: Missing package and types
const char* source = 
    "if (condition) { action(); } else { other(); }";

// ✅ CORRECT: Complete program with proper structure
const char* source = 
    "package test;\n"
    "\n"
    "pub fn main(none) -> void {\n"
    "    let result: i32 = if condition { 42 } else { 0 };\n"
    "    return ();\n"  // Explicit return required
    "}\n";
```

### 3. Expression Testing Helper

Use the `parse_and_analyze_expression()` helper for expression tests:

```c
/**
 * Parse and analyze an expression with proper context
 * This creates a minimal program with variable declarations,
 * runs semantic analysis, and returns the analyzed expression.
 */
ASTNode* parse_and_analyze_expression(const char* expr_str, SemanticAnalyzer* analyzer) {
    // Creates a program with common test variables (a, b, x, y, etc.)
    // Runs semantic analysis on the full program
    // Extracts and returns the expression with type information
}
```

### 4. Semantic Analyzer Connection

Always connect the semantic analyzer to the code generator:

```c
// ✅ CORRECT: Connect semantic analyzer
CodeGenerator* generator = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
generator->semantic_analyzer = analyzer;

// Or use the fixture pattern:
CodeGenTestFixture* fixture = setup_codegen_fixture();
// fixture->generator->semantic_analyzer is already connected
```

### 5. Control Flow as Expressions

Remember that if-else are expressions, not statements:

```c
// ✅ CORRECT: If-else as expression
"let value: i32 = if x > 0 { x } else { -x };"

// Note: Asthra grammar currently doesn't support if-else expressions
// This is a grammar limitation, not a test pattern issue
```

### 6. Return Statements

Always include explicit returns in void functions:

```c
// ❌ INCORRECT: Missing return
"pub fn process(none) -> void {\n"
"    action();\n"
"}"

// ✅ CORRECT: Explicit return
"pub fn process(none) -> void {\n"
"    action();\n"
"    return ();\n"
"}"
```

## Common Pitfalls to Avoid

### 1. Testing Without Semantic Analysis

Never attempt to generate code for expressions that haven't been semantically analyzed:

```c
// ❌ INCORRECT: Will trigger architectural violation
ASTNode* expr = parse_expression_string("x + y").ast;
code_generate_expression(generator, expr, REG_RAX);
```

### 2. Incomplete Test Programs

Always include package declarations and proper function signatures:

```c
// ❌ INCORRECT: Missing package
"fn main() { ... }"

// ✅ CORRECT: Complete program
"package test;\n"
"pub fn main(none) -> void { ... }"
```

### 3. Statement-Oriented Patterns

Avoid C-style statement patterns in tests:

```c
// ❌ INCORRECT: Statement-oriented
"x = 5;"
"if (x > 0) print(x);"

// ✅ CORRECT: Expression-oriented
"let x: i32 = 5;"
"let result: () = if x > 0 { print(x) } else { () };"
```

## Test Infrastructure Guidelines

### 1. Test Fixture Setup

Use the standard test fixture pattern:

```c
CodeGenTestFixture* fixture = setup_codegen_fixture();
// Includes:
// - Code generator with proper configuration
// - Semantic analyzer with initialized builtins
// - Connected semantic_analyzer field
// - Output buffer for results
```

### 2. Expression Parsing Helpers

For expression-specific tests, use the helpers in `expression_parsing_helper.h`:

- `parse_test_expression_string()`: Parse bare expression (no analysis)
- `parse_and_analyze_expression()`: Parse and analyze with context (recommended)

### 3. Cleanup

Always clean up test resources:

```c
cleanup_codegen_fixture(fixture);
ast_free_node(expr);
```

## Examples

### Example 1: Arithmetic Expression Test

```c
AsthraTestResult test_arithmetic_expression(AsthraTestContext* context) {
    CodeGenTestFixture* fixture = setup_codegen_fixture();
    
    // Parse and analyze expression with proper context
    const char* expr_str = "a + b * c";
    fixture->test_ast = parse_and_analyze_expression(expr_str, fixture->analyzer);
    
    // Generate code (semantic analyzer already connected)
    bool result = code_generate_expression(
        fixture->generator, 
        fixture->test_ast, 
        REG_RAX
    );
    
    // Verify success
    ASSERT_TRUE(result, "Expression code generation should succeed");
    
    cleanup_codegen_fixture(fixture);
    return ASTHRA_TEST_PASS;
}
```

### Example 2: Function Call Test

```c
const char* source = 
    "package test;\n"
    "\n"
    "pub struct Point { pub x: f64, pub y: f64 }\n"
    "\n"
    "impl Point {\n"
    "    pub fn distance(self, other: Point) -> f64 {\n"
    "        let dx: f64 = self.x - other.x;\n"
    "        let dy: f64 = self.y - other.y;\n"
    "        return sqrt(dx * dx + dy * dy);\n"
    "    }\n"
    "}\n"
    "\n"
    "pub fn main(none) -> void {\n"
    "    let p1: Point = Point { x: 0.0, y: 0.0 };\n"
    "    let p2: Point = Point { x: 3.0, y: 4.0 };\n"
    "    let dist: f64 = p1.distance(p2);\n"
    "    return ();\n"
    "}\n";
```

## Summary

Expression-oriented test patterns ensure that:
1. All expressions have proper type information from semantic analysis
2. Test programs are complete and valid Asthra code
3. The architectural separation between compiler phases is maintained
4. Tests accurately reflect how Asthra code will be compiled in practice

By following these patterns, tests will properly validate the compiler's behavior while respecting its architectural design principles.