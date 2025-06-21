# Testing Workflows

**Comprehensive testing strategies and workflows for Asthra development**

This guide covers all aspects of testing in Asthra, from unit tests to integration testing and test-driven development practices.

## Overview

- **Purpose**: Ensure code quality, prevent regressions, and maintain reliability
- **Time**: 15 minutes - 2 hours (depending on scope)
- **Prerequisites**: Understanding of testing principles and C testing frameworks
- **Outcome**: Comprehensive test coverage with reliable, maintainable tests

## Testing Philosophy

Asthra follows a multi-layered testing approach:

1. **Unit Tests**: Test individual functions and components in isolation
2. **Integration Tests**: Test component interactions and workflows
3. **System Tests**: Test complete compiler pipeline end-to-end
4. **Performance Tests**: Ensure performance characteristics are maintained
5. **Regression Tests**: Prevent previously fixed bugs from reoccurring

## Test Categories & Structure

### Current Test Organization
```
tests/
├── lexer/           # Lexer component tests
├── parser/          # Parser component tests  
├── semantic/        # Semantic analysis tests
├── codegen/         # Code generation tests
├── integration/     # Cross-component tests
├── performance/     # Performance benchmarks
├── regression/      # Bug regression tests
├── utils/           # Utility function tests
└── framework/       # Test framework code
```

### Test Naming Conventions
- **Unit tests**: `test_<component>_<function>.c`
- **Integration tests**: `test_<feature>_integration.c`
- **Performance tests**: `test_<component>_performance.c`
- **Regression tests**: `test_bug_fix_<issue_number>.c`

## Workflow 1: Writing Unit Tests (15-45 minutes)

### Step 1: Identify Test Requirements
```c
// Example: Testing a lexer function
// src/lexer/lexer.c
Token* lexer_next_token(Lexer *lexer);

// Test requirements:
// 1. Returns correct token for valid input
// 2. Handles end of input gracefully
// 3. Reports errors for invalid input
// 4. Maintains correct position tracking
// 5. Handles edge cases (empty input, whitespace)
```

### Step 2: Create Test File Structure
```c
// tests/lexer/test_lexer_next_token.c
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../../src/lexer/lexer.h"
#include "../framework/test_framework.h"

// Test helper functions
static Lexer* create_test_lexer(const char* input) {
    Lexer *lexer = malloc(sizeof(Lexer));
    lexer_init(lexer, input);
    return lexer;
}

static void cleanup_test_lexer(Lexer *lexer) {
    lexer_cleanup(lexer);
    free(lexer);
}

// Individual test functions
void test_next_token_identifier() {
    Lexer *lexer = create_test_lexer("hello");
    
    Token *token = lexer_next_token(lexer);
    
    assert(token != NULL);
    assert(token->type == TOKEN_IDENTIFIER);
    assert(strcmp(token->value, "hello") == 0);
    assert(token->line == 1);
    assert(token->column == 1);
    
    token_free(token);
    cleanup_test_lexer(lexer);
    printf("✅ test_next_token_identifier passed\n");
}

void test_next_token_end_of_input() {
    Lexer *lexer = create_test_lexer("");
    
    Token *token = lexer_next_token(lexer);
    
    assert(token != NULL);
    assert(token->type == TOKEN_EOF);
    
    token_free(token);
    cleanup_test_lexer(lexer);
    printf("✅ test_next_token_end_of_input passed\n");
}

void test_next_token_invalid_character() {
    Lexer *lexer = create_test_lexer("@#$");
    
    Token *token = lexer_next_token(lexer);
    
    // Should return error token or handle gracefully
    assert(token != NULL);
    assert(token->type == TOKEN_ERROR || lexer->has_error);
    
    token_free(token);
    cleanup_test_lexer(lexer);
    printf("✅ test_next_token_invalid_character passed\n");
}

// Main test runner
int main() {
    printf("Running lexer_next_token tests...\n");
    
    test_next_token_identifier();
    test_next_token_end_of_input();
    test_next_token_invalid_character();
    
    printf("All lexer_next_token tests passed! ✅\n");
    return 0;
}
```

### Step 3: Add to Build System
```makefile
# make/tests/lexer.mk
LEXER_TEST_SOURCES = tests/lexer/test_lexer_next_token.c
LEXER_TEST_OBJECTS = $(LEXER_TEST_SOURCES:.c=.o)

test-lexer-next-token: $(LEXER_TEST_OBJECTS) $(LEXER_OBJECTS)
	$(CC) $(CFLAGS) -o build/tests/test_lexer_next_token \
		$(LEXER_TEST_OBJECTS) $(LEXER_OBJECTS) $(LDFLAGS)
	./build/tests/test_lexer_next_token

.PHONY: test-lexer-next-token
```

### Step 4: Run and Validate Tests
```bash
# Run the specific test
make test-lexer-next-token

# Run all lexer tests
make test-lexer

# Run with memory checking
make clean && make asan
make test-lexer-next-token
```

## Workflow 2: Integration Testing (30-90 minutes)

### Step 1: Identify Integration Points
```c
// Example: Testing lexer + parser integration
// Integration points:
// 1. Lexer produces tokens that parser can consume
// 2. Parser handles all token types correctly
// 3. Error propagation between components
// 4. Memory management across component boundaries
```

### Step 2: Create Integration Test
```c
// tests/integration/test_lexer_parser_integration.c
#include <assert.h>
#include <stdio.h>
#include "../../src/lexer/lexer.h"
#include "../../src/parser/parser.h"
#include "../framework/test_framework.h"

void test_simple_function_parsing() {
    const char *source = "fn main() -> i32 { return 42; }";
    
    // Initialize lexer
    Lexer lexer;
    lexer_init(&lexer, source);
    
    // Initialize parser with lexer
    Parser parser;
    parser_init(&parser, &lexer);
    
    // Parse the function
    ASTNode *ast = parse_function_declaration(&parser);
    
    // Verify parsing succeeded
    assert(ast != NULL);
    assert(ast->type == AST_FUNCTION_DECL);
    assert(strcmp(ast->data.function.name, "main") == 0);
    assert(ast->data.function.return_type != NULL);
    assert(ast->data.function.body != NULL);
    
    // Verify no errors
    assert(!parser.has_error);
    assert(!lexer.has_error);
    
    // Cleanup
    ast_node_free(ast);
    parser_cleanup(&parser);
    lexer_cleanup(&lexer);
    
    printf("✅ test_simple_function_parsing passed\n");
}

void test_error_propagation() {
    const char *source = "fn main( -> i32 { return 42; }"; // Missing )
    
    Lexer lexer;
    lexer_init(&lexer, source);
    
    Parser parser;
    parser_init(&parser, &lexer);
    
    ASTNode *ast = parse_function_declaration(&parser);
    
    // Should handle error gracefully
    assert(ast == NULL || parser.has_error);
    
    // Cleanup
    if (ast) ast_node_free(ast);
    parser_cleanup(&parser);
    lexer_cleanup(&lexer);
    
    printf("✅ test_error_propagation passed\n");
}

void test_memory_management() {
    const char *source = "fn test() -> none { let x = 42; }";
    
    // Test multiple parse cycles to check for leaks
    for (int i = 0; i < 100; i++) {
        Lexer lexer;
        lexer_init(&lexer, source);
        
        Parser parser;
        parser_init(&parser, &lexer);
        
        ASTNode *ast = parse_function_declaration(&parser);
        
        if (ast) ast_node_free(ast);
        parser_cleanup(&parser);
        lexer_cleanup(&lexer);
    }
    
    printf("✅ test_memory_management passed\n");
}

int main() {
    printf("Running lexer-parser integration tests...\n");
    
    test_simple_function_parsing();
    test_error_propagation();
    test_memory_management();
    
    printf("All integration tests passed! ✅\n");
    return 0;
}
```

### Step 3: End-to-End Pipeline Testing
```c
// tests/integration/test_full_pipeline.c
void test_complete_compilation() {
    const char *source = 
        "fn fibonacci(n: i32) -> i32 {\n"
        "    if n <= 1 {\n"
        "        return n;\n"
        "    }\n"
        "    return fibonacci(n - 1) + fibonacci(n - 2);\n"
        "}\n"
        "\n"
        "fn main() -> i32 {\n"
        "    return fibonacci(5);\n"
        "}\n";
    
    // Test complete pipeline: Lexer → Parser → Semantic → Codegen
    
    // 1. Lexical analysis
    Lexer lexer;
    lexer_init(&lexer, source);
    assert(!lexer.has_error);
    
    // 2. Parsing
    Parser parser;
    parser_init(&parser, &lexer);
    ASTNode *ast = parse_program(&parser);
    assert(ast != NULL);
    assert(!parser.has_error);
    
    // 3. Semantic analysis
    SemanticContext semantic_ctx;
    semantic_init(&semantic_ctx);
    bool semantic_ok = semantic_analyze(&semantic_ctx, ast);
    assert(semantic_ok);
    
    // 4. Code generation
    CodegenContext codegen_ctx;
    codegen_init(&codegen_ctx);
    bool codegen_ok = generate_code(&codegen_ctx, ast);
    assert(codegen_ok);
    
    // 5. Verify generated code is valid C
    const char *generated_c = codegen_get_output(&codegen_ctx);
    assert(generated_c != NULL);
    assert(strlen(generated_c) > 0);
    
    // Cleanup
    codegen_cleanup(&codegen_ctx);
    semantic_cleanup(&semantic_ctx);
    ast_node_free(ast);
    parser_cleanup(&parser);
    lexer_cleanup(&lexer);
    
    printf("✅ test_complete_compilation passed\n");
}
```

## Workflow 3: Test-Driven Development (TDD) (45-120 minutes)

### Step 1: Write Failing Test First
```c
// Example: Implementing a new feature using TDD
// Feature: Add support for hex literals (0x1234)

// tests/lexer/test_hex_literals.c
void test_hex_literal_parsing() {
    Lexer *lexer = create_test_lexer("0x1234");
    
    Token *token = lexer_next_token(lexer);
    
    // This test will fail initially
    assert(token != NULL);
    assert(token->type == TOKEN_HEX_LITERAL);
    assert(strcmp(token->value, "0x1234") == 0);
    assert(token->numeric_value == 0x1234);
    
    token_free(token);
    cleanup_test_lexer(lexer);
    printf("✅ test_hex_literal_parsing passed\n");
}
```

### Step 2: Run Test (Should Fail)
```bash
make test-lexer-hex-literals
# Expected: Test fails because hex literals not implemented
```

### Step 3: Implement Minimal Code to Pass Test
```c
// src/lexer/lexer.c
Token* lexer_next_token(Lexer *lexer) {
    // ... existing code ...
    
    // Add hex literal support
    if (current_char == '0' && peek_char(lexer) == 'x') {
        return scan_hex_literal(lexer);
    }
    
    // ... rest of existing code ...
}

static Token* scan_hex_literal(Lexer *lexer) {
    Token *token = token_create(TOKEN_HEX_LITERAL);
    
    // Skip '0x'
    advance_char(lexer);  // skip '0'
    advance_char(lexer);  // skip 'x'
    
    // Scan hex digits
    while (is_hex_digit(peek_char(lexer))) {
        advance_char(lexer);
    }
    
    // Extract value and convert
    token->value = extract_token_value(lexer);
    token->numeric_value = strtol(token->value + 2, NULL, 16);
    
    return token;
}
```

### Step 4: Run Test (Should Pass)
```bash
make test-lexer-hex-literals
# Expected: Test passes
```

### Step 5: Refactor and Add More Tests
```c
// Add edge cases and error handling
void test_hex_literal_edge_cases() {
    // Test uppercase
    test_hex_literal("0X1234", 0x1234);
    
    // Test mixed case
    test_hex_literal("0xAbCd", 0xAbCd);
    
    // Test maximum values
    test_hex_literal("0xFFFFFFFF", 0xFFFFFFFF);
    
    // Test error cases
    test_hex_literal_error("0x");        // No digits
    test_hex_literal_error("0xGHIJ");    // Invalid digits
}
```

## Workflow 4: Performance Testing (30-60 minutes)

### Step 1: Create Performance Benchmarks
```c
// tests/performance/test_lexer_performance.c
#include <time.h>
#include <stdio.h>
#include "../../src/lexer/lexer.h"

void benchmark_lexer_throughput() {
    // Create large input for benchmarking
    const int input_size = 100000;
    char *large_input = generate_large_program(input_size);
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    Lexer lexer;
    lexer_init(&lexer, large_input);
    
    int token_count = 0;
    Token *token;
    while ((token = lexer_next_token(&lexer))->type != TOKEN_EOF) {
        token_count++;
        token_free(token);
    }
    token_free(token);
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double elapsed = (end.tv_sec - start.tv_sec) + 
                    (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("Lexer Performance:\n");
    printf("  Input size: %d characters\n", input_size);
    printf("  Tokens processed: %d\n", token_count);
    printf("  Time elapsed: %.6f seconds\n", elapsed);
    printf("  Throughput: %.0f tokens/second\n", token_count / elapsed);
    printf("  Character rate: %.0f chars/second\n", input_size / elapsed);
    
    // Performance assertions
    assert(elapsed < 1.0);  // Should complete within 1 second
    assert(token_count / elapsed > 10000);  // Minimum throughput
    
    lexer_cleanup(&lexer);
    free(large_input);
}

void benchmark_memory_usage() {
    const char *input = "fn test() -> i32 { return 42; }";
    
    // Measure memory usage over many iterations
    size_t initial_memory = get_memory_usage();
    
    for (int i = 0; i < 1000; i++) {
        Lexer lexer;
        lexer_init(&lexer, input);
        
        Token *token;
        while ((token = lexer_next_token(&lexer))->type != TOKEN_EOF) {
            token_free(token);
        }
        token_free(token);
        
        lexer_cleanup(&lexer);
    }
    
    size_t final_memory = get_memory_usage();
    size_t memory_growth = final_memory - initial_memory;
    
    printf("Memory Usage:\n");
    printf("  Initial: %zu bytes\n", initial_memory);
    printf("  Final: %zu bytes\n", final_memory);
    printf("  Growth: %zu bytes\n", memory_growth);
    
    // Should not have significant memory growth (indicates leaks)
    assert(memory_growth < 1024);  // Less than 1KB growth
}
```

### Step 2: Set Performance Baselines
```bash
# Create performance baseline file
cat > tests/performance/baselines.txt << 'EOF'
# Asthra Performance Baselines
# Updated: $(date)

lexer_throughput_min: 10000 tokens/second
lexer_memory_growth_max: 1024 bytes
parser_throughput_min: 5000 nodes/second
semantic_analysis_time_max: 2.0 seconds (large file)
codegen_throughput_min: 1000 lines/second
EOF
```

### Step 3: Automated Performance Monitoring
```bash
# tests/performance/monitor_performance.sh
#!/bin/bash

echo "Running performance monitoring..."

# Run performance tests
make test-performance > performance_results.txt 2>&1

# Check against baselines
python3 << 'EOF'
import re

# Parse results
with open('performance_results.txt', 'r') as f:
    content = f.read()

# Extract metrics
throughput_match = re.search(r'Throughput: (\d+) tokens/second', content)
memory_match = re.search(r'Growth: (\d+) bytes', content)

if throughput_match:
    throughput = int(throughput_match.group(1))
    if throughput < 10000:
        print(f"❌ Performance regression: throughput {throughput} < 10000")
        exit(1)
    else:
        print(f"✅ Throughput OK: {throughput} tokens/second")

if memory_match:
    memory_growth = int(memory_match.group(1))
    if memory_growth > 1024:
        print(f"❌ Memory regression: growth {memory_growth} > 1024 bytes")
        exit(1)
    else:
        print(f"✅ Memory usage OK: {memory_growth} bytes growth")

print("All performance checks passed!")
EOF
```

## Workflow 5: Regression Testing (20-40 minutes)

### Step 1: Create Regression Test for Bug Fix
```c
// tests/regression/test_bug_fix_1234.c
// Regression test for issue #1234: Parser crash on empty function body

void test_bug_1234_empty_function_body() {
    // This used to crash the parser
    const char *source = "fn empty() -> none { }";
    
    Lexer lexer;
    lexer_init(&lexer, source);
    
    Parser parser;
    parser_init(&parser, &lexer);
    
    // Should not crash
    ASTNode *ast = parse_function_declaration(&parser);
    
    // Should handle gracefully
    assert(ast != NULL || parser.has_error);
    
    // If successful, verify structure
    if (ast && !parser.has_error) {
        assert(ast->type == AST_FUNCTION_DECL);
        assert(ast->data.function.body != NULL);
        assert(ast->data.function.body->type == AST_BLOCK);
    }
    
    // Cleanup
    if (ast) ast_node_free(ast);
    parser_cleanup(&parser);
    lexer_cleanup(&lexer);
    
    printf("✅ Bug #1234 regression test passed\n");
}
```

### Step 2: Add to Regression Suite
```makefile
# make/tests/regression.mk
REGRESSION_TESTS = \
    test_bug_fix_1234 \
    test_bug_fix_1235 \
    test_bug_fix_1236

test-regression: $(REGRESSION_TESTS)
	@echo "All regression tests passed!"

test_bug_fix_1234: tests/regression/test_bug_fix_1234.o $(PARSER_OBJECTS)
	$(CC) $(CFLAGS) -o build/tests/test_bug_fix_1234 $^ $(LDFLAGS)
	./build/tests/test_bug_fix_1234
```

## Test Quality Guidelines

### Good Test Characteristics

**1. Independent**: Tests don't depend on each other
```c
// Good: Each test sets up its own state
void test_parser_function() {
    Parser parser;
    parser_init(&parser, "fn test() {}");
    // ... test logic ...
    parser_cleanup(&parser);
}

// Bad: Depends on previous test state
static Parser global_parser;  // Shared state
```

**2. Deterministic**: Same input always produces same result
```c
// Good: Predictable input
void test_lexer_identifier() {
    test_token("hello", TOKEN_IDENTIFIER, "hello");
}

// Bad: Random or time-dependent input
void test_lexer_random() {
    char random_input[100];
    generate_random_string(random_input);  // Unpredictable
    // ... test logic ...
}
```

**3. Fast**: Tests run quickly
```c
// Good: Minimal, focused test
void test_symbol_lookup() {
    SymbolTable table;
    symbol_table_init(&table);
    symbol_table_insert(&table, "test", SYMBOL_VARIABLE);
    
    Symbol *found = symbol_table_lookup(&table, "test");
    assert(found != NULL);
    
    symbol_table_cleanup(&table);
}

// Bad: Unnecessarily complex test
void test_symbol_lookup_slow() {
    // Creates 10,000 symbols just to test lookup
    for (int i = 0; i < 10000; i++) {
        // ... excessive setup ...
    }
}
```

**4. Clear**: Test intent is obvious
```c
// Good: Clear test name and structure
void test_parser_rejects_invalid_syntax() {
    const char *invalid_syntax = "fn missing_body() -> i32";
    
    Parser parser;
    parser_init(&parser, invalid_syntax);
    
    ASTNode *ast = parse_function_declaration(&parser);
    
    // Should reject invalid syntax
    assert(ast == NULL || parser.has_error);
    
    parser_cleanup(&parser);
}

// Bad: Unclear what's being tested
void test_parser_stuff() {
    // ... unclear test logic ...
}
```

### Test Coverage Goals

- **Unit Tests**: 90%+ line coverage for core components
- **Integration Tests**: All major component interactions
- **Edge Cases**: Boundary conditions and error paths
- **Performance**: Key performance characteristics
- **Regression**: All fixed bugs have regression tests

## Running Tests

### Individual Tests
```bash
# Run specific test
make test-lexer-next-token

# Run component tests
make test-lexer
make test-parser
make test-semantic

# Run test category
make test-integration
make test-performance
```

### Comprehensive Testing
```bash
# Run all tests
make test-all

# Run curated test suite (high-confidence tests)
make test-curated

# Run with memory checking
make clean && make asan
make test-curated

# Run with coverage analysis
make clean && make coverage
make test-all
```

### Continuous Integration
```bash
# CI-friendly test command
make test-ci

# Performance regression check
make test-performance-regression

# Memory leak check
make test-memory-safety
```

## Debugging Test Failures

### Common Debugging Techniques

**1. Isolate the Problem**
```bash
# Run single failing test
make test-specific-failing-test

# Run with verbose output
ASTHRA_TEST_VERBOSE=1 make test-specific-failing-test

# Run with debugger
gdb ./build/tests/test_specific_failing_test
```

**2. Add Debug Output**
```c
void test_failing_function() {
    printf("DEBUG: Starting test\n");
    
    Parser parser;
    parser_init(&parser, "test input");
    printf("DEBUG: Parser initialized\n");
    
    ASTNode *ast = parse_expression(&parser);
    printf("DEBUG: AST = %p, has_error = %d\n", ast, parser.has_error);
    
    // ... rest of test ...
}
```

**3. Use Memory Debugging**
```bash
# Check for memory issues
valgrind --tool=memcheck ./build/tests/test_failing_test

# Check for leaks
valgrind --tool=memcheck --leak-check=full ./build/tests/test_failing_test
```

## Success Criteria

Before considering testing complete:

- [ ] **Coverage**: Adequate test coverage for new/modified code
- [ ] **Quality**: Tests are independent, deterministic, fast, and clear
- [ ] **Integration**: Component interactions are tested
- [ ] **Performance**: Performance characteristics are verified
- [ ] **Regression**: Bug fixes have regression tests
- [ ] **Documentation**: Test purpose and setup are documented

## Related Workflows

- **[Adding Features](adding-features.md)** - Testing new features
- **[Fixing Bugs](fixing-bugs.md)** - Creating regression tests
- **[Performance Optimization](performance-optimization.md)** - Performance testing
- **[Code Review Process](code-review.md)** - Test review guidelines

---

**🧪 Testing Complete!**

*Your tests are now ready to ensure code quality and prevent regressions. Remember that good tests are as important as good code.* 