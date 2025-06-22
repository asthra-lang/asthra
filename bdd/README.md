# BDD Testing Infrastructure for Asthra

This directory contains the Behavior-Driven Development (BDD) testing infrastructure for the Asthra programming language. BDD tests ensure the compiler behaves correctly from a user's perspective by describing functionality in terms of scenarios and expected outcomes.

## Quick Start

```bash
# Clone and build with BDD tests enabled
git clone https://github.com/asthra-lang/asthra.git
cd asthra
cmake -B build -DBUILD_BDD_TESTS=ON
cmake --build build

# Run all BDD tests
cmake --build build --target run_bdd_tests

# Run specific test category
cmake --build build --target bdd_unit -j8
```

## Directory Structure

```
bdd/
├── CMakeLists.txt        # Build configuration for BDD tests
├── README.md             # This file
├── CONTRIBUTING.md       # Guidelines for contributing BDD tests
├── features/             # Cucumber/Gherkin feature files
│   ├── compiler/         # End-to-end compiler features
│   ├── parser/           # Parser-specific features
│   └── semantic/         # Semantic analysis features
├── steps/                # Step definition implementations
│   ├── unit/            # Unit test step definitions
│   │   ├── parser/      # Parser unit tests
│   │   ├── semantic/    # Semantic analysis unit tests
│   │   └── codegen/     # Code generation unit tests
│   ├── integration/     # Integration test steps
│   ├── acceptance/      # Acceptance test steps
│   ├── scenario/        # Complex scenario tests
│   └── feature/         # Complete feature tests
├── support/             # BDD framework and utilities
│   ├── bdd_support.h    # Core BDD API
│   ├── bdd_support.c    # BDD implementation
│   └── test_helpers.h   # Common test utilities
├── fixtures/            # Test data and example programs
│   ├── valid/           # Valid Asthra programs
│   ├── invalid/         # Programs with intentional errors
│   └── examples/        # Complex test scenarios
└── bin/                 # Compiled test executables (generated)
```

## Building and Running BDD Tests

### Prerequisites

- CMake 3.20+
- C17-compatible compiler (GCC 11+, Clang 14+)
- (Optional) Ruby and Cucumber for Gherkin features
- (Optional) Cgreen testing framework

### Enable BDD Tests

```bash
# Configure with BDD tests enabled
cmake -B build -DBUILD_BDD_TESTS=ON

# Build the compiler and BDD test infrastructure
cmake --build build

# Build test executables (required after clean)
cmake --build build --target bdd_tests
```

### Available CMake Targets

| Target | Description |
|--------|-------------|
| `bdd_tests` | Build all BDD test executables |
| `run_bdd_tests` | Execute the full BDD test suite |
| `bdd_cucumber` | Run Cucumber features (requires Ruby/Cucumber) |
| `bdd_unit` | Run unit-level BDD tests |
| `bdd_integration` | Run integration BDD tests |
| `bdd_acceptance` | Run acceptance BDD tests |
| `bdd-report` | Generate HTML test report |
| `bdd-clean` | Clean BDD test outputs |

### Running Tests

```bash
# Run all BDD tests
cmake --build build --target run_bdd_tests

# Run specific test categories
cmake --build build --target bdd_unit        # Unit tests only
cmake --build build --target bdd_integration # Integration tests
cmake --build build --target bdd_acceptance  # Acceptance tests

# Run tests in parallel for faster execution
cmake --build build --target bdd_unit -j8

# Run with verbose output
ctest --test-dir build -V -L bdd

# Run specific test executable
./build/bdd/bin/test_parser_expressions

# Generate HTML report
cmake --build build --target bdd-report
open build/bdd/reports/bdd-report.html
```

## Writing BDD Tests

### C-Based BDD Tests (Primary Approach)

Our BDD framework provides a simple, expressive API for writing tests in C:

```c
#include "bdd_support.h"
#include "compiler.h"

int main(void) {
    bdd_init("Variable Declaration");
    
    bdd_scenario("Declare and initialize an integer variable");
    
    bdd_given("a variable declaration with initialization");
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> void {\n"
        "    let x: int = 42;\n"
        "    return ();\n"
        "}\n";
    
    bdd_when("compiling the source");
    CompilerResult result = compile_string(source);
    
    bdd_then("compilation succeeds");
    BDD_ASSERT_TRUE(result.success);
    BDD_ASSERT_NULL(result.error);
    
    bdd_and("the variable is properly typed");
    Symbol* var = find_symbol(result.symbols, "x");
    BDD_ASSERT_NOT_NULL(var);
    BDD_ASSERT_STR_EQ(var->type->name, "int");
    
    return bdd_report();
}
```

#### Available Assertions

```c
BDD_ASSERT_TRUE(condition)              // Assert condition is true
BDD_ASSERT_FALSE(condition)             // Assert condition is false
BDD_ASSERT_EQ(expected, actual)         // Assert values are equal
BDD_ASSERT_NEQ(expected, actual)        // Assert values are not equal
BDD_ASSERT_NULL(pointer)                // Assert pointer is NULL
BDD_ASSERT_NOT_NULL(pointer)            // Assert pointer is not NULL
BDD_ASSERT_STR_EQ(expected, actual)     // Assert strings are equal
BDD_ASSERT_STR_CONTAINS(str, substr)    // Assert string contains substring
BDD_ASSERT_GT(a, b)                     // Assert a > b
BDD_ASSERT_LT(a, b)                     // Assert a < b
BDD_ASSERT_GTE(a, b)                    // Assert a >= b
BDD_ASSERT_LTE(a, b)                    // Assert a <= b
```

### Cucumber Features (Optional)

When Cucumber is available, you can write feature files in Gherkin:

```gherkin
Feature: Pattern Matching
  As an Asthra developer
  I want to use pattern matching
  So that I can write expressive conditional logic

  Scenario: Match on enum variants
    Given the following Asthra source:
      """
      package example;
      
      enum Result {
          Ok(value: int),
          Err(message: string)
      }
      
      pub fn handle_result(r: Result) -> string {
          match r {
              Ok(v) => return "Success: " + int_to_string(v);
              Err(msg) => return "Error: " + msg;
          }
      }
      """
    When I compile the source
    Then compilation should succeed
    And the match expression should be exhaustive
```

## Test Categories

### Unit Tests (`steps/unit/`)
Fast, focused tests of individual components:
- Parser: Expression parsing, statement parsing, declarations
- Semantic: Type checking, inference, symbol resolution
- Codegen: Instruction generation, register allocation

### Integration Tests (`steps/integration/`)
Tests of component interactions:
- Parser + Semantic analysis
- Semantic + Code generation
- End-to-end compilation

### Acceptance Tests (`steps/acceptance/`)
High-level feature validation:
- Complete program compilation
- Language feature compliance
- Error reporting quality

### Scenario Tests (`steps/scenario/`)
Complex use cases and edge cases:
- Error recovery
- Performance scenarios
- Platform-specific behavior

### Feature Tests (`steps/feature/`)
Complete language feature validation:
- Pattern matching
- Memory safety
- FFI integration

## Best Practices

1. **Keep tests focused** - One scenario per behavior
2. **Use descriptive names** - Scenarios should clearly indicate what's tested
3. **Test both success and failure** - Include negative test cases
4. **Follow Asthra patterns** - Respect expression-oriented design
5. **Clean up resources** - Free allocated memory, close files
6. **Maintain independence** - Tests shouldn't depend on each other

Example of a well-structured test:

```c
bdd_scenario("Type inference for if-else expressions");

bdd_given("an if-else expression with consistent branch types");
const char* source = 
    "package test;\n"
    "pub fn choose(condition: bool) -> int {\n"
    "    let result = if condition { 42 } else { 0 };\n"
    "    return result;\n"
    "}\n";

bdd_when("analyzing the expression");
SemanticResult result = analyze_source(source);

bdd_then("the if-else expression type is inferred correctly");
BDD_ASSERT_TRUE(result.success);
Expression* if_expr = find_expression(result.ast, EXPR_IF_ELSE);
BDD_ASSERT_NOT_NULL(if_expr);
BDD_ASSERT_STR_EQ(if_expr->type->name, "int");
```

## Platform Support

The BDD infrastructure is tested and supported on:

- **Linux**: Ubuntu 22.04+, Debian 11+, Fedora 36+
- **macOS**: macOS 14+ (Apple Silicon and Intel)
- **Windows**: Not currently supported (use WSL2)

Platform-specific configurations are handled automatically by CMake.

## Troubleshooting

### Common Issues

1. **BDD tests not found**
   ```bash
   # Ensure BDD tests are enabled
   cmake -B build -DBUILD_BDD_TESTS=ON
   cmake --build build --target bdd_tests
   ```

2. **Test executables missing**
   ```bash
   # Build test executables first
   cmake --build build --target bdd_tests
   ```

3. **Cucumber not available**
   ```bash
   # Install Ruby and Cucumber (optional)
   gem install cucumber
   ```

4. **Memory leaks detected**
   ```bash
   # Run with AddressSanitizer
   cmake -B build -DSANITIZER=Address -DBUILD_BDD_TESTS=ON
   ```

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines on:
- Writing new BDD tests
- Test organization standards
- Naming conventions
- Review process

## Documentation

- [BDD Testing Guide](../docs/contributor/guides/bdd-testing-guide.md) - Comprehensive testing guide
- [BDD Best Practices](../docs/contributor/reference/bdd-best-practices.md) - Testing best practices
- [Test Fixtures](fixtures/README.md) - Guide to test fixtures

## CI/CD Integration

BDD tests run automatically on:
- Pull requests
- Pushes to main branch
- Nightly builds

See `.github/workflows/bdd.yml` for CI configuration.