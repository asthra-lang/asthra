# Best Practices

## Overview

This guide consolidates best practices for using the Asthra Standard Library effectively, safely, and efficiently. These patterns have been proven in production use and help avoid common pitfalls.

## Error Handling

### Always Handle Results

Never ignore `Result<T, E>` types - the compiler will enforce this:

```asthra
// ✅ CORRECT - Explicit error handling
let file_result: Result<string, fs.IOError> = fs.read_to_string("config.txt");
match file_result {
    Result.Ok(content) => {
        // Process content
    },
    Result.Err(error) => {
        // Handle error appropriately
    }
}

// ❌ WRONG - This won't compile
let content = fs.read_to_string("config.txt");  // Compiler error!
```

### Use Early Returns for Error Propagation

Avoid deep nesting by returning errors early:

```asthra
fn load_and_process_config() -> Result<Config, string> {
    // Early return pattern
    let content: string = match fs.read_to_string("config.json") {
        Result.Ok(data) => data,
        Result.Err(error) => return Result.Err("Failed to read config: " + error)
    };
    
    let trimmed: string = string.trim(content);
    if string.is_empty(trimmed) {
        return Result.Err("Configuration file is empty");
    }
    
    let config: Config = match parse_json(trimmed) {
        Result.Ok(cfg) => cfg,
        Result.Err(error) => return Result.Err("Invalid JSON: " + error)
    };
    
    return Result.Ok(config);
}
```

### Provide Context in Error Messages

Include relevant information in error messages:

```asthra
// ❌ POOR - Generic error
return Result.Err("File error");

// ✅ GOOD - Specific context
return Result.Err("Failed to read configuration file 'app.conf': " + error);

// ✅ BETTER - Include recovery suggestions
return Result.Err("Configuration file 'app.conf' not found. Please create it or run with --generate-config");
```

## Memory Management

### Choose the Right Memory Zone

Select memory zones based on your use case:

```asthra
import "stdlib/asthra/core";

fn memory_zone_selection() -> void {
    // GC zone - for most allocations (default)
    let user_data: *mut void = core.alloc(1024, core.MemoryZone.GC).unwrap();
    // Automatically managed, can be moved by GC
    
    // Manual zone - for precise control
    let buffer: *mut void = core.alloc(1024, core.MemoryZone.Manual).unwrap();
    // Must be freed manually - use for performance-critical code
    core.free(buffer, core.MemoryZone.Manual);
    
    // Pinned zone - for FFI safety
    let ffi_buffer: *mut void = core.alloc(1024, core.MemoryZone.Pinned).unwrap();
    // Won't be moved by GC, safe for C interop
    
    // Stack zone - for temporary data
    let temp_data: *mut void = core.alloc(256, core.MemoryZone.Stack).unwrap();
    // Automatically freed when scope ends
}
```

### Use RAII for Resource Management

Implement automatic cleanup with destructors:

```asthra
struct FileHandle {
    file: fs.File,
    path: string
}

impl FileHandle {
    fn open(path: string) -> Result<FileHandle, string> {
        let file_result: Result<fs.File, fs.IOError> = fs.open(path, fs.OpenMode.Read);
        match file_result {
            Result.Ok(file) => {
                return Result.Ok(FileHandle {
                    file: file,
                    path: path
                });
            },
            Result.Err(error) => {
                return Result.Err("Failed to open " + path + ": " + error);
            }
        }
    }
    
    fn drop(&mut self) -> void {
        // Automatic cleanup when FileHandle goes out of scope
        log("CLEANUP", "Closing file: " + self.path);
        // File is automatically closed by the runtime
    }
}
```

### Monitor Memory Usage

Use GC statistics to understand memory patterns:

```asthra
fn monitor_memory_usage() -> void {
    let stats: core.GCStats = core.gc_stats();
    
    log("MEMORY", "Current usage: " + stats.current_usage + " bytes");
    log("MEMORY", "GC cycles: " + stats.gc_cycles);
    
    // Trigger GC if memory usage is high
    if stats.current_usage > 100_000_000 {  // 100MB
        log("MEMORY", "High memory usage, triggering GC");
        core.gc_collect();
    }
}
```

## String Processing

### Validate UTF-8 from External Sources

Always validate UTF-8 when processing external data:

```asthra
fn process_external_text(data: []u8) -> Result<string, string> {
    // Try strict validation first
    let strict_result: Result<string, string.StringError> = string.from_utf8(data);
    
    match strict_result {
        Result.Ok(text) => {
            return Result.Ok(text);
        },
        Result.Err(string.StringError.InvalidUtf8(msg)) => {
            log("WARNING", "Invalid UTF-8 detected: " + msg);
            // Fall back to lossy conversion
            let lossy_text: string = string.from_utf8_lossy(data);
            return Result.Ok(lossy_text);
        },
        Result.Err(error) => {
            return Result.Err("String processing failed: " + error);
        }
    }
}
```

### Use Appropriate String Functions

Choose the right function for your needs:

```asthra
fn efficient_string_operations(text: string) -> void {
    // Use len() for byte length (faster)
    let byte_length: usize = string.len(text);
    
    // Use char_count() for Unicode character count
    let char_length: usize = string.char_count(text);
    
    // Use contains() for simple existence checks
    if string.contains(text, "error") {
        log("ERROR", "Error found in text");
    }
    
    // Use find() when you need the position
    let pos_result: Result<usize, string> = string.find(text, "error");
    match pos_result {
        Result.Ok(pos) => log("POSITION", "Error at position: " + pos),
        Result.Err(_) => log("INFO", "No error found")
    }
}
```

### Minimize String Allocations

Reduce memory allocations in performance-critical code:

```asthra
fn efficient_text_processing(lines: []string) -> string {
    // Collect non-empty lines first
    let mut valid_lines: []string = [];
    
    let mut i: usize = 0;
    while i < lines.len() {
        let trimmed: string = string.trim(lines[i]);
        if !string.is_empty(trimmed) {
            valid_lines.push(trimmed);
        }
        i = i + 1;
    }
    
    // Single allocation for final result
    return string.join(valid_lines, "\n");
}
```

## Collections

### Choose the Right Data Structure

Select collections based on access patterns:

```asthra
import "stdlib/asthra/collections";

fn collection_selection() -> void {
    // Vec - for indexed access and iteration
    let mut list: collections.Vec<string> = collections.vec_new();
    collections.vec_push(&mut list, "item");
    
    // HashMap - for key-value lookups
    let mut cache: collections.HashMap<string, i32> = collections.hashmap_new();
    collections.hashmap_insert(&mut cache, "key", 42);
    
    // LinkedList - for frequent insertions/deletions at ends
    let mut queue: collections.LinkedList<string> = collections.list_new();
    collections.list_push_back(&mut queue, "task");
}
```

### Handle Collection Errors

Always check bounds and handle collection errors:

```asthra
fn safe_collection_access(vec: *collections.Vec<i32>, index: usize) -> Result<i32, string> {
    let element_result: Result<*i32, string> = collections.vec_get(vec, index);
    
    match element_result {
        Result.Ok(element_ptr) => {
            unsafe {
                let value: i32 = *element_ptr;
                return Result.Ok(value);
            }
        },
        Result.Err(error) => {
            return Result.Err("Index " + index + " out of bounds: " + error);
        }
    }
}
```

### Use Iterators When Available

Prefer functional approaches for data processing:

```asthra
fn process_numbers(numbers: []i32) -> []i32 {
    let mut results: []i32 = [];
    
    let mut i: usize = 0;
    while i < numbers.len() {
        let num: i32 = numbers[i];
        
        // Filter and transform
        if num > 0 {
            results.push(num * 2);
        }
        i = i + 1;
    }
    
    return results;
}
```

## File System Operations

### Use Proper Error Handling for File Operations

Handle different types of file system errors appropriately:

```asthra
fn robust_file_operations(filename: string) -> Result<string, string> {
    let file_result: Result<string, fs.IOError> = fs.read_to_string(filename);
    
    match file_result {
        Result.Ok(content) => {
            return Result.Ok(content);
        },
        Result.Err(fs.IOError.NotFound(_)) => {
            log("INFO", "File not found, creating default: " + filename);
            let default_content: string = "# Default configuration\n";
            
            let write_result: Result<void, fs.IOError> = fs.write_string(filename, default_content);
            match write_result {
                Result.Ok(_) => return Result.Ok(default_content),
                Result.Err(error) => return Result.Err("Failed to create default file: " + error)
            }
        },
        Result.Err(fs.IOError.PermissionDenied(_)) => {
            return Result.Err("Permission denied accessing: " + filename);
        },
        Result.Err(error) => {
            return Result.Err("File operation failed: " + error);
        }
    }
}
```

### Check File Existence Before Operations

Avoid unnecessary errors by checking file existence:

```asthra
fn conditional_file_operations(filename: string) -> Result<void, string> {
    if !fs.exists(filename) {
        log("INFO", "File does not exist: " + filename);
        return Result.Ok(void);
    }
    
    if !fs.is_file(filename) {
        return Result.Err("Path is not a file: " + filename);
    }
    
    // Proceed with file operations
    let content_result: Result<string, fs.IOError> = fs.read_to_string(filename);
    match content_result {
        Result.Ok(content) => {
            log("SUCCESS", "Read " + content.len() + " bytes from " + filename);
            return Result.Ok(void);
        },
        Result.Err(error) => {
            return Result.Err("Failed to read file: " + error);
        }
    }
}
```

### Use Path Utilities for Cross-Platform Compatibility

Build paths using the path utilities:

```asthra
fn build_config_path() -> string {
    let home_result: Result<string, string> = env.home_dir();
    let home: string = match home_result {
        Result.Ok(dir) => dir,
        Result.Err(_) => "/tmp"  // Fallback
    };
    
    let home_path: fs.Path = fs.path_new(home);
    let config_path: fs.Path = fs.path_join(home_path, ".config");
    let app_config_path: fs.Path = fs.path_join(config_path, "app.conf");
    
    return fs.path_to_string(app_config_path);
}
```

## Concurrency and Performance

### Use Timeouts for Operations

Protect against hanging operations with timeouts:

```asthra
import "stdlib/datetime/time";

fn operation_with_timeout() -> Result<void, string> {
    let timeout: time.Duration = time.duration_from_secs(30);
    let deadline: time.Deadline = time.deadline_from_duration(timeout);
    
    while !time.deadline_is_expired(deadline) {
        let work_result: Result<void, string> = do_some_work();
        
        match work_result {
            Result.Ok(_) => {
                log("SUCCESS", "Operation completed");
                return Result.Ok(void);
            },
            Result.Err(error) => {
                log("WARNING", "Work failed, retrying: " + error);
                time.sleep_millis(100);  // Brief pause before retry
            }
        }
    }
    
    return Result.Err("Operation timed out after 30 seconds");
}
```

### Monitor Performance

Use timing to identify performance bottlenecks:

```asthra
fn timed_operation() -> Result<void, string> {
    let start: time.Instant = time.instant_now();
    
    // Perform operation
    let result: Result<void, string> = expensive_operation();
    
    let elapsed: time.Duration = time.instant_elapsed(start);
    let elapsed_ms: u64 = time.duration_as_millis(elapsed);
    
    log("PERF", "Operation took " + elapsed_ms + "ms");
    
    if elapsed_ms > 1000 {  // More than 1 second
        log("WARNING", "Operation was slow, consider optimization");
    }
    
    return result;
}
```

## Testing and Debugging

### Use Assertions for Development

Add assertions to catch bugs early:

```asthra
import "stdlib/asthra/panic";

fn validate_data(data: []i32) -> void {
    // Debug assertions (only active in debug builds)
    panic.debug_assert(data.len() > 0, "Data array cannot be empty");
    panic.debug_assert(data[0] >= 0, "First element must be non-negative");
    
    // Production assertions (always active)
    panic.assert(data.len() <= 1000, "Data array too large");
}
```

### Log at Appropriate Levels

Use consistent logging levels:

```asthra
fn comprehensive_logging() -> Result<void, string> {
    log("DEBUG", "Starting operation");
    
    let result: Result<string, fs.IOError> = fs.read_to_string("data.txt");
    
    match result {
        Result.Ok(content) => {
            log("INFO", "Successfully loaded data file");
            log("DEBUG", "Data size: " + content.len() + " bytes");
            return Result.Ok(void);
        },
        Result.Err(fs.IOError.NotFound(_)) => {
            log("WARNING", "Data file not found, using defaults");
            return Result.Ok(void);
        },
        Result.Err(error) => {
            log("ERROR", "Failed to load data file: " + error);
            return Result.Err("Data loading failed");
        }
    }
}
```

### Test Error Paths

Test both success and failure scenarios:

```asthra
fn test_file_operations() -> void {
    // Test success case
    let write_result: Result<void, fs.IOError> = fs.write_string("test.txt", "test data");
    panic.assert(write_result.is_ok(), "File write should succeed");
    
    let read_result: Result<string, fs.IOError> = fs.read_to_string("test.txt");
    match read_result {
        Result.Ok(content) => {
            panic.assert_eq_string(content, "test data", "Content should match");
        },
        Result.Err(error) => {
            panic.panic("File read should succeed: " + error);
        }
    }
    
    // Test error case
    let error_result: Result<string, fs.IOError> = fs.read_to_string("nonexistent.txt");
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
    
    log("TEST", "All file operation tests passed");
}
```

## Security Considerations

### Validate External Input

Always validate data from external sources:

```asthra
fn validate_user_input(input: string) -> Result<string, string> {
    // Check length
    if string.len(input) > 1000 {
        return Result.Err("Input too long (max 1000 bytes)");
    }
    
    // Check for valid UTF-8
    let bytes: []u8 = string.to_utf8(input);
    let validation_result: Result<string, string.StringError> = string.from_utf8(bytes);
    match validation_result {
        Result.Ok(_) => {},
        Result.Err(_) => return Result.Err("Invalid UTF-8 in input")
    }
    
    // Sanitize content
    let trimmed: string = string.trim(input);
    if string.is_empty(trimmed) {
        return Result.Err("Input cannot be empty");
    }
    
    return Result.Ok(trimmed);
}
```

### Use Secure Random for Cryptographic Operations

Use secure random generation for security-sensitive operations:

```asthra
import "stdlib/asthra/random";

fn generate_session_token() -> Result<string, string> {
    let mut token_bytes: [32]u8;  // 256-bit token
    
    let random_result: Result<void, string> = random.secure_random_bytes(token_bytes);
    match random_result {
        Result.Ok(_) => {
            // Convert to hex string
            let token: string = bytes_to_hex(token_bytes);
            log("SECURITY", "Generated secure session token");
            return Result.Ok(token);
        },
        Result.Err(error) => {
            return Result.Err("Failed to generate secure token: " + error);
        }
    }
}
```

## Summary

Following these best practices will help you:

- **Write robust code** with proper error handling
- **Manage memory efficiently** with appropriate zones
- **Process text safely** with UTF-8 validation
- **Use collections effectively** with proper error checking
- **Handle files reliably** with comprehensive error handling
- **Build performant applications** with timeouts and monitoring
- **Debug effectively** with assertions and logging
- **Maintain security** with input validation and secure randomness

These patterns have been proven in production Asthra applications and will help you avoid common pitfalls while building reliable, efficient software. 
