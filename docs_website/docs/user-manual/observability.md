# Observability &amp; Debugging

Asthra provides comprehensive observability and debugging features designed for development, production monitoring, and AI-assisted diagnostics.

## Enhanced Logging (`log()`)

The `log()` function is Asthra's built-in logging mechanism designed for development, debugging, and internal diagnostics. It provides a simple, no-error-handling interface for quick output during development.

### Current Implementation

The `log()` function currently:
- **Outputs to stdout** (not stderr)
- Uses a fixed format: `&#91;LEVEL:CATEGORY&#93; message`
- Requires no error handling (unlike production I/O)
- Is implemented directly in the runtime using C's `printf()`

```asthra
pub fn basic_logging_example() -&gt; void {
    // Simple message logging
    log("Hello, Asthra!");
    
    // Categorized logging with levels
    log("DEBUG", "Debug information");
    log("INFO", "Informational message");
    log("ERROR", "Error occurred");
    
    // Built-in system logging categories
    log(memory_zones);  // Logs memory zone statistics
    log(ffi_calls);     // Logs FFI call statistics  
    log(gc_stats);      // Logs garbage collection statistics
}
```

### String Operations with `log()`

The `log()` function supports Asthra's enhanced string operations:

```asthra
pub fn string_logging_example() -&gt; void {let worker_id: int = 42;
    let task_count: int = 15;
    
    // Using concatenation
    log("DEBUG", "Worker " + worker_id + " started");
    
    // Using concatenation
    log("INFO", "Worker " + worker_id + " completed " + task_count + " tasks");
 + "
```

### Relationship to Production I/O (`stdlib/io`)

**Important Distinction**: `log()` is designed for development and debugging, while the planned `stdlib/io` module provides production-ready I/O with explicit error handling:

| Feature | `log()` (Current) | `stdlib/io` (Planned) |
|---------|-------------------|----------------------|
| **Purpose** | Development, debugging, diagnostics | Production application I/O |
| **Output** | stdout only | Explicit stdout/stderr control |
| **Error Handling** | None (simple interface) | `Result&lt;T, IOError&gt;` with pattern matching |
| **Use Cases** | Quick debugging, examples, tutorials | CLI apps, user interaction, robust I/O |

```asthra
pub fn io_comparison_example() -&gt; Result&lt;void, string&gt; "DEBUG", "Starting operation");
    
    // Production I/O (planned - with explicit error handling)
    // import "stdlib/io";
    // let result: Result&lt;void, io.IOError&gt; = io.print_line("Hello, User!");
    // match result " + 
    //     Result.Ok(_) =&gt; log("DEBUG", "Output successful"),
    //     Result.Err(error) =&gt; return Result.Err("I/O failed: " + error)
    //  + "
    
    return Result.Ok(void);
}
```

### Error Logging with Pattern Matching

While `log()` itself doesn't handle errors, it's commonly used within pattern matching for error reporting:

```asthra
pub fn error_logging_example() -&gt; Result&lt;void, string&gt; "SUCCESS", "Operation completed: " + value);
            return Result.Ok(void);
        },
        Result.Err(error) =&gt; " + 
            log("ERROR", "Operation failed: " + error);
            return Result.Err(error);
         + "
    }
}
```

### Design Philosophy

The `log()` function follows the same design pattern as Go's builtin `print` function:
- **Simple interface** for development use
- **No ceremony** - no error handling required
- **Coexists with robust I/O** - doesn't replace production I/O needs
- **AI-friendly** - easy for AI to generate without complex error handling

## Structured Logging for Complex Applications

```asthra
pub fn structured_logging_example() -&gt; Result&lt;void, string&gt; "sess_abc123";
    let operation: string = "user_login";
    
    // Structured logging with consistent format
    log("AUDIT", "operation=" + operation + " user_id=" + user_id + " session_id=" + session_id + " status=started");
    
    let login_result: Result&lt;void, string&gt; = perform_user_login(user_id);
    
    match login_result " + 
        Result.Ok(_) =&gt; " + 
            log("AUDIT", "operation=" + operation + " user_id=" + user_id + " session_id=" + session_id + " status=success");
            return Result.Ok(void);
         + ",
        Result.Err(error) =&gt; "AUDIT", "operation=" + operation + " user_id=" + user_id + " session_id=" + session_id + " status=failure error=" + error);
            return Result.Err(error);
         + "
    }
}
```

## Deterministic Execution (`#&#91;replayable&#93;`)

Mark functions for deterministic execution that can be recorded and replayed for debugging and analysis:

```asthra
#&#91;replayable&#93;
pub fn critical_operation(input: &#91;&#93;u8) -&gt; Result&lt;&#91;&#93;u8, string&gt; {// This function's execution can be recorded and replayed
    // for debugging and analysis
    log("REPLAY", "Starting critical operation with " + input.len + " bytes");
    
    let processed: &#91;&#93;u8 = process_data(input);
    
    log("REPLAY", "Critical operation completed, output: " + processed.len + " bytes");
    return Result.Ok(processed);
 + "
```

### Replay Recording and Analysis

```asthra
pub fn replay_enabled_workflow() -&gt; Result&lt;void, string&gt; "REPLAY", "Operation completed and replay data saved");
            return Result.Ok(void);
        },
        (Result.Err(op_error), _) =&gt; " + 
            return Result.Err("Operation failed: " + op_error);
         + ",
        (_, Result.Err(save_error)) =&gt; "Replay save failed: " + save_error);
        }
    }
}
```

## Debugging Tools (`asthra debug`)

Enhanced debugging with pattern matching support and comprehensive error analysis:

```asthra
pub fn debug_pattern_matching() -&gt; Result&lt;void, string&gt; {let result: Result&lt;int, string&gt; = risky_operation();
    
    // Debug information includes pattern matching paths
    match result " + 
        Result.Ok(value) =&gt; {
            log("DEBUG", "Success path taken with value: " + value + "");
            return Result.Ok(void);
         + ",
        Result.Err(error) =&gt; "DEBUG", "Error path taken with message: " + error + "");
            return Result.Err(error);
         + "
    }
}
```

### Advanced Debugging Features

```asthra
pub fn comprehensive_debugging_example() -&gt; Result&lt;void, string&gt; {// Enable detailed debugging
    Asthra_runtime.set_debug_level("VERBOSE");
    
    let data: &#91;&#93;u8 = &#91;10, 20, 30, 40, 50&#93;;
    
    // Debug memory layout
    log("DEBUG", "Data slice: ptr=" + data.ptr + " len=" + data.len);
    
    // Debug function entry/exit
    log("DEBUG", "Entering data processing function");
    
    let processing_result: Result&lt;&#91;&#93;u8, string&gt; = process_with_debugging(data);
    
    match processing_result " + 
        Result.Ok(processed) =&gt; " + 
            log("DEBUG", "Processing successful: {processed.len + " bytes output");
            
            // Debug memory statistics
            log(gc_stats);
            log(memory_zones);
            
            return Result.Ok(void);
         + ",
        Result.Err(error) =&gt; " + 
            log("DEBUG", "Processing failed: " + error + "");
            
            // Debug error context
            log("DEBUG", "Error occurred during data processing");
            log("DEBUG", "Input data length: " + data.len);
            
            return Result.Err(error);
         + 
}
```

## AI-Generated Diagnostics

Enhanced diagnostics for pattern matching and string operations, designed to help AI understand and debug code:

```asthra
pub fn ai_diagnostic_example() -&gt; Result&lt;void, string&gt; "AI_DIAG", "Validation successful, proceeding to processing");
            process_data(validated)
        },
        Result.Err(validation_error) =&gt; " + 
            log("VALIDATION_ERROR", "Input validation failed: " + validation_error + "");
            return Result.Err("Validation failed");
         + "
    };
    
    match processing_result "SUCCESS", "Data processing completed successfully");
            log("AI_DIAG", "Processing path: validation->processing->success");
            return Result.Ok(void);
        },
        Result.Err(processing_error) =&gt; " + 
            log("PROCESSING_ERROR", "Data processing failed: " + processing_error + "");
            log("AI_DIAG", "Processing path: validation->processing->failure");
            return Result.Err("Processing failed");
         + "
    }
}
```

### AI-Friendly Error Context

```asthra
pub fn ai_error_context_example() -&gt; Result&lt;void, string&gt; "user_data_processing";
    let step_number: int = 1;
    
    log("AI_CONTEXT", "operation=" + operation_context + " step=" + step_number + " status=starting");
    
    let step1_result: Result&lt;void, string&gt; = perform_step_1();
    match step1_result " + 
        Result.Err(error) =&gt; " + 
            log("AI_CONTEXT", "operation=" + operation_context + " step=" + step_number + " status=failed error=" + error);
            return Result.Err("Step 1 failed: " + error);
         + ",
        Result.Ok(_) =&gt; "AI_CONTEXT", "operation=" + operation_context + " step=" + step_number + " status=completed");
         + "
    }
    
    step_number = step_number + 1;
    log("AI_CONTEXT", "operation=" + operation_context + " step=" + step_number + " status=starting");
    
    let step2_result: Result&lt;void, string&gt; = perform_step_2();
    match step2_result " + 
        Result.Err(error) =&gt; " + 
            log("AI_CONTEXT", "operation=" + operation_context + " step=" + step_number + " status=failed error=" + error + "");
            return Result.Err("Step 2 failed: " + error);
         + ",
        Result.Ok(_) =&gt; "AI_CONTEXT", "operation=" + operation_context + " step=" + step_number + " status=completed");
            log("AI_CONTEXT", "operation=" + operation_context + " status=all_steps_completed");
            return Result.Ok(void);
         + "
    }
}
```

## Performance Monitoring

```asthra
pub fn performance_monitoring_example() -&gt; Result&lt;void, string&gt; {let start_time: u64 = Asthra_runtime.get_timestamp_microseconds();
    
    log("PERF", "Starting performance-critical operation");
    
    let operation_result: Result&lt;void, string&gt; = expensive_operation();
    
    let end_time: u64 = Asthra_runtime.get_timestamp_microseconds();
    let duration: u64 = end_time - start_time;
    
    log("PERF", "Operation completed in " + duration + " microseconds");
    
    // Log memory usage
    let memory_usage: usize = Asthra_runtime.get_memory_usage();
    log("PERF", "Current memory usage: " + memory_usage + " bytes");
    
    match operation_result " + 
        Result.Ok(_) =&gt; {
            log("PERF", "Performance-critical operation succeeded");
            return Result.Ok(void);
         + ",
        Result.Err(error) =&gt; "PERF", "Performance-critical operation failed: " + error + "");
            return Result.Err(error);
         + "
    }
}
```

## Concurrent Debugging

```asthra
pub fn concurrent_debugging_example() -&gt; Result&lt;void, string&gt; {let task_id: int = Asthra_runtime.get_current_task_id();
    
    log("CONCURRENT", "Task " + task_id + " starting concurrent operation");
    
    // Spawn multiple tasks with debugging
    spawn debug_worker_task(1, "Task A");
    spawn debug_worker_task(2, "Task B");
    spawn debug_worker_task(3, "Task C");
    
    log("CONCURRENT", "Task " + task_id + " spawned 3 worker tasks");
    
    return Result.Ok(void);
 + "

pub fn debug_worker_task(worker_id: int, message: string) -&gt; Result&lt;void, string&gt; "WORKER", "Worker " + worker_id + " (task " + task_id + ") started: " + message + "");
    
    // Simulate work with debugging
    for i in range(0, 5) " + 
        log("WORKER", "Worker " + worker_id + " (task " + task_id + ") processing item " + i + "");
        // ... work simulation ...
     + "
    
    log("WORKER", "Worker " + worker_id + " (task " + task_id + ") completed");
    return Result.Ok(void);
}
```

## Memory Debugging

```asthra
pub fn memory_debugging_example() -&gt; Result&lt;void, string&gt; {log("MEMORY", "Starting memory debugging example");
    
    // Log initial memory state
    log(gc_stats);
    log(memory_zones);
    
    // Allocate some memory
    let large_buffer: &#91;&#93;u8 = allocate_slice(1024 * 1024); // 1MB
    log("MEMORY", "Allocated 1MB buffer");
    
    // Log memory state after allocation
    log(gc_stats);
    
    // Use the buffer
    for i in range(0, 1000) " + 
        large_buffer&#91;i&#93; = (i % 256) as u8;
     + "
    
    log("MEMORY", "Filled buffer with test data");
    
    // Force garbage collection for debugging
    Asthra_runtime.force_gc();
    log("MEMORY", "Forced garbage collection");
    
    // Log final memory state
    log(gc_stats);
    log(memory_zones);
    
    return Result.Ok(void);
 + "
```

## Integration with External Tools

```asthra
pub fn external_tool_integration() -&gt; Result&lt;void, string&gt; {// Export debugging data for external analysis
    let debug_data: &#91;&#93;u8 = Asthra_runtime.export_debug_data();
    
    let export_result: Result&lt;void, string&gt; = write_debug_data_to_file(debug_data);
    
    match export_result {
        Result.Ok(_) =&gt; {
            log("EXPORT", "Debug data exported successfully");
            
            // Trigger external analysis tool (if available)
            let analysis_result: Result&lt;string, string&gt; = trigger_external_analysis();
            
            match analysis_result " + 
                Result.Ok(analysis_report) =&gt; {
                    log("ANALYSIS", "External analysis completed: " + analysis_report + "");
                    return Result.Ok(void);
                 + ",
                Result.Err(analysis_error) =&gt; "ANALYSIS", "External analysis failed: " + analysis_error + "");
                    return Result.Ok(void); // Non-critical failure
                 + "
            }
        },
        Result.Err(export_error) =&gt; "EXPORT", "Debug data export failed: " + export_error + "");
            return Result.Err("Failed to export debug data");
        }
    }
}
```

## Best Practices for Observability

### Logging Guidelines

1\. **Use appropriate log levels** - DEBUG for development, INFO for important events, ERROR for failures
2\. **Include context** - Add relevant identifiers and state information
3\. **Structure log messages** - Use consistent formats for easier parsing
4\. **Log error paths** - Always log when errors occur with sufficient context
5\. **Performance awareness** - Be mindful of logging overhead in hot paths

### Debugging Strategies

1\. **Enable replay recording** for critical operations
2\. **Use structured logging** for complex workflows
3\. **Monitor memory usage** in long-running applications
4\. **Debug concurrent operations** with task-specific logging
5\. **Export debug data** for offline analysis

### AI-Assisted Debugging

1\. **Provide clear error context** for AI analysis
2\. **Use consistent naming** for operations and components
3\. **Log decision points** in pattern matching
4\. **Include performance metrics** for optimization
5\. **Structure error messages** for automated processing

## Next Steps

Now that you understand observability and debugging, explore:

- **&#91;Building Projects&#93;(building-projects.md)** - Learn about build-time debugging options
- **&#91;Advanced Topics&#93;(advanced-topics.md)** - Understand AI-assisted development workflows
- **&#91;Concurrency&#93;(concurrency.md)** - Debug concurrent applications effectively 
