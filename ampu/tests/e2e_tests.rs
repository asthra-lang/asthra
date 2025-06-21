//! End-to-End Tests for Ampu CLI
//! 
//! This test suite uses the recommended `assert_cmd` approach for CLI testing.
//! Only use `std::process::Command` when you need interactive process control,
//! complex shell pipelines, or raw binary output testing.

use std::fs;
use tempfile::TempDir;
use assert_cmd::Command as AssertCommand;
use predicates::prelude::*;

/// Helper function to create a test project structure
fn create_test_asthra_toml(project_name: &str) -> String {
    format!(
        r#"[package]
name = "{}"
version = "1.0.0"
asthra_version = "1.0"

[dependencies]

[build]
target = "native"
optimization = "debug"
"#,
        project_name
    )
}

// =============================================================================
// Core CLI functionality tests
// =============================================================================

#[test]
fn test_version_command() -> Result<(), Box<dyn std::error::Error>> {
    AssertCommand::cargo_bin("ampu")?
        .arg("--version")
        .assert()
        .success()
        .stdout(predicate::str::contains("ampu"));
    Ok(())
}

#[test]
fn test_help_command() -> Result<(), Box<dyn std::error::Error>> {
    AssertCommand::cargo_bin("ampu")?
        .arg("--help")
        .assert()
        .success()
        .stdout(predicate::str::contains("Usage"))
        .stdout(predicate::str::contains("build"));
    Ok(())
}

#[test]
fn test_invalid_command() -> Result<(), Box<dyn std::error::Error>> {
    AssertCommand::cargo_bin("ampu")?
        .arg("nonexistent-command")
        .assert()
        .failure()
        .stderr(predicate::str::contains("error"));
    Ok(())
}

// =============================================================================
// Project management tests
// =============================================================================

#[test]
fn test_project_initialization() -> Result<(), Box<dyn std::error::Error>> {
    let temp_dir = TempDir::new()?;
    let project_name = "test-project";

    AssertCommand::cargo_bin("ampu")?
        .args(&["init", project_name])
        .current_dir(temp_dir.path())
        .assert()
        .success();

    // Verify files were created
    let project_path = temp_dir.path().join(project_name);
    assert!(project_path.join("asthra.toml").exists());
    assert!(project_path.join("src").exists());

    Ok(())
}

#[test]
fn test_library_initialization() -> Result<(), Box<dyn std::error::Error>> {
    let temp_dir = TempDir::new()?;
    let project_name = "test-lib";

    AssertCommand::cargo_bin("ampu")?
        .args(&["init", project_name, "--lib"])
        .current_dir(temp_dir.path())
        .assert()
        .success();

    let project_path = temp_dir.path().join(project_name);
    assert!(project_path.join("asthra.toml").exists());
    assert!(project_path.join("src").exists());

    Ok(())
}

#[test]
fn test_build_without_project() -> Result<(), Box<dyn std::error::Error>> {
    let temp_dir = TempDir::new()?;

    AssertCommand::cargo_bin("ampu")?
        .arg("build")
        .current_dir(temp_dir.path())
        .assert()
        .failure()
        .stderr(predicate::str::contains("asthra.toml"));

    Ok(())
}

#[test]
fn test_init_existing_directory() -> Result<(), Box<dyn std::error::Error>> {
    let temp_dir = TempDir::new()?;
    let project_name = "existing-dir";

    // Create the directory first
    let project_path = temp_dir.path().join(project_name);
    fs::create_dir_all(&project_path)?;
    fs::write(project_path.join("existing.txt"), "existing content")?;

    // Try to initialize in existing directory
    AssertCommand::cargo_bin("ampu")?
        .args(&["init", project_name])
        .current_dir(temp_dir.path())
        .assert()
        .failure()
        .stderr(predicate::str::contains("exists").or(predicate::str::contains("directory")));

    Ok(())
}

// =============================================================================
// Build and check tests
// =============================================================================

#[test]
fn test_access_control_validation() -> Result<(), Box<dyn std::error::Error>> {
    let temp_dir = TempDir::new()?;
    let project_name = "access-test";
    
    // Create project structure
    let project_path = temp_dir.path().join(project_name);
    fs::create_dir_all(&project_path)?;
    fs::create_dir_all(project_path.join("src"))?;

    // Create asthra.toml
    fs::write(
        project_path.join("asthra.toml"), 
        create_test_asthra_toml(project_name)
    )?;

    // Create main.asthra with invalid internal import
    fs::write(
        project_path.join("src").join("main.asthra"),
        r#"import "internal/runtime/memory_allocator";
fn main() { }
"#
    )?;

    // This should fail due to access control
    AssertCommand::cargo_bin("ampu")?
        .args(&["check", "--access-control"])
        .current_dir(&project_path)
        .assert()
        .failure()
        .stderr(predicate::str::contains("internal"));

    Ok(())
}

#[test]
fn test_invalid_asthra_toml() -> Result<(), Box<dyn std::error::Error>> {
    let temp_dir = TempDir::new()?;
    
    // Create invalid asthra.toml
    fs::write(temp_dir.path().join("asthra.toml"), "invalid toml content [[[[")?;

    AssertCommand::cargo_bin("ampu")?
        .arg("check")
        .current_dir(temp_dir.path())
        .assert()
        .failure()
        .stderr(predicate::str::contains("toml").or(predicate::str::contains("parse")));

    Ok(())
}

// =============================================================================
// Dependency management tests
// =============================================================================

#[test]
fn test_list_dependencies() -> Result<(), Box<dyn std::error::Error>> {
    let temp_dir = TempDir::new()?;
    let project_name = "deps-test";

    // Initialize project first
    AssertCommand::cargo_bin("ampu")?
        .args(&["init", project_name])
        .current_dir(temp_dir.path())
        .assert()
        .success();

    let project_path = temp_dir.path().join(project_name);

    // Test list command (should work even with no dependencies)
    AssertCommand::cargo_bin("ampu")?
        .arg("list")
        .current_dir(&project_path)
        .assert()
        .success();

    Ok(())
}

#[test]
fn test_clean_command() -> Result<(), Box<dyn std::error::Error>> {
    let temp_dir = TempDir::new()?;
    let project_name = "clean-test";

    // Initialize project
    AssertCommand::cargo_bin("ampu")?
        .args(&["init", project_name])
        .current_dir(temp_dir.path())
        .assert()
        .success();

    let project_path = temp_dir.path().join(project_name);

    // Create a target directory with some dummy files
    let target_dir = project_path.join("target");
    fs::create_dir_all(&target_dir.join("debug"))?;
    fs::write(target_dir.join("debug").join("dummy.o"), "dummy content")?;

    // Test clean command
    AssertCommand::cargo_bin("ampu")?
        .arg("clean")
        .current_dir(&project_path)
        .assert()
        .success();

    // Verify target directory is cleaned
    assert!(!target_dir.join("debug").join("dummy.o").exists());

    Ok(())
}

// =============================================================================
// Environment and configuration tests
// =============================================================================

#[test]
fn test_environment_variables() -> Result<(), Box<dyn std::error::Error>> {
    AssertCommand::cargo_bin("ampu")?
        .arg("--help")
        .env("NO_COLOR", "1")
        .assert()
        .success();
    Ok(())
}

#[test]
fn test_custom_environment() -> Result<(), Box<dyn std::error::Error>> {
    let temp_dir = TempDir::new()?;
    let project_name = "env-test";

    AssertCommand::cargo_bin("ampu")?
        .args(&["init", project_name])
        .current_dir(temp_dir.path())
        .env("AMPU_LOG_LEVEL", "debug")
        .env("AMPU_CACHE_DIR", temp_dir.path().join("custom-cache"))
        .assert()
        .success();

    assert!(temp_dir.path().join(project_name).exists());
    Ok(())
}

// =============================================================================
// Workflow and integration tests
// =============================================================================

#[test]
fn test_complete_workflow() -> Result<(), Box<dyn std::error::Error>> {
    let temp_dir = TempDir::new()?;
    let project_name = "workflow-test";

    // Step 1: Initialize project
    AssertCommand::cargo_bin("ampu")?
        .args(&["init", project_name])
        .current_dir(temp_dir.path())
        .assert()
        .success();

    let project_path = temp_dir.path().join(project_name);

    // Step 2: Check the project
    AssertCommand::cargo_bin("ampu")?
        .arg("check")
        .current_dir(&project_path)
        .assert()
        .success();

    // Step 3: List dependencies
    AssertCommand::cargo_bin("ampu")?
        .arg("list")
        .current_dir(&project_path)
        .assert()
        .success();

    // Step 4: Clean
    AssertCommand::cargo_bin("ampu")?
        .arg("clean")
        .current_dir(&project_path)
        .assert()
        .success();

    Ok(())
}

#[test]
fn test_file_generation_and_content() -> Result<(), Box<dyn std::error::Error>> {
    let temp_dir = TempDir::new()?;
    let project_name = "content-test";

    AssertCommand::cargo_bin("ampu")?
        .args(&["init", project_name])
        .current_dir(temp_dir.path())
        .assert()
        .success();

    let project_path = temp_dir.path().join(project_name);

    // Verify asthra.toml content
    let toml_content = fs::read_to_string(project_path.join("asthra.toml"))?;
    assert!(toml_content.contains("[package]"));
    assert!(toml_content.contains(&format!("name = \"{}\"", project_name)));
    assert!(toml_content.contains("version = "));

    // Verify .gitignore was created (if applicable)
    let gitignore_path = project_path.join(".gitignore");
    if gitignore_path.exists() {
        let gitignore_content = fs::read_to_string(&gitignore_path)?;
        assert!(gitignore_content.contains("target/") || gitignore_content.contains("/target"));
    }

    Ok(())
}

// =============================================================================
// Performance and stress tests
// =============================================================================

#[test]
fn test_command_performance() -> Result<(), Box<dyn std::error::Error>> {
    use std::time::{Duration, Instant};

    let start = Instant::now();
    
    AssertCommand::cargo_bin("ampu")?
        .arg("--help")
        .timeout(Duration::from_secs(5))
        .assert()
        .success();

    let duration = start.elapsed();
    assert!(duration < Duration::from_secs(2), "Command took too long: {:?}", duration);

    Ok(())
}

#[test]
fn test_concurrent_operations() -> Result<(), Box<dyn std::error::Error>> {
    use std::thread;
    use std::sync::Arc;
    use std::sync::atomic::{AtomicUsize, Ordering};

    let success_count = Arc::new(AtomicUsize::new(0));
    let mut handles = vec![];

    for i in 0..3 {
        let success_count = Arc::clone(&success_count);
        let handle = thread::spawn(move || {
            let temp_dir = TempDir::new().unwrap();
            let project_name = format!("concurrent-{}", i);

            let result = AssertCommand::cargo_bin("ampu")
                .unwrap()
                .args(&["init", &project_name])
                .current_dir(temp_dir.path())
                .assert()
                .try_success();

            if result.is_ok() {
                success_count.fetch_add(1, Ordering::SeqCst);
            }
        });
        handles.push(handle);
    }

    for handle in handles {
        handle.join().unwrap();
    }

    assert!(success_count.load(Ordering::SeqCst) > 0);
    Ok(())
}

// =============================================================================
// Special cases requiring std::process::Command
// =============================================================================

#[test]
fn test_interactive_stdin() {
    use std::process::{Command, Stdio};
    use std::io::Write;

    // This demonstrates when std::process::Command is actually needed:
    // when you need to interact with stdin/stdout of a running process
    let mut child = Command::new("cargo")
        .args(&["run", "--bin", "ampu", "--", "--version"])
        .stdin(Stdio::piped())
        .stdout(Stdio::piped())
        .stderr(Stdio::piped())
        .spawn()
        .expect("Failed to spawn process");

    // Write to stdin (demonstration only - --version doesn't need input)
    if let Some(stdin) = child.stdin.as_mut() {
        stdin.write_all(b"y\n").expect("Failed to write to stdin");
    }

    let output = child.wait_with_output().expect("Failed to read output");
    assert!(output.status.success());
} 
