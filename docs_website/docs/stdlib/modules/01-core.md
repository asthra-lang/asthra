# stdlib::asthra::core

**Package**: `stdlib::asthra::core`  
**Import**: `import "stdlib/asthra/core";`  
**Purpose**: Runtime internals, memory management, and type system support.

## Overview

The core module provides access to Asthra's runtime system, including memory management, garbage collection, type system introspection, and build information. This is the foundation module that other stdlib modules depend on.

## Runtime Information

### `runtime_version() -&gt; string`

Returns the current Asthra runtime version.

```asthra
import "stdlib/asthra/core";

fn check_runtime() -&gt; void {
    let version: string = core::runtime_version();
    log("INFO", "Running Asthra runtime version: " + version);
}
```

### `build_info() -&gt; BuildInfo`

Returns detailed build information including version, build date, target platform, and enabled features.

```asthra
struct BuildInfo {
    version: string,
    build_date: string,
    build_time: string,
    target_platform: string,
    features: &#91;&#93;string
}

fn display_build_info() -&gt; void {
    let info: core::BuildInfo = core::build_info();
    
    log("BUILD", "Version: " + info.version);
    log("BUILD", "Build date: " + info.build_date);
    log("BUILD", "Build time: " + info.build_time);
    log("BUILD", "Target platform: " + info.target_platform);
    log("BUILD", "Features: " + info.features.len(&#125; enabled");
    
    // List all features
    let mut i: usize = 0;
    while i &lt; info.features.len() {
        log("FEATURE", "  " + info.features&#91;i&#93;);
        i = i + 1;
    }
}
```

### `target_triple() -&gt; string`

Returns the target architecture triple (e.g., "x86_64-linux-gnu").

```asthra
fn check_target() -&gt; void {
    let target: string = core::target_triple();
    log("INFO", "Target architecture: " + target);
    
    // Platform-specific logic
    if target.contains("windows") {
        log("PLATFORM", "Running on Windows");
    } else if target.contains("linux") {
        log("PLATFORM", "Running on Linux");
    } else if target.contains("darwin") {
        log("PLATFORM", "Running on macOS");
    }
}
```

## Type System Support

### `type_of<T>(value: T) -&gt; u32`

Returns the type ID for a given value.

```asthra
fn examine_types() -&gt; void {
    let number: i32 = 42;
    let text: string = "hello";
    let flag: bool = true;
    let decimal: f64 = 3.14;
    
    let number_type: u32 = core::type_of(number);
    let text_type: u32 = core::type_of(text);
    let flag_type: u32 = core::type_of(flag);
    let decimal_type: u32 = core::type_of(decimal);
    
    log("TYPE", "i32 type ID: " + number_type);
    log("TYPE", "string type ID: " + text_type);
    log("TYPE", "bool type ID: " + flag_type);
    log("TYPE", "f64 type ID: " + decimal_type);
}
```

### `get_type_name(type_id: u32) -&gt; Result&lt;string, string&gt;`

Returns the name of a type given its ID.

```asthra
fn get_type_info() -&gt; Result&lt;void, string&gt; {
    let value: f64 = 3.14;
    let type_id: u32 = core::type_of(value);
    
    let type_name_result: Result&lt;string, string&gt; = core::get_type_name(type_id);
    match type_name_result {
        Result.Ok(name) =&gt; {
            log("INFO", "Type name: " + name);
            return Result.Ok(void);
        },
        Result.Err(error) =&gt; {
            return Result.Err("Failed to get type name: " + error);
        }
    }
}
```

### `get_type_size(type_id: u32) -&gt; Result&lt;usize, string&gt;`

Returns the size in bytes of a type given its ID.

```asthra
fn analyze_type_sizes() -&gt; Result&lt;void, string&gt; {
    let types: &#91;&#93;string = &#91;"i32", "i64", "f32", "f64", "bool"&#93;;
    let values: &#91;&#93;interface{} = &#91;42, 42i64, 3.14f32, 3.14, true&#93;;
    
    let mut i: usize = 0;
    while i &lt; types.len() {
        let type_id: u32 = core::type_of(values&#91;i&#93;);
        let size_result: Result<usize, string&gt; = core::get_type_size(type_id);
        
        match size_result {
            Result.Ok(size) =&gt; {
                log("SIZE", types&#91;i&#93; + " = " + size + " bytes");
            },
            Result.Err(error) =&gt; {
                return Result.Err("Failed to get size for " + types&#91;i&#93; + ": " + error);
            }
        }
        i = i + 1;
    }
    
    return Result.Ok(void);
}
```

### `register_type(name: string, size: usize, destructor: fn(*mut void)) -&gt; u32`

Registers a custom type with the runtime system.

```asthra
fn cleanup_custom_type(ptr: *mut void) -&gt; void {
    // Custom cleanup logic
    log("CLEANUP", "Cleaning up custom type");
}

fn register_custom_type() -&gt; u32 {
    let type_id: u32 = core::register_type(
        "MyCustomType",
        64,  // 64 bytes
        cleanup_custom_type
    );
    
    log("REGISTER", "Registered custom type with ID: " + type_id);
    return type_id;
}
```

## Memory Management

### Memory Zones

Asthra supports four memory zones for different use cases:

```asthra
enum MemoryZone {
    GC,      // Garbage collected (default)
    Manual,  // Manual management
    Pinned,  // Pinned for FFI safety
    Stack    // Stack allocation
}
```

### `alloc(size: usize, zone: MemoryZone) -&gt; Result&lt;*mut void, string&gt;`

Allocates memory in the specified zone.

```asthra
fn allocate_memory() -&gt; Result&lt;void, string&gt; {
    // Allocate 1KB in manual zone
    let manual_ptr: Result&lt;*mut void, string&gt; = core::alloc(1024, core::MemoryZone.Manual);
    
    match manual_ptr {
        Result.Ok(ptr) =&gt; {
            log("ALLOC", "Allocated 1024 bytes in manual zone");
            
            // Use the memory...
            unsafe {
                // Write some data
                let byte_ptr: *mut u8 = ptr as *mut u8;
                *byte_ptr = 42;
            }
            
            // Don't forget to free manual memory
            let free_result: Result&lt;void, string&gt; = core::free(ptr, core::MemoryZone.Manual);
            match free_result {
                Result.Ok(_) =&gt; {
                    log("ALLOC", "Memory freed successfully");
                    return Result.Ok(void);
                },
                Result.Err(error) =&gt; {
                    return Result.Err("Failed to free memory: " + error);
                }
            }
        },
        Result.Err(error) =&gt; {
            return Result.Err("Allocation failed: " + error);
        }
    }
}
```

### `alloc_zeroed(size: usize, zone: MemoryZone) -&gt; Result&lt;*mut void, string&gt;`

Allocates zero-initialized memory.

```asthra
fn allocate_zeroed_buffer() -&gt; Result&lt;*mut void, string&gt; {
    let buffer_result: Result&lt;*mut void, string&gt; = core::alloc_zeroed(512, core::MemoryZone.Pinned);
    
    match buffer_result {
        Result.Ok(ptr) =&gt; {
            log("ALLOC", "Allocated 512 bytes of zeroed memory");
            
            // Verify it's zeroed
            unsafe {
                let byte_ptr: *mut u8 = ptr as *mut u8;
                let first_byte: u8 = *byte_ptr;
                assert(first_byte == 0, "Memory should be zeroed");
            }
            
            return Result.Ok(ptr);
        },
        Result.Err(error) =&gt; {
            return Result.Err("Zeroed allocation failed: " + error);
        }
    }
}
```

### `realloc(ptr: *mut void, new_size: usize, zone: MemoryZone) -&gt; Result&lt;*mut void, string&gt;`

Reallocates memory to a new size.

```asthra
fn resize_buffer() -&gt; Result&lt;void, string&gt; {
    // Initial allocation
    let initial_ptr: Result&lt;*mut void, string&gt; = core::alloc(256, core::MemoryZone.Manual);
    let ptr: *mut void = match initial_ptr {
        Result.Ok(p) =&gt; p,
        Result.Err(error) =&gt; return Result.Err("Initial allocation failed: " + error)
    };
    
    // Resize to larger buffer
    let resized_result: Result&lt;*mut void, string&gt; = core::realloc(ptr, 512, core::MemoryZone.Manual);
    let new_ptr: *mut void = match resized_result {
        Result.Ok(p) =&gt; {
            log("REALLOC", "Buffer resized from 256 to 512 bytes");
            p
        },
        Result.Err(error) =&gt; {
            // Free original on failure
            core::free(ptr, core::MemoryZone.Manual);
            return Result.Err("Reallocation failed: " + error);
        }
    };
    
    // Clean up
    let free_result: Result&lt;void, string&gt; = core::free(new_ptr, core::MemoryZone.Manual);
    match free_result {
        Result.Ok(_) =&gt; return Result.Ok(void),
        Result.Err(error) =&gt; return Result.Err("Failed to free resized memory: " + error)
    }
}
```

### `free(ptr: *mut void, zone: MemoryZone) -&gt; Result&lt;void, string&gt;`

Frees memory allocated in the specified zone.

```asthra
fn manual_memory_management() -&gt; Result&lt;void, string&gt; {
    let ptr: *mut void = match core::alloc(128, core::MemoryZone.Manual) {
        Result.Ok(p) =&gt; p,
        Result.Err(error) =&gt; return Result.Err("Allocation failed: " + error)
    };
    
    // Use the memory...
    log("MEMORY", "Using allocated memory");
    
    // Always free manual memory
    match core::free(ptr, core::MemoryZone.Manual) {
        Result.Ok(_) =&gt; {
            log("MEMORY", "Memory freed successfully");
            return Result.Ok(void);
        },
        Result.Err(error) =&gt; {
            return Result.Err("Failed to free memory: " + error);
        }
    }
}
```

## Garbage Collection

### `gc_stats() -&gt; GCStats`

Returns garbage collection statistics.

```asthra
struct GCStats {
    total_allocated: usize,
    total_freed: usize,
    current_usage: usize,
    gc_cycles: u64,
    last_gc_duration: u64
}

fn monitor_gc() -&gt; void {
    let stats: core::GCStats = core::gc_stats();
    
    log("GC", "Current usage: " + stats.current_usage + " bytes");
    log("GC", "Total allocated: " + stats.total_allocated + " bytes");
    log("GC", "Total freed: " + stats.total_freed + " bytes");
    log("GC", "GC cycles: " + stats.gc_cycles);
    log("GC", "Last GC duration: " + stats.last_gc_duration + " nanoseconds");
    
    // Calculate efficiency
    let efficiency: f64 = (stats.total_freed as f64) / (stats.total_allocated as f64) * 100.0;
    log("GC", "Collection efficiency: " + efficiency + "%");
}
```

### `gc_collect() -&gt; void`

Manually triggers garbage collection.

```asthra
fn force_gc() -&gt; void {
    let before: core::GCStats = core::gc_stats();
    log("GC", "Memory usage before GC: " + before.current_usage + " bytes");
    
    log("GC", "Triggering manual garbage collection");
    core::gc_collect();
    
    let after: core::GCStats = core::gc_stats();
    log("GC", "Memory usage after GC: " + after.current_usage + " bytes");
    
    let freed: usize = before.current_usage - after.current_usage;
    log("GC", "Freed " + freed + " bytes");
}
```

### `gc_register_root(ptr: *mut void) -&gt; void`

Registers a pointer as a GC root (prevents collection).

```asthra
fn register_important_data() -&gt; void {
    let important_data: *mut void = match core::alloc(1024, core::MemoryZone.GC) {
        Result.Ok(ptr) =&gt; ptr,
        Result.Err(_) =&gt; return
    };
    
    // Register as GC root to prevent collection
    core::gc_register_root(important_data);
    log("GC", "Registered important data as GC root");
    
    // Later, when no longer needed
    core::gc_unregister_root(important_data);
    log("GC", "Unregistered GC root");
}
```

### `gc_pin_memory(ptr: *mut void, size: usize) -&gt; void`

Pins memory to prevent it from being moved by the GC.

```asthra
fn pin_for_ffi() -&gt; void {
    let buffer: *mut void = match core::alloc(512, core::MemoryZone.GC) {
        Result.Ok(ptr) =&gt; ptr,
        Result.Err(_) =&gt; return
    };
    
    // Pin memory for FFI call
    core::gc_pin_memory(buffer, 512);
    log("FFI", "Memory pinned for FFI operation");
    
    // Perform FFI operation...
    // call_c_function(buffer);
    
    // Unpin when done
    core::gc_unpin_memory(buffer);
    log("FFI", "Memory unpinned after FFI operation");
}
```

## Best Practices

### Memory Zone Selection

Choose the appropriate memory zone for your use case:

```asthra
fn memory_zone_examples() -&gt; void {
    // GC zone - for most allocations
    let gc_data: *mut void = core::alloc(256, core::MemoryZone.GC).unwrap();
    
    // Manual zone - for precise control
    let manual_data: *mut void = core::alloc(256, core::MemoryZone.Manual).unwrap();
    // Must free manually
    core::free(manual_data, core::MemoryZone.Manual);
    
    // Pinned zone - for FFI safety
    let pinned_data: *mut void = core::alloc(256, core::MemoryZone.Pinned).unwrap();
    // Won't be moved by GC
    
    // Stack zone - for temporary data
    let stack_data: *mut void = core::alloc(256, core::MemoryZone.Stack).unwrap();
    // Automatically freed when scope ends
}
```

### Error Handling

Always handle allocation failures:

```asthra
fn safe_allocation() -&gt; Result&lt;*mut void, string&gt; {
    let ptr_result: Result&lt;*mut void, string&gt; = core::alloc(1024, core::MemoryZone.Manual);
    
    match ptr_result {
        Result.Ok(ptr) =&gt; {
            log("ALLOC", "Allocation successful");
            return Result.Ok(ptr);
        },
        Result.Err(error) =&gt; {
            log("ERROR", "Allocation failed: " + error);
            return Result.Err("Memory allocation failed");
        }
    }
}
```

### Resource Management

Use RAII patterns for automatic cleanup:

```asthra
struct ManagedBuffer {
    ptr: *mut void,
    size: usize,
    zone: core::MemoryZone
}

impl ManagedBuffer {
    fn new(size: usize, zone: core::MemoryZone) -&gt; Result&lt;ManagedBuffer, string&gt; {
        let ptr_result: Result&lt;*mut void, string&gt; = core::alloc(size, zone);
        match ptr_result {
            Result.Ok(ptr) =&gt; {
                return Result.Ok(ManagedBuffer {
                    ptr: ptr,
                    size: size,
                    zone: zone
                });
            },
            Result.Err(error) =&gt; {
                return Result.Err("Buffer allocation failed: " + error);
            }
        }
    }
    
    fn drop(&amp;mut self) -&gt; void {
        if self.zone == core::MemoryZone.Manual {
            core::free(self.ptr, self.zone);
        }
    }
}
```

## Common Patterns

### Memory Pool

```asthra
struct MemoryPool {
    blocks: &#91;&#93;(*mut void),
    block_size: usize,
    zone: core::MemoryZone
}

impl MemoryPool {
    fn new(block_size: usize, initial_blocks: usize, zone: core::MemoryZone) -&gt; Result&lt;MemoryPool, string&gt; {
        let mut blocks: &#91;&#93;(*mut void) = &#91;&#93;;
        
        let mut i: usize = 0;
        while i &lt; initial_blocks {
            let block_result: Result<*mut void, string&gt; = core::alloc(block_size, zone);
            match block_result {
                Result.Ok(block) =&gt; blocks.push(block),
                Result.Err(error) =&gt; return Result.Err("Pool allocation failed: " + error)
            }
            i = i + 1;
        }
        
        return Result.Ok(MemoryPool {
            blocks: blocks,
            block_size: block_size,
            zone: zone
        });
    }
    
    fn get_block(&amp;mut self) -&gt; Result&lt;*mut void, string&gt; {
        if self.blocks.len() &gt; 0 {
            return Result.Ok(self.blocks.pop());
        } else {
            return core::alloc(self.block_size, self.zone);
        }
    }
    
    fn return_block(&amp;mut self, block: *mut void) -&gt; void {
        self.blocks.push(block);
    }
}
```

## Summary

The `stdlib::asthra::core` module provides:

- **Runtime Information**: Version, build details, target platform
- **Type System**: Type introspection and registration
- **Memory Management**: Multiple zones for different use cases
- **Garbage Collection**: Statistics, manual collection, root management
- **Safety**: Explicit error handling for all operations

This module forms the foundation for all other stdlib modules and provides the essential runtime services needed for robust Asthra applications. 
