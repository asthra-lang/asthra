# CLI Testing in Rust: Which Approach to Use?

## TL;DR: Use `assert_cmd` for 95% of your tests

**Recommendation**: Start with `assert_cmd` and only use `std::process::Command` when you have a specific need that `assert_cmd` can't handle.

## Quick Comparison

| Feature | `assert_cmd` | `std::process::Command` |
|---------|-------------|------------------------|
| **Readability** | ✅ Excellent | ❌ Verbose |
| **Error Messages** | ✅ Descriptive | ❌ Generic |
| **Binary Path** | ✅ Automatic | ❌ Manual |
| **Predicates** | ✅ Built-in | ❌ Manual |
| **Learning Curve** | ✅ Easy | ❌ Steeper |
| **Maintenance** | ✅ Low | ❌ Higher |

## The Recommended Approach: `assert_cmd`

### Basic Test Pattern
```rust
#[test]
fn test_version() -> Result<(), Box<dyn std::error::Error>> {
    AssertCommand::cargo_bin("ampu")?
        .arg("--version")
        .assert()
        .success()
        .stdout(predicate::str::contains("ampu"));
    Ok(())
}
```

### Why This is Better
1. **Cleaner code** - Less boilerplate
2. **Better errors** - Shows exactly what failed
3. **Automatic binary resolution** - No need for `cargo run --bin`
4. **Built-in predicates** - Rich assertion library
5. **Fluent API** - Easy to read and chain

### Common Patterns with `assert_cmd`

#### Testing Success Cases
```rust
#[test]
fn test_help() -> Result<(), Box<dyn std::error::Error>> {
    AssertCommand::cargo_bin("ampu")?
        .arg("--help")
        .assert()
        .success()
        .stdout(predicate::str::contains("USAGE"))
        .stdout(predicate::str::contains("build"));
    Ok(())
}
```

#### Testing Failure Cases
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

#### Testing with Environment Variables
```rust
#[test]
fn test_with_env() -> Result<(), Box<dyn std::error::Error>> {
    AssertCommand::cargo_bin("ampu")?
        .arg("build")
        .env("RUST_LOG", "debug")
        .current_dir(&project_path)
        .assert()
        .success();
    Ok(())
}
```

#### Testing with Timeouts
```rust
#[test]
fn test_performance() -> Result<(), Box<dyn std::error::Error>> {
    AssertCommand::cargo_bin("ampu")?
        .arg("build")
        .timeout(Duration::from_secs(30))
        .assert()
        .success();
    Ok(())
}
```

## When to Use `std::process::Command`

Only use the manual approach in these specific cases:

### 1. Interactive Process Control
```rust
#[test]
fn test_interactive_session() {
    let mut child = Command::new("ampu")
        .arg("repl")
        .stdin(Stdio::piped())
        .stdout(Stdio::piped())
        .spawn()
        .expect("Failed to start REPL");

    // Send multiple commands over time
    writeln!(child.stdin.as_mut().unwrap(), "help").unwrap();
    writeln!(child.stdin.as_mut().unwrap(), "quit").unwrap();
    
    let output = child.wait_with_output().unwrap();
    // ... assertions
}
```

### 2. Complex Shell Pipelines
```rust
#[test]
fn test_shell_integration() {
    let output = Command::new("sh")
        .arg("-c")
        .arg("ampu build 2>&1 | grep -c 'warning'")
        .output()
        .unwrap();
    
    let warning_count: i32 = String::from_utf8_lossy(&output.stdout)
        .trim()
        .parse()
        .unwrap();
    
    assert!(warning_count < 5);
}
```

### 3. Binary Output Testing
```rust
#[test]
fn test_binary_output() {
    let output = Command::new("ampu")
        .arg("export-binary")
        .output()
        .unwrap();
    
    // Check binary file header
    assert_eq!(&output.stdout[0..4], b"\x89PNG");
}
```

## Migration Strategy

If you already have `std::process::Command` tests, here's how to migrate:

### Before (Manual)
```rust
#[test]
fn test_version() {
    let output = Command::new("cargo")
        .args(&["run", "--bin", "ampu", "--", "--version"])
        .output()
        .expect("Failed to execute");
    
    assert!(output.status.success());
    let stdout = String::from_utf8_lossy(&output.stdout);
    assert!(stdout.contains("ampu"));
}
```

### After (assert_cmd)
```rust
#[test]
fn test_version() -> Result<(), Box<dyn std::error::Error>> {
    AssertCommand::cargo_bin("ampu")?
        .arg("--version")
        .assert()
        .success()
        .stdout(predicate::str::contains("ampu"));
    Ok(())
}
```

**Benefits of migration:**
- 50% less code
- Better error messages
- More maintainable
- Less platform-specific issues

## Practical Example: Complete Test Suite

Here's what a real test suite should look like:

```rust
use assert_cmd::Command as AssertCommand;
use predicates::prelude::*;
use tempfile::TempDir;
use std::fs;

// 95% of your tests should look like this
#[test]
fn test_basic_commands() -> Result<(), Box<dyn std::error::Error>> {
    // Version
    AssertCommand::cargo_bin("ampu")?
        .arg("--version")
        .assert()
        .success();

    // Help
    AssertCommand::cargo_bin("ampu")?
        .arg("--help")
        .assert()
        .success()
        .stdout(predicate::str::contains("USAGE"));

    // Invalid command
    AssertCommand::cargo_bin("ampu")?
        .arg("invalid")
        .assert()
        .failure();

    Ok(())
}

#[test]
fn test_project_workflow() -> Result<(), Box<dyn std::error::Error>> {
    let temp_dir = TempDir::new()?;
    
    // Init
    AssertCommand::cargo_bin("ampu")?
        .args(&["init", "test-project"])
        .current_dir(temp_dir.path())
        .assert()
        .success();

    let project_path = temp_dir.path().join("test-project");
    
    // Build
    AssertCommand::cargo_bin("ampu")?
        .arg("build")
        .current_dir(&project_path)
        .assert()
        .success();

    // Clean
    AssertCommand::cargo_bin("ampu")?
        .arg("clean")
        .current_dir(&project_path)
        .assert()
        .success();

    Ok(())
}

// Only use std::process::Command when you actually need it
#[test]
fn test_interactive_feature() {
    use std::process::{Command, Stdio};
    use std::io::Write;

    let mut child = Command::new("ampu")
        .arg("interactive-mode")
        .stdin(Stdio::piped())
        .stdout(Stdio::piped())
        .spawn()
        .expect("Failed to start interactive mode");

    writeln!(child.stdin.as_mut().unwrap(), "status").unwrap();
    writeln!(child.stdin.as_mut().unwrap(), "exit").unwrap();

    let output = child.wait_with_output().unwrap();
    assert!(output.status.success());
}
```

## Final Recommendation

1. **Start with `assert_cmd`** for all new tests
2. **Only use `std::process::Command`** when you need:
   - Interactive process control
   - Complex shell integration
   - Raw binary output testing
3. **Migrate existing manual tests** to `assert_cmd` when you touch them
4. **Keep it simple** - most CLI testing doesn't need the complexity of manual process management

The goal is **readable, maintainable tests** that clearly express what you're testing. `assert_cmd` achieves this for the vast majority of CLI testing scenarios. 
