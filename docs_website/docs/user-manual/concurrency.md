# Concurrency

Asthra provides lightweight concurrent programming features designed for safe, efficient parallel execution with seamless C interoperability.

## Lightweight Tasks (`spawn`)

Asthra supports lightweight concurrent tasks, similar to goroutines, using the `spawn` keyword. These tasks are managed by Asthra's runtime scheduler and provide efficient concurrency without the overhead of OS threads.

```asthra
pub fn worker_task(id: int, message: string) -&gt; Result&lt;void, string&gt; {
    log("WORKER", "Worker " + id + " received: " + message + "");
    // ... perform concurrent work ...
    return Result.Ok(void);
}

pub fn start_workers(void) -&gt; Result&lt;void, string&gt; " + spawn worker_task(1, "Task A"); // Spawn a new concurrent task
    spawn worker_task(2, "Task B");
    
    log("MAIN", "Main function continues after spawning workers.");
    // ... main logic might wait for tasks or proceed ...
    return Result.Ok(void);
 + "
```

### Task Communication Patterns

```asthra
pub fn producer_consumer_example(void) -&gt; Result&lt;void, string&gt; "data_source_1");
    spawn produce_data("data_source_2");
    
    // Consumer tasks
    spawn consume_data("consumer_1");
    spawn consume_data("consumer_2");
    
    log("COORDINATOR", "All tasks spawned successfully");
    return Result.Ok(void);
 + "

pub fn produce_data(source: string) -&gt; Result&lt;void, string&gt; " + for i in range(0, 10) " + 
        let data: string = "Data from " + source + ": item " + i + "";
        // Send data to consumers (implementation depends on communication mechanism)
        log("PRODUCER", "Produced: " + data + "");
     + "
    return Result.Ok(void);
 + "

pub fn consume_data(consumer_id.to_string(&#125;
```

## Future Concurrency Extensions

Asthra v1.2 provides foundation for future `await` support and advanced concurrency patterns:

```asthra
// Future syntax (not yet implemented)
pub fn advanced_concurrency_example(void) -&gt; Result&lt;string, string&gt; " + // let handle1: auto = spawn_with_handle worker_task(1, "Task A");
    // let handle2: auto = spawn_with_handle worker_task(2, "Task B");
    
    // let result1: auto = await handle1;
    // let result2: auto = await handle2;
    
    // match (result1, result2) " + 
    //     (Result.Ok(_), Result.Ok(_)) =&gt; return Result.Ok("All tasks completed"),
    //     _ =&gt; return Result.Err("Some tasks failed")
    //  + "
    
    return Result.Ok("Foundation ready for await support");
 + "
```

### Planned Async Features

The following features are planned for future versions:

- **Task Handles**: Return values from `spawn` for task management
- **Await Syntax**: `await` keyword for waiting on task completion
- **Channels**: Message passing between tasks
- **Select Statements**: Waiting on multiple concurrent operations
- **Task Cancellation**: Graceful task termination

## Isolated Heaps for C-Bound Threads

When Asthra calls a C function that might block, the runtime can isolate the OS thread to prevent stalling Asthra's scheduler. This ensures that blocking C operations don't interfere with other concurrent tasks.

```asthra
extern "blocking_lib" fn long_running_c_function(data: *const u8, len: usize) -&gt; i32;

pub fn handle_blocking_c_call(data: &#91;&#93;u8) -&gt; Result&lt;void, string&gt; " + // This call may block, so it will be isolated
    let result: i32 = unsafe " +  
        long_running_c_function(data.ptr, data.len) 
     + ";
    
    match result " + 
        0 =&gt; {
            log("BLOCKING", "C function completed successfully");
            return Result.Ok(void);
         + ",
        _ =&gt; "C function failed with code.to_string(&#125;

pub fn concurrent_with_blocking(void) -&gt; Result&lt;void, string&gt; {let data: &#91;&#93;u8 = &#91;1, 2, 3, 4, 5&#93;;
    
    // Spawn task that may block - runtime will handle isolation
    spawn handle_blocking_c_call(data);
    
    // Other tasks continue running on the main scheduler
    spawn worker_task(1, "Non-blocking task");
    spawn worker_task(2, "Another non-blocking task");
    
    return Result.Ok(void);
 + "
```

## Mutex API (`Asthra_runtime.lock_*`)

For explicit synchronization with comprehensive error handling, Asthra provides a mutex API through the runtime:

```asthra
pub fn synchronized_operation(void) -&gt; Result&lt;void, string&gt; "SYNC", "Executing critical section");
            
            let unlock_result: AsthraResult = Asthra_runtime.mutex_unlock(mutex);
            Asthra_runtime.mutex_destroy(mutex);
            
            match Asthra_runtime.result_is_ok(unlock_result) " + 
                true =&gt; return Result.Ok(void),
                false =&gt; return Result.Err("Failed to unlock mutex")
             + "
        },
        false =&gt; "Failed to lock mutex");
        }
    }
}
```

### Shared Resource Protection

```asthra
// Global mutex for shared resource (conceptual - actual implementation may vary)
let shared_counter_mutex: *mut AsthraMutex = Asthra_runtime.mutex_create(void);
let shared_counter: int = 0;

pub fn increment_shared_counter(void) -&gt; Result&lt;void, string&gt; " + let lock_result: AsthraResult = Asthra_runtime.mutex_lock(shared_counter_mutex);
    
    match Asthra_runtime.result_is_ok(lock_result) " + 
        true =&gt; " + 
            // Critical section - modify shared resource
            shared_counter = shared_counter + 1;
            log("COUNTER", "Counter incremented to: " + shared_counter + "");
            
            let unlock_result.to_string(&#125;
}

pub fn concurrent_counter_example(void) -&gt; Result&lt;void, string&gt; {// Spawn multiple tasks that increment the shared counter
    spawn increment_shared_counter(void);
    spawn increment_shared_counter(void);
    spawn increment_shared_counter(void);
    
    log("MAIN", "Spawned counter increment tasks");
    return Result.Ok(void);
 + "
```

## Async Callbacks (C to Asthra)

C code can enqueue asynchronous callbacks to be executed by Asthra's scheduler with comprehensive error handling.

```asthra
// Callback function that C can invoke asynchronously
pub fn async_callback_handler(data: *mut u8, len: usize) -&gt; Result&lt;void, string&gt; " + // Convert C data to Asthra slice
    let asthra_data: &#91;&#93;u8 = unsafe " + 
        Asthra_runtime.slice_from_raw_parts(data, len, false, ownership_c)
     + ";
    
    log("CALLBACK", "Received async callback with " + len + " bytes");
    
    // Process the data
    for i in range(0, asthra_data.len) " + 
        // Process each byte
        let byte_value: u8 = asthra_data&#91;i&#93;;
        // ... processing logic ...
     + "
    
    return Result.Ok(void);
 + "

// Register callback with C library
extern "async_lib" fn register_callback(
    callback: fn(*mut u8, usize) -&gt; i32
) -&gt; i32;

pub fn setup_async_callbacks(void) -&gt; Result&lt;void, string&gt; " + // Register our callback with the C library
    let result.to_string(&#125;

// Wrapper function to bridge C callback to Asthra
pub fn async_callback_wrapper(data: *mut u8, len: usize) -&gt; i32 " + 
    // Enqueue the callback to be executed by Asthra's scheduler
    let enqueue_result: AsthraResult = Asthra_runtime.enqueue_callback(
        async_callback_handler, 
        data, 
        len
    );
    
    match Asthra_runtime.result_is_ok(enqueue_result) {
        true =&gt; return 0,  // Success
        false =&gt; return -1 // Error
     + "
}
```

## Concurrency Best Practices

### Task Design Guidelines

1\. **Keep tasks focused** - Each task should have a single, well-defined responsibility
2\. **Minimize shared state** - Use message passing instead of shared memory when possible
3\. **Handle errors gracefully** - Always return `Result` types from task functions
4\. **Use appropriate synchronization** - Mutex for shared resources, channels for communication

### Performance Considerations

1\. **Task granularity** - Balance between too many small tasks and too few large tasks
2\. **Blocking operations** - Be aware that C function calls may block the scheduler
3\. **Memory allocation** - Consider GC pressure from many concurrent allocations
4\. **Resource cleanup** - Ensure proper cleanup in all task execution paths

### Error Handling in Concurrent Code

```asthra
pub fn robust_concurrent_operation(void) -&gt; Result&lt;void, string&gt; " + // Spawn multiple tasks with error handling
    let task_results: &#91;&#93;Result&lt;void, string&gt; = &#91;&#93;;
    
    // In future versions, this might look like:
    // let handle1: auto = spawn_with_handle risky_task(1);
    // let handle2: auto = spawn_with_handle risky_task(2);
    // let handle3: auto = spawn_with_handle risky_task(3);
    
    // For now, we spawn and handle errors within tasks
    spawn risky_task_with_logging(1);
    spawn risky_task_with_logging(2);
    spawn risky_task_with_logging(3);
    
    return Result.Ok(void);
 + "

pub fn risky_task_with_logging(task_id: int) -&gt; Result&lt;void, string&gt; " + let result: Result&lt;void, string&gt; = risky_operation(task_id);
    
    match result " + 
        Result.Ok(_) =&gt; {
            log("TASK", "Task " + task_id + " completed successfully");
            return Result.Ok(void);
         + ",
        Result.Err(error) =&gt; "ERROR", "Task " + task_id + " failed.to_string(&#125;
```

## Integration with Memory Management

Concurrent tasks work seamlessly with Asthra's memory management:

- **GC-managed objects** are safely shared between tasks
- **C-managed memory** requires explicit synchronization
- **Pinned memory** remains stable across task switches

```asthra
pub fn memory_safe_concurrency(void) -&gt; Result&lt;void, string&gt; {// GC-managed data - safe to share
    let shared_data: &#91;&#93;int = &#91;1, 2, 3, 4, 5&#93;;
    
    // Spawn tasks that can safely access GC-managed data
    spawn process_gc_data(shared_data);
    spawn process_gc_data(shared_data);
    
    // C-managed memory requires careful handling
    let c_buffer: *mut u8 #&#91;ownership(c)&#93; = unsafe { libc.malloc(1024) as *mut u8  + ";
    
    match c_buffer == (0 as *mut u8) "Memory allocation failed"),
        false =&gt; {
            // Only one task should own C memory at a time
            spawn process_c_data_exclusive(c_buffer);
            return Result.Ok(void);
        }
    }
}
```

## Next Steps

Now that you understand concurrency, explore:

- **&#91;Security Features&#93;(security.md)** - Learn about secure concurrent programming
- **&#91;Memory Management&#93;(memory-management.md)** - Understand memory safety in concurrent contexts
- **&#91;Observability&#93;(observability.md)** - Debug and monitor concurrent applications 
