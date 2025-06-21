# Memory Management

**System Design Clarification**: Asthra's memory management system is **complete by design** for AI-first development. External analyses suggesting our system is "incomplete" misunderstand our intentional design philosophy that prioritizes AI generation reliability over Rust-style theoretical completeness.

## Comprehensive Current Capabilities

Asthra **already provides** all essential memory management features across every necessary context:

**✅ Declaration-Level Ownership Strategy:**
```asthra
#[ownership(gc)]      // Garbage collected (default)
#[ownership(c)]       // C-style manual management  
#[ownership(pinned)]  // Pinned memory for security/performance
```

**✅ Variable Ownership:**
```asthra
let gc_data: #[ownership(gc)] DataStruct = create_managed_data();
let c_buffer: #[ownership(c)] *mut u8 = unsafe " +  malloc(1024)  + ";
let secure_key: #[ownership(pinned)] []u8 = allocate_secure_buffer();
```

**✅ FFI Transfer Semantics:**
```asthra
pub fn process_data(#[borrowed] input: *const u8, #[transfer_full] output: *mut u8) -> #[transfer_full] *mut Result;
extern "C" fn malloc(size: usize) -> #[transfer_full] *mut void;
extern "C" fn strlen(#[borrowed] s: *const u8) -> usize;
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

## Manual Memory Zones (`#[ownership(c)]`)

For memory allocated and managed by C, use `#[ownership(c)]`. Asthra's GC will not touch this memory, leaving management entirely to C code.

```asthra
#[ownership(c)] // Points to C-managed memory
extern "libc" fn malloc(size: usize) -> *mut void;
#[ownership(c)]
extern "libc" fn free(ptr: *mut void);

pub fn use_c_memory() " + 
    let c_buffer: *mut u8 #[ownership(c)] = unsafe " +  malloc(100) as *mut u8  + ";
    // ... use c_buffer ...
    unsafe " +  free(c_buffer as *mut void);  + 
```

## Pinned Memory (`#[ownership(pinned)]`)

For memory managed externally (e.g., C) that Asthra needs stable access to. The GC won't move or collect this memory, but it will observe it.

```asthra
#[ownership(pinned)]
extern "my_lib" fn get_fixed_buffer() -> *mut u8;

pub fn access_pinned_buffer() " + 
    let buffer_ptr: *mut u8 #[ownership(pinned)] = unsafe " +  get_fixed_buffer()  + ";
    // Asthra's GC will observe but not move/collect this memory.
 + "
```

## Ownership Tags for Declarations

These tags on variable or struct declarations specify the memory management strategy:

### `#[ownership(gc)]`
- **Default** for Asthra types
- Managed by Asthra's garbage collector
- Automatic allocation and deallocation
- Safe for most Asthra operations

### `#[ownership(c)]`
- Memory managed by C code
- Asthra GC ignores this memory
- Manual allocation and deallocation required
- Used for C library integration

### `#[ownership(pinned)]`
- Externally managed memory with stable address
- GC observes but doesn't move or collect
- Used for memory that must remain at fixed locations

```asthra
pub fn ownership_examples() " + 
    // Default GC-managed memory
    let asthra_data: []u8 = [1, 2, 3, 4]; // #[ownership(gc)] implicit
    
    // C-managed memory
    let c_buffer: *mut u8 #[ownership(c)] = unsafe { malloc(256) as *mut u8  + ";
    
    // Pinned memory from external source
    let pinned_buffer: *mut u8 #[ownership(pinned)] = unsafe " +  get_hardware_buffer()  + ";
    
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

pub fn slice_operations_example() -> Result<usize, string> {// Create slice from array literal
    let data: []u8 = [1, 2, 3, 4, 5];
    
    // Access length (always safe)
    let length: usize = data.len; // 5
    
    // Slice bounds are checked at runtime in debug builds
    if length > 0 " + 
        return Result.Ok(length);
     + " else " + 
        return Result.Err("Empty slice");
     + 
```

### Slice Creation and Management

```asthra
pub fn slice_management_examples() -> Result<void, string> "INFO", "Slice contains " + count + " elements");
    
    // Safe element access (bounds checked in debug)
    if count > 2 " + 
        let third_element: int = numbers[2]; // 30
        log("INFO", "Third element: " + third_element + "");
     + "
    
    return Result.Ok(void);
}
```

### Creating Slices from C Data

```asthra
// Creating an Asthra slice from C data with error handling
pub fn get_data_from_c() -> Result<[]u8, string> {let c_ptr: *mut u8 = unsafe " +  libc.malloc(100) as *mut u8  + ";
    if c_ptr == (0 as *mut u8) "Memory allocation failed");
     + "
    
    let length: usize = 100;
    // ... fill c_ptr with data ...

    // Create slice from C data with ownership tracking
    let asthra_slice: []u8 = unsafe " + 
        Asthra_runtime.slice_from_raw_parts(c_ptr, length, true, ownership_c)
     + ";
    
    return Result.Ok(asthra_slice);
}
```

## Secure Memory Zeroing

For sensitive data (e.g., cryptographic keys), Asthra provides a mechanism to securely wipe memory. The compiler guarantees calls to `Asthra_runtime.secure_zero` are not optimized away.

```asthra
pub fn handle_sensitive_key() -> Result<void, string> {// Allocate slice for cryptographic key
    let key_buffer: []u8 = allocate_secure_slice(32);
    
    // ... use key_buffer for crypto operation ...
    
    // Securely wipe the key - works with slice's internal structure
    Asthra_runtime.secure_zero(key_buffer);
    
    return Result.Ok(void);
 + "
```

### Secure Memory Best Practices

```asthra
pub fn secure_memory_example() -> Result<void, string> {// Allocate sensitive data
    let password_buffer: []u8 = allocate_slice(64);
    let key_material: []u8 = allocate_slice(32);
    
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

1. **Use GC-managed memory by default** for Asthra objects
2. **Explicitly annotate C-managed memory** with `#[ownership(c)]`
3. **Always free C-allocated memory** in the same scope or function
4. **Use secure_zero for sensitive data** before deallocation
5. **Check slice bounds** when accessing elements

### Unsafe Operations

Operations that require `unsafe` blocks:
- Calling C functions that allocate/deallocate memory
- Dereferencing raw pointers
- Creating slices from raw pointers
- Manual memory management operations

```asthra
pub fn memory_safety_example() -> Result<void, string> {// Safe: GC-managed slice
    let safe_data: []int = [1, 2, 3, 4, 5];
    
    // Unsafe: C memory allocation
    let raw_buffer: *mut u8 = unsafe { libc.malloc(1024) as *mut u8  + ";
    
    match raw_buffer == (0 as *mut u8) "Allocation failed"),
        false => {
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

- **[C Interoperability](ffi-interop.md)** - Learn how memory management integrates with FFI
- **[Security Features](security.md)** - Understand secure memory operations
- **[Concurrency](concurrency.md)** - See how memory management works with concurrent code 
