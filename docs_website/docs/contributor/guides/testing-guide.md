# Testing Guide

**Comprehensive guide to testing in the Asthra compiler project**

This guide covers advanced testing strategies, patterns, and best practices for ensuring code quality and reliability in the Asthra compiler.

## Overview

- **Purpose**: Master testing strategies and contribute to test quality
- **Audience**: All contributors, from beginners to advanced
- **Prerequisites**: Basic understanding of testing concepts and C programming
- **Outcome**: Ability to write comprehensive, maintainable tests

## Testing Philosophy

Asthra follows a multi-layered testing approach that ensures reliability at every level:

1\. **Unit Tests**: Test individual functions and components in isolation
2\. **Integration Tests**: Test component interactions and workflows
3\. **System Tests**: Test complete compilation pipeline end-to-end
4\. **Performance Tests**: Ensure performance requirements are met
5\. **Regression Tests**: Prevent previously fixed bugs from reoccurring

### Quality Standards

- **Test Coverage**: Aim for >90% line coverage, >80% branch coverage
- **Test Reliability**: Tests should be deterministic and fast
- **Test Maintainability**: Clear, readable tests that serve as documentation
- **Test Isolation**: Each test should be independent and repeatable

## Unified Testing Framework

Asthra uses a standardized testing framework across all 31 test categories, providing consistent APIs and enhanced developer experience.

### Framework Modes

#### Lightweight Mode
For simple test categories with basic functionality:

```c
#include "../../src/framework/test_framework.h"

int main(void) {
    AsthraTestSuite *suite = asthra_test_suite_create_lightweight("Simple Tests");
    
    asthra_test_suite_add_test(suite, "test_basic_functionality", test_basic_functionality);
    asthra_test_suite_add_test(suite, "test_edge_cases", test_edge_cases);
    
    return asthra_test_suite_run_and_exit(suite);
}

void test_basic_functionality(void) {
    // Use standard framework assertions
    ASSERT_NOT_NULL(create_test_object());
    ASSERT_EQUALS_INT(calculate_value(5), 25);
    ASSERT_TRUE(validate_condition());
}
```

#### Standard Mode
For most test categories with enhanced features:

```c
int main(void) {
    AsthraTestSuite *suite = asthra_test_suite_create("Parser Tests", "Comprehensive parser testing");
    
    // Enhanced features available
    asthra_test_suite_add_test_with_timeout(suite, "test_complex_parsing", 
                                           test_complex_parsing, 30000); // 30s timeout
    asthra_test_suite_add_test_with_setup_teardown(suite, "test_with_resources",
                                                  test_with_resources, setup_resources, teardown_resources);
    
    return asthra_test_suite_run_and_exit(suite);
}
```

#### Detailed Mode
For complex test categories with custom analysis:

```c
int main(void) {
    AsthraTestSuite *suite = asthra_test_suite_create_detailed("Codegen Tests", 
                                                              "Advanced code generation testing");
    
    // Custom configuration
    AsthraTestSuiteConfig config = {
        .statistics_tracking = true,
        .reporting_level = ASTHRA_TEST_REPORTING_DETAILED,
        .default_timeout_ns = 60000000000ULL, // 60s
        .parallel_execution = false
    };
    asthra_test_suite_set_config(suite, &amp;config);
    
    // Custom analysis callback for domain-specific validation
    asthra_test_suite_set_analysis_callback(suite, custom_analysis_callback);
    
    return asthra_test_suite_run_and_exit(suite);
}
```

## Test Categories and Organization

### Core Compiler Tests

#### Lexer Tests (`tests/lexer/`)
Test tokenization and lexical analysis:

```c
// Test numeric literal parsing
void test_lexer_hex_literals(void) {
    const char *source = "0xFF 0x123 0xABCDEF";
    LexerResult result = lexer_tokenize(source);
    
    ASSERT_TRUE(result.success);
    ASSERT_EQUALS_INT(result.token_count, 4); // 3 numbers + EOF
    
    // Verify each hex literal
    ASSERT_EQUALS_INT(result.tokens&#91;0&#93;.type, TOKEN_NUMBER);
    ASSERT_EQUALS_STRING(result.tokens&#91;0&#93;.value, "0xFF");
    ASSERT_EQUALS_INT(result.tokens&#91;0&#93;.location.line, 1);
    ASSERT_EQUALS_INT(result.tokens&#91;0&#93;.location.column, 1);
    
    lexer_result_free(&amp;result);
}

// Test error recovery
void test_lexer_error_recovery(void) {
    const char *source = "valid_token @invalid_char another_token";
    LexerResult result = lexer_tokenize(source);
    
    ASSERT_FALSE(result.success);
    ASSERT_NOT_NULL(result.error_message);
    ASSERT_TRUE(strstr(result.error_message, "invalid_char") != NULL);
    
    lexer_result_free(&amp;result);
}
```

#### Parser Tests (`tests/parser/`)
Test AST construction and grammar rules:

```c
// Test function declaration parsing
void test_parser_function_declaration(void) {
    const char *source = "fn add(a: i32, b: i32) -&gt; i32 { return a + b; }";
    
    LexerResult lex_result = lexer_tokenize(source);
    ASSERT_TRUE(lex_result.success);
    
    ParseResult parse_result = parser_parse(lex_result.tokens, lex_result.token_count);
    ASSERT_TRUE(parse_result.success);
    ASSERT_NOT_NULL(parse_result.ast);
    
    // Verify AST structure
    ASTNode *root = parse_result.ast;
    ASSERT_EQUALS_INT(root->type, AST_PROGRAM);
    ASSERT_EQUALS_INT(root->child_count, 1);
    
    ASTNode *function = root->children&#91;0&#93;;
    ASSERT_EQUALS_INT(function->type, AST_FUNCTION);
    ASSERT_EQUALS_STRING(function->data.function.name, "add");
    ASSERT_EQUALS_INT(function->data.function.parameter_count, 2);
    
    lexer_result_free(&amp;lex_result);
    parse_result_free(&amp;parse_result);
}
```

#### Semantic Analysis Tests (`tests/semantic/`)
Test type checking and symbol resolution:

```c
// Test type inference
void test_semantic_type_inference(void) {
    const char *source = "let x = 42; let y = x + 10;";
    
    CompilationContext ctx;
    compilation_context_init(&amp;ctx);
    
    SemanticResult result = semantic_analyze_source(&amp;ctx, source);
    ASSERT_TRUE(result.success);
    
    // Check inferred types
    Symbol *x_symbol = symbol_table_lookup(result.symbols, "x");
    ASSERT_NOT_NULL(x_symbol);
    ASSERT_EQUALS_INT(x_symbol->type_info->kind, TYPE_PRIMITIVE);
    ASSERT_EQUALS_INT(x_symbol->type_info->data.primitive, PRIMITIVE_I32);
    
    Symbol *y_symbol = symbol_table_lookup(result.symbols, "y");
    ASSERT_NOT_NULL(y_symbol);
    ASSERT_EQUALS_INT(y_symbol->type_info->kind, TYPE_PRIMITIVE);
    ASSERT_EQUALS_INT(y_symbol->type_info->data.primitive, PRIMITIVE_I32);
    
    semantic_result_free(&amp;result);
    compilation_context_cleanup(&amp;ctx);
}

// Test error detection
void test_semantic_type_mismatch_error(void) {
    const char *source = "let x: i32 = \"hello\";"; // Type mismatch
    
    CompilationContext ctx;
    compilation_context_init(&amp;ctx);
    
    SemanticResult result = semantic_analyze_source(&amp;ctx, source);
    ASSERT_FALSE(result.success);
    ASSERT_NOT_NULL(result.error_message);
    ASSERT_TRUE(strstr(result.error_message, "type mismatch") != NULL);
    
    semantic_result_free(&amp;result);
    compilation_context_cleanup(&amp;ctx);
}
```

#### Code Generation Tests (`tests/codegen/`)
Test C code generation and compilation:

```c
// Test function code generation
void test_codegen_function_generation(void) {
    const char *source = "fn factorial(n: i32) -&gt; i32 { if n &lt;= 1 { return 1; } return n * factorial(n - 1); }";
    
    CodegenResult result = compile_to_c_code(source, "test_factorial.c");
    ASSERT_TRUE(result.success);
    
    // Verify generated C code exists and compiles
    ASSERT_TRUE(file_exists("test_factorial.c"));
    
    CompilationResult c_result = compile_c_file("test_factorial.c", "test_factorial");
    ASSERT_TRUE(c_result.success);
    
    // Test execution
    ExecutionResult exec_result = run_executable_with_args("test_factorial", NULL);
    ASSERT_EQUALS_INT(exec_result.exit_code, 0);
    
    cleanup_files("test_factorial.c", "test_factorial", NULL);
    codegen_result_free(&amp;result);
}
```

## Advanced Testing Patterns

### Parameterized Tests

```c
// Test data structure
typedef struct {
    const char *input;
    int expected_token_count;
    TokenType expected_first_token;
} LexerTestCase;

// Test cases
static LexerTestCase lexer_test_cases&#91;&#93; = {
    {"42", 2, TOKEN_NUMBER},
    {"\"hello\"", 2, TOKEN_STRING},
    {"identifier", 2, TOKEN_IDENTIFIER},
    {"fn main() {}", 6, TOKEN_FN},
    {NULL, 0, TOKEN_EOF} // Sentinel
};

// Parameterized test function
void test_lexer_parameterized(void) {
    for (int i = 0; lexer_test_cases&#91;i&#93;.input != NULL; i++) {
        LexerTestCase *test_case = &amp;lexer_test_cases&#91;i&#93;;
        
        LexerResult result = lexer_tokenize(test_case-&gt;input);
        
        // Use descriptive assertion messages
        char message&#91;256&#93;;
        snprintf(message, sizeof(message), "Test case %d: input='%s'", i, test_case->input);
        
        ASSERT_TRUE_MSG(result.success, message);
        ASSERT_EQUALS_INT_MSG(result.token_count, test_case->expected_token_count, message);
        ASSERT_EQUALS_INT_MSG(result.tokens&#91;0&#93;.type, test_case->expected_first_token, message);
        
        lexer_result_free(&amp;result);
    }
}
```

### Property-Based Testing

```c
// Property: Lexer should always produce valid tokens
void test_lexer_property_valid_tokens(void) {
    for (int i = 0; i &lt; 100; i++) {
        // Generate random valid input
        char *random_input = generate_random_asthra_code(i);
        
        LexerResult result = lexer_tokenize(random_input);
        
        if (result.success) {
            // Property: All tokens should have valid types
            for (size_t j = 0; j &lt; result.token_count; j++) {
                ASSERT_TRUE(is_valid_token_type(result.tokens&#91;j&#93;.type));
                ASSERT_NOT_NULL(result.tokens&#91;j&#93;.value);
            }
            
            // Property: Last token should be EOF
            ASSERT_EQUALS_INT(result.tokens&#91;result.token_count - 1&#93;.type, TOKEN_EOF);
        }
        
        lexer_result_free(&amp;result);
        free(random_input);
    }
}
```

## Test Execution and Automation

### Running Tests

#### Basic Test Execution
```bash
# Run all tests
make test-all

# Run specific test categories
make test-lexer
make test-parser
make test-semantic
make test-codegen

# Run with verbose output
make test-all TEST_VERBOSE=1

# Run with specific format
make test-all TEST_FORMAT=json
make test-all TEST_FORMAT=html
make test-all TEST_FORMAT=junit
```

#### Advanced Test Options
```bash
# Parallel execution
make test-all TEST_PARALLEL=8

# Timeout configuration
make test-all TEST_TIMEOUT=60

# Fail fast mode
make test-all TEST_FAIL_FAST=1

# Filter tests by pattern
make test-all TEST_FILTER="lexer_*"

# Retry failed tests
make test-all TEST_RETRY_COUNT=3

# Memory testing
make test-all TEST_MEMORY_CHECK=1
```

## Success Criteria

You've mastered testing in Asthra when:

- &#91; &#93; **Framework Proficiency**: Can use all three testing framework modes effectively
- &#91; &#93; **Test Categories**: Understand when to use unit, integration, and system tests
- &#91; &#93; **Advanced Patterns**: Can implement parameterized, property-based, and performance tests
- &#91; &#93; **Test Quality**: Write maintainable, reliable tests with good coverage
- &#91; &#93; **Debugging Skills**: Can efficiently debug and fix test failures
- &#91; &#93; **Automation**: Understand CI/CD integration and automated testing workflows

## Related Guides

- **&#91;Testing Workflows&#93;(../workflows/testing-workflows.md)** - Comprehensive testing strategies and workflows
- **&#91;Code Review Process&#93;(../workflows/code-review.md)** - Review guidelines including test review
- **&#91;Performance Optimization&#93;(../workflows/performance-optimization.md)** - Performance testing strategies
- **&#91;Compiler Development&#93;(compiler-development.md)** - Testing compiler components

## Reference Materials

- **&#91;Testing Framework API&#93;(../reference/testing-framework.md)** - Complete API documentation
- **&#91;Test Categories&#93;(../reference/test-categories.md)** - All 31 test categories explained
- **&#91;CI/CD Integration&#93;(../reference/ci-cd.md)** - Continuous integration setup
- **&#91;Performance Benchmarks&#93;(../reference/performance-benchmarks.md)** - Performance testing standards

---

**🧪 Testing Mastery Achieved!**

*Comprehensive testing ensures code quality, prevents regressions, and builds confidence in the Asthra compiler. Well-written tests serve as both verification and documentation of expected behavior.*
