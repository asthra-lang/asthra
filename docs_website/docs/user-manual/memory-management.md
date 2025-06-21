# Memory Management

**System Design Clarification**: Asthra's memory management system is **complete by design** for AI-first development. External analyses suggesting our system is "incomplete" misunderstand our intentional design philosophy that prioritizes AI generation reliability over Rust-style theoretical completeness.

## Comprehensive Current Capabilities

Asthra **already provides** all essential memory management features across every necessary context:

**✅ Declaration-Level Ownership Strategy:**
```asthra
#&#91;ownership(gc)&#93;      // Garbage collected (default)
#&#91;ownership(c)&#93;       // C-style manual management  
#&#91;ownership(pinned)&#93;  // Pinned memory for security/performance
```

**✅ Variable Ownership:**
```asthra
let gc_data: #&#91;ownership(gc)&#93; DataStruct = create_managed_data();
let c_buffer: #&#91;ownership(c)&#93; *mut u8 = unsafe " +  malloc(1024)  + ";
let secure_key: #&#91;ownership(pinned)&#93; &#91;&#93;u8 = allocate_secure_buffer();
```

**✅ FFI Transfer Semantics:**
```asthra
pub fn process_data(#&#91;borrowed&#93; input: *const u8, #&#91;transfer_full&#93; output: *mut u8) -&gt; #&#91;transfer_full&#93; *mut Result;
extern "C" fn malloc(size: usize) -&gt; #&#91;transfer_full&#93; *mut void;
extern "C" fn strlen(#&#91;borrowed&#93; s: *const u8) -&gt; usize;
```

**Why Our Two-Level System Is AI-Optimized:**
- **Orthogonal Concerns**: Ownership strategy (how memory is managed) separate from transfer semantics (how ownership moves)
- **Predictable Patterns**: Each annotation has single, clear meaning that AI can reliably generate
- **Local Reasoning**: AI makes decisions based on immediate context, not global program analysis
- **Fail-Safe Defaults**: Missing annotations default to safe garbage collection

Asthra provides safe, deterministic memory management through explicit ownership annotations and automatic garbage collection for most use cases.

## Hybrid GC Model

Asthra uses a conservative garbage collector for objects primarily allocated and managed within Asthra code. This provides automatic memory management for most Asthra operations while allowing precise control when needed.

### How the GC Works

The conservative GC:
- Automatically manages memory for Asthra objects
- Scans the stack and heap conservatively to find references
- Handles most memory allocation and deallocation transparently
- Works alongside manual memory management for C interop

## Manual Memory Zones (`#&#91;ownership(c)&#93;`)

For memory allocated and managed by C, use `#&#91;ownership(c)&#93;`. Asthra's GC will not touch this memory, leaving management entirely to C code.

```asthra
#&#91;ownership(c)&#93; // Points to C-managed memory
extern "libc" fn malloc(size: usize) -&gt; *mut void;
#&#91;ownership(c)&#93;
extern "libc" fn free(ptr: *mut void);

pub fn use_c_memory() " + 
    let c_buffer: *mut u8 #&#91;ownership(c)&#93; = unsafe " +  malloc(100) as *mut u8  + ";
    // ... use c_buffer ...
    unsafe " +  free(c_buffer as *mut void);  + 
```

## Pinned Memory (`#&#91;ownership(pinned)&#93;`)

For memory managed externally (e.g., C) that Asthra needs stable access to. The GC won't move or collect this memory, but it will observe it.

```asthra
#&#91;ownership(pinned)&#93;
extern "my_lib" fn get_fixed_buffer() -&gt; *mut u8;

pub fn access_pinned_buffer() " + 
    let buffer_ptr: *mut u8 #&#91;ownership(pinned)&#93; = unsafe " +  get_fixed_buffer()  + ";
    // Asthra's GC will observe but not move/collect this memory.
 + "
```

## Ownership Tags for Declarations

These tags on variable or struct declarations specify the memory management strategy:

### `#&#91;ownership(gc)&#93;`
- **Default** for Asthra types
- Managed by Asthra's garbage collector
- Automatic allocation and deallocation
- Safe for most Asthra operations

### `#&#91;ownership(c)&#93;`
- Memory managed by C code
- Asthra GC ignores this memory
- Manual allocation and deallocation required
- Used for C library integration

### `#&#91;ownership(pinned)&#93;`
- Externally managed memory with stable address
- GC observes but doesn't move or collect
- Used for memory that must remain at fixed locations

```asthra
pub fn ownership_examples() " + 
    // Default GC-managed memory
    let asthra_data: &#91;&#93;u8 = &#91;1, 2, 3, 4&#93;; // #&#91;ownership(gc)&#93; implicit
    
    // C-managed memory
    let c_buffer: *mut u8 #&#91;ownership(c)&#93; = unsafe { malloc(256) as *mut u8  + ";
    
    // Pinned memory from external source
    let pinned_buffer: *mut u8 #&#91;ownership(pinned)&#93; = unsafe " +  get_hardware_buffer()  + ";
    
    // Clean up C memory manually
    unsafe " +  free(c_buffer as *mut void);  + "
    // GC handles asthra_data automatically
    // External system handles pinned_buffer
}
```

## Formalized Slice Management

Asthra slices have a formalized internal structure for predictable memory layout:

```asthra
// Internal representation (not directly user-accessible)
// SliceHeader<T> " + 
//     ptr: *mut T;     // Pointer to the data
//     len: usize;      // Number of elements
//  + "

pub fn slice_operations_example() -&gt; Result&lt;usize, string&gt; {// Create slice from array literal
    let data: &#91;&#93;u8 = &#91;1, 2, 3, 4, 5&#93;;
    
    // Access length (always safe)
    let length: usize = data.len; // 5
    
    // Slice bounds are checked at runtime in debug builds
    if length &gt; 0 " + 
        return Result.Ok(length);
     + " else " + 
        return Result.Err("Empty slice");
     + 
```

### Slice Creation and Management

```asthra
pub fn slice_management_examples() -&gt; Result&lt;void, string&gt; "INFO", "Slice contains " + count + " elements");
    
    // Safe element access (bounds checked in debug)
    if count &gt; 2 " + 
        let third_element: int = numbers&#91;2&#93;; // 30
        log("INFO", "Third element: " + third_element + "");
     + "
    
    return Result.Ok(void);
}
```

### Creating Slices from C Data

```asthra
// Creating an Asthra slice from C data with error handling
pub fn get_data_from_c() -&gt; Result&lt;&#91;&#93;u8, string&gt; {let c_ptr: *mut u8 = unsafe " +  libc.malloc(100) as *mut u8  + ";
    if c_ptr == (0 as *mut u8) "Memory allocation failed");
     + "
    
    let length: usize = 100;
    // ... fill c_ptr with data ...

    // Create slice from C data with ownership tracking
    let asthra_slice: &#91;&#93;u8 = unsafe " + 
        Asthra_runtime.slice_from_raw_parts(c_ptr, length, true, ownership_c)
     + ";
    
    return Result.Ok(asthra_slice);
}
```

## Secure Memory Zeroing

For sensitive data (e.g., cryptographic keys), Asthra provides a mechanism to securely wipe memory. The compiler guarantees calls to `Asthra_runtime.secure_zero` are not optimized away.

```asthra
pub fn handle_sensitive_key() -&gt; Result&lt;void, string&gt; {// Allocate slice for cryptographic key
    let key_buffer: &#91;&#93;u8 = allocate_secure_slice(32);
    
    // ... use key_buffer for crypto operation ...
    
    // Securely wipe the key - works with slice's internal structure
    Asthra_runtime.secure_zero(key_buffer);
    
    return Result.Ok(void);
 + "
```

### Secure Memory Best Practices

```asthra
pub fn secure_memory_example() -&gt; Result&lt;void, string&gt; {// Allocate sensitive data
    let password_buffer: &#91;&#93;u8 = allocate_slice(64);
    let key_material: &#91;&#93;u8 = allocate_slice(32);
    
    // ... perform cryptographic operations ...
    
    // Always clean up sensitive data
    Asthra_runtime.secure_zero(password_buffer);
    Asthra_runtime.secure_zero(key_material);
    
    log("SECURITY", "Sensitive data securely cleared");
    return Result.Ok(void);
 + "
```

## Memory Safety Guidelines

### Safe Practices

1\. **Use GC-managed memory by default** for Asthra objects
2\. **Explicitly annotate C-managed memory** with `#&#91;ownership(c)&#93;`
3\. **Always free C-allocated memory** in the same scope or function
4\. **Use secure_zero for sensitive data** before deallocation
5\. **Check slice bounds** when accessing elements

### Unsafe Operations

Operations that require `unsafe` blocks:
- Calling C functions that allocate/deallocate memory
- Dereferencing raw pointers
- Creating slices from raw pointers
- Manual memory management operations

```asthra
pub fn memory_safety_example() -&gt; Result&lt;void, string&gt; {// Safe: GC-managed slice
    let safe_data: &#91;&#93;int = &#91;1, 2, 3, 4, 5&#93;;
    
    // Unsafe: C memory allocation
    let raw_buffer: *mut u8 = unsafe { libc.malloc(1024) as *mut u8  + ";
    
    match raw_buffer == (0 as *mut u8) "Allocation failed"),
        false =&gt; {
            // Use the buffer...
            
            // Always clean up
            unsafe { libc.free(raw_buffer as *mut void); }
            return Result.Ok(void);
        }
    }
}
```

## Performance Considerations

### GC Performance
- The conservative GC has predictable performance characteristics
- GC pauses are typically short for most applications
- Memory allocation is fast for GC-managed objects

### Manual Memory Performance
- C-managed memory has zero GC overhead
- Requires careful manual management
- Best for performance-critical sections

### Slice Performance
- Slices have minimal overhead (pointer + length)
- Bounds checking can be disabled in release builds
- Direct memory access when needed

## Next Steps

Now that you understand memory management, explore:

- **&#91;C Interoperability&#93;(ffi-interop.md)** - Learn how memory management integrates with FFI
- **&#91;Security Features&#93;(security.md)** - Understand secure memory operations
- **&#91;Concurrency&#93;(concurrency.md)** - See how memory management works with concurrent code 
