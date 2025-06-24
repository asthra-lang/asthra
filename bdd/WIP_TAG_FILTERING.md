# BDD @wip Tag Filtering

This document explains how the BDD framework in Asthra supports skipping scenarios marked with the `@wip` (Work In Progress) tag.

## Overview

The enhanced BDD framework automatically skips scenarios tagged with `@wip` by default. This allows developers to:
- Write scenarios for features not yet implemented
- Keep failing tests in the codebase without breaking CI/CD
- Gradually implement features while maintaining a green test suite

## Usage

### Default Behavior

By default, all scenarios marked with `@wip` in feature files are skipped:

```gherkin
@wip
Scenario: Boolean AND operator
  Given I have code using the && operator
  When I compile it
  Then it should work correctly
```

### Environment Variables

Control tag filtering using environment variables:

1. **Skip @wip scenarios (default)**:
   ```bash
   ./bdd_test  # @wip scenarios are skipped
   ```

2. **Run all scenarios including @wip**:
   ```bash
   BDD_SKIP_WIP=0 ./bdd_test
   ```

3. **Run only @wip scenarios**:
   ```bash
   BDD_TAG_FILTER="@wip" ./bdd_test
   ```

4. **Skip scenarios with specific tags**:
   ```bash
   BDD_TAG_FILTER="not @slow" ./bdd_test
   ```

## Implementation Methods

### Method 1: Enhanced BDD Framework (Recommended)

Use the enhanced BDD support files that provide automatic tag filtering:

```c
#include "bdd_support_enhanced.h"

BDD_TEST_SCENARIO(test_and_operator, "Boolean AND operator", "@wip") {
    // Test implementation
}

int main(void) {
    bdd_init("Feature");
    test_and_operator();  // Automatically skipped if @wip
    return bdd_report();
}
```

### Method 2: Minimal Changes to Existing Tests

Add simple checks in the main function:

```c
int main(void) {
    bdd_init("Feature");
    
    // Run non-@wip scenarios
    test_working_feature();
    
    // Conditionally run @wip scenarios
    if (getenv("BDD_SKIP_WIP") == NULL || strcmp(getenv("BDD_SKIP_WIP"), "0") != 0) {
        printf("\n  Scenario: WIP feature [@wip]\n");
        printf("    ⏭️  SKIPPED: Work in progress\n");
    } else {
        test_wip_feature();
    }
    
    return bdd_report();
}
```

### Method 3: Using CMake/CTest

The BDD CMakeLists.txt already supports tag filtering:

```cmake
# Set environment variable in CMake
set_tests_properties(${test_target} PROPERTIES
    ENVIRONMENT "BDD_SKIP_WIP=1"
)
```

## Migration Guide

To update existing BDD tests:

1. **Automatic migration** (for many files):
   ```bash
   python3 bdd/support/migrate_to_enhanced_bdd.py \
       bdd/steps/unit/boolean_operators_steps.c \
       --feature bdd/features/boolean_operators.feature
   ```

2. **Manual migration**:
   - Replace `#include "bdd_support.h"` with `#include "bdd_support_enhanced.h"`
   - Update test function definitions to use `BDD_TEST_SCENARIO` macro
   - Remove manual `@wip` comments from main()

## Benefits

1. **Clean test output**: Only see results for implemented features
2. **CI/CD friendly**: Tests pass even with incomplete features
3. **Developer friendly**: Easy to toggle between all tests and stable tests
4. **Progress tracking**: See how many scenarios are still WIP

## Best Practices

1. Mark scenarios as `@wip` when:
   - The feature is not yet implemented
   - The test is written but the implementation is incomplete
   - The scenario documents future functionality

2. Remove `@wip` tag when:
   - The feature is fully implemented
   - The test passes consistently
   - The implementation is ready for production

3. Use additional tags for more granular control:
   - `@slow` - Long-running tests
   - `@integration` - Integration tests
   - `@requires-network` - Tests needing network access

## Example Test Output

With @wip filtering enabled:
```
Feature: Boolean operators

  Scenario: Boolean literals
    Given the Asthra compiler is available
      ✓ exists should be true
    ...
    Then the exit code should be 0
      ✓ execution_exit_code should equal expected_code

  Scenario: Logical NOT operator [@wip]
    ⏭️  SKIPPED: Work in progress

  Scenario: Logical AND operator [@wip]
    ⏭️  SKIPPED: Work in progress

Test Summary for 'Boolean operators':
  Passed: 15
  Failed: 0
  Skipped: 2
  Total:  17
  Note: @wip scenarios were skipped
```