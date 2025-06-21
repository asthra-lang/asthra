# Testing Best Practices Guide

**Version**: 1.0  
**Date**: 2025-01-27  
**Target Audience**: Asthra Developers and Contributors

## Table of Contents

- [Overview](#overview)
- [Framework Selection](#framework-selection)
- [Test Structure and Organization](#test-structure-and-organization)
- [Writing Effective Tests](#writing-effective-tests)
- [Performance Testing](#performance-testing)
- [Memory Safety Testing](#memory-safety-testing)
- [CI/CD Integration](#cicd-integration)
- [Debugging and Troubleshooting](#debugging-and-troubleshooting)
- [Advanced Testing Patterns](#advanced-testing-patterns)

## Overview

This guide establishes best practices for writing, organizing, and maintaining tests in the Asthra compiler project. All tests now use the **unified testing framework** with three modes (Lightweight, Standard, Detailed) to provide consistent APIs and enhanced capabilities across all 31 test categories.

### Core Principles

1. **Consistency**: Use standardized framework patterns across all tests
2. **Clarity**: Write descriptive test names and clear assertions
3. **Reliability**: Ensure tests are deterministic and robust
4. **Performance**: Minimize test execution time while maintaining thoroughness
5. **Maintainability**: Structure tests for easy understanding and modification

## Framework Selection

### Choosing the Right Framework Mode

**Lightweight Mode** - Use for simple functionality tests:
```c
// When to use: Basic unit tests, simple validation, minimal setup
#include "../../src/framework/test_framework.h"

int main(void) {
    AsthraTestSuite *suite = asthra_test_suite_create_lightweight("Token Recognition Tests");
    
    asthra_test_suite_add_test(suite, "test_keyword_recognition", test_keyword_recognition);
    asthra_test_suite_add_test(suite, "test_identifier_parsing", test_identifier_parsing);
    
    return asthra_test_suite_run_and_exit(suite);
}
```

**Standard Mode** - Use for comprehensive testing:
```c
// When to use: Integration tests, complex setup/teardown, timeout requirements
int main(void) {
    AsthraTestSuite *suite = asthra_test_suite_create("Parser Integration Tests", 
                                                     "Comprehensive parser functionality testing");
    
    asthra_test_suite_add_test_with_timeout(suite, "test_large_file_parsing", 
                                           test_large_file_parsing, 30000);
    asthra_test_suite_add_test_with_setup_teardown(suite, "test_parser_state_management",
                                                  test_parser_state_management, 
                                                  setup_parser_environment, 
                                                  teardown_parser_environment);
    
    return asthra_test_suite_run_and_exit(suite);
}
```

**Detailed Mode** - Use for complex scenarios with custom analysis:
```c
// When to use: Performance analysis, custom statistics, legacy framework integration
int main(void) {
    AsthraTestSuite *suite = asthra_test_suite_create_detailed("Code Generation Performance Tests",
                                                              "Advanced code generation analysis");
    
    AsthraTestSuiteConfig config = {
        .statistics_tracking = true,
        .reporting_level = ASTHRA_TEST_REPORTING_DETAILED,
        .default_timeout_ns = 60000000000ULL,
        .parallel_execution = false
    };
    asthra_test_suite_set_config(suite, &config);
    asthra_test_suite_set_analysis_callback(suite, performance_analysis_callback);
    
    return asthra_test_suite_run_and_exit(suite);
}
```

### Framework Mode Selection Matrix

| Test Category | Recommended Mode | Reason |
|---------------|------------------|--------|
| Utils, Basic, Character | Lightweight | Simple functionality, minimal overhead |
| Parser, Semantic, Types | Standard | Comprehensive testing with enhanced features |
| Codegen, Runtime, Performance | Detailed | Custom analysis, performance metrics |
| Legacy Integration | Detailed + Adapter | Preserve existing functionality |

## Test Structure and Organization

### File Organization

**Naming Conventions**:
```
tests/
├── category/
│   ├── test_category_basic.c          # Basic functionality tests
│   ├── test_category_advanced.c       # Complex feature tests
│   ├── test_category_performance.c    # Performance-specific tests
│   ├── test_category_integration.c    # Integration tests
│   └── test_category_edge_cases.c     # Edge case and error handling
```

**Test Function Naming**:
```c
// Pattern: test_<component>_<functionality>_<scenario>
void test_lexer_keyword_recognition_basic(void);
void test_parser_expression_parsing_nested(void);
void test_semantic_type_validation_generic_structs(void);
void test_codegen_function_calls_with_self_parameter(void);
```

### Test Function Structure

**Standard Test Pattern**:
```c
void test_feature_functionality(void) {
    // 1. Setup - Prepare test environment
    TestObject *obj = create_test_object();
    ASSERT_NOT_NULL(obj);
    
    // 2. Input preparation
    const char *test_input = "test data";
    
    // 3. Execute the functionality being tested
    Result result = feature_function(obj, test_input);
    
    // 4. Verify results with specific assertions
    ASSERT_TRUE(result.success);
    ASSERT_EQUALS_STRING(result.output, "expected output");
    ASSERT_GREATER_THAN_INT(result.value, 0);
    
    // 5. Cleanup resources
    destroy_test_object(obj);
}
```

**Error Testing Pattern**:
```c
void test_feature_error_handling(void) {
    // Test NULL pointer handling
    Result result1 = feature_function(NULL, "input");
    ASSERT_FALSE(result1.success);
    ASSERT_EQUALS_INT(result1.error_code, ERROR_NULL_POINTER);
    
    // Test invalid input handling
    TestObject *obj = create_test_object();
    Result result2 = feature_function(obj, NULL);
    ASSERT_FALSE(result2.success);
    ASSERT_EQUALS_INT(result2.error_code, ERROR_INVALID_INPUT);
    
    // Test empty input handling
    Result result3 = feature_function(obj, "");
    ASSERT_FALSE(result3.success);
    ASSERT_EQUALS_INT(result3.error_code, ERROR_EMPTY_INPUT);
    
    destroy_test_object(obj);
}
```

## Writing Effective Tests

### Assertion Best Practices

**Use Specific Assertions**:
```c
// ❌ Generic assertions
ASSERT_TRUE(value == 42);
ASSERT_TRUE(strcmp(str, "expected") == 0);

// ✅ Specific assertions  
ASSERT_EQUALS_INT(value, 42);
ASSERT_EQUALS_STRING(str, "expected");
```

**Comprehensive Assertion Library**:
```c
// Basic assertions
ASSERT_TRUE(condition);
ASSERT_FALSE(condition);
ASSERT_NULL(pointer);
ASSERT_NOT_NULL(pointer);

// Equality assertions
ASSERT_EQUALS_INT(actual, expected);
ASSERT_EQUALS_STRING(actual, expected);
ASSERT_EQUALS_DOUBLE(actual, expected, tolerance);

// Comparison assertions
ASSERT_GREATER_THAN_INT(actual, expected);
ASSERT_LESS_THAN_INT(actual, expected);
ASSERT_GREATER_THAN_OR_EQUAL_DOUBLE(actual, expected);

// Memory assertions
ASSERT_NO_MEMORY_LEAKS();
ASSERT_VALID_POINTER(ptr);

// Collection assertions
ASSERT_ARRAY_EQUALS_INT(actual_array, expected_array, length);
ASSERT_CONTAINS_STRING(haystack, needle);
```

### Test Data Management

**Embedded Test Data**:
```c
void test_parser_expression_parsing(void) {
    // Small, focused test data embedded in test
    const char *expressions[] = {
        "42 + 37",
        "(x * y) / z", 
        "function_call(arg1, arg2)",
        "struct_access.field_name"
    };
    
    for (size_t i = 0; i < ARRAY_LENGTH(expressions); i++) {
        Result result = parse_expression(expressions[i]);
        ASSERT_TRUE(result.success);
    }
}
```

**External Test Data Files**:
```c
void test_parser_large_program_parsing(void) {
    // Large test data in separate files
    char *program_source = load_test_file("tests/data/large_program.asthra");
    ASSERT_NOT_NULL(program_source);
    
    Result result = parse_program(program_source);
    ASSERT_TRUE(result.success);
    ASSERT_NOT_NULL(result.ast);
    
    free(program_source);
}
```

### Test Documentation

**Function Documentation**:
```c
/**
 * @brief Test struct field access code generation with nested structures
 * 
 * Verifies that nested struct field access like `outer.inner.field` 
 * generates correct C code with proper pointer dereferencing.
 * 
 * Test cases:
 * - Simple field access: obj.field
 * - Nested field access: obj.inner.field  
 * - Deep nesting: obj.a.b.c.field
 * - Mixed with array access: obj.array[0].field
 */
void test_codegen_nested_struct_field_access(void) {
    // Implementation...
}
```

**Test Suite Documentation**:
```c
int main(void) {
    AsthraTestSuite *suite = asthra_test_suite_create("Lexer Token Recognition Tests",
        "Comprehensive testing of lexer token recognition including:\n"
        "- Keyword identification (pub, fn, struct, etc.)\n"
        "- Operator recognition (+, -, *, /, ==, !=, etc.)\n" 
        "- Literal parsing (integers, floats, strings, chars)\n"
        "- Identifier validation and reserved word handling\n"
        "- Error recovery and position tracking");
        
    // Add tests...
}
```

## Performance Testing

### Performance Test Structure

**Basic Performance Testing**:
```c
void test_lexer_performance_large_file(void) {
    // Generate or load large test data
    const size_t file_size = 1024 * 1024; // 1MB
    char *large_source = generate_test_source(file_size);
    ASSERT_NOT_NULL(large_source);
    
    // Measure execution time
    clock_t start = clock();
    
    Lexer *lexer = lexer_create(large_source);
    ASSERT_NOT_NULL(lexer);
    
    // Process all tokens
    int token_count = 0;
    while (lexer_has_more_tokens(lexer)) {
        Token *token = lexer_next_token(lexer);
        ASSERT_NOT_NULL(token);
        token_destroy(token);
        token_count++;
    }
    
    clock_t end = clock();
    double duration = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // Performance assertions
    double throughput = file_size / duration;
    ASSERT_GREATER_THAN_DOUBLE(throughput, 10 * 1024 * 1024); // 10MB/sec minimum
    ASSERT_LESS_THAN_DOUBLE(duration, 0.5); // Complete within 500ms
    ASSERT_GREATER_THAN_INT(token_count, 1000); // Reasonable token count
    
    // Cleanup
    lexer_destroy(lexer);
    free(large_source);
}
```

**Memory Usage Testing**:
```c
void test_parser_memory_efficiency(void) {
    size_t initial_memory = get_current_memory_usage();
    
    // Parse multiple programs
    for (int i = 0; i < 100; i++) {
        char *source = generate_test_program(1000); // 1000 lines
        
        Parser *parser = parser_create(source);
        ASTNode *ast = parser_parse_program(parser);
        
        ASSERT_NOT_NULL(ast);
        
        // Cleanup immediately to test memory management
        ast_node_destroy(ast);
        parser_destroy(parser);
        free(source);
    }
    
    size_t final_memory = get_current_memory_usage();
    size_t memory_growth = final_memory - initial_memory;
    
    // Memory growth should be minimal (< 1MB for 100 programs)
    ASSERT_LESS_THAN_INT(memory_growth, 1024 * 1024);
}
```

### Performance Benchmarking

**Benchmark Test Framework**:
```c
typedef struct {
    const char *name;
    void (*setup)(void);
    void (*benchmark)(void);
    void (*teardown)(void);
    double expected_max_time_ms;
} PerformanceBenchmark;

void run_performance_benchmarks(PerformanceBenchmark *benchmarks, size_t count) {
    for (size_t i = 0; i < count; i++) {
        PerformanceBenchmark *bench = &benchmarks[i];
        
        if (bench->setup) bench->setup();
        
        clock_t start = clock();
        bench->benchmark();
        clock_t end = clock();
        
        if (bench->teardown) bench->teardown();
        
        double duration_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
        
        printf("Benchmark %s: %.2fms (limit: %.2fms)\n", 
               bench->name, duration_ms, bench->expected_max_time_ms);
               
        ASSERT_LESS_THAN_DOUBLE(duration_ms, bench->expected_max_time_ms);
    }
}
```

## Memory Safety Testing

### Memory Leak Detection

**Automatic Leak Detection**:
```c
void test_parser_memory_management(void) {
    // Framework automatically tracks memory allocations
    
    Parser *parser = parser_create("test program");
    ASSERT_NOT_NULL(parser);
    
    ASTNode *ast = parser_parse_program(parser);
    ASSERT_NOT_NULL(ast);
    
    // Test normal cleanup
    ast_node_destroy(ast);
    parser_destroy(parser);
    
    // Framework verifies no memory leaks at test completion
    ASSERT_NO_MEMORY_LEAKS();
}
```

**Manual Memory Tracking**:
```c
void test_complex_memory_scenario(void) {
    size_t initial_allocations = get_allocation_count();
    
    // Complex operations that allocate/deallocate memory
    for (int i = 0; i < 1000; i++) {
        Object *obj = create_object(i);
        process_object(obj);
        destroy_object(obj);
    }
    
    size_t final_allocations = get_allocation_count();
    
    // Verify all allocations were cleaned up
    ASSERT_EQUALS_INT(final_allocations, initial_allocations);
}
```

### AddressSanitizer Integration

**Running Tests with AddressSanitizer**:
```bash
# Build and run tests with AddressSanitizer
make test-asan

# Run specific category with AddressSanitizer
make test-asan CATEGORY=parser

# Debug specific test with AddressSanitizer
make debug-test-asan TEST=test_parser_expressions
```

**AddressSanitizer-Aware Test Code**:
```c
void test_buffer_operations(void) {
    // AddressSanitizer will detect buffer overflows
    char buffer[100];
    
    // Safe operations
    strncpy(buffer, "test string", sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    ASSERT_EQUALS_STRING(buffer, "test string");
    
    // AddressSanitizer will catch any boundary violations
}
```

## CI/CD Integration

### Automated Test Execution

**GitHub Actions Integration**:
```yaml
name: Asthra Test Suite
on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    strategy:
      matrix:
        build_type: [debug, release, asan]
        
    steps:
    - uses: actions/checkout@v2
    
    - name: Install Dependencies
      run: |
        sudo apt-get update
        sudo apt-get install clang libjson-c-dev
        
    - name: Build Asthra
      run: make ${{ matrix.build_type }}
      
    - name: Run Tests
      run: |
        make test-ci TEST_FORMAT=junit TEST_FAIL_FAST=1 TEST_RETRY_COUNT=3
        
    - name: Generate Reports
      run: |
        make test-html
        make test-trends --days 7
        
    - name: Upload Test Results
      uses: actions/upload-artifact@v2
      with:
        name: test-results-${{ matrix.build_type }}
        path: build/test-results/
```

**Local CI Testing**:
```bash
# Simulate CI environment locally
make clean && make test-ci

# Test with different configurations
TEST_FORMAT=json TEST_FAIL_FAST=1 make test-ci
TEST_PARALLEL=4 TEST_TIMEOUT=60 make test-ci
```

### Test Result Analysis

**Automated Trend Analysis**:
```bash
# Run trend analysis on recent results
scripts/test_trend_analysis.py --days 30 --format html

# Category-specific analysis
scripts/test_trend_analysis.py --category parser --export csv

# Performance regression detection
scripts/test_trend_analysis.py --metrics performance --threshold 10%
```

**CI/CD Reporting**:
```bash
# Generate CI-friendly reports
make test-junit    # JUnit XML for CI systems
make test-json     # JSON for custom processing
make test-csv      # CSV for data analysis
```

## Debugging and Troubleshooting

### Test Debugging Techniques

**Verbose Test Execution**:
```bash
# Run with detailed output
make test-parser TEST_VERBOSE=1

# Debug single test
make debug-test-parser-expressions
gdb ./build/tests/parser/test_parser_expressions
```

**In-Test Debugging**:
```c
void test_complex_parsing_scenario(void) {
    // Enable debug output
    #ifdef ASTHRA_TEST_DEBUG
    printf("DEBUG: Starting complex parsing test\n");
    #endif
    
    Parser *parser = parser_create("complex program");
    
    #ifdef ASTHRA_TEST_DEBUG
    printf("DEBUG: Parser created, state: %d\n", parser->state);
    #endif
    
    ASTNode *ast = parser_parse_program(parser);
    
    // Conditional debugging based on result
    if (!ast) {
        printf("ERROR: Parse failed, last token: %s\n", parser->current_token->value);
        parser_dump_state(parser);
    }
    
    ASSERT_NOT_NULL(ast);
    
    parser_destroy(parser);
}
```

### Common Issues and Solutions

**Test Timeouts**:
```c
// ❌ Test may timeout due to infinite loop
void test_parser_recursive_descent(void) {
    // Missing termination condition
    while (parser_has_tokens(parser)) {
        // Potential infinite loop
    }
}

// ✅ Proper timeout handling
void test_parser_recursive_descent(void) {
    int max_iterations = 10000;
    int iteration_count = 0;
    
    while (parser_has_tokens(parser) && iteration_count < max_iterations) {
        process_token(parser);
        iteration_count++;
    }
    
    ASSERT_LESS_THAN_INT(iteration_count, max_iterations);
}
```

**Memory Leaks**:
```c
// ❌ Memory leak in error path
void test_with_memory_leak(void) {
    Object *obj = create_object();
    
    if (some_condition()) {
        return; // Memory leak - obj not freed
    }
    
    destroy_object(obj);
}

// ✅ Proper cleanup in all paths
void test_without_memory_leak(void) {
    Object *obj = create_object();
    
    if (some_condition()) {
        destroy_object(obj);
        return;
    }
    
    destroy_object(obj);
}
```

### Test Framework Debug Features

**Debug Macros**:
```c
// Available debug macros
TEST_DEBUG_PRINT("Processing token: %s", token->value);
TEST_DEBUG_ASSERT(condition, "Custom failure message");
TEST_DEBUG_MEMORY_DUMP();
TEST_DEBUG_STACK_TRACE();
```

**Framework Configuration**:
```c
// Enable comprehensive debugging
#define ASTHRA_TEST_DEBUG 1
#define ASTHRA_TEST_MEMORY_TRACKING 1
#define ASTHRA_TEST_VERBOSE_ASSERTIONS 1

// Framework will provide detailed output and memory tracking
```

## Advanced Testing Patterns

### Property-Based Testing

**Property Testing Pattern**:
```c
void test_lexer_token_roundtrip_property(void) {
    // Property: For any valid token, lexing its string representation
    // should produce the same token
    
    TokenType types[] = {TOKEN_IDENTIFIER, TOKEN_INTEGER, TOKEN_STRING, TOKEN_KEYWORD};
    
    for (size_t i = 0; i < ARRAY_LENGTH(types); i++) {
        for (int iteration = 0; iteration < 100; iteration++) {
            // Generate random valid token
            Token *original = generate_random_token(types[i]);
            
            // Convert to string and lex again
            char *token_string = token_to_string(original);
            Token *roundtrip = lex_token_from_string(token_string);
            
            // Property: Should be equivalent
            ASSERT_EQUALS_INT(original->type, roundtrip->type);
            ASSERT_EQUALS_STRING(original->value, roundtrip->value);
            
            // Cleanup
            token_destroy(original);
            token_destroy(roundtrip);
            free(token_string);
        }
    }
}
```

### Parameterized Testing

**Parameterized Test Pattern**:
```c
typedef struct {
    const char *input;
    TokenType expected_type;
    const char *expected_value;
} TokenTestCase;

void test_lexer_token_recognition_parameterized(void) {
    TokenTestCase test_cases[] = {
        {"42", TOKEN_INTEGER, "42"},
        {"3.14", TOKEN_FLOAT, "3.14"},
        {"\"hello\"", TOKEN_STRING, "hello"},
        {"identifier", TOKEN_IDENTIFIER, "identifier"},
        {"pub", TOKEN_KEYWORD, "pub"},
        {"fn", TOKEN_KEYWORD, "fn"}
    };
    
    for (size_t i = 0; i < ARRAY_LENGTH(test_cases); i++) {
        TokenTestCase *tc = &test_cases[i];
        
        Token *token = lex_single_token(tc->input);
        ASSERT_NOT_NULL(token);
        ASSERT_EQUALS_INT(token->type, tc->expected_type);
        ASSERT_EQUALS_STRING(token->value, tc->expected_value);
        
        token_destroy(token);
    }
}
```

### Snapshot Testing

**Snapshot Testing for AST**:
```c
void test_parser_ast_snapshot(void) {
    const char *source = 
        "pub fn fibonacci(n: i32) -> i32 {\n"
        "    if n <= 1 {\n"
        "        return n;\n"
        "    }\n"
        "    return fibonacci(n - 1) + fibonacci(n - 2);\n"
        "}\n";
    
    ASTNode *ast = parse_program(source);
    ASSERT_NOT_NULL(ast);
    
    char *ast_string = ast_to_string(ast);
    
    // Compare with saved snapshot
    char *expected_snapshot = load_test_file("tests/snapshots/fibonacci_ast.txt");
    
    if (!expected_snapshot) {
        // First run - save snapshot
        save_test_file("tests/snapshots/fibonacci_ast.txt", ast_string);
        printf("Snapshot saved for first run\n");
    } else {
        // Compare with existing snapshot
        ASSERT_EQUALS_STRING(ast_string, expected_snapshot);
        free(expected_snapshot);
    }
    
    free(ast_string);
    ast_node_destroy(ast);
}
```

## Conclusion

These best practices provide a comprehensive foundation for writing, organizing, and maintaining high-quality tests in the Asthra compiler project. By following these guidelines:

- **Consistency**: All tests use the unified framework with appropriate modes
- **Quality**: Tests are well-structured, documented, and maintainable  
- **Performance**: Tests execute efficiently and detect performance regressions
- **Reliability**: Tests are deterministic and provide clear failure information
- **Integration**: Tests integrate seamlessly with CI/CD pipelines

The unified testing framework provides powerful capabilities while maintaining simplicity for common use cases. By choosing the appropriate framework mode and following these patterns, developers can write effective tests that contribute to the overall quality and reliability of the Asthra compiler.

For additional information:
- **[Testing Framework Guide](TESTING_FRAMEWORK_GUIDE.md)** - Detailed framework usage
- **[Testing Migration Guide](TESTING_MIGRATION_GUIDE.md)** - Migration procedures  
- **[Testing Framework Architecture](../architecture/testing-framework.md)** - Technical details 
