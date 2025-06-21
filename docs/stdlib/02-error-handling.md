# Error Handling Patterns

## Overview

All Asthra standard library functions that can fail return `Result<T, E>` types. This ensures explicit error handling and prevents runtime failures through compile-time checking.

## Result Type Basics

### Built-in Result Type

`Result<T, E>` is a built-in generic type in Asthra that doesn't require any declaration or import. It has two variants:

```asthra
Result.Ok(T)      // Success case with value
Result.Err(E)     // Error case with error information
```

### Common Error Types

```asthra
// Filesystem errors
enum fs.IOError {
    NotFound(string),
    PermissionDenied(string),
    AlreadyExists(string),
    InvalidInput(string),
    UnexpectedEof,
    Other(string)
}

// String processing errors
enum string.StringError {
    InvalidUtf8(string),
    InvalidFormat(string),
    OutOfBounds(string)
}

// Process management errors
enum process.ProcessError {
    NotFound(string),
    PermissionDenied(string),
    InvalidInput(string),
    Other(string)
}
```

## Error Handling Patterns

### Pattern 1: Match All Cases

Handle each specific error type with appropriate responses:

```asthra
import "stdlib/asthra/fs";

fn load_config() -> Result<string, string> {
    let file_result: Result<string, fs.IOError> = fs.read_to_string("config.json");
    
    match file_result {
        Result.Ok(content) => {
            log("SUCCESS", "Configuration loaded successfully");
            return Result.Ok(content);
        },
        Result.Err(fs.IOError.NotFound(path)) => {
            log("ERROR", "Configuration file not found: " + path);
            return Result.Err("Configuration file missing");
        },
        Result.Err(fs.IOError.PermissionDenied(path)) => {
            log("ERROR", "Permission denied accessing: " + path);
            return Result.Err("Cannot access configuration file");
        },
        Result.Err(fs.IOError.InvalidInput(msg)) => {
            log("ERROR", "Invalid configuration format: " + msg);
            return Result.Err("Configuration file format error");
        },
        Result.Err(error) => {
            log("ERROR", "Unexpected filesystem error: " + error);
            return Result.Err("Unknown configuration error");
        }
    }
}
```

### Pattern 2: Early Return on Error

Use early returns to avoid deep nesting:

```asthra
import "stdlib/asthra/env";
import "stdlib/asthra/fs";

fn load_user_config() -> Result<string, string> {
    // Get home directory
    let home_result: Result<string, string> = env.get_env("HOME");
    let home_dir: string = match home_result {
        Result.Ok(dir) => dir,
        Result.Err(error) => return Result.Err("HOME environment variable not set: " + error)
    };
    
    // Build config path
    let config_path: string = home_dir + "/.config/app.conf";
    
    // Check if config exists
    if !fs.exists(config_path) {
        return Result.Err("Configuration file does not exist: " + config_path);
    }
    
    // Read config file
    let config_result: Result<string, fs.IOError> = fs.read_to_string(config_path);
    let config_content: string = match config_result {
        Result.Ok(content) => content,
        Result.Err(error) => return Result.Err("Failed to read configuration: " + error)
    };
    
    return Result.Ok(config_content);
}
```

### Pattern 3: Error Propagation

Chain operations and propagate errors up the call stack:

```asthra
import "stdlib/asthra/fs";
import "stdlib/string";

fn process_data_file(filename: string) -> Result<[]string, string> {
    // Read file
    let content_result: Result<string, fs.IOError> = fs.read_to_string(filename);
    let content: string = match content_result {
        Result.Ok(data) => data,
        Result.Err(error) => return Result.Err("File read failed: " + error)
    };
    
    // Process content
    let trimmed: string = string.trim(content);
    if string.is_empty(trimmed) {
        return Result.Err("File is empty or contains only whitespace");
    }
    
    // Split into lines
    let lines: []string = string.lines(trimmed);
    if lines.len() == 0 {
        return Result.Err("No valid lines found in file");
    }
    
    return Result.Ok(lines);
}
```

### Pattern 4: Default Values

Provide fallback values for non-critical errors:

```asthra
import "stdlib/asthra/env";

fn get_config_value(key: string, default_value: string) -> string {
    let env_result: Result<string, string> = env.get_env(key);
    
    match env_result {
        Result.Ok(value) => {
            log("CONFIG", "Using " + key + " = " + value);
            return value;
        },
        Result.Err(_) => {
            log("CONFIG", "Using default " + key + " = " + default_value);
            return default_value;
        }
    }
}

fn setup_server() -> void {
    let port: string = get_config_value("PORT", "8080");
    let host: string = get_config_value("HOST", "localhost");
    let debug: string = get_config_value("DEBUG", "false");
    
    log("SERVER", "Starting server on " + host + ":" + port);
    log("SERVER", "Debug mode: " + debug);
}
```

### Pattern 5: Error Transformation

Convert between different error types:

```asthra
import "stdlib/asthra/fs";
import "stdlib/string";

enum AppError {
    ConfigError(string),
    DataError(string),
    SystemError(string)
}

fn load_and_parse_config() -> Result<Config, AppError> {
    // Load config file
    let content_result: Result<string, fs.IOError> = fs.read_to_string("app.conf");
    let content: string = match content_result {
        Result.Ok(data) => data,
        Result.Err(fs.IOError.NotFound(_)) => {
            return Result.Err(AppError.ConfigError("Configuration file not found"));
        },
        Result.Err(error) => {
            return Result.Err(AppError.SystemError("File system error: " + error));
        }
    };
    
    // Parse config
    let config_result: Result<Config, ParseError> = parse_config(content);
    let config: Config = match config_result {
        Result.Ok(cfg) => cfg,
        Result.Err(error) => {
            return Result.Err(AppError.DataError("Invalid configuration: " + error));
        }
    };
    
    return Result.Ok(config);
}
```

## Best Practices

### 1. Always Handle Errors

Never ignore Result types - always match on them:

```asthra
// ❌ WRONG - Ignoring potential errors
let content = fs.read_to_string("file.txt");  // This won't compile!

// ✅ CORRECT - Explicit error handling
let content_result = fs.read_to_string("file.txt");
match content_result {
    Result.Ok(content) => {
        // Use content
    },
    Result.Err(error) => {
        // Handle error
    }
}
```

### 2. Use Descriptive Error Messages

Provide context about what went wrong and why:

```asthra
// ❌ POOR - Generic error message
return Result.Err("Error");

// ✅ GOOD - Descriptive error message
return Result.Err("Failed to parse configuration file 'app.conf': invalid JSON syntax at line 15");
```

### 3. Log Errors Appropriately

Log errors at the appropriate level:

```asthra
match fs.read_to_string("optional_config.txt") {
    Result.Ok(content) => {
        log("INFO", "Optional configuration loaded");
        process_config(content);
    },
    Result.Err(fs.IOError.NotFound(_)) => {
        log("INFO", "Optional configuration not found, using defaults");
    },
    Result.Err(error) => {
        log("WARNING", "Failed to read optional configuration: " + error);
    }
}
```

### 4. Fail Fast for Critical Errors

Use panic for unrecoverable errors:

```asthra
import "stdlib/asthra/panic";

fn initialize_critical_system() -> Result<void, string> {
    let result = setup_critical_component();
    match result {
        Result.Ok(_) => return Result.Ok(void),
        Result.Err(error) => {
            // Critical system failure - cannot continue
            panic.panic("Critical system initialization failed: " + error);
        }
    }
}
```

### 5. Provide Recovery Mechanisms

When possible, offer ways to recover from errors:

```asthra
fn robust_file_operation(primary_path: string, backup_path: string) -> Result<string, string> {
    // Try primary file first
    let primary_result = fs.read_to_string(primary_path);
    match primary_result {
        Result.Ok(content) => {
            log("INFO", "Loaded from primary file: " + primary_path);
            return Result.Ok(content);
        },
        Result.Err(error) => {
            log("WARNING", "Primary file failed: " + error + ", trying backup");
        }
    }
    
    // Try backup file
    let backup_result = fs.read_to_string(backup_path);
    match backup_result {
        Result.Ok(content) => {
            log("INFO", "Loaded from backup file: " + backup_path);
            return Result.Ok(content);
        },
        Result.Err(error) => {
            return Result.Err("Both primary and backup files failed: " + error);
        }
    }
}
```

## Error Handling Anti-Patterns

### Don't Ignore Errors

```asthra
// ❌ WRONG - This won't compile
let content = fs.read_to_string("file.txt");

// ❌ WRONG - Ignoring errors with unwrap-like behavior
let content = fs.read_to_string("file.txt").unwrap();  // Not available in Asthra
```

### Don't Use Generic Error Types

```asthra
// ❌ POOR - Generic string errors lose information
fn process_file() -> Result<Data, string> {
    // ...
}

// ✅ BETTER - Specific error types
enum ProcessError {
    FileNotFound(string),
    ParseError(string),
    ValidationError(string)
}

fn process_file() -> Result<Data, ProcessError> {
    // ...
}
```

### Don't Swallow Errors Silently

```asthra
// ❌ WRONG - Silent failure
match fs.read_to_string("config.txt") {
    Result.Ok(content) => process_config(content),
    Result.Err(_) => {
        // Silent failure - no logging or handling
    }
}

// ✅ CORRECT - Explicit handling
match fs.read_to_string("config.txt") {
    Result.Ok(content) => process_config(content),
    Result.Err(error) => {
        log("ERROR", "Failed to load configuration: " + error);
        use_default_config();
    }
}
```

## Testing Error Conditions

Test both success and error paths:

```asthra
fn test_file_operations() -> void {
    // Test success case
    let success_result = fs.write_string("test.txt", "test content");
    panic.assert(success_result.is_ok(), "File write should succeed");
    
    let read_result = fs.read_to_string("test.txt");
    match read_result {
        Result.Ok(content) => {
            panic.assert_eq_string(content, "test content", "Content should match");
        },
        Result.Err(error) => {
            panic.panic("File read should succeed: " + error);
        }
    }
    
    // Test error case
    let error_result = fs.read_to_string("nonexistent.txt");
    match error_result {
        Result.Ok(_) => {
            panic.panic("Reading nonexistent file should fail");
        },
        Result.Err(fs.IOError.NotFound(_)) => {
            // Expected error
        },
        Result.Err(error) => {
            panic.panic("Unexpected error type: " + error);
        }
    }
}
```

## Summary

- **Always handle Result types** - never ignore potential errors
- **Use pattern matching** to handle different error cases appropriately
- **Provide descriptive error messages** with context
- **Log errors at appropriate levels** (INFO, WARNING, ERROR)
- **Fail fast for critical errors** using panic
- **Offer recovery mechanisms** when possible
- **Test both success and error paths** in your code
- **Use specific error types** rather than generic strings when possible 
