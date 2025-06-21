# Best Practices

## Overview

This guide consolidates best practices for using the Asthra Standard Library effectively, safely, and efficiently. These patterns have been proven in production use and help avoid common pitfalls.

## Error Handling

### Always Handle Results

Never ignore `Result&lt;T, E&gt;` types - the compiler will enforce this:

```asthra
// ✅ CORRECT - Explicit error handling
let file_result: Result&lt;string, fs.IOError&gt; = fs.read_to_string("config.txt");
match file_result {
    Result.Ok(content) =&gt; {
        // Process content
    },
    Result.Err(error) =&gt; {
        // Handle error appropriately
    }
}

// ❌ WRONG - This won't compile
let content = fs.read_to_string("config.txt");  // Compiler error!
```

### Use Early Returns for Error Propagation

Avoid deep nesting by returning errors early:

```asthra
fn load_and_process_config() -&gt; Result&lt;Config, string&gt; {
    // Early return pattern
    let content: string = match fs.read_to_string("config.json") {
        Result.Ok(data) =&gt; data,
        Result.Err(error) =&gt; return Result.Err("Failed to read config: " + error)
    };
    
    let trimmed: string = string.trim(content);
    if string.is_empty(trimmed) {
        return Result.Err("Configuration file is empty");
    }
    
    let config: Config = match parse_json(trimmed) {
        Result.Ok(cfg) =&gt; cfg,
        Result.Err(error) =&gt; return Result.Err("Invalid JSON: " + error)
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

fn memory_zone_selection() -&gt; void {
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
    fn open(path: string) -&gt; Result&lt;FileHandle, string&gt; {
        let file_result: Result&lt;fs.File, fs.IOError&gt; = fs.open(path, fs.OpenMode.Read);
        match file_result {
            Result.Ok(file) =&gt; {
                return Result.Ok(FileHandle {
                    file: file,
                    path: path
                });
            },
            Result.Err(error) =&gt; {
                return Result.Err("Failed to open " + path + ": " + error);
            }
        }
    }
    
    fn drop(&amp;mut self) -&gt; void {
        // Automatic cleanup when FileHandle goes out of scope
        log("CLEANUP", "Closing file: " + self.path);
        // File is automatically closed by the runtime
    }
}
```

### Monitor Memory Usage

Use GC statistics to understand memory patterns:

```asthra
fn monitor_memory_usage() -&gt; void {
    let stats: core.GCStats = core.gc_stats();
    
    log("MEMORY", "Current usage: " + stats.current_usage + " bytes");
    log("MEMORY", "GC cycles: " + stats.gc_cycles);
    
    // Trigger GC if memory usage is high
    if stats.current_usage &gt; 100_000_000 {  // 100MB
        log("MEMORY", "High memory usage, triggering GC");
        core.gc_collect();
    }
}
```

## String Processing

### Validate UTF-8 from External Sources

Always validate UTF-8 when processing external data:

```asthra
fn process_external_text(data: &#91;&#93;u8) -&gt; Result&lt;string, string&gt; {
    // Try strict validation first
    let strict_result: Result&lt;string, string.StringError&gt; = string.from_utf8(data);
    
    match strict_result {
        Result.Ok(text) =&gt; {
            return Result.Ok(text);
        },
        Result.Err(string.StringError.InvalidUtf8(msg)) =&gt; {
            log("WARNING", "Invalid UTF-8 detected: " + msg);
            // Fall back to lossy conversion
            let lossy_text: string = string.from_utf8_lossy(data);
            return Result.Ok(lossy_text);
        },
        Result.Err(error) =&gt; {
            return Result.Err("String processing failed: " + error);
        }
    }
}
```

### Use Appropriate String Functions

Choose the right function for your needs:

```asthra
fn efficient_string_operations(text: string) -&gt; void {
    // Use len() for byte length (faster)
    let byte_length: usize = string.len(text);
    
    // Use char_count() for Unicode character count
    let char_length: usize = string.char_count(text);
    
    // Use contains() for simple existence checks
    if string.contains(text, "error") {
        log("ERROR", "Error found in text");
    }
    
    // Use find() when you need the position
    let pos_result: Result&lt;usize, string&gt; = string.find(text, "error");
    match pos_result {
        Result.Ok(pos) =&gt; log("POSITION", "Error at position: " + pos),
        Result.Err(_) =&gt; log("INFO", "No error found")
    }
}
```

### Minimize String Allocations

Reduce memory allocations in performance-critical code:

```asthra
fn efficient_text_processing(lines: &#91;&#93;string) -&gt; string {
    // Collect non-empty lines first
    let mut valid_lines: &#91;&#93;string = &#91;&#93;;
    
    let mut i: usize = 0;
    while i &lt; lines.len() {
        let trimmed: string = string.trim(lines&#91;i&#93;);
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

fn collection_selection() -&gt; void {
    // Vec - for indexed access and iteration
    let mut list: collections.Vec<string> = collections.vec_new();
    collections.vec_push(&amp;mut list, "item");
    
    // HashMap - for key-value lookups
    let mut cache: collections.HashMap&lt;string, i32&gt; = collections.hashmap_new();
    collections.hashmap_insert(&amp;mut cache, "key", 42);
    
    // LinkedList - for frequent insertions/deletions at ends
    let mut queue: collections.LinkedList<string> = collections.list_new();
    collections.list_push_back(&amp;mut queue, "task");
}
```

### Handle Collection Errors

Always check bounds and handle collection errors:

```asthra
fn safe_collection_access(vec: *collections.Vec<i32>, index: usize) -&gt; Result&lt;i32, string&gt; {
    let element_result: Result&lt;*i32, string&gt; = collections.vec_get(vec, index);
    
    match element_result {
        Result.Ok(element_ptr) =&gt; {
            unsafe {
                let value: i32 = *element_ptr;
                return Result.Ok(value);
            }
        },
        Result.Err(error) =&gt; {
            return Result.Err("Index " + index + " out of bounds: " + error);
        }
    }
}
```

### Use Iterators When Available

Prefer functional approaches for data processing:

```asthra
fn process_numbers(numbers: &#91;&#93;i32) -&gt; &#91;&#93;i32 {
    let mut results: &#91;&#93;i32 = &#91;&#93;;
    
    let mut i: usize = 0;
    while i &lt; numbers.len() {
        let num: i32 = numbers&#91;i&#93;;
        
        // Filter and transform
        if num &gt; 0 {
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
fn robust_file_operations(filename: string) -&gt; Result&lt;string, string&gt; {
    let file_result: Result&lt;string, fs.IOError&gt; = fs.read_to_string(filename);
    
    match file_result {
        Result.Ok(content) =&gt; {
            return Result.Ok(content);
        },
        Result.Err(fs.IOError.NotFound(_)) =&gt; {
            log("INFO", "File not found, creating default: " + filename);
            let default_content: string = "# Default configuration\n";
            
            let write_result: Result&lt;void, fs.IOError&gt; = fs.write_string(filename, default_content);
            match write_result {
                Result.Ok(_) =&gt; return Result.Ok(default_content),
                Result.Err(error) =&gt; return Result.Err("Failed to create default file: " + error)
            }
        },
        Result.Err(fs.IOError.PermissionDenied(_)) =&gt; {
            return Result.Err("Permission denied accessing: " + filename);
        },
        Result.Err(error) =&gt; {
            return Result.Err("File operation failed: " + error);
        }
    }
}
```

### Check File Existence Before Operations

Avoid unnecessary errors by checking file existence:

```asthra
fn conditional_file_operations(filename: string) -&gt; Result&lt;void, string&gt; {
    if !fs.exists(filename) {
        log("INFO", "File does not exist: " + filename);
        return Result.Ok(void);
    }
    
    if !fs.is_file(filename) {
        return Result.Err("Path is not a file: " + filename);
    }
    
    // Proceed with file operations
    let content_result: Result&lt;string, fs.IOError&gt; = fs.read_to_string(filename);
    match content_result {
        Result.Ok(content) =&gt; {
            log("SUCCESS", "Read " + content.len(&#125; bytes from " + filename);
            return Result.Ok(void);
        },
        Result.Err(error) =&gt; {
            return Result.Err("Failed to read file: " + error);
        }
    }
}
```

### Use Path Utilities for Cross-Platform Compatibility

Build paths using the path utilities:

```asthra
fn build_config_path() -&gt; string {
    let home_result: Result&lt;string, string&gt; = env.home_dir();
    let home: string = match home_result {
        Result.Ok(dir) =&gt; dir,
        Result.Err(_) =&gt; "/tmp"  // Fallback
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

fn operation_with_timeout() -&gt; Result&lt;void, string&gt; {
    let timeout: time.Duration = time.duration_from_secs(30);
    let deadline: time.Deadline = time.deadline_from_duration(timeout);
    
    while !time.deadline_is_expired(deadline) {
        let work_result: Result&lt;void, string&gt; = do_some_work();
        
        match work_result {
            Result.Ok(_) =&gt; {
                log("SUCCESS", "Operation completed");
                return Result.Ok(void);
            },
            Result.Err(error) =&gt; {
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
fn timed_operation() -&gt; Result&lt;void, string&gt; {
    let start: time.Instant = time.instant_now();
    
    // Perform operation
    let result: Result&lt;void, string&gt; = expensive_operation();
    
    let elapsed: time.Duration = time.instant_elapsed(start);
    let elapsed_ms: u64 = time.duration_as_millis(elapsed);
    
    log("PERF", "Operation took " + elapsed_ms + "ms");
    
    if elapsed_ms &gt; 1000 {  // More than 1 second
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

fn validate_data(data: &#91;&#93;i32) -&gt; void {
    // Debug assertions (only active in debug builds)
    panic.debug_assert(data.len() &gt; 0, "Data array cannot be empty");
    panic.debug_assert(data&#91;0&#93; &gt;= 0, "First element must be non-negative");
    
    // Production assertions (always active)
    panic.assert(data.len() &lt;= 1000, "Data array too large");
}
```

### Log at Appropriate Levels

Use consistent logging levels:

```asthra
fn comprehensive_logging() -&gt; Result&lt;void, string&gt; {
    log("DEBUG", "Starting operation");
    
    let result: Result&lt;string, fs.IOError&gt; = fs.read_to_string("data.txt");
    
    match result {
        Result.Ok(content) =&gt; {
            log("INFO", "Successfully loaded data file");
            log("DEBUG", "Data size: " + content.len(&#125; bytes");
            return Result.Ok(void);
        },
        Result.Err(fs.IOError.NotFound(_)) =&gt; {
            log("WARNING", "Data file not found, using defaults");
            return Result.Ok(void);
        },
        Result.Err(error) =&gt; {
            log("ERROR", "Failed to load data file: " + error);
            return Result.Err("Data loading failed");
        }
    }
}
```

### Test Error Paths

Test both success and failure scenarios:

```asthra
fn test_file_operations() -&gt; void {
    // Test success case
    let write_result: Result&lt;void, fs.IOError&gt; = fs.write_string("test.txt", "test data");
    panic.assert(write_result.is_ok(), "File write should succeed");
    
    let read_result: Result&lt;string, fs.IOError&gt; = fs.read_to_string("test.txt");
    match read_result {
        Result.Ok(content) =&gt; {
            panic.assert_eq_string(content, "test data", "Content should match");
        },
        Result.Err(error) =&gt; {
            panic.panic("File read should succeed: " + error);
        }
    }
    
    // Test error case
    let error_result: Result&lt;string, fs.IOError&gt; = fs.read_to_string("nonexistent.txt");
    match error_result {
        Result.Ok(_) =&gt; {
            panic.panic("Reading nonexistent file should fail");
        },
        Result.Err(fs.IOError.NotFound(_)) =&gt; {
            // Expected error
        },
        Result.Err(error) =&gt; {
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
fn validate_user_input(input: string) -&gt; Result&lt;string, string&gt; {
    // Check length
    if string.len(input) &gt; 1000 {
        return Result.Err("Input too long (max 1000 bytes)");
    }
    
    // Check for valid UTF-8
    let bytes: &#91;&#93;u8 = string.to_utf8(input);
    let validation_result: Result&lt;string, string.StringError&gt; = string.from_utf8(bytes);
    match validation_result {
        Result.Ok(_) =&gt; {},
        Result.Err(_) =&gt; return Result.Err("Invalid UTF-8 in input")
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

fn generate_session_token() -&gt; Result&lt;string, string&gt; {
    let mut token_bytes: &#91;32&#93;u8;  // 256-bit token
    
    let random_result: Result&lt;void, string&gt; = random.secure_random_bytes(token_bytes);
    match random_result {
        Result.Ok(_) =&gt; {
            // Convert to hex string
            let token: string = bytes_to_hex(token_bytes);
            log("SECURITY", "Generated secure session token");
            return Result.Ok(token);
        },
        Result.Err(error) =&gt; {
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
