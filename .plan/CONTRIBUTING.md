# Contributing to Asthra Compiler

**Version:** 1.0  
**Date:** January 2025  
**Status:** Active Development  

## Welcome Contributors!

Thank you for your interest in contributing to the Asthra programming language compiler! This guide will help you get started with development, understand our processes, and contribute effectively to this AI-assisted compiler project.

## Table of Contents

- [Quick Start](#quick-start)
- [Development Setup](#development-setup)
- [Third-Party Integration](#third-party-integration)
- [Code Style Guidelines](#code-style-guidelines)
- [Testing Strategy](#testing-strategy)
- [AI Collaboration](#ai-collaboration)
- [Review Process](#review-process)
- [Contribution Types](#contribution-types)
- [Getting Help](#getting-help)

## Quick Start

### Prerequisites

- **C17 Compiler**: Clang 6+ (preferred) or GCC 7+
- **Make**: GNU Make 4.0+
- **Git**: Version control
- **Platform**: macOS, Linux, or Windows (with MinGW)

### First Build

```bash
# Clone the repository
git clone https://github.com/asthra-lang/asthra.git
cd asthra

# Build the compiler
make -j$(nproc)

# Run basic tests
make test-basic

# Verify installation
./bin/asthra --version
```

### Your First Contribution

1. **Find an issue**: Look for `good-first-issue` labels
2. **Fork the repository**: Create your own fork
3. **Create a branch**: `git checkout -b feature/your-feature`
4. **Make changes**: Follow our coding standards
5. **Test thoroughly**: Run relevant test suites
6. **Submit PR**: Create a pull request with clear description

## Development Setup

### Required Tools

#### Compilers
```bash
# macOS (preferred)
brew install llvm
export CC=clang

# Linux
sudo apt-get install clang-12 gcc
# or
sudo yum install clang gcc

# Windows
# Install MinGW-w64 or Visual Studio 2019+
```

#### Development Tools
```bash
# Static analysis
brew install cppcheck clang-tidy

# Memory debugging
brew install valgrind  # Linux only
# AddressSanitizer built into clang/gcc

# Performance profiling
brew install gperftools
```

### IDE Configuration

#### VS Code (Recommended)
```json
// .vscode/settings.json
{
    "C_Cpp.default.cStandard": "c17",
    "C_Cpp.default.compilerPath": "/usr/bin/clang",
    "C_Cpp.default.includePath": [
        "${workspaceFolder}/src",
        "${workspaceFolder}/runtime/include"
    ],
    "files.associations": {
        "*.h": "c",
        "*.c": "c"
    }
}
```

#### CLion
- Set C standard to C17
- Configure CMake or Makefile project
- Enable Clang-Tidy integration
- Set up run configurations for tests

### Build System

#### Standard Build
```bash
make all           # Build everything
make clean         # Clean build artifacts
make -j8           # Parallel build with 8 jobs
```

#### Debug Build
```bash
make debug         # Build with debug symbols
make asan          # Build with AddressSanitizer
make tsan          # Build with ThreadSanitizer
make valgrind      # Build for Valgrind analysis
```

#### Release Build
```bash
make release       # Optimized release build
make strip         # Strip debug symbols
make package       # Create distribution package
```

### Environment Variables

```bash
# Compiler selection
export CC=clang
export CXX=clang++

# Debug options
export ASTHRA_DEBUG=1
export ASTHRA_VERBOSE=1

# Test options
export ASTHRA_TEST_TIMEOUT=30
export ASTHRA_TEST_PARALLEL=8

# Third-party integration
export JSON_C_SYSTEM=1              # Use system json-c library
```

## Third-Party Integration

Asthra supports integrating third-party libraries through a comprehensive system that maintains clean project organization, build system integration, and license compliance.

### Current Third-Party Dependencies

#### JSON-C Integration (Production Ready ✅)
```bash
# Install system library (recommended)
brew install json-c                 # macOS
sudo apt-get install libjson-c-dev  # Ubuntu/Debian

# Use system library
export JSON_C_SYSTEM=1
make json-c

# Test integration
make test-utils-json
```

### Adding New Third-Party Dependencies

#### Step 1: Add as Git Submodule
```bash
# Add new library as submodule
git submodule add https://github.com/owner/library.git third-party/library-name

# Initialize submodules
git submodule update --init --recursive
```

#### Step 2: Update Build System
Edit `make/third-party.mk` to add build rules:

```makefile
# Add new library configuration
LIBRARY_DIR = $(THIRD_PARTY_DIR)/library-name
LIBRARY_BUILD_DIR = $(BUILD_DIR)/third-party/library-name
LIBRARY_LIB = $(LIBRARY_BUILD_DIR)/liblibrary.a
LIBRARY_INCLUDE = $(LIBRARY_DIR)/include

# Add build target
library-name: $(LIBRARY_LIB)

$(LIBRARY_LIB): | $(LIBRARY_BUILD_DIR)
	@echo "Building library-name..."
	cd $(LIBRARY_BUILD_DIR) && cmake $(CURDIR)/$(LIBRARY_DIR) \
		-DCMAKE_BUILD_TYPE=Release \
		-DBUILD_SHARED_LIBS=OFF
	$(MAKE) -C $(LIBRARY_BUILD_DIR)

# Update include and library paths
THIRD_PARTY_INCLUDES += -I$(LIBRARY_INCLUDE)
THIRD_PARTY_LIBS += $(LIBRARY_LIB)
```

#### Step 3: Create Utility Wrapper
Create wrapper API in `src/utils/`:

```c
// src/utils/library_utils.h
#ifndef ASTHRA_LIBRARY_UTILS_H
#define ASTHRA_LIBRARY_UTILS_H

#include <library/library.h>

// Asthra-specific result structure
typedef struct {
    library_object_t *object;
    char *error_message;
    int success;
} AsthrraLibraryResult;

// Core functions
AsthrraLibraryResult asthra_library_init(const char *config);
void asthra_library_result_free(AsthrraLibraryResult *result);

// ... other wrapper functions

#endif // ASTHRA_LIBRARY_UTILS_H
```

```c
// src/utils/library_utils.c
#include "library_utils.h"
#include <stdlib.h>
#include <string.h>

AsthrraLibraryResult asthra_library_init(const char *config) {
    AsthrraLibraryResult result = {NULL, NULL, 0};
    
    // Initialize library with error handling
    library_object_t *obj = library_create(config);
    if (!obj) {
        result.error_message = strdup("Failed to initialize library");
        return result;
    }
    
    result.object = obj;
    result.success = 1;
    return result;
}

void asthra_library_result_free(AsthrraLibraryResult *result) {
    if (!result) return;
    
    if (result->object) {
        library_destroy(result->object);
        result->object = NULL;
    }
    
    if (result->error_message) {
        free(result->error_message);
        result->error_message = NULL;
    }
}
```

#### Step 4: Add Comprehensive Tests
Create test suite in `tests/utils/`:

```c
// tests/utils/test_library_utils.c
#include <assert.h>
#include <stdio.h>
#include "../../src/utils/library_utils.h"

void test_library_initialization() {
    AsthrraLibraryResult result = asthra_library_init("test_config");
    assert(result.success == 1);
    assert(result.object != NULL);
    assert(result.error_message == NULL);
    
    asthra_library_result_free(&result);
    printf("✅ Library initialization test passed\n");
}

void test_library_error_handling() {
    AsthrraLibraryResult result = asthra_library_init(NULL);
    assert(result.success == 0);
    assert(result.object == NULL);
    assert(result.error_message != NULL);
    
    asthra_library_result_free(&result);
    printf("✅ Library error handling test passed\n");
}

int main() {
    test_library_initialization();
    test_library_error_handling();
    printf("All library utility tests passed!\n");
    return 0;
}
```

#### Step 5: Update License Compliance
```bash
# Run license collection script
./scripts/collect_licenses.sh

# Verify license compatibility
cat third-party/licenses/README.md
```

#### Step 6: Update Documentation
Update the following files:
- `third-party/README.md` - Add dependency information
- `THIRD_PARTY_INTEGRATION_PLAN.md` - Update implementation status
- `README.md` - Add to third-party dependencies section

### Third-Party Integration Guidelines

#### Design Principles
1. **Clean Separation**: Third-party code isolated in `third-party/` directory
2. **Dual Build Support**: System libraries preferred, bundled source as fallback
3. **Wrapper APIs**: Asthra-specific wrappers for better integration
4. **License Compliance**: Automated license tracking and documentation
5. **Comprehensive Testing**: Full test coverage for all integrations

#### Build System Patterns
```bash
# System library preference
export LIBRARY_SYSTEM=1

# Check for system library using pkg-config
ifeq ($(LIBRARY_SYSTEM),1)
    LIBRARY_CFLAGS = $(shell pkg-config --cflags library-name 2>/dev/null)
    LIBRARY_LIBS = $(shell pkg-config --libs library-name 2>/dev/null)
    
    ifneq ($(LIBRARY_CFLAGS)$(LIBRARY_LIBS),)
        # System library found
        THIRD_PARTY_INCLUDES += $(LIBRARY_CFLAGS)
        THIRD_PARTY_LIBS += $(LIBRARY_LIBS)
    else
        # Fall back to bundled build
        include $(MAKE_DIR)/third-party-manual.mk
    endif
endif
```

#### Error Handling Standards
```c
// Consistent error handling pattern
typedef struct {
    void *data;
    char *error_message;
    int error_code;
    int success;
} AsthrraThirdPartyResult;

#define ASTHRA_THIRD_PARTY_SUCCESS 0
#define ASTHRA_THIRD_PARTY_ERROR_INIT -1
#define ASTHRA_THIRD_PARTY_ERROR_MEMORY -2
#define ASTHRA_THIRD_PARTY_ERROR_INVALID_INPUT -3
```

#### Memory Management
```c
// RAII-style cleanup functions
void asthra_third_party_result_free(AsthrraThirdPartyResult *result) {
    if (!result) return;
    
    // Clean up library-specific resources
    if (result->data) {
        third_party_cleanup(result->data);
        result->data = NULL;
    }
    
    // Clean up error messages
    if (result->error_message) {
        free(result->error_message);
        result->error_message = NULL;
    }
}
```

### Documentation Resources

- **[Third-Party Integration Plan](../THIRD_PARTY_INTEGRATION_PLAN.md)**: Comprehensive integration strategy
- **[Third-Party Setup Guide](../THIRD_PARTY_SETUP.md)**: Quick setup instructions
- **[Third-Party Dependencies](../third-party/README.md)**: Current dependencies documentation

### Troubleshooting Third-Party Integration

#### Common Issues
```bash
# Missing development headers
# Fix: Install -dev or -devel packages
sudo apt-get install liblibrary-dev

# Library not found during linking
# Fix: Check pkg-config configuration
pkg-config --libs library-name

# Build failures with bundled source
# Fix: Check cmake/make requirements
cmake --version
```

#### Debug Commands
```bash
# Test specific third-party integration
make clean && JSON_C_SYSTEM=1 make json-c
make test-utils-json

# Check third-party build status
make help-third-party

# Verify license compliance
./scripts/collect_licenses.sh
cat third-party/licenses/README.md
```

## Code Style Guidelines

### C17 Standards

#### File Organization
```c
// File header with purpose and copyright
/*
 * Asthra Compiler - Lexer Core Implementation
 * Copyright (c) 2025 Asthra Language Team
 * Licensed under MIT License
 */

// System includes first
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Project includes second
#include "lexer_core.h"
#include "token.h"
#include "parser_error.h"

// Local includes last
#include "lexer_internal.h"
```

#### Naming Conventions
```c
// Functions: snake_case
int parse_expression(Parser* parser);
void lexer_advance_position(Lexer* lexer);

// Types: PascalCase
typedef struct {
    TokenType type;
    char* value;
    SourcePosition position;
} Token;

// Constants: UPPER_SNAKE_CASE
#define MAX_TOKEN_LENGTH 1024
#define DEFAULT_BUFFER_SIZE 4096

// Variables: snake_case
int token_count = 0;
char* source_buffer = NULL;
```

#### Function Structure
```c
/**
 * Parse a function declaration from the token stream.
 * 
 * @param parser The parser instance
 * @param visibility The visibility modifier (pub/priv)
 * @return AST node for function declaration, or NULL on error
 */
ASTNode* parse_function_declaration(Parser* parser, Visibility visibility) {
    // Validate parameters
    if (!parser || !parser->lexer) {
        parser_error(parser, "Invalid parser state");
        return NULL;
    }
    
    // Local variables
    Token* name_token = NULL;
    ASTNode* function_node = NULL;
    
    // Implementation
    name_token = lexer_expect_token(parser->lexer, TOKEN_IDENTIFIER);
    if (!name_token) {
        parser_error(parser, "Expected function name");
        goto cleanup;
    }
    
    // ... implementation ...
    
    return function_node;
    
cleanup:
    // Cleanup resources
    if (name_token) {
        token_destroy(name_token);
    }
    if (function_node) {
        ast_node_destroy(function_node);
    }
    return NULL;
}
```

#### Memory Management
```c
// Always check allocations
char* buffer = malloc(size);
if (!buffer) {
    return ASTHRA_ERROR_OUT_OF_MEMORY;
}

// Use consistent cleanup patterns
void cleanup_parser(Parser* parser) {
    if (!parser) return;
    
    if (parser->lexer) {
        lexer_destroy(parser->lexer);
        parser->lexer = NULL;
    }
    
    if (parser->ast_root) {
        ast_node_destroy(parser->ast_root);
        parser->ast_root = NULL;
    }
    
    free(parser);
}

// Use RAII-style patterns where possible
#define CLEANUP_PARSER __attribute__((cleanup(cleanup_parser_auto)))
void cleanup_parser_auto(Parser** parser) {
    if (parser && *parser) {
        cleanup_parser(*parser);
        *parser = NULL;
    }
}
```

#### Error Handling
```c
// Use consistent error codes
typedef enum {
    ASTHRA_SUCCESS = 0,
    ASTHRA_ERROR_INVALID_ARGUMENT = -1,
    ASTHRA_ERROR_OUT_OF_MEMORY = -2,
    ASTHRA_ERROR_PARSE_ERROR = -3,
    ASTHRA_ERROR_SEMANTIC_ERROR = -4
} AsthraResult;

// Provide detailed error information
typedef struct {
    AsthraResult code;
    char* message;
    SourcePosition position;
    char* suggestion;
} AsthraError;

// Use error propagation patterns
AsthraResult parse_statement(Parser* parser, ASTNode** result) {
    AsthraResult status = ASTHRA_SUCCESS;
    
    status = validate_parser_state(parser);
    if (status != ASTHRA_SUCCESS) {
        return status;
    }
    
    // ... implementation ...
    
    return ASTHRA_SUCCESS;
}
```

### Documentation Standards

#### Function Documentation
```c
/**
 * @brief Parse a struct declaration with optional generic parameters
 * 
 * Parses struct declarations following the grammar:
 * StructDecl <- 'struct' SimpleIdent TypeParams? '{' StructFields? '}'
 * 
 * @param parser The parser instance (must not be NULL)
 * @param visibility The visibility modifier (pub or priv)
 * @param annotations Optional annotations array (can be NULL)
 * @param annotation_count Number of annotations (0 if annotations is NULL)
 * 
 * @return AST node for struct declaration on success, NULL on error
 * 
 * @note The returned AST node must be freed with ast_node_destroy()
 * @note Parser errors are reported through parser->error_handler
 * 
 * @example
 * ```c
 * ASTNode* struct_node = parse_struct_declaration(parser, VISIBILITY_PUB, NULL, 0);
 * if (struct_node) {
 *     // Use struct_node
 *     ast_node_destroy(struct_node);
 * }
 * ```
 */
```

#### File Headers
```c
/**
 * @file lexer_core.c
 * @brief Core lexical analysis implementation for Asthra compiler
 * 
 * This module implements the main lexer functionality including:
 * - Token stream generation
 * - Source position tracking
 * - Error recovery and reporting
 * - Memory management for tokens
 * 
 * The lexer uses a streaming approach to minimize memory usage
 * and supports all Asthra language constructs including:
 * - Keywords and identifiers
 * - Numeric literals (decimal, hex, binary, octal)
 * - String literals with escape sequences
 * - Operators and punctuation
 * - Comments and whitespace handling
 * 
 * @author Asthra Language Team
 * @date January 2025
 * @version 1.0
 */
```

## Testing Strategy

### Unified Testing Framework

Asthra uses a **standardized testing framework** across all 31 test categories, providing consistent APIs, unified tooling, and enhanced developer experience. All test categories have been migrated to use the same testing infrastructure while preserving advanced features where needed.

#### Framework Modes

**Lightweight Mode** - For simple test categories:
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

**Standard Mode** - For most test categories:
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

**Detailed Mode** - For complex test categories with custom analysis:
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
    asthra_test_suite_set_config(suite, &config);
    
    // Custom analysis callback for domain-specific validation
    asthra_test_suite_set_analysis_callback(suite, custom_analysis_callback);
    
    return asthra_test_suite_run_and_exit(suite);
}
```

### Test Execution

#### Unified Test Runner

The standardized framework provides a unified interface for running all tests:

```bash
# Run all tests with unified interface
make test-all-unified

# Advanced options with unified runner
make test-all-unified TEST_FORMAT=json TEST_VERBOSE=1 TEST_PARALLEL=4

# Category-specific testing with standard options
make test-parser TEST_TIMEOUT=30 TEST_FAIL_FAST=1
make test-semantic TEST_FILTER="symbol_*" TEST_RETRY_COUNT=3

# CI/CD friendly commands
make test-json          # JSON output for CI/CD integration
make test-html          # HTML report generation
make test-junit         # JUnit XML for CI systems
make test-ci            # Optimized for CI/CD with fail-fast and retry
```

#### Output Formats

**JSON Output** (standardized across all categories):
```json
{
  "suite_name": "Parser Tests",
  "total_tests": 15,
  "passed": 14,
  "failed": 1,
  "skipped": 0,
  "duration_ms": 1250,
  "timestamp": "2025-01-27T10:30:00Z",
  "tests": [
    {
      "name": "test_expression_parsing",
      "status": "PASSED",
      "duration_ms": 45,
      "assertions": 12
    }
  ]
}
```

**HTML Report** (with visual formatting):
```bash
make test-html
open build/test-results/test_report.html
```

### Test Development Guidelines

#### Writing New Tests

**Step 1: Choose Framework Mode**
```bash
# Simple functionality testing
cp tests/utils/test_json_utils.c tests/newmodule/test_newmodule_basic.c

# Parser/semantic testing  
cp tests/parser/test_grammar_expressions.c tests/newmodule/test_newmodule_advanced.c

# Complex testing with custom analysis
cp tests/codegen/test_variant_types_main_standardized.c tests/newmodule/test_newmodule_complex.c
```

**Step 2: Update Build System**
```makefile
# Add to appropriate make/tests/<category>.mk file
NEWMODULE_TEST_SOURCES = tests/newmodule/test_newmodule_basic.c
NEWMODULE_TEST_OBJECTS = $(NEWMODULE_TEST_SOURCES:.c=.o)

test-newmodule-basic: $(NEWMODULE_TEST_OBJECTS) $(TEST_FRAMEWORK_OBJECTS)
	@echo "Running newmodule basic tests..."
	@$(CC) $(CFLAGS) -o build/test_newmodule_basic $(NEWMODULE_TEST_OBJECTS) $(TEST_FRAMEWORK_OBJECTS) $(LIBS)
	@./build/test_newmodule_basic
```

**Step 3: Follow Testing Patterns**
```c
// Standard assertion patterns
void test_feature_functionality(void) {
    // Setup
    TestObject *obj = create_test_object();
    ASSERT_NOT_NULL(obj);
    
    // Execute and verify
    Result result = feature_function(obj, "test_input");
    ASSERT_TRUE(result.success);
    ASSERT_EQUALS_STRING(result.output, "expected_output");
    ASSERT_GREATER_THAN_INT(result.value, 0);
    
    // Cleanup
    destroy_test_object(obj);
}

// Error handling patterns
void test_error_conditions(void) {
    ASSERT_NULL(feature_function(NULL, "input"));  // NULL pointer
    ASSERT_FALSE(feature_function(obj, NULL).success);  // Invalid input
    ASSERT_EQUALS_INT(feature_function(obj, "").error_code, ERROR_EMPTY_INPUT);
}
```

### Testing Best Practices

#### Test Organization
```c
// Group related tests
void test_basic_operations(void) { /* ... */ }
void test_edge_cases(void) { /* ... */ }
void test_error_handling(void) { /* ... */ }
void test_performance_requirements(void) { /* ... */ }

// Use descriptive test names
void test_struct_field_access_with_nested_structs(void) { /* ... */ }
void test_function_call_with_generic_parameters(void) { /* ... */ }
void test_memory_management_during_error_recovery(void) { /* ... */ }
```

#### Memory Safety Testing
```c
void test_memory_management(void) {
    // Test allocation
    Object *obj = create_object(1024);
    ASSERT_NOT_NULL(obj);
    
    // Test usage
    ASSERT_EQUALS_INT(get_object_size(obj), 1024);
    
    // Test cleanup
    destroy_object(obj);
    // Framework automatically checks for memory leaks
}
```

#### Performance Testing
```c
void test_performance_requirements(void) {
    clock_t start = clock();
    
    // Performance-critical operation
    for (int i = 0; i < 10000; i++) {
        process_item(test_data[i]);
    }
    
    clock_t end = clock();
    double duration = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // Should complete within 100ms
    ASSERT_LESS_THAN_DOUBLE(duration, 0.1);
}
```

### Test Categories Overview

All 31 test categories now use the standardized framework:

**Core Language Categories** (Lightweight/Standard Mode):
- `lexer/` - Token recognition and scanning
- `parser/` - Grammar parsing and AST generation  
- `semantic/` - Semantic analysis and validation
- `types/` - Type system and inference
- `core/` - Core language functionality

**Advanced Feature Categories** (Standard/Detailed Mode):
- `codegen/` - Code generation (Detailed mode with custom analysis)
- `runtime/` - Runtime system (Detailed mode with adapter pattern)
- `concurrency/` - Concurrency primitives (Adapter pattern for legacy compatibility)
- `memory/` - Memory management
- `optimization/` - Compiler optimizations

**Utility and Integration Categories** (Lightweight Mode):
- `utils/` - Utility functions
- `basic/` - Basic compiler functionality
- `integration/` - Integration testing
- `performance/` - Performance validation
- `security/` - Security features

### Test Trend Analysis

The framework includes comprehensive trend analysis tools:

```bash
# Run trend analysis on recent test results
scripts/test_trend_analysis.py --days 30 --format html --output build/trends.html

# Category-specific analysis
scripts/test_trend_analysis.py --category parser --since 2025-01-01

# Performance trend tracking
scripts/test_trend_analysis.py --metrics performance --export csv
```

### Debugging Test Failures

#### Common Debugging Commands
```bash
# Run single test with detailed output
make test-parser-expressions TEST_VERBOSE=1

# Run with memory debugging
make test-asan CATEGORY=semantic

# Run with performance profiling
make test-profile CATEGORY=codegen

# Generate test coverage report
make test-coverage
```

#### Test Framework Debug Features
```c
// Enable debug mode in tests
#define ASTHRA_TEST_DEBUG 1

void test_with_debugging(void) {
    TEST_DEBUG_PRINT("Starting complex operation...");
    
    Object *obj = create_object();
    TEST_DEBUG_ASSERT(obj != NULL, "Object creation failed");
    
    Result result = complex_operation(obj);
    TEST_DEBUG_PRINT("Operation result: %d", result.value);
    
    ASSERT_TRUE(result.success);
}
```

### Integration with CI/CD

The unified testing framework provides excellent CI/CD integration:

```yaml
# GitHub Actions example
- name: Run Asthra Tests
  run: |
    make test-ci TEST_FORMAT=junit TEST_FAIL_FAST=1 TEST_RETRY_COUNT=3
    
- name: Upload Test Results
  uses: actions/upload-artifact@v2
  with:
    name: test-results
    path: build/test-results/

- name: Generate Test Report
  run: |
    make test-html
    make test-trends --days 7
```

For detailed testing framework documentation, see:
- **[Testing Framework Guide](TESTING_FRAMEWORK_GUIDE.md)** - Comprehensive framework usage
- **[Testing Migration Guide](TESTING_MIGRATION_GUIDE.md)** - Migration procedures and patterns
- **[Testing Framework Architecture](../architecture/testing-framework.md)** - Technical architecture details

## AI Collaboration

### Working with AI-Generated Code

#### Code Review for AI Contributions
- **Verify correctness**: AI code may have subtle bugs
- **Check memory management**: Ensure proper allocation/deallocation
- **Validate error handling**: AI often misses edge cases
- **Test thoroughly**: AI-generated code needs comprehensive testing
- **Document assumptions**: AI may make implicit assumptions

#### Effective AI Prompts
```
Good prompt:
"Implement a function to parse Asthra struct declarations following the PEG grammar:
StructDecl <- 'struct' SimpleIdent TypeParams? '{' StructFields? '}'

The function should:
1. Take a Parser* and return ASTNode*
2. Handle optional generic type parameters
3. Parse struct fields with proper error recovery
4. Follow the existing error handling patterns in grammar_toplevel_*.c
5. Include comprehensive error messages for common mistakes

Please include proper memory management and follow the C17 coding style used in the project."

Bad prompt:
"Write a struct parser"
```

#### AI Code Integration Checklist
- [ ] Code follows project naming conventions
- [ ] Memory management is correct (no leaks, double-frees)
- [ ] Error handling follows project patterns
- [ ] Function documentation is complete
- [ ] Unit tests are included
- [ ] Integration tests pass
- [ ] Performance is acceptable
- [ ] Code is readable and maintainable

### AI Development Patterns

#### Modular Development
```c
// AI-friendly: Small, focused functions
ASTNode* parse_struct_name(Parser* parser) {
    // Single responsibility: parse just the struct name
}

ASTNode* parse_struct_fields(Parser* parser) {
    // Single responsibility: parse just the fields
}

ASTNode* parse_struct_declaration(Parser* parser) {
    // Compose smaller functions
    ASTNode* name = parse_struct_name(parser);
    ASTNode* fields = parse_struct_fields(parser);
    return create_struct_node(name, fields);
}
```

#### Error Handling Patterns
```c
// AI-friendly: Consistent error patterns
typedef enum {
    PARSE_SUCCESS,
    PARSE_ERROR_UNEXPECTED_TOKEN,
    PARSE_ERROR_MISSING_SEMICOLON,
    PARSE_ERROR_INVALID_TYPE
} ParseResult;

ParseResult parse_with_recovery(Parser* parser, ASTNode** result) {
    if (!parser || !result) {
        return PARSE_ERROR_INVALID_ARGUMENT;
    }
    
    // Try parsing
    if (parse_attempt(parser, result) == PARSE_SUCCESS) {
        return PARSE_SUCCESS;
    }
    
    // Error recovery
    parser_recover_to_next_statement(parser);
    return PARSE_ERROR_UNEXPECTED_TOKEN;
}
```

## Review Process

### Pull Request Guidelines

#### PR Description Template
```markdown
## Summary
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Performance improvement
- [ ] Refactoring
- [ ] Documentation
- [ ] Test improvement

## Testing
- [ ] Unit tests added/updated
- [ ] Integration tests pass
- [ ] Performance tests pass
- [ ] Memory tests pass (no leaks)

## Checklist
- [ ] Code follows style guidelines
- [ ] Self-review completed
- [ ] Documentation updated
- [ ] Breaking changes documented
```

#### Code Review Checklist

**Functionality**
- [ ] Code does what it's supposed to do
- [ ] Edge cases are handled
- [ ] Error conditions are handled gracefully
- [ ] Performance is acceptable

**Code Quality**
- [ ] Code is readable and well-structured
- [ ] Functions are appropriately sized
- [ ] Variable names are descriptive
- [ ] Comments explain why, not what

**Safety**
- [ ] Memory management is correct
- [ ] No buffer overflows or underflows
- [ ] Thread safety where required
- [ ] Input validation is present

**Testing**
- [ ] Adequate test coverage
- [ ] Tests are meaningful and comprehensive
- [ ] Tests pass consistently
- [ ] Performance tests included where appropriate

### Review Process

1. **Automated Checks**: CI runs tests, linting, and static analysis
2. **Peer Review**: At least one team member reviews the code
3. **AI Review**: AI assistant may provide additional feedback
4. **Integration Testing**: Changes are tested in integration environment
5. **Approval**: Maintainer approves and merges the PR

## Contribution Types

### Bug Fixes
- **Report bugs**: Use GitHub issues with detailed reproduction steps
- **Fix bugs**: Include test cases that reproduce the bug
- **Regression tests**: Ensure the bug doesn't reoccur

### New Features
- **Feature requests**: Discuss in GitHub issues first
- **Implementation**: Follow the feature development process
- **Documentation**: Update user documentation and examples

### Performance Improvements
- **Benchmarking**: Include before/after performance measurements
- **Profiling**: Use profiling tools to identify bottlenecks
- **Testing**: Ensure improvements don't break functionality

### Documentation
- **API documentation**: Keep code documentation up to date
- **User guides**: Help users understand new features
- **Architecture docs**: Document design decisions and patterns

### Testing
- **Test coverage**: Improve test coverage for existing code
- **Test quality**: Make tests more comprehensive and reliable
- **Test infrastructure**: Improve testing tools and frameworks

## Getting Help

### Communication Channels
- **GitHub Issues**: Bug reports and feature requests
- **GitHub Discussions**: General questions and discussions
- **Discord**: Real-time chat with the community
- **Email**: Direct contact with maintainers

### Documentation Resources
- **[Architecture Documentation](architecture/README.md)**: Compiler architecture overview
- **[API Documentation](api/README.md)**: Generated API documentation
- **[User Manual](user-manual/README.md)**: Language user documentation
- **[Examples](examples/README.md)**: Example programs and tutorials
- **[Third-Party Integration](THIRD_PARTY_INTEGRATION_PLAN.md)**: Complete third-party integration guide
- **[Third-Party Setup](THIRD_PARTY_SETUP.md)**: Quick setup for dependencies

### Development Resources
- **[Debugging Guide](guides/debugging-development.md)**: How to debug compiler issues
- **[Adding Features](guides/adding-features.md)**: How to add new language features
- **[Performance Guide](guides/performance-optimization.md)**: Optimization techniques

### Common Issues

#### Build Problems
```bash
# Clean build
make clean && make -j8

# Check dependencies
make check-deps

# Verbose build
make VERBOSE=1
```

#### Test Failures
```bash
# Run specific test with debugging
make debug-test-lexer-numbers
gdb ./build/tests/lexer/test_lexer_numbers

# Check for memory leaks
make test-asan CATEGORY=lexer
```

#### Performance Issues
```bash
# Profile the compiler
make profile
gprof ./bin/asthra gmon.out > profile.txt

# Benchmark specific components
make benchmark-lexer
make benchmark-parser
```

## Code of Conduct

We are committed to providing a welcoming and inclusive environment for all contributors. Please read our [Code of Conduct](CODE_OF_CONDUCT.md) for details on our community standards.

## License

By contributing to Asthra, you agree that your contributions will be licensed under the MIT License. See [LICENSE](LICENSE) for details.

---

Thank you for contributing to Asthra! Your efforts help make this compiler better for everyone. If you have questions or need help getting started, don't hesitate to reach out through our communication channels. 
