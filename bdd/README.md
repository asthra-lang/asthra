# BDD Testing Infrastructure for Asthra

This directory contains the Behavior-Driven Development (BDD) testing infrastructure for the Asthra programming language.

## Directory Structure

- `features/` - Cucumber/Gherkin feature files describing test scenarios
- `steps/` - Step definition implementations organized by category
  - `unit/` - Unit test step definitions
  - `integration/` - Integration test step definitions
  - `acceptance/` - Acceptance test step definitions
  - `scenario/` - Scenario test step definitions
  - `feature/` - Feature test step definitions
- `support/` - Common support code and utilities
- `fixtures/` - Test fixtures and data files
- `bin/` - Compiled BDD test executables (generated)

## Building and Running BDD Tests

### Enable BDD Tests

```bash
cmake -B build -DBUILD_BDD_TESTS=ON
cmake --build build
```

### Available Targets

- `bdd_tests` - Build all BDD test executables
- `run_bdd_tests` - Execute the full BDD test suite
- `bdd_cucumber` - Run Cucumber features (requires Cucumber installed)
- `bdd_unit` - Run C-based BDD unit tests
- `bdd_integration` - Run BDD integration tests
- `bdd-report` - Generate BDD test report
- `bdd-clean` - Clean BDD test outputs

### Running Tests

```bash
# Build and run all BDD tests
cmake --build build --target bdd_tests
cmake --build build --target run_bdd_tests

# Run specific test categories
cmake --build build --target bdd_unit
cmake --build build --target bdd_integration

# Run Cucumber features (if available)
cmake --build build --target bdd_cucumber

# Generate test report
cmake --build build --target bdd-report
```

## Writing BDD Tests

### C-Based BDD Tests

1. Create step definition files in appropriate category directory
2. Use the BDD support API from `support/bdd_support.h`
3. Register the test in `CMakeLists.txt`

Example:
```c
#include "bdd_support.h"

int main(void) {
    bdd_init("Feature Name");
    
    bdd_scenario("Scenario description");
    bdd_given("initial condition");
    bdd_when("action is performed");
    bdd_then("expected outcome");
    BDD_ASSERT_TRUE(condition);
    
    return bdd_report();
}
```

### Cucumber Features

Write Gherkin feature files in the `features/` directory:

```gherkin
Feature: Feature name
  As a user
  I want functionality
  So that benefit

  Scenario: Scenario name
    Given initial state
    When action happens
    Then expected result
```

## Platform Support

The BDD infrastructure supports:
- Linux (Ubuntu 22.04+)
- macOS (14+)

Platform-specific configurations are handled automatically by CMake.