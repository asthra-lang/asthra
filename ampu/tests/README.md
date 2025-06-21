# End-to-End Testing for Ampu

This directory contains comprehensive end-to-end (e2e) tests for the Ampu CLI tool and Asthra language features using the recommended `assert_cmd` approach for CLI testing in Rust.

## Test Files Overview

### `e2e_tests.rs` - CLI Functionality Tests
Comprehensive e2e tests covering CLI tool functionality:
- **Core commands**: Version, help, and error handling
- **Project management**: Initialization, library creation, error cases
- **Build and validation**: Access control, TOML parsing, project checking
- **Dependency management**: Listing dependencies, cleaning builds
- **Environment handling**: Custom environment variables and configuration
- **Workflows**: Complete project lifecycle testing
- **Performance**: Command timing and concurrent operations
- **Special cases**: Interactive stdin handling (when `std::process::Command` is needed)

### `language_tests.rs` - Asthra Language Feature Tests
Tests that verify Asthra language features as defined in the specification:
- **Syntax parsing**: Function declarations, imports, type annotations
- **Type system**: Primitive types, slices, pointers, Result types
- **String operations**: Concatenation (`+`) and interpolation (`{}`)
- **Memory model**: Ownership annotations, FFI boundaries
- **Control flow**: If/else statements, for loops, pattern matching
- **Error handling**: Result types, access control restrictions
- **Observability**: Built-in logging primitives
- **Type inference**: Go-style simple type inference
- **Build integration**: Complete Asthra source to executable compilation

### `integration_tests.rs`
Internal API integration tests for testing Rust modules directly.

## Why Two Types of Tests?

### CLI Tests (`e2e_tests.rs`)
- **Purpose**: Test the `ampu` build tool functionality
- **Focus**: Command-line interface, project management, build orchestration
- **Examples**: `ampu init`, `ampu build`, `ampu clean`, error handling

### Language Tests (`language_tests.rs`)
- **Purpose**: Test Asthra language features and semantics
- **Focus**: Syntax parsing, type checking, code generation, language compliance
- **Examples**: Function declarations, type inference, string operations, FFI

This hybrid approach ensures comprehensive coverage of both the **build tool** and the **programming language** itself.

## Dependencies

The e2e tests use the following crates:

```toml
[dev-dependencies]
tempfile = "3.0"        # Temporary files and directories
tokio-test = "0.4"      # Async testing utilities
assert_cmd = "2.0"      # Ergonomic CLI testing
predicates = "3.1"      # Powerful assertion predicates
```

## Running the Tests

### Run All Tests
```bash
cargo test
```

### Run Specific Test Suites
```bash
# CLI functionality tests
cargo test --test e2e_tests

# Language feature tests
cargo test --test language_tests

# Internal integration tests
cargo test --test integration_tests
```

### Run Specific Test Functions
```bash
# CLI tests
cargo test test_version_command --test e2e_tests -- --exact

# Language tests
cargo test test_basic_function_declaration --test language_tests -- --exact
```

### Run Tests with Output
```bash
# Show stdout/stderr from tests
cargo test -- --nocapture

# Show test names as they run
cargo test -- --test-threads=1
```

### Run Tests in Release Mode
```bash
cargo test --release
```

## Testing Approach: `assert_cmd`

We use the `assert_cmd` crate for 95% of our CLI testing because it provides:

- **Cleaner code** with less boilerplate
- **Better error messages** that show exactly what failed
- **Automatic binary resolution** (no need for `cargo run --bin`)
- **Built-in predicates** for flexible assertions
- **Fluent API** that's easy to read and chain

### Basic Test Pattern

```rust
#[test]
fn test_version_command() -> Result<(), Box<dyn std::error::Error>> {
    AssertCommand::cargo_bin("ampu")?
        .arg("--version")
        .assert()
        .success()
        .stdout(predicate::str::contains("ampu"));
    Ok(())
}
```

### Language Feature Testing Pattern

```rust
#[test]
fn test_basic_function_declaration() -> Result<(), Box<dyn std::error::Error>> {
    let asthra_code = r#"
package main;

fn main() -> i32 {
    return 0;
}
"#;

    let temp_dir = create_asthra_project_with_source("syntax-test", asthra_code)?;
    let project_path = temp_dir.path().join("syntax-test");

    // Test that basic function syntax parses correctly
    AssertCommand::cargo_bin("ampu")?
        .arg("check")
        .current_dir(&project_path)
        .assert()
        .success();

    Ok(())
}
```

## Common Testing Patterns

### Testing CLI Success Cases
```rust
#[test]
fn test_help_command() -> Result<(), Box<dyn std::error::Error>> {
    AssertCommand::cargo_bin("ampu")?
        .arg("--help")
        .assert()
        .success()
        .stdout(predicate::str::contains("USAGE"))
        .stdout(predicate::str::contains("build"));
    Ok(())
}
```

### Testing CLI Failure Cases
```rust
#[test]
fn test_invalid_command() -> Result<(), Box<dyn std::error::Error>> {
    AssertCommand::cargo_bin("ampu")?
        .arg("nonexistent-command")
        .assert()
        .failure()
        .stderr(predicate::str::contains("error"));
    Ok(())
}
```

### Testing Language Syntax
```rust
#[test]
fn test_string_concatenation() -> Result<(), Box<dyn std::error::Error>> {
    let asthra_code = r#"
package main;

fn test_string_ops() -> void {
    let name: string = "World";
    let greeting: string = "Hello, " + name + "!";
}

fn main() -> i32 {
    test_string_ops();
    return 0;
}
"#;

    let temp_dir = create_asthra_project_with_source("string-concat-test", asthra_code)?;
    let project_path = temp_dir.path().join("string-concat-test");

    AssertCommand::cargo_bin("ampu")?
        .arg("check")
        .current_dir(&project_path)
        .assert()
        .success();

    Ok(())
}
```

### Testing Language Error Cases
```rust
#[test]
fn test_type_mismatch_error() -> Result<(), Box<dyn std::error::Error>> {
    let asthra_code = r#"
package main;

fn main() -> i32 {
    let number: i32 = "this is a string";
    return number;
}
"#;

    let temp_dir = create_asthra_project_with_source("type-error-test", asthra_code)?;
    let project_path = temp_dir.path().join("type-error-test");

    AssertCommand::cargo_bin("ampu")?
        .arg("check")
        .current_dir(&project_path)
        .assert()
        .failure()
        .stderr(predicate::str::contains("type"));

    Ok(())
}
```

### Testing with Environment Variables
```rust
#[test]
fn test_environment_variables() -> Result<(), Box<dyn std::error::Error>> {
    AssertCommand::cargo_bin("ampu")?
        .arg("--help")
        .env("NO_COLOR", "1")
        .assert()
        .success();
    Ok(())
}
```

### Testing with Timeouts
```rust
#[test]
fn test_command_performance() -> Result<(), Box<dyn std::error::Error>> {
    AssertCommand::cargo_bin("ampu")?
        .arg("--help")
        .timeout(Duration::from_secs(5))
        .assert()
        .success();
    Ok(())
}
```

## When to Use `std::process::Command`

Only use `std::process::Command` for special cases that `assert_cmd` can't handle:

### Interactive Process Control
```rust
#[test]
fn test_interactive_stdin() {
    use std::process::{Command, Stdio};
    use std::io::Write;

    let mut child = Command::new("cargo")
        .args(&["run", "--bin", "ampu", "--", "repl"])
        .stdin(Stdio::piped())
        .stdout(Stdio::piped())
        .spawn()
        .expect("Failed to spawn process");

    writeln!(child.stdin.as_mut().unwrap(), "help").unwrap();
    writeln!(child.stdin.as_mut().unwrap(), "quit").unwrap();

    let output = child.wait_with_output().unwrap();
    assert!(output.status.success());
}
```

## Best Practices

### 1. Test Isolation
- Use `TempDir` for temporary files and directories
- Each test should be independent
- Automatic cleanup with `TempDir`

```rust
let temp_dir = TempDir::new()?;  // Automatic cleanup
let project_path = temp_dir.path().join(project_name);
```

### 2. Clear Assertions
- Use descriptive predicates for flexible matching
- Test both success and failure cases
- Provide helpful error messages

```rust
.stderr(predicate::str::contains("asthra.toml").or(predicate::str::contains("project")))
```

### 3. Language Feature Coverage
- Test syntax parsing for all language constructs
- Verify type system behavior
- Test error conditions and edge cases
- Ensure spec compliance

### 4. Environment Testing
- Test with custom environment variables
- Verify environment-dependent behavior
- Test cross-platform compatibility

```rust
.env("AMPU_LOG_LEVEL", "debug")
.env("NO_COLOR", "1")
```

### 5. Performance Considerations
- Use timeouts for long-running commands
- Test concurrent operations
- Measure command execution time

```rust
.timeout(Duration::from_secs(30))
```

## Test Organization

### CLI Tests are organized into:
1. **Core CLI functionality** - Basic commands and error handling
2. **Project management** - Initialization, library creation, validation
3. **Build and check** - Access control, TOML parsing, project validation
4. **Dependency management** - Listing, adding, removing dependencies
5. **Environment and configuration** - Environment variables, custom settings
6. **Workflow and integration** - Complete project lifecycle testing
7. **Performance and stress** - Timing, concurrent operations, large projects
8. **Special cases** - Interactive features requiring `std::process::Command`

### Language Tests are organized into:
1. **Basic syntax** - Function declarations, imports, package declarations
2. **Type system** - Primitive types, slices, pointers, Result types
3. **String operations** - Concatenation and interpolation (spec section 6)
4. **FFI and memory model** - Extern functions, ownership annotations (spec section 3)
5. **Control flow** - If/else, loops, pattern matching
6. **Observability** - Built-in logging primitives (spec section 7)
7. **Access control** - Import restrictions (spec section 1.6)
8. **Build integration** - Complete compilation pipeline
9. **Type inference** - Go-style inference system (spec section 6.1)
10. **Error cases** - Invalid syntax, type mismatches, language restrictions

## Running the Tests

To run these comprehensive e2e tests:

```bash
# Build the project first
cargo build

# Run all tests
cargo test

# Run only CLI tests
cargo test --test e2e_tests

# Run only language tests
cargo test --test language_tests

# Run with output visible
cargo test -- --nocapture

# Run specific test category
cargo test test_string -- --nocapture  # All string-related tests
cargo test test_type -- --nocapture    # All type-related tests
```

## Test Results Interpretation

- **Passing tests** indicate working functionality
- **Failing tests** reveal features that need implementation
- Tests serve as specifications for expected behavior
- Use test failures to guide development priorities

### CLI Test Results
- Show which `ampu` commands are working
- Indicate build system functionality status
- Guide CLI feature development

### Language Test Results
- Show which Asthra language features are implemented
- Indicate parser and compiler functionality
- Guide language feature development priorities
- Ensure compliance with the Asthra specification

The test suite provides comprehensive coverage of both CLI functionality and language features while maintaining clean, readable, and maintainable test code using the `assert_cmd` approach. 
