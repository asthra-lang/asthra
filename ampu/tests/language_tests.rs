//! Language Feature Tests for Asthra
//! 
//! These tests verify that the Asthra language features defined in the specification
//! work correctly. They complement the CLI e2e tests by focusing on language semantics,
//! syntax parsing, type checking, and code generation.

use std::fs;
use tempfile::TempDir;
use assert_cmd::Command as AssertCommand;
use predicates::prelude::*;

/// Helper to create a test project with Asthra source code
fn create_asthra_project_with_source(
    project_name: &str,
    main_content: &str,
) -> Result<TempDir, Box<dyn std::error::Error>> {
    let temp_dir = TempDir::new()?;
    let project_path = temp_dir.path().join(project_name);
    
    // Create project structure
    fs::create_dir_all(&project_path)?;
    fs::create_dir_all(project_path.join("src"))?;

    // Create asthra.toml with valid configuration values
    let asthra_toml = format!(
        r#"[package]
name = "{}"
version = "1.0.0"
asthra_version = "1.2"

[dependencies]

[build]
target = "native"
optimization = "none"
"#,
        project_name
    );
    fs::write(project_path.join("asthra.toml"), asthra_toml)?;

    // Create main.asthra with provided content
    fs::write(project_path.join("src").join("main.asthra"), main_content)?;

    Ok(temp_dir)
}

// =============================================================================
// Basic Language Syntax Tests
// =============================================================================

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

#[test]
fn test_import_declarations() -> Result<(), Box<dyn std::error::Error>> {
    let asthra_code = r#"
package main;

import "stdlib/string";
import "stdlib/asthra/core";

fn main() -> i32 {
    return 0;
}
"#;

    let temp_dir = create_asthra_project_with_source("import-test", asthra_code)?;
    let project_path = temp_dir.path().join("import-test");

    // Test that import declarations parse correctly
    AssertCommand::cargo_bin("ampu")?
        .arg("check")
        .current_dir(&project_path)
        .assert()
        .success();

    Ok(())
}

#[test]
fn test_import_aliases() -> Result<(), Box<dyn std::error::Error>> {
    let asthra_code = r#"
package main;

import "stdlib/string" as str;
import "stdlib/asthra/core" as core;

fn main() -> i32 {
    return 0;
}
"#;

    let temp_dir = create_asthra_project_with_source("alias-test", asthra_code)?;
    let project_path = temp_dir.path().join("alias-test");

    // Test that import aliases parse correctly (spec section 6.2)
    AssertCommand::cargo_bin("ampu")?
        .arg("check")
        .current_dir(&project_path)
        .assert()
        .success();

    Ok(())
}

// =============================================================================
// Type System Tests
// =============================================================================

#[test]
fn test_primitive_types() -> Result<(), Box<dyn std::error::Error>> {
    let asthra_code = r#"
package main;

fn test_types() -> void {
    let int_val: i32 = 42;
    let uint_val: u32 = 100;
    let float_val: f64 = 3.14;
    let bool_val: bool = true;
    let string_val: string = "hello";
    let size_val: usize = 1024;
}

fn main() -> i32 {
    test_types();
    return 0;
}
"#;

    let temp_dir = create_asthra_project_with_source("types-test", asthra_code)?;
    let project_path = temp_dir.path().join("types-test");

    // Test that primitive types are recognized (spec section 6.1)
    AssertCommand::cargo_bin("ampu")?
        .arg("check")
        .current_dir(&project_path)
        .assert()
        .success();

    Ok(())
}

#[test]
fn test_slice_types() -> Result<(), Box<dyn std::error::Error>> {
    let asthra_code = r#"
package main;

fn test_slices() -> void {
    let byte_slice: []u8 = [1, 2, 3];
    let int_slice: []i32 = [10, 20, 30];
    let string_slice: []string = ["hello", "world"];
    
    let length: usize = byte_slice.len;
}

fn main() -> i32 {
    test_slices();
    return 0;
}
"#;

    let temp_dir = create_asthra_project_with_source("slice-test", asthra_code)?;
    let project_path = temp_dir.path().join("slice-test");

    // Test that slice types work correctly (spec section 4)
    AssertCommand::cargo_bin("ampu")?
        .arg("check")
        .current_dir(&project_path)
        .assert()
        .success();

    Ok(())
}

#[test]
fn test_pointer_types() -> Result<(), Box<dyn std::error::Error>> {
    let asthra_code = r#"
package main;

fn test_pointers() -> void {
    let mut_ptr: *mut i32;
    let const_ptr: *const u8;
}

fn main() -> i32 {
    test_pointers();
    return 0;
}
"#;

    let temp_dir = create_asthra_project_with_source("pointer-test", asthra_code)?;
    let project_path = temp_dir.path().join("pointer-test");

    // Test that pointer types parse correctly (spec section 3)
    AssertCommand::cargo_bin("ampu")?
        .arg("check")
        .current_dir(&project_path)
        .assert()
        .success();

    Ok(())
}

// =============================================================================
// String Operations Tests (Spec Section 6)
// =============================================================================

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

    // Test string concatenation with + operator (spec section 6)
    AssertCommand::cargo_bin("ampu")?
        .arg("check")
        .current_dir(&project_path)
        .assert()
        .success();

    Ok(())
}

// =============================================================================
// Result Type and Error Handling Tests (Spec Section 5)
// =============================================================================

#[test]
fn test_result_type_declaration() -> Result<(), Box<dyn std::error::Error>> {
    let asthra_code = r#"
package main;

fn might_fail(input: i32) -> Result<i32, string> {
    if input < 0 {
        return Result.Err("Input cannot be negative");
    }
    return Result.Ok(input * 2);
}

fn main() -> i32 {
    let result: Result<i32, string> = might_fail(10);
    return 0;
}
"#;

    let temp_dir = create_asthra_project_with_source("result-test", asthra_code)?;
    let project_path = temp_dir.path().join("result-test");

    // Test Result type syntax (spec section 5)
    AssertCommand::cargo_bin("ampu")?
        .arg("check")
        .current_dir(&project_path)
        .assert()
        .success();

    Ok(())
}

// =============================================================================
// FFI and Memory Model Tests (Spec Section 3)
// =============================================================================

#[test]
fn test_extern_function_declaration() -> Result<(), Box<dyn std::error::Error>> {
    let asthra_code = r#"
package main;

extern "libc" fn malloc(size: usize) -> *mut void;
extern "libc" fn free(ptr: *mut void) -> void;

fn main() -> i32 {
    return 0;
}
"#;

    let temp_dir = create_asthra_project_with_source("extern-test", asthra_code)?;
    let project_path = temp_dir.path().join("extern-test");

    // Test extern function declarations (spec section 3)
    AssertCommand::cargo_bin("ampu")?
        .arg("check")
        .current_dir(&project_path)
        .assert()
        .success();

    Ok(())
}

#[test]
fn test_ownership_annotations() -> Result<(), Box<dyn std::error::Error>> {
    let asthra_code = r#"
package main;

#[ownership(gc)]
struct AsthraData {
    field1: i32;
    field2: *mut u8;
}

extern "libc" #[ownership(c)] fn malloc(size: usize) -> *mut void;

fn main() -> i32 {
    return 0;
}
"#;

    let temp_dir = create_asthra_project_with_source("ownership-test", asthra_code)?;
    let project_path = temp_dir.path().join("ownership-test");

    // Test ownership annotations (spec section 3)
    AssertCommand::cargo_bin("ampu")?
        .arg("check")
        .current_dir(&project_path)
        .assert()
        .success();

    Ok(())
}

// =============================================================================
// Control Flow Tests
// =============================================================================

#[test]
fn test_if_else_statements() -> Result<(), Box<dyn std::error::Error>> {
    let asthra_code = r#"
package main;

fn test_control_flow(x: i32) -> i32 {
    if x > 0 {
        return 1;
    } else if x < 0 {
        return -1;
    } else {
        return 0;
    }
}

fn main() -> i32 {
    let result: i32 = test_control_flow(5);
    return result;
}
"#;

    let temp_dir = create_asthra_project_with_source("control-flow-test", asthra_code)?;
    let project_path = temp_dir.path().join("control-flow-test");

    // Test if/else control flow
    AssertCommand::cargo_bin("ampu")?
        .arg("check")
        .current_dir(&project_path)
        .assert()
        .success();

    Ok(())
}

#[test]
fn test_for_loop() -> Result<(), Box<dyn std::error::Error>> {
    let asthra_code = r#"
package main;

fn test_loop() -> i32 {
    let sum: i32 = 0;
    for i in range(0, 10) {
        sum = sum + i;
    }
    return sum;
}

fn main() -> i32 {
    return test_loop();
}
"#;

    let temp_dir = create_asthra_project_with_source("loop-test", asthra_code)?;
    let project_path = temp_dir.path().join("loop-test");

    // Test for/in/range loops
    AssertCommand::cargo_bin("ampu")?
        .arg("check")
        .current_dir(&project_path)
        .assert()
        .success();

    Ok(())
}

// =============================================================================
// Observability System Tests (Spec Section 7)
// =============================================================================

#[test]
fn test_logging_primitives() -> Result<(), Box<dyn std::error::Error>> {
    let asthra_code = r#"
package main;

fn test_logging() -> void {
    log(memory_zones);
    log(ffi_calls);
    log(gc_stats);
    
    log("DEBUG", "Processing item: 42");
    log("ERROR", "Failed to allocate memory");
}

fn main() -> i32 {
    test_logging();
    return 0;
}
"#;

    let temp_dir = create_asthra_project_with_source("logging-test", asthra_code)?;
    let project_path = temp_dir.path().join("logging-test");

    // Test built-in logging primitives (spec section 7)
    AssertCommand::cargo_bin("ampu")?
        .arg("check")
        .current_dir(&project_path)
        .assert()
        .success();

    Ok(())
}

// =============================================================================
// Access Control Tests (Spec Section 1.6)
// =============================================================================

#[test]
fn test_internal_import_restriction() -> Result<(), Box<dyn std::error::Error>> {
    let asthra_code = r#"
package main;

import "internal/runtime/memory_allocator";

fn main() -> i32 {
    return 0;
}
"#;

    let temp_dir = create_asthra_project_with_source("access-control-test", asthra_code)?;
    let project_path = temp_dir.path().join("access-control-test");

    // Test that internal imports are forbidden (spec section 1.6)
    AssertCommand::cargo_bin("ampu")?
        .args(&["check", "--access-control"])
        .current_dir(&project_path)
        .assert()
        .failure()
        .stderr(predicate::str::contains("internal"));

    Ok(())
}

// =============================================================================
// Build Integration Tests
// =============================================================================

#[test]
fn test_complete_build_process() -> Result<(), Box<dyn std::error::Error>> {
    let asthra_code = r#"
package main;

import "stdlib/string";

fn greet(name: string) -> string {
    return "Hello, " + name + "!";
}

fn main() -> i32 {
    let message: string = greet("Asthra");
    log("INFO", message);
    return 0;
}
"#;

    let temp_dir = create_asthra_project_with_source("build-test", asthra_code)?;
    let project_path = temp_dir.path().join("build-test");

    // Test complete build process from Asthra source to executable
    AssertCommand::cargo_bin("ampu")?
        .arg("build")
        .current_dir(&project_path)
        .assert()
        .success();

    // Verify that executable was created
    let target_dir = project_path.join("target");
    assert!(target_dir.exists());

    Ok(())
}

// =============================================================================
// Type Inference Tests (Spec Section 6.1)
// =============================================================================

#[test]
fn test_type_inference() -> Result<(), Box<dyn std::error::Error>> {
    let asthra_code = r#"
package main;

fn test_inference() -> void {
    let integer = 42;        // Should infer i32
    let float = 3.14;        // Should infer f64
    let text = "hello";      // Should infer string
    let flag = true;         // Should infer bool
    let bytes = [1, 2, 3];   // Should infer []i32
    
    // Explicit override when needed
    let small_num: i16 = 42;
    let precise: f32 = 3.14;
    let byte_array: []u8 = [1, 2, 3];
}

fn main() -> i32 {
    test_inference();
    return 0;
}
"#;

    let temp_dir = create_asthra_project_with_source("inference-test", asthra_code)?;
    let project_path = temp_dir.path().join("inference-test");

    // Test type inference system (spec section 6.1)
    AssertCommand::cargo_bin("ampu")?
        .arg("check")
        .current_dir(&project_path)
        .assert()
        .success();

    Ok(())
}

// =============================================================================
// Error Cases - Testing Language Restrictions
// =============================================================================

#[test]
fn test_invalid_syntax_error() -> Result<(), Box<dyn std::error::Error>> {
    let asthra_code = r#"
package main;

fn main() -> i32 {
    let invalid syntax here;
    return 0;
}
"#;

    let temp_dir = create_asthra_project_with_source("invalid-syntax-test", asthra_code)?;
    let project_path = temp_dir.path().join("invalid-syntax-test");

    // Test that invalid syntax is properly rejected
    AssertCommand::cargo_bin("ampu")?
        .arg("check")
        .current_dir(&project_path)
        .assert()
        .failure()
        .stderr(predicate::str::contains("syntax").or(predicate::str::contains("parse")));

    Ok(())
}

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

    // Test that type mismatches are caught
    AssertCommand::cargo_bin("ampu")?
        .arg("check")
        .current_dir(&project_path)
        .assert()
        .failure()
        .stderr(predicate::str::contains("type"));

    Ok(())
} 
