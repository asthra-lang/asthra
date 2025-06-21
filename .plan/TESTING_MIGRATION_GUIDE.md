# Testing Framework Migration Guide

**Version**: 1.0  
**Last Updated**: 2025-01-27  
**Status**: Implementation Guide

## Overview

This guide provides step-by-step instructions for migrating test categories from legacy testing patterns to Asthra's standardized testing framework. The migration preserves all existing functionality while providing enhanced features and tooling support.

## Table of Contents

- [Migration Strategy](#migration-strategy)
- [Pre-Migration Checklist](#pre-migration-checklist)
- [Simple Testing Migration](#simple-testing-migration)
- [Custom Framework Migration](#custom-framework-migration)
- [Build System Updates](#build-system-updates)
- [Validation and Testing](#validation-and-testing)
- [Common Migration Issues](#common-migration-issues)
- [Category-Specific Examples](#category-specific-examples)

## Migration Strategy

### Phase-Based Approach

1. **Assessment**: Analyze current testing patterns
2. **Planning**: Choose appropriate framework mode
3. **Implementation**: Convert tests to standardized framework
4. **Validation**: Ensure all tests pass and functionality is preserved
5. **Integration**: Update build system and documentation

### Framework Mode Selection

| Current Pattern | Recommended Mode | Reason |
|---|---|---|
| Simple TEST_ASSERT macros | Lightweight | Minimal overhead, easy migration |
| Basic main() with manual counting | Lightweight | Direct replacement |
| Custom test orchestrators | Standard | Enhanced features with compatibility |
| Complex statistics/analysis | Detailed | Preserve advanced functionality |
| Sophisticated custom frameworks | Detailed + Adapter | Maintain features, add standardization |

## Pre-Migration Checklist

Before starting migration:

- [ ] **Document Current Behavior**: Record all test outputs and behaviors
- [ ] **Identify Dependencies**: Note any external dependencies or special requirements
- [ ] **Backup Tests**: Create backup of current test files
- [ ] **Review Test Logic**: Ensure tests are logically sound
- [ ] **Plan Mode Selection**: Choose appropriate framework mode
- [ ] **Prepare Build System**: Plan Makefile updates

## Simple Testing Migration

### Pattern 1: Basic TEST_ASSERT Macros

**Before** (`tests/lexer/test_lexer_simple.c`):
```c
#include <stdio.h>
#include <assert.h>

#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "FAIL: %s\n", message); \
            return 0; \
        } \
    } while (0)

#define TEST_PASS(message) \
    do { \
        printf("PASS: %s\n", message); \
        return 1; \
    } while (0)

static int test_basic_tokens(void) {
    // Test logic
    TEST_ASSERT(condition, "Basic tokens should work");
    TEST_PASS("Basic token recognition");
}

int main(void) {
    int passed = 0, total = 0;
    total++; passed += test_basic_tokens();
    printf("Results: %d/%d passed\n", passed, total);
    return (passed == total) ? 0 : 1;
}
```

**After**:
```c
#include "../framework/test_framework.h"

AsthraTestResult test_basic_tokens(AsthraTestContext *context) {
    // Same test logic, updated assertions
    ASTHRA_TEST_ASSERT_TRUE(context, condition, "Basic tokens should work");
    return ASTHRA_TEST_PASS;
}

int main(void) {
    AsthraTestSuite *suite = asthra_test_suite_create_lightweight("Lexer Simple Tests");
    asthra_test_suite_add_test(suite, "basic_tokens", "Basic token recognition", test_basic_tokens);
    return asthra_test_suite_run_and_exit(suite);
}
```

### Migration Steps:

1. **Update Includes**:
   ```c
   // Remove
   #include <stdio.h>
   #include <assert.h>
   
   // Add
   #include "../framework/test_framework.h"
   ```

2. **Convert Test Functions**:
   ```c
   // Before
   static int test_function(void) {
       TEST_ASSERT(condition, "message");
       TEST_PASS("success message");
   }
   
   // After
   AsthraTestResult test_function(AsthraTestContext *context) {
       ASTHRA_TEST_ASSERT_TRUE(context, condition, "message");
       return ASTHRA_TEST_PASS;
   }
   ```

3. **Replace Main Function**:
   ```c
   // Before
   int main(void) {
       int passed = 0, total = 0;
       total++; passed += test_function();
       // Manual counting...
   }
   
   // After
   int main(void) {
       AsthraTestSuite *suite = asthra_test_suite_create_lightweight("Test Suite Name");
       asthra_test_suite_add_test(suite, "test_name", "description", test_function);
       return asthra_test_suite_run_and_exit(suite);
   }
   ```

### Pattern 2: Direct Assert Usage

**Before** (`tests/utils/test_json_utils.c`):
```c
#include <assert.h>

void test_json_parsing() {
    printf("Testing JSON parsing...\n");
    assert(result.success);
    assert(result.root != NULL);
    printf("✓ JSON parsing test passed\n");
}

int main() {
    test_json_parsing();
    test_json_creation();
    printf("🎉 All tests passed!\n");
    return 0;
}
```

**After**:
```c
#include "../framework/test_framework.h"

AsthraTestResult test_json_parsing(AsthraTestContext *context) {
    // Test logic...
    ASTHRA_TEST_ASSERT_TRUE(context, result.success, "JSON parsing should succeed");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, result.root, "JSON root should not be null");
    return ASTHRA_TEST_PASS;
}

int main() {
    AsthraTestSuite *suite = asthra_test_suite_create_lightweight("JSON Utils Tests");
    asthra_test_suite_add_test(suite, "json_parsing", "JSON parsing functionality", test_json_parsing);
    asthra_test_suite_add_test(suite, "json_creation", "JSON creation functionality", test_json_creation);
    return asthra_test_suite_run_and_exit(suite);
}
```

## Custom Framework Migration

### Pattern 3: Custom Test Orchestrators

**Before** (`tests/codegen/test_variant_types_main.c`):
```c
typedef struct {
    int total_tests;
    int passed_tests;
    int failed_tests;
    // Custom statistics...
} VariantTestStats;

static void run_variant_test(const VariantTestInfo* test_info, VariantTestStats* stats) {
    printf("Running: %s... ", test_info->name);
    AsthraTestResult result = test_info->test_func(&context);
    
    if (result == ASTHRA_TEST_PASS) {
        printf("✅ PASS\n");
        stats->passed_tests++;
    } else {
        printf("❌ FAIL\n");
        stats->failed_tests++;
    }
}

int main(void) {
    VariantTestStats stats = {0};
    
    for (int i = 0; i < NUM_VARIANT_TESTS; i++) {
        run_variant_test(&variant_tests[i], &stats);
    }
    
    // Custom analysis and reporting...
    return (stats.failed_tests > 0) ? 1 : 0;
}
```

**After** (Detailed Mode with Custom Analysis):
```c
#include "../framework/test_framework.h"

// Custom analysis callback
void variant_analysis_callback(AsthraTestSuite *suite, AsthraTestStatistics *stats) {
    printf("=== Variant Type Analysis ===\n");
    
    if (stats->critical_passed == stats->critical_total) {
        printf("✅ All critical variant functionality working\n");
    }
    
    if (stats->pass_rate >= 75.0) {
        printf("✅ Variant type system in good condition\n");
    }
    
    // Custom analysis logic...
}

int main(void) {
    AsthraTestSuite *suite = asthra_test_suite_create_detailed("Variant Types", 
        "Comprehensive variant type testing with custom analysis");
    
    // Configure for detailed mode
    AsthraTestSuiteConfig config = {
        .statistics_tracking = true,
        .reporting_level = ASTHRA_TEST_REPORTING_DETAILED,
        .custom_main = false
    };
    asthra_test_suite_set_config(suite, &config);
    
    // Set custom analysis
    asthra_test_suite_set_analysis_callback(suite, variant_analysis_callback);
    
    // Register tests
    for (int i = 0; i < NUM_VARIANT_TESTS; i++) {
        asthra_test_suite_add_test(suite, 
            variant_tests[i].name,
            variant_tests[i].description,
            variant_tests[i].test_func);
    }
    
    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);
    
    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
```

### Pattern 4: Complex Custom Frameworks

**Before** (`tests/concurrency/test_spawn_main.c`):
```c
typedef struct {
    // Complex custom context
    AsthraV12TestContext ctx;
    // Custom timing and performance metrics
    // Specialized concurrency testing features
} CustomConcurrencyContext;

int main(void) {
    AsthraV12TestContext ctx;
    test_context_init(&ctx);
    
    // Custom test execution with specialized features
    SpawnTestStats stats;
    // Complex custom orchestration...
    
    return (stats.failed_tests > 0) ? 1 : 0;
}
```

**After** (Adapter Pattern):
```c
#include "../framework/test_framework.h"

// Adapter to bridge custom framework with standard interface
typedef struct {
    AsthraV12TestContext legacy_ctx;
    AsthraTestSuite *standard_suite;
} ConcurrencyTestAdapter;

// Adapter functions
AsthraTestResult legacy_test_adapter(AsthraTestContext *context) {
    // Extract legacy context from user data
    ConcurrencyTestAdapter *adapter = (ConcurrencyTestAdapter*)context->user_data;
    
    // Run legacy test with legacy context
    return legacy_test_function(&adapter->legacy_ctx);
}

int main(void) {
    // Initialize adapter
    ConcurrencyTestAdapter adapter;
    test_context_init(&adapter.legacy_ctx);
    
    // Create standard suite with adapter
    adapter.standard_suite = asthra_test_suite_create_detailed("Concurrency Tests",
        "Concurrency testing with legacy framework compatibility");
    
    // Configure for custom main and detailed reporting
    AsthraTestSuiteConfig config = {
        .custom_main = true,
        .statistics_tracking = true,
        .reporting_level = ASTHRA_TEST_REPORTING_DETAILED
    };
    asthra_test_suite_set_config(adapter.standard_suite, &config);
    
    // Register legacy tests through adapter
    asthra_test_suite_add_test_with_data(adapter.standard_suite,
        "spawn_basic", "Basic spawn functionality", 
        legacy_test_adapter, &adapter);
    
    // Run with both legacy and standard reporting
    AsthraTestResult result = asthra_test_suite_run(adapter.standard_suite);
    
    // Custom cleanup
    test_context_cleanup(&adapter.legacy_ctx);
    asthra_test_suite_destroy(adapter.standard_suite);
    
    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
```

## Build System Updates

### Makefile Updates

**Before**:
```makefile
test-lexer: $(LEXER_TEST_OBJECTS)
	@echo "Running lexer tests..."
	@./build/test_lexer_simple
	@./build/test_lexer_tokens
	# Multiple individual test executables...

test-lexer-all: test-lexer
	@echo "All lexer tests completed"
```

**After**:
```makefile
# Unified test executable per category
LEXER_TEST_SOURCES = tests/lexer/test_lexer_simple.c \
                     tests/lexer/test_lexer_tokens.c \
                     # ... other test files

LEXER_TEST_OBJECTS = $(LEXER_TEST_SOURCES:.c=.o)

test-lexer: $(LEXER_TEST_OBJECTS) $(FRAMEWORK_OBJECTS)
	@echo "Building lexer test suite..."
	@$(CC) $(CFLAGS) -o build/test_lexer_suite $(LEXER_TEST_OBJECTS) $(FRAMEWORK_OBJECTS) $(LIBS)
	@echo "Running lexer tests..."
	@./build/test_lexer_suite

# Support different output formats
test-lexer-json: test-lexer
	@./build/test_lexer_suite --format=json > build/results/lexer.json

test-lexer-verbose: test-lexer
	@./build/test_lexer_suite --verbose

# Integration with unified test runner
test-all: test-lexer test-parser test-semantic # ... all categories
```

### CMake Updates (if applicable)

```cmake
# Unified test target per category
function(add_test_category CATEGORY_NAME)
    set(TEST_SOURCES "")
    file(GLOB_RECURSE TEST_SOURCES "tests/${CATEGORY_NAME}/*.c")
    
    add_executable(test_${CATEGORY_NAME}_suite ${TEST_SOURCES})
    target_link_libraries(test_${CATEGORY_NAME}_suite 
        asthra_test_framework 
        ${CATEGORY_NAME}_lib
    )
    
    add_test(NAME ${CATEGORY_NAME}_tests 
             COMMAND test_${CATEGORY_NAME}_suite)
endfunction()

# Add all test categories
add_test_category(lexer)
add_test_category(parser)
add_test_category(semantic)
# ... etc
```

## Validation and Testing

### Migration Validation Checklist

- [ ] **All Tests Pass**: Verify all migrated tests still pass
- [ ] **Same Coverage**: Ensure test coverage is maintained or improved
- [ ] **Performance**: Check that test execution time is acceptable
- [ ] **Output Compatibility**: Verify output format meets requirements
- [ ] **Error Handling**: Test error conditions and failure cases
- [ ] **Build Integration**: Ensure build system works correctly
- [ ] **Documentation**: Update any test-specific documentation

### Validation Script Example

```bash
#!/bin/bash
# validate_migration.sh

CATEGORY=$1
echo "Validating migration for category: $CATEGORY"

# Run original tests (if still available)
echo "Running original tests..."
make test-${CATEGORY}-original > original_results.txt 2>&1
ORIGINAL_EXIT_CODE=$?

# Run migrated tests
echo "Running migrated tests..."
make test-${CATEGORY} > migrated_results.txt 2>&1
MIGRATED_EXIT_CODE=$?

# Compare results
if [ $ORIGINAL_EXIT_CODE -eq $MIGRATED_EXIT_CODE ]; then
    echo "✅ Exit codes match"
else
    echo "❌ Exit codes differ: original=$ORIGINAL_EXIT_CODE, migrated=$MIGRATED_EXIT_CODE"
    exit 1
fi

# Check test counts
ORIGINAL_COUNT=$(grep -o "passed" original_results.txt | wc -l)
MIGRATED_COUNT=$(grep -o "PASS" migrated_results.txt | wc -l)

if [ $ORIGINAL_COUNT -eq $MIGRATED_COUNT ]; then
    echo "✅ Test counts match: $ORIGINAL_COUNT tests"
else
    echo "❌ Test counts differ: original=$ORIGINAL_COUNT, migrated=$MIGRATED_COUNT"
    exit 1
fi

echo "✅ Migration validation passed for $CATEGORY"
```

## Common Migration Issues

### Issue 1: Assertion Mapping

**Problem**: Direct assert() calls need conversion
```c
// Problem
assert(condition);

// Solution
ASTHRA_TEST_ASSERT_TRUE(context, condition, "Descriptive message");
```

### Issue 2: Return Value Changes

**Problem**: Test functions return int instead of AsthraTestResult
```c
// Problem
static int test_function(void) {
    if (condition) return 1;
    return 0;
}

// Solution
AsthraTestResult test_function(AsthraTestContext *context) {
    if (condition) return ASTHRA_TEST_PASS;
    return ASTHRA_TEST_FAIL;
}
```

### Issue 3: Context Parameter

**Problem**: Test functions don't take context parameter
```c
// Problem
void test_function(void) {
    // Test logic
}

// Solution
AsthraTestResult test_function(AsthraTestContext *context) {
    // Test logic - now has access to context
    return ASTHRA_TEST_PASS;
}
```

### Issue 4: Custom Statistics Preservation

**Problem**: Complex custom statistics need preservation
```c
// Problem: Custom stats struct lost

// Solution: Use detailed mode with custom analysis
void custom_analysis_callback(AsthraTestSuite *suite, AsthraTestStatistics *stats) {
    // Access standard stats
    int custom_metric = calculate_custom_metric(stats);
    
    // Custom reporting
    printf("Custom Metric: %d\n", custom_metric);
}
```

### Issue 5: Build System Integration

**Problem**: Multiple test executables need consolidation
```bash
# Problem: Multiple targets
test-category: test1 test2 test3 test4

# Solution: Single unified target
test-category: build/test_category_suite
```

## Category-Specific Examples

### Lexer Category Migration

**Key Changes**:
- Convert TOKEN_* assertions to standard framework
- Preserve lexer-specific test utilities
- Maintain character encoding tests

```c
// Before
TEST_ASSERT(token.type == TOKEN_IDENTIFIER, "Expected identifier token");

// After
ASTHRA_TEST_ASSERT_EQ(context, token.type, TOKEN_IDENTIFIER, "Expected identifier token");
```

### Parser Category Migration

**Note**: Parser category already uses test suite framework, but may need configuration updates.

**Key Changes**:
- Ensure consistent configuration across all parser tests
- Standardize setup/teardown patterns
- Update any remaining custom assertion patterns

### Semantic Category Migration

**Key Changes**:
- Convert symbol table assertions
- Preserve semantic analysis context
- Maintain type checking validations

```c
// Before
assert(symbol != NULL);
assert(symbol->type == SYMBOL_VARIABLE);

// After
ASTHRA_TEST_ASSERT_NOT_NULL(context, symbol, "Symbol should exist");
ASTHRA_TEST_ASSERT_EQ(context, symbol->type, SYMBOL_VARIABLE, "Symbol should be variable");
```

### Utils Category Migration

**Key Changes**:
- Convert direct assert() calls
- Preserve utility-specific test data
- Maintain third-party integration tests

## Post-Migration Tasks

### Documentation Updates

1. **Update README files** in test categories
2. **Update contributor documentation**
3. **Create migration completion report**
4. **Update build system documentation**

### Tooling Integration

1. **Test unified test runner**
2. **Verify CI/CD integration**
3. **Update automated testing scripts**
4. **Create test reporting dashboard**

### Training and Support

1. **Create developer training materials**
2. **Document troubleshooting procedures**
3. **Establish support channels**
4. **Plan knowledge transfer sessions**

## Conclusion

The migration to standardized testing framework provides significant benefits while preserving all existing functionality. The phased approach ensures minimal disruption and allows for careful validation at each step.

For questions or issues during migration:
1. Consult this guide and the main testing framework documentation
2. Review category-specific examples
3. Use the validation procedures to ensure correctness
4. Reach out to the development team for support

**Next Steps**: Begin migration with simple categories, validate thoroughly, then proceed to more complex frameworks. 
