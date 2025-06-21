# Asthra Testing Framework Guide

**Version**: 2.0 (Standardized Framework)  
**Last Updated**: 2025-01-27  
**Status**: Implementation Ready

## Overview

This guide provides comprehensive documentation for Asthra's standardized testing framework. All test categories now use a unified approach for consistency, maintainability, and enhanced tooling support.

## Table of Contents

- [Quick Start](#quick-start)
- [Framework Modes](#framework-modes)
- [Writing Tests](#writing-tests)
- [Test Assertions](#test-assertions)
- [Configuration Options](#configuration-options)
- [Build System Integration](#build-system-integration)
- [Best Practices](#best-practices)
- [Migration Guide](#migration-guide)
- [Troubleshooting](#troubleshooting)

## Quick Start

### Simple Test Example

```c
#include "../framework/test_framework.h"

AsthraTestResult test_basic_functionality(AsthraTestContext *context) {
    // Your test logic here
    ASTHRA_TEST_ASSERT_TRUE(context, 1 + 1 == 2, "Basic arithmetic should work");
    return ASTHRA_TEST_PASS;
}

int main(void) {
    AsthraTestSuite *suite = asthra_test_suite_create_lightweight("Basic Tests");
    asthra_test_suite_add_test(suite, "basic_functionality", "Tests basic functionality", test_basic_functionality);
    return asthra_test_suite_run_and_exit(suite);
}
```

### Running Tests

```bash
# Build and run a specific test category
make test-lexer

# Run with verbose output
make test-lexer VERBOSE=1

# Run with JSON output for tooling
make test-lexer FORMAT=json
```

## Framework Modes

The testing framework supports three modes to accommodate different complexity levels:

### 1. Lightweight Mode

**Best for**: Simple tests, basic functionality verification  
**Categories**: lexer, utils, basic, ai_linter, etc.

```c
AsthraTestSuite *suite = asthra_test_suite_create_lightweight("Simple Tests");
// Minimal overhead, basic reporting
```

**Features**:
- Minimal memory footprint
- Basic pass/fail reporting
- Standard assertions
- Simple test registration

### 2. Standard Mode

**Best for**: Most test categories  
**Categories**: parser, semantic, types, etc.

```c
AsthraTestSuite *suite = asthra_test_suite_create("Standard Tests", "Description");
// Full featured testing with setup/teardown
```

**Features**:
- Setup and teardown functions
- Detailed test metadata
- Enhanced reporting
- Timeout support
- Skip functionality

### 3. Detailed Mode

**Best for**: Complex tests requiring custom analysis  
**Categories**: codegen, performance, integration

```c
AsthraTestSuite *suite = asthra_test_suite_create_detailed("Complex Tests", "Description");
asthra_test_suite_set_config(suite, &config);
// Advanced features and custom analysis
```

**Features**:
- Custom statistics tracking
- Analysis callbacks
- Performance metrics
- Parallel execution support
- Custom reporting formats

## Writing Tests

### Test Function Signature

All test functions must follow this signature:

```c
AsthraTestResult test_function_name(AsthraTestContext *context);
```

### Test Registration

```c
// Add test to suite
asthra_test_suite_add_test(suite, 
    "test_name",           // Unique test identifier
    "Test description",    // Human-readable description
    test_function_name     // Function pointer
);
```

### Test Context

The `AsthraTestContext` provides access to:

```c
typedef struct {
    AsthraTestMetadata metadata;    // Test metadata (name, file, line)
    AsthraTestStatistics *stats;    // Global test statistics
    const char *error_message;      // Current error message
    AsthraTestResult result;        // Current test result
    uint64_t start_time_ns;        // Test start time
    uint64_t end_time_ns;          // Test end time
} AsthraTestContext;
```

## Test Assertions

### Basic Assertions

```c
// Boolean assertions
ASTHRA_TEST_ASSERT_TRUE(context, condition, "message");
ASTHRA_TEST_ASSERT_FALSE(context, condition, "message");

// Equality assertions
ASTHRA_TEST_ASSERT_EQ(context, actual, expected, "message");
ASTHRA_TEST_ASSERT_NE(context, actual, expected, "message");

// Pointer assertions
ASTHRA_TEST_ASSERT_NULL(context, pointer, "message");
ASTHRA_TEST_ASSERT_NOT_NULL(context, pointer, "message");

// String assertions
ASTHRA_TEST_ASSERT_STR_EQ(context, actual, expected, "message");
ASTHRA_TEST_ASSERT_STR_NE(context, actual, expected, "message");
```

### Advanced Assertions

```c
// Range assertions
ASTHRA_TEST_ASSERT_IN_RANGE(context, value, min, max, "message");
ASTHRA_TEST_ASSERT_FLOAT_EQ(context, actual, expected, tolerance, "message");

// Memory assertions
ASTHRA_TEST_ASSERT_MEMORY_EQ(context, ptr1, ptr2, size, "message");

// Custom assertions
ASTHRA_TEST_ASSERT_CUSTOM(context, condition, format, ...);
```

### Error Handling

```c
AsthraTestResult test_error_handling(AsthraTestContext *context) {
    // Test setup
    if (!setup_test_environment()) {
        ASTHRA_TEST_FAIL(context, "Failed to setup test environment");
        return ASTHRA_TEST_ERROR;
    }
    
    // Test logic with proper cleanup
    AsthraTestResult result = ASTHRA_TEST_PASS;
    
    if (!test_condition()) {
        ASTHRA_TEST_FAIL(context, "Test condition failed");
        result = ASTHRA_TEST_FAIL;
    }
    
    // Cleanup
    cleanup_test_environment();
    return result;
}
```

## Configuration Options

### Suite Configuration

```c
AsthraTestSuiteConfig config = {
    .name = "Custom Test Suite",
    .description = "Description of test suite",
    .lightweight_mode = false,
    .custom_main = false,
    .statistics_tracking = true,
    .reporting_level = ASTHRA_TEST_REPORTING_STANDARD,
    .default_timeout_ns = 30000000000ULL,  // 30 seconds
    .parallel_execution = false
};

asthra_test_suite_set_config(suite, &config);
```

### Reporting Levels

```c
typedef enum {
    ASTHRA_TEST_REPORTING_MINIMAL,   // Just pass/fail counts
    ASTHRA_TEST_REPORTING_STANDARD,  // Standard test output
    ASTHRA_TEST_REPORTING_DETAILED,  // Detailed with timing and stats
    ASTHRA_TEST_REPORTING_JSON       // Machine-readable JSON
} AsthraTestReportingLevel;
```

### Test Metadata

```c
AsthraTestMetadata metadata = {
    .name = "test_name",
    .file = __FILE__,
    .line = __LINE__,
    .function = __func__,
    .severity = ASTHRA_TEST_SEVERITY_HIGH,
    .timeout_ns = 10000000000ULL,  // 10 seconds
    .skip = false,
    .skip_reason = NULL
};
```

## Build System Integration

### Makefile Integration

Each test category should follow this pattern:

```makefile
# Category-specific test target
test-$(CATEGORY): $($(CATEGORY)_TEST_OBJECTS)
	@echo "Running $(CATEGORY) tests..."
	@./build/test_$(CATEGORY) $(TEST_ARGS)

# With different output formats
test-$(CATEGORY)-json: $($(CATEGORY)_TEST_OBJECTS)
	@./build/test_$(CATEGORY) --format=json > build/results/$(CATEGORY).json

# Verbose mode
test-$(CATEGORY)-verbose: $($(CATEGORY)_TEST_OBJECTS)
	@./build/test_$(CATEGORY) --verbose
```

### CI/CD Integration

```bash
# Standard CI test run
make test-all --format=json --parallel --timeout=60s --fail-fast

# Generate reports
make test-report --output=junit --file=build/test-results.xml
```

## Setup and Teardown

### Suite-Level Setup/Teardown

```c
AsthraTestResult suite_setup(AsthraTestContext *context) {
    // Initialize global resources
    if (!initialize_test_resources()) {
        return ASTHRA_TEST_ERROR;
    }
    return ASTHRA_TEST_PASS;
}

AsthraTestResult suite_teardown(AsthraTestContext *context) {
    // Cleanup global resources
    cleanup_test_resources();
    return ASTHRA_TEST_PASS;
}

// Register with suite
asthra_test_suite_set_setup(suite, suite_setup);
asthra_test_suite_set_teardown(suite, suite_teardown);
```

### Test-Level Setup/Teardown

```c
AsthraTestResult test_with_setup(AsthraTestContext *context) {
    // Per-test setup
    TestData *data = create_test_data();
    if (!data) {
        return ASTHRA_TEST_ERROR;
    }
    
    // Test logic
    ASTHRA_TEST_ASSERT_NOT_NULL(context, data, "Test data should be created");
    
    // Per-test cleanup
    destroy_test_data(data);
    return ASTHRA_TEST_PASS;
}
```

## Best Practices

### Test Organization

1. **One test per function**: Each test function should verify one specific behavior
2. **Descriptive names**: Use clear, descriptive test and function names
3. **Consistent structure**: Follow setup → test → cleanup pattern
4. **Proper cleanup**: Always clean up resources, even on failure

### Error Messages

```c
// Good: Specific and actionable
ASTHRA_TEST_ASSERT_EQ(context, result, expected, 
    "Hash function should return 0x1234 for input 'test'");

// Bad: Vague and unhelpful
ASTHRA_TEST_ASSERT_EQ(context, result, expected, "Hash failed");
```

### Performance Considerations

1. **Use lightweight mode** for simple tests
2. **Avoid expensive setup** in frequently run tests
3. **Use parallel execution** for independent tests
4. **Profile test suites** to identify bottlenecks

### Memory Management

```c
AsthraTestResult test_memory_safety(AsthraTestContext *context) {
    char *buffer = malloc(1024);
    if (!buffer) {
        return ASTHRA_TEST_ERROR;
    }
    
    // Test logic here
    bool success = test_buffer_operations(buffer);
    
    // Always cleanup
    free(buffer);
    
    if (!success) {
        ASTHRA_TEST_FAIL(context, "Buffer operations failed");
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}
```

## Common Patterns

### Testing Error Conditions

```c
AsthraTestResult test_error_handling(AsthraTestContext *context) {
    // Test that function properly handles invalid input
    Result result = function_under_test(NULL);
    
    ASTHRA_TEST_ASSERT_TRUE(context, result.is_error, 
        "Function should return error for NULL input");
    ASTHRA_TEST_ASSERT_STR_EQ(context, result.error_message, "Invalid input",
        "Error message should be descriptive");
    
    return ASTHRA_TEST_PASS;
}
```

### Testing Asynchronous Code

```c
AsthraTestResult test_async_operation(AsthraTestContext *context) {
    AsyncHandle handle = start_async_operation();
    
    // Wait with timeout
    bool completed = wait_for_completion(handle, 5000); // 5 second timeout
    ASTHRA_TEST_ASSERT_TRUE(context, completed, 
        "Async operation should complete within 5 seconds");
    
    Result result = get_async_result(handle);
    ASTHRA_TEST_ASSERT_TRUE(context, result.success, 
        "Async operation should succeed");
    
    cleanup_async_handle(handle);
    return ASTHRA_TEST_PASS;
}
```

### Parameterized Tests

```c
typedef struct {
    const char *input;
    const char *expected;
} TestCase;

AsthraTestResult test_string_processing(AsthraTestContext *context) {
    TestCase cases[] = {
        {"hello", "HELLO"},
        {"world", "WORLD"},
        {"", ""},
        {NULL, NULL}
    };
    
    for (int i = 0; cases[i].input != NULL; i++) {
        char *result = string_to_upper(cases[i].input);
        ASTHRA_TEST_ASSERT_STR_EQ(context, result, cases[i].expected,
            "String case conversion failed for: %s", cases[i].input);
        free(result);
    }
    
    return ASTHRA_TEST_PASS;
}
```

## Troubleshooting

### Common Issues

**Issue**: Test segmentation faults
**Solution**: Check for null pointer dereferences, use proper memory management

**Issue**: Tests hang indefinitely  
**Solution**: Set appropriate timeouts, check for infinite loops

**Issue**: Inconsistent test results
**Solution**: Ensure proper cleanup, avoid global state dependencies

**Issue**: Memory leaks in tests
**Solution**: Always pair malloc/free, use valgrind for detection

### Debugging Tests

```c
// Debug assertions for development
#ifdef DEBUG
    ASTHRA_TEST_DEBUG(context, "Variable x = %d", x);
    ASTHRA_TEST_DUMP_MEMORY(context, buffer, size);
#endif

// Conditional testing
if (asthra_test_is_verbose_mode()) {
    printf("Detailed debug information: %s\n", debug_info);
}
```

### Integration with External Tools

```bash
# Run with Valgrind
make test-lexer VALGRIND=1

# Run with AddressSanitizer
make test-lexer ASAN=1

# Generate coverage report
make test-lexer COVERAGE=1
```

## Migration from Legacy Patterns

See [TESTING_MIGRATION_GUIDE.md](TESTING_MIGRATION_GUIDE.md) for detailed migration instructions from legacy testing patterns.

## API Reference

For complete API documentation, see the header files:
- `tests/framework/test_framework.h` - Main framework APIs
- `tests/framework/test_suite.h` - Test suite management
- `tests/framework/test_assertions.h` - Assertion macros

## Contributing

When adding new test categories or modifying existing ones:

1. Follow the patterns established in this guide
2. Use appropriate framework mode for complexity level
3. Write clear, descriptive test names and messages
4. Include proper setup and cleanup
5. Update documentation as needed

For questions or issues, consult the troubleshooting section or reach out to the development team. 
