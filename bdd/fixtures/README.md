# Asthra BDD Test Fixtures

This directory contains comprehensive test fixtures for the Asthra programming language, designed to support Behavior-Driven Development (BDD) testing with Cucumber.

## Directory Structure

```
bdd/fixtures/
├── valid/           # Valid Asthra programs
├── invalid/         # Invalid programs for error testing
├── examples/        # Edge cases and complex scenarios
└── performance/     # Performance and stress test fixtures
```

## Valid Programs (`valid/`)

### Basic Examples
- **hello_world_basic.at** - Simplest valid Asthra program
- **hello_world_with_variables.at** - Hello world with variable declarations

### Language Features
- **functions_basic.at** - Function definitions, calls, and visibility
- **structs_basic.at** - Struct definitions, fields, and methods
- **structs_with_generics.at** - Generic struct definitions and usage

### Control Flow
- **control_flow_if_else.at** - If-else expressions and conditionals
- **control_flow_loops.at** - For loops, iterators, break/continue
- **control_flow_pattern_matching.at** - Match expressions with enums/options

### Concurrency
- **concurrency_basic.at** - Spawn and spawn_with_handle demonstrations

### FFI Integration
- **ffi_basic.at** - External C function declarations and usage
- **ffi_struct_interop.at** - C struct compatibility and memory management

## Invalid Programs (`invalid/`)

### Syntax Errors
- **syntax_missing_package.at** - Missing package declaration
- **syntax_missing_semicolon.at** - Missing semicolons
- **syntax_invalid_function.at** - Function syntax errors
- **syntax_invalid_struct.at** - Struct syntax errors
- **syntax_invalid_control_flow.at** - Control flow syntax errors

### Semantic Errors
- **type_mismatch.at** - Type system violations
- **semantic_undefined_symbols.at** - Undefined variables/functions/types
- **semantic_mutability_errors.at** - Immutability violations
- **semantic_visibility_errors.at** - Private member access violations

## Edge Cases (`examples/`)

- **edge_case_deeply_nested.at** - Deeply nested structures and control flow
- **edge_case_large_expressions.at** - Very long expressions and identifiers

## Performance Tests (`performance/`)

- **large_program.at** - Many functions, types, and symbols
- **memory_intensive.at** - Memory allocation and data structure operations

## Usage with Cucumber

These fixtures are designed to be used with Cucumber feature files. Example:

```gherkin
Feature: Asthra Compilation

  Scenario: Compile valid hello world program
    Given an Asthra source file "bdd/fixtures/valid/hello_world_basic.at"
    When I compile the program
    Then compilation should succeed
    And no errors should be reported

  Scenario: Detect missing package declaration
    Given an Asthra source file "bdd/fixtures/invalid/syntax_missing_package.at"
    When I compile the program
    Then compilation should fail
    And error should contain "package"
```

## Key Language Features Covered

1. **Package System** - All files demonstrate package declarations
2. **Type System** - Primitives, structs, enums, generics, options, results
3. **Expression-Oriented** - If-else expressions, match expressions
4. **Memory Safety** - Immutability by default, explicit mutability
5. **Concurrency** - Deterministic task spawning
6. **FFI Safety** - Ownership annotations, safe C interop
7. **Visibility** - Explicit public/private modifiers

## Testing Strategy

- **Positive Tests**: Valid programs should compile and run correctly
- **Negative Tests**: Invalid programs should produce specific error messages
- **Edge Cases**: Test parser/compiler limits and corner cases
- **Performance**: Measure compilation time and memory usage

## Contributing

When adding new fixtures:
1. Follow naming convention: `category_description.at`
2. Include descriptive comments explaining the test purpose
3. Ensure fixtures align with Asthra's design principles
4. Update this README with new fixtures