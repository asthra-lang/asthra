# Asthra Safety Specification

**Version:** 1.0  
**Last Updated:** 2025-07-01  
**Status:** ✅ PRODUCTION READY

## Overview

Asthra implements a comprehensive multi-layered safety system designed to prevent common programming errors while maintaining the performance and control required for systems programming. This document consolidates all safety features and their specifications.

## Table of Contents

1. [Core Safety Principles](#core-safety-principles)
2. [Memory Safety](#memory-safety)
3. [Type Safety](#type-safety)
4. [Concurrency Safety](#concurrency-safety)
5. [FFI Safety](#ffi-safety)
6. [Initialization Safety](#initialization-safety)
7. [Bounds Safety](#bounds-safety)
8. [Null Safety](#null-safety)
9. [Unsafe Blocks](#unsafe-blocks)
10. [Safety Validation](#safety-validation)

## Core Safety Principles

### Philosophy: "Safe by Default, Explicit When Unsafe"

1. **Default Safety**: All operations are safe unless explicitly marked unsafe
2. **Local Reasoning**: Safety can be verified through local code analysis
3. **Explicit Boundaries**: Clear demarcation between safe and unsafe code
4. **Zero-Cost Abstractions**: Safety features have minimal runtime overhead
5. **AI-Friendly**: Safety patterns that AI can reliably understand and generate

## Memory Safety

### Ownership System

Asthra uses a two-level ownership system optimized for AI code generation:

```asthra
// Ownership strategies - where memory comes from
#[ownership(gc)]     // Garbage collected
#[ownership(c)]      // C-managed (malloc/free)
#[ownership(stack)]  // Stack allocated

// Transfer semantics - who owns it now
#[transfer_full]     // Ownership transferred to recipient
#[transfer_none]     // Caller retains ownership
#[borrowed]          // Temporary borrow, no ownership change
```

### Pointer Safety Rules

1. **Creating pointers is safe** - Address-of operator `&` can be used in safe code
2. **Dereferencing requires unsafe** - All pointer dereferences must be in unsafe blocks
3. **Type-safe pointers** - `*const T` and `*mut T` maintain type information

```asthra
pub fn pointer_safety_example(none) -> i32 {
    let value: i32 = 42;
    let ptr: *const i32 = &value;  // ✅ SAFE: Taking address
    
    // ❌ COMPILE ERROR: Cannot dereference outside unsafe
    // let x: i32 = *ptr;
    
    unsafe {
        let x: i32 = *ptr;  // ✅ UNSAFE: Explicit dereference
        return x;
    }
}
```

### Memory Management

- **Automatic reference counting** for managed objects
- **Explicit lifetime annotations** at FFI boundaries
- **No use-after-free** through ownership tracking
- **No double-free** through linear types for C resources

## Type Safety

### Strong Static Typing

- **No implicit conversions** between types
- **All variables must have explicit types** (no type inference)
- **Generic type parameters** with constraints
- **Algebraic data types** (enums with associated data)

```asthra
// ✅ Type annotations required
let count: i32 = 42;
let name: string = "Asthra";
let data: []u8 = [1, 2, 3];

// ❌ COMPILE ERROR: Missing type annotation
// let value = 42;
```

### Type Checking

- **Compile-time verification** of all type constraints
- **No runtime type errors** in safe code
- **Pattern matching** with exhaustiveness checking
- **Result<T, E>** and **Option<T>** for error handling

## Concurrency Safety

### Structured Concurrency Model

```asthra
// Spawn returns a handle - structured concurrency
pub fn concurrent_example(none) -> i32 {
    let handle: TaskHandle = spawn calculate_value();
    
    // Do other work...
    let local_result: i32 = 10;
    
    // Await ensures completion before proceeding
    let spawned_result: i32 = await handle;
    
    return spawned_result + local_result;
}
```

### Concurrency Guarantees

1. **No data races** in safe code
2. **Structured task lifecycle** through spawn/await
3. **Deterministic execution** by default
4. **Explicit non-determinism** with `#[non_deterministic]`

## FFI Safety

### Ownership Annotations

All FFI functions must declare ownership semantics:

```asthra
// Memory allocation - caller gets ownership
#[extern("C")]
#[transfer_full]
pub fn malloc(size: usize) -> *mut void;

// String length - borrowed reference
#[extern("C")]
#[borrowed]
pub fn strlen(str: *const char) -> usize;

// File operations - ownership retained
#[extern("C")]
#[transfer_none]
pub fn fwrite(buffer: *const void, size: usize, count: usize, file: *mut FILE) -> usize;
```

### FFI Safety Rules

1. **Mandatory annotations** for all extern functions
2. **Ownership transfer** explicitly declared
3. **Unsafe blocks required** for FFI calls
4. **Type marshaling** verified at compile time

## Initialization Safety

### No Uninitialized Variables

```asthra
// ❌ COMPILE ERROR: Uninitialized variable
// let x: i32;
// return x;

// ✅ All variables must be initialized
let x: i32 = 0;
let name: string = "";
let data: []u8 = [none];
```

### Definite Assignment Analysis

- **Compiler tracks** initialization state
- **All paths** must initialize variables
- **No partial initialization** allowed
- **Explicit `none`** for optional values

## Bounds Safety

### Array and Slice Bounds Checking

```asthra
pub fn bounds_safe_access(none) -> i32 {
    let array: [5]i32 = [10, 20, 30, 40, 50];
    
    // ✅ Safe indexing with runtime bounds check
    let value: i32 = array[2];  // Returns 30
    
    // 🔴 RUNTIME PANIC: Index out of bounds
    // let bad: i32 = array[10];
    
    // ✅ Safe slice operations
    let slice: []i32 = array[1:4];  // Elements [20, 30, 40]
    
    return len(slice);  // Returns 3
}
```

### Bounds Checking Features

1. **Runtime verification** of all array/slice access
2. **Compile-time checks** where possible
3. **Panic on violation** with clear error message
4. **Length tracking** for all slices

## Null Safety

### Option Type for Nullable Values

```asthra
// No null pointers - use Option<T>
pub fn find_user(id: i32) -> Option<User> {
    let user: User = lookup_user(id);
    if user.is_valid() {
        return Option::Some(user);
    }
    return Option::None;
}

// Pattern matching forces null handling
pub fn process_user(id: i32) -> string {
    let maybe_user: Option<User> = find_user(id);
    
    match maybe_user {
        Option::Some(user) => return user.name,
        Option::None => return "User not found"
    }
}
```

### Null Safety Guarantees

1. **No null pointer dereferences** in safe code
2. **Explicit Option type** for nullable values
3. **Pattern matching** enforces handling both cases
4. **No implicit null** values

## Unsafe Blocks

### Syntax and Semantics

```asthra
pub fn unsafe_operations_example(none) -> i32 {
    let mut value: i32 = 42;
    let ptr: *mut i32 = &value;
    
    unsafe {
        // Allowed in unsafe blocks:
        *ptr = 100;                    // Pointer dereference
        let buffer: *mut u8 = malloc(1024);  // FFI calls
        copy_memory(dest, src, len);   // Raw memory operations
    }
    
    return value;
}
```

### Unsafe Operations

Operations requiring unsafe blocks:
1. **Pointer dereferencing** (`*ptr`)
2. **FFI function calls**
3. **Type transmutation**
4. **Raw memory operations**
5. **Inline assembly** (if supported)

### Unsafe Block Principles

1. **Minimal scope** - Keep unsafe blocks small
2. **Clear documentation** - Comment safety invariants
3. **Local verification** - Safety provable within function
4. **No unsafe propagation** - Unsafe doesn't "leak" out

## Safety Validation

### Compile-Time Checks

1. **Type safety** - All type constraints verified
2. **Ownership rules** - No use-after-free or double-free
3. **Initialization** - All variables initialized before use
4. **Pattern exhaustiveness** - All cases handled
5. **Visibility rules** - Access control enforced

### Runtime Checks

1. **Bounds checking** - Array and slice access
2. **Overflow checking** - Integer operations (optional)
3. **Null checks** - Option unwrapping
4. **Assert conditions** - Debug assertions
5. **Panic handling** - Controlled failure

### Testing Safety

```asthra
// Safety-focused testing
#[test]
pub fn test_bounds_safety(none) -> void {
    let array: [3]i32 = [1, 2, 3];
    
    // Test valid access
    assert(array[0] == 1);
    assert(array[2] == 3);
    
    // Test bounds violation
    assert_panic(() => array[5]);
}
```

## Best Practices

### Writing Safe Code

1. **Minimize unsafe blocks** - Use only when necessary
2. **Document invariants** - Explain safety assumptions
3. **Encapsulate unsafe** - Wrap in safe abstractions
4. **Test edge cases** - Verify safety boundaries
5. **Use standard types** - Result, Option for errors

### Code Review Checklist

- [ ] All pointer dereferences in unsafe blocks?
- [ ] FFI functions have ownership annotations?
- [ ] No uninitialized variables?
- [ ] Array accesses bounds-checked?
- [ ] Option/Result used for nullable/fallible operations?
- [ ] Unsafe blocks documented with safety invariants?

## Comparison with Other Languages

| Feature | Asthra | Rust | C | Go |
|---------|--------|------|---|-----|
| Memory Safety | ✅ Ownership + GC | ✅ Ownership | ❌ Manual | ✅ GC |
| Null Safety | ✅ Option type | ✅ Option type | ❌ Null pointers | ❌ Nil |
| Type Safety | ✅ Strong static | ✅ Strong static | ⚠️ Weak | ✅ Strong static |
| Unsafe Blocks | ✅ Statement-level | ✅ Statement-level | N/A (all unsafe) | ✅ Package unsafe |
| Bounds Checking | ✅ Runtime | ✅ Runtime | ❌ None | ✅ Runtime |
| Concurrency Safety | ✅ Structured | ✅ Send/Sync | ❌ None | ✅ Channels |

## Future Enhancements

While Asthra's safety system is production-ready, potential future enhancements include:

1. **Lifetime inference** for common patterns
2. **Compile-time bounds checking** where possible
3. **Safe transmutation** patterns
4. **Verified unsafe** with formal proofs
5. **Safety lints** for common mistakes

## Conclusion

Asthra's safety system provides comprehensive protection against common programming errors while maintaining the control needed for systems programming. The design prioritizes:

- **Local reasoning** over global analysis
- **Explicit boundaries** over implicit rules
- **AI-friendly patterns** over complex inference
- **Practical safety** over theoretical purity

This approach ensures that both human developers and AI assistants can write safe, reliable code with confidence.