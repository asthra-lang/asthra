# BDD @wip Tag Filtering

This document explains how the BDD framework in Asthra supports skipping scenarios marked with the `@wip` (Work In Progress) tag.

## Overview

The BDD framework allows scenarios to be marked as work-in-progress using the `@wip` tag. This allows developers to:
- Keep failing tests in the codebase without breaking CI/CD
- Write scenarios for features not yet implemented
- Gradually implement features while maintaining a green test suite

## Usage

### Two-Part System

The @wip filtering system requires coordination between two files:

1. **Feature files** (`bdd/features/*.feature`) - Mark scenarios with `@wip`
2. **Step implementation files** (`bdd/steps/unit/*_steps.c`) - Use `BDD_WIP_TEST_CASE()` macro

### Marking Scenarios as WIP

#### Step 1: In Feature Files

Mark scenarios with the `@wip` tag:

```gherkin
@wip
Scenario: Boolean AND operator
  Given I have code using the && operator
  When I compile it
  Then it should work correctly
```

#### Step 2: In Step Implementation Files

Use `BDD_WIP_TEST_CASE()` instead of `BDD_TEST_CASE()`:

```c
// Define test cases
BddTestCase boolean_operators_test_cases[] = {
    BDD_TEST_CASE(boolean_literals, test_boolean_literals),        // Regular test
    BDD_WIP_TEST_CASE(logical_and, test_logical_and),             // WIP test - will be skipped
    BDD_WIP_TEST_CASE(logical_or, test_logical_or),              // WIP test - will be skipped
    BDD_TEST_CASE(boolean_negation, test_boolean_negation),      // Regular test
};
```

### Environment Variables

Control tag filtering using environment variables:

1. **Skip @wip scenarios (default)**:
   ```bash
   ./bdd_test  # @wip scenarios are skipped by default
   ```

2. **Run all scenarios including @wip**:
   ```bash
   BDD_SKIP_WIP=0 ./bdd_test
   ```

## Implementation Details

### How It Works

The BDD framework uses the `bdd_test_framework.c` implementation which:

1. Checks the `is_wip` flag on each `BddTestCase`
2. By default skips tests marked with `BDD_WIP_TEST_CASE()`
3. Respects the `BDD_SKIP_WIP` environment variable

### Test Case Structure

```c
typedef struct {
    const char* name;
    void (*test_function)(void);
    int is_wip;  // Set to 1 by BDD_WIP_TEST_CASE macro
} BddTestCase;

// Macros for defining test cases
#define BDD_TEST_CASE(name, func) {#name, func, 0}
#define BDD_WIP_TEST_CASE(name, func) {#name, func, 1}
```

## Migration Guide

To mark failing tests as WIP:

1. **Add @wip tag to feature file**:
   ```gherkin
   @wip
   Scenario: My failing scenario
     Given ...
     When ...
     Then ...
   ```

2. **Update step implementation file**:
   ```c
   // Change from:
   BDD_TEST_CASE(my_failing_scenario, test_my_failing_scenario),
   
   // To:
   BDD_WIP_TEST_CASE(my_failing_scenario, test_my_failing_scenario),
   ```

3. **Rebuild and run tests**:
   ```bash
   cmake --build build
   ./run-bdd-tests-local.sh
   ```

## Benefits

1. **Clean test output**: Only see results for implemented features
2. **CI/CD friendly**: Tests pass even with incomplete features  
3. **Developer friendly**: Easy to toggle between all tests and stable tests
4. **Progress tracking**: See how many scenarios are still WIP

## Best Practices

1. **Always keep feature files and step files in sync**:
   - If a scenario has `@wip` in the feature file, use `BDD_WIP_TEST_CASE()` in the step file
   - If a scenario doesn't have `@wip`, use `BDD_TEST_CASE()` in the step file

2. **Mark scenarios as `@wip` when**:
   - The feature is not yet implemented
   - The test is written but the implementation is incomplete
   - The scenario is failing due to known issues

3. **Remove `@wip` tag when**:
   - The feature is fully implemented
   - The test passes consistently
   - Remember to change `BDD_WIP_TEST_CASE()` back to `BDD_TEST_CASE()`

## Example Test Output

With @wip filtering enabled (default):
```
Feature: Const Declarations

  Scenario: Basic const declaration with integer
    Given the Asthra compiler is available
      ✓ bdd_compiler_available() should be true
    ...
    Then the exit code should be 0
      ✓ execution_exit_code should equal expected_exit_code

  Scenario: Const declaration with string [@wip]
    ⏭️  SKIPPED (work in progress)

  Scenario: Const declaration with boolean [@wip]
    ⏭️  SKIPPED (work in progress)

Test Summary for 'Const Declarations':
  Passed: 89
  Failed: 0
  Skipped: 3
  Total:  92
```

## Common Issues

1. **@wip tag not working**: Make sure both the feature file has `@wip` AND the step file uses `BDD_WIP_TEST_CASE()`

2. **Tests still failing**: The framework only skips tests properly registered with `BDD_WIP_TEST_CASE()`. Check that you've updated the step implementation file.

3. **Want to run @wip tests**: Set `BDD_SKIP_WIP=0` environment variable:
   ```bash
   BDD_SKIP_WIP=0 ./run-bdd-tests-local.sh
   ```