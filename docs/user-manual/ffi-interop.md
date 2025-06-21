# C Interoperability (FFI)

Asthra's FFI is designed for direct, safe, and zero-cost interaction with C, including support for variadic functions and comprehensive ownership annotations.

**⚠️ IMPORTANT:** Asthra v1.10 introduces a critical safety fix for FFI annotations. The old system that allowed multiple conflicting annotations like `#[transfer_full] #[transfer_none]` has been replaced with a safer single-annotation system. See the [FFI Annotation Migration Guide](../FFI_ANNOTATION_MIGRATION_GUIDE.md) for upgrade instructions.

## Declaring Foreign Functions (`extern`)

Use `extern "library_name"` to declare C functions. Include FFI annotations for parameters and return types for proper memory management.

```asthra
extern "libc" fn printf(format: *const u8, ...) -> i32; // Variadic function support

extern "libcrypto" fn AES_encrypt(
    #[borrowed] in_data: *const u8, 
    out_data: *mut u8, // Assume out_data is allocated by caller, so borrowed by default
    #[borrowed] key: *const u8, 
    nr: i32
) -> void;
```

## The `unsafe " +  + "` Block

Interactions with C code or operations Asthra can't guarantee memory safety for must be in an `unsafe " +  + "` block. This makes the boundary between safe Asthra code and potentially unsafe C operations explicit.

```asthra
pub fn safe_wrapper_example() -> Result<void, string> {let data: []u8 = [1, 2, 3, 4, 5];
    
    // Safe Asthra operations
    let length: usize = data.len;
    
    // Unsafe C operations must be wrapped
    unsafe " + 
        let c_result: i32 = some_c_function(data.ptr, length);
        match c_result {
            0 => return Result.Ok(void),
            _ => return Result.Err("C function failed")
         + 
}
```

## Enhanced Data Marshaling

### Primitive Types (Automatic)

Basic types (int, float, bool, pointers, etc.) are automatically marshaled by value with no conversion overhead.

```asthra
extern "libc" fn abs(n: i32) -> i32;

pub fn use_primitive_marshaling() -> i32 {let negative_number: i32 = -42;
    let result: i32 = unsafe { abs(negative_number)  + "; // Direct marshaling
    return result; // 42
}
```

### Enhanced String Operations

Asthra strings are immutable UTF-8. C strings are typically `char*`. Asthra provides runtime functions for safe conversion.

```asthra
pub fn safe_string_operations(text: string) -> Result<void, string> {unsafe " + 
        // Method 1: Direct conversion for simple cases
        let c_text: *mut u8 = Asthra_runtime.string_to_cstr(text, false);
        libc.puts(c_text as *const u8);
        
        // Method 2: Using string concatenation
        let formatted: string = "Processing: " + text;
        let c_formatted: *mut u8 = Asthra_runtime.string_to_cstr(formatted, false);
        libc.puts(c_formatted as *const u8);
        
        return Result.Ok(void);
     + 
```

### String Conversion Best Practices

```asthra
pub fn comprehensive_string_example() -> Result<void, string> "Hello from Asthra";
    
    unsafe " + 
        // Convert to C string
        let c_string: *const u8 = Asthra_runtime.string_to_cstr(asthra_string, false);
        
        // Use with C function
        let result: i32 = libc.puts(c_string);
        
        // Convert C string back to Asthra (if needed)
        let back_to_asthra: string = Asthra_runtime.cstr_to_string(c_string);
        
        match result >= 0 " + 
            true => return Result.Ok(void),
            false => return Result.Err("puts() failed")
         + 
}
```

### Formalized Slices (`[]Type`) and C Arrays

Asthra slices provide a formalized structure with pointer and length. When interacting with C functions expecting raw pointers and lengths, slices are ideal.

```asthra
extern "my_c_lib" fn process_bytes(data_ptr: *mut u8, len: usize) -> i32;

pub fn process_slice_in_c(data_slice: []u8) -> Result<void, string> {unsafe " + 
        // The compiler handles passing the correct pointer and length from the slice
        let result: i32 = process_bytes(data_slice.ptr, data_slice.len);
        
        match result {
            0 => return Result.Ok(void),
            _ => return Result.Err("C function failed with code: " + result)
         + 
}
```

### Creating Asthra Slices from C Data

```asthra
// Creating an Asthra slice from C data with error handling
pub fn get_data_from_c() -> Result<[]u8, string> {let c_ptr: *mut u8 = unsafe { libc.malloc(100) as *mut u8  + ";
    if c_ptr == (0 as *mut u8) "Memory allocation failed");
    }
    
    let length: usize = 100;
    // ... fill c_ptr with data ...

    // Create slice from C data with ownership tracking
    let asthra_slice: []u8 = unsafe " + 
        Asthra_runtime.slice_from_raw_parts(c_ptr, length, true, ownership_c)
     + ";
    
    return Result.Ok(asthra_slice);
}
```

### Structs (`#[repr(C)]`, `#[repr(packed)]`)

Use `#[repr(C)]` for C-compatible layout or `#[repr(packed)]` to minimize padding.

```asthra
#[repr(C)]
pub struct Point " + 
    x: f64;
    y: f64;
 + "

#[repr(packed)]
pub struct PackedData " + 
    flag: u8;
    value: u32;
 + "

extern "graphics_lib" fn draw_point(#[borrowed] point: *const Point) -> i32;

pub fn use_c_struct(void) -> Result<void, string> {let point: Point = Point " +  x: 10.0, y: 20.0  + ";
    
    unsafe " + 
        let result: i32 = draw_point(&point as *const Point);
        match result {
            0 => return Result.Ok(void),
            _ => return Result.Err("Drawing failed")
         + 
}
```

## Enhanced Ownership and Lifetime Annotations in FFI

**Memory Management System Clarification**: Asthra's current FFI annotation system **already provides comprehensive coverage** for all practical memory safety scenarios. Our design intentionally differs from Rust-style complexity to optimize for AI code generation reliability while maintaining complete safety guarantees.

### Current System Completeness

Our grammar **already supports** all essential FFI memory management patterns:

**✅ Parameter Annotations** (all contexts supported):
```asthra
pub fn process_data(#[borrowed] input: *const u8, #[transfer_full] output: *mut u8) " +  ...  + "
extern "C" fn free(#[transfer_full] ptr: *mut void);
extern "C" fn strlen(#[borrowed] s: *const u8) -> usize;
```

**✅ Return Type Annotations** (transfer semantics supported):
```asthra
extern "C" fn malloc(size: usize) -> #[transfer_full] *mut void;
extern "C" fn getenv(name: *const u8) -> #[transfer_none] *mut u8;
pub fn create_buffer() -> #[transfer_full] *mut u8 " +  ...  + "
```

**✅ Variable Declarations** (ownership strategy supported):
```asthra
let file: #[ownership(c)] FileHandle = open_file("data.bin");
let buffer: #[ownership(pinned)] []u8 = allocate_secure_buffer();
```

**Why This Design Is Superior for AI Generation:**
- **Clear Separation**: Ownership strategy (`#[ownership(...)]`) separate from transfer semantics (`#[transfer_full]`, etc.)
- **Local Reasoning**: AI can make decisions based on immediate context, not global program analysis
- **Predictable Patterns**: Each annotation has single, unambiguous meaning
- **Conflict Prevention**: Grammar prevents dangerous combinations like `#[transfer_full] #[transfer_none]`

These annotations on function/FFI parameters and return types are crucial for specifying memory ownership and lifetime rules across the FFI boundary.

### `#[transfer_full]`
- **On return type**: The callee returns a pointer, and ownership is fully transferred to the Asthra caller
- **On parameter**: Asthra passes a pointer to C, and C takes full ownership

### `#[transfer_none]`
- **On return type**: The callee returns a pointer, but ownership is *not* transferred
- **On parameter**: Asthra passes a pointer to C, but Asthra retains ownership

### `#[borrowed]`
- **Typically on parameters**: Signifies the parameter is a temporary borrow with lifetime tied to the call scope

### Safety Rules (v1.10)

**✅ ALLOWED: Single, clear annotations**
```asthra
// C function returns memory that Asthra now owns
extern "libc" fn strdup(s: *const u8) -> #[transfer_full] *mut u8;

// C function borrows memory from Asthra
extern "libc" fn puts(#[borrowed] s: *const u8) -> i32;

// Clear parameter semantics
pub fn process_buffer(#[transfer_full] buffer: *mut u8) " +  /* takes ownership */  + "
pub fn read_buffer(#[borrowed] buffer: *const u8) " +  /* borrows temporarily */  + "
```

**❌ PREVENTED: Multiple conflicting annotations (compilation error)**
```asthra
// These patterns are now compilation errors in v1.10:
// extern fn bad_malloc() -> #[transfer_full] #[transfer_none] *mut u8;
// fn bad_function(#[borrowed] #[transfer_full] ptr: *mut u8) " +   + "
// extern fn bad_process(#[transfer_full] #[transfer_none] data: *mut u8) -> i32;
```

pub fn use_strdup() -> Result<void, string> {let original: string = "Test me";
    let c_original: *const u8 = Asthra_runtime.string_to_cstr(original, false);
    
    let duplicated_c_str: *mut u8 #[ownership(c)] = unsafe { 
        strdup(c_original) 
     + ";

    match duplicated_c_str == (0 as *mut u8) "strdup failed");
        },
        false => " + 
            // ... use duplicated_c_str ...
            unsafe " +  libc.free(duplicated_c_str as *mut void);  + "
            return Result.Ok(void);
         + "
    }
}
```

### Complex Ownership Example

```asthra
extern "my_lib" fn #[transfer_full] create_buffer(size: usize) -> *mut u8;
extern "my_lib" fn process_buffer(#[borrowed] buffer: *mut u8, size: usize) -> i32;
extern "my_lib" fn destroy_buffer(#[transfer_full] buffer: *mut u8);

pub fn complex_ownership_example() -> Result<void, string> {// Get ownership of buffer from C
    let buffer: *mut u8 #[ownership(c)] = unsafe " +  create_buffer(1024)  + ";
    
    match buffer == (0 as *mut u8) " + 
        true => return Result.Err("Buffer creation failed"),
        false => {
            // Borrow buffer to C for processing
            let result: i32 = unsafe { process_buffer(buffer, 1024)  + ";
            
            // Transfer ownership back to C for cleanup
            unsafe " +  destroy_buffer(buffer)  + ";
            
            match result "Processing failed")
             + "
        }
    }
}
```

## Human Review for FFI Code

Foreign function interface code often requires human review due to memory safety concerns, complex C library interactions, and potential security vulnerabilities.

### FFI Review Guidelines

```asthra
#[human_review(low, context = "ffi")]
extern "libc" fn strlen(s: *const u8) -> usize;

#[human_review(medium, context = "ffi")]
pub fn safe_c_string_wrapper(input: string) -> Result<usize, string> {// Medium review for FFI wrappers with safety checks
    let c_str: *const u8 = input.as_ptr();
    let length: usize = unsafe " +  strlen(c_str)  + ";
    return Result.Ok(length);
 + "

#[human_review(high, context = "ffi")]
pub fn complex_c_integration(data: []u8) -> Result<[]u8, string> {// High-risk operations require careful review
    // Complex memory management and error handling
    let output_buffer: *mut u8 = unsafe " +  malloc(data.len * 2)  + ";
    
    if output_buffer.is_null() " + 
        return Result.Err("Memory allocation failed");
     + "
    
    let result: i32 = unsafe " + 
        complex_c_operation(data.ptr, data.len, output_buffer)
     + ";
    
    if result != 0 " + 
        unsafe { free(output_buffer)  + ";
        return Result.Err("C operation failed");
     + "
    
    let output_slice: []u8 = unsafe " +  slice_from_raw_parts(output_buffer, data.len * 2)  + ";
    return Result.Ok(output_slice);
}
```

### FFI Annotation Combinations

Combine human review with FFI-specific annotations:

```asthra
#[human_review(medium, context = "ffi")]
extern "crypto_lib" fn encrypt_data(
    #[transfer_none] input: *const u8,
    input_len: usize,
    #[transfer_full] output: *mut u8,
    #[borrowed] key: *const u8
) -> i32;

#[human_review(high, context = "ffi")]
pub fn secure_encryption_wrapper(plaintext: []u8, key: []u8) -> Result<[]u8, string> {// High-priority review for security-critical FFI
    let output_size: usize = plaintext.len + 16; // Add padding
    let output_buffer: *mut u8 = unsafe " +  malloc(output_size)  + ";
    
    if output_buffer.is_null() " + 
        return Result.Err("Memory allocation failed");
     + "
    
    let result: i32 = unsafe " + 
        encrypt_data(plaintext.ptr, plaintext.len, output_buffer, key.ptr)
     + ";
    
    if result != 0 " + 
        unsafe { free(output_buffer)  + ";
        return Result.Err("Encryption failed");
     + "
    
    let encrypted: []u8 = unsafe " +  slice_from_raw_parts(output_buffer, output_size)  + ";
    return Result.Ok(encrypted);
}
```

### Review Priority Guidelines

- **Simple C Library Calls**: `low` priority
- **Memory Management Operations**: `medium` to `high` priority
- **Security Library Integration**: `high` priority
- **Custom C Code Integration**: `high` priority
- **Unsafe Memory Operations**: `high` priority

## Enhanced Error Handling Across FFI

When calling C functions, check C error codes and convert them into Asthra's `Result<T, E>` type using pattern matching.

```asthra
extern "libc" fn open(path: *const u8, flags: i32) -> i32;
extern "libc" fn get_errno() -> i32;

pub fn open_file_asthra(path_str: string) -> Result<i32, string> {let c_path: *const u8 = Asthra_runtime.string_to_cstr(path_str, false);
    let fd: i32 = unsafe " +  open(c_path, 0)  + "; // O_RDONLY

    match fd < 0 " + 
        true => {
            let err_code: i32 = unsafe { get_errno()  + ";
            return Result.Err("Failed to open file. Errno: " + err_code);
         + ",
        false => " + 
            return Result.Ok(fd);
         + "
    }
}
```

### Comprehensive Error Handling Pattern

```asthra
pub fn robust_c_interaction() -> Result<string, string> {// Multiple C operations with comprehensive error handling
    let file_result: Result<i32, string> = open_file_asthra("/tmp/test.txt");
    
    match file_result " + 
        Result.Ok(fd) => {
            // File opened successfully, now read from it
            let read_result: Result<[]u8, string> = read_from_fd(fd);
            
            // Always close the file
            unsafe { libc.close(fd)  + ";
            
            match read_result " + 
                Result.Ok(data) => {
                    let content: string = Asthra_runtime.bytes_to_string(data);
                    return Result.Ok(content);
                 + ",
                Result.Err(read_error) => "Read failed: " + read_error);
                 + "
            }
        },
        Result.Err(open_error) => " + 
            return Result.Err("Open failed: " + open_error);
         + "
    }
}
```

## Variadic Function Handling

Asthra provides safe handling of variadic C functions through wrapper patterns:

```asthra
// Variadic function declaration
extern "libc" fn printf(format: *const u8, ...) -> i32;

// Safe wrapper using Asthra's type system
pub fn safe_printf(format: string, args: []Variant) -> Result<i32, string> {unsafe " + 
        let c_format: *const u8 = Asthra_runtime.string_to_cstr(format, false);
        // Variadic argument marshaling through runtime
        let result: i32 = Asthra_runtime.call_variadic_printf(c_format, args.ptr, args.len);
        
        match result < 0 {
            true => return Result.Err("printf failed"),
            false => return Result.Ok(result)
         + 
}
```

### Practical Variadic Usage

```asthra
pub fn variadic_example() -> Result<void, string> "Alice";
    let age: int = 30;
    let score: float = 95.5;
    
    // Create variant array for arguments
    let args: []Variant = [
        Variant.String(name),
        Variant.Int(age),
        Variant.Float(score)
    ];
    
    let result: Result<i32, string> = safe_printf("Name: %s, Age: %d, Score: %.1f\n", args);
    
    match result " + 
        Result.Ok(chars_written) => " + 
            log("INFO", "Printf wrote " + chars_written + " characters");
            return Result.Ok(void);
         + ",
        Result.Err(error) => "Printf failed: " + error);
         + "
    }
}
```

## FFI Best Practices

### Safety Guidelines

1. **Always use `unsafe` blocks** for C function calls
2. **Check return values** and convert to `Result` types
3. **Manage memory ownership explicitly** with annotations
4. **Validate pointers** before dereferencing
5. **Handle C errors systematically** with pattern matching

### Performance Tips

1. **Minimize string conversions** between Asthra and C
2. **Use slices for bulk data transfer** instead of individual elements
3. **Prefer borrowed parameters** when C doesn't need ownership
4. **Cache C string conversions** when calling functions repeatedly

### Common Patterns

```asthra
// Pattern 1: Simple C function call with error checking
pub fn simple_c_call() -> Result<void, string> {let result: i32 = unsafe { some_c_function()  + ";
    match result " + 
        0 => Result.Ok(void),
        _ => Result.Err("Function failed with code: " + result)
     + "
}

// Pattern 2: C function with input/output parameters
pub fn c_function_with_io(input: []u8) -> Result<[]u8, string> {let output_buffer: *mut u8 = unsafe " +  libc.malloc(input.len) as *mut u8  + ";
    
    match output_buffer == (0 as *mut u8) "Memory allocation failed"),
        false => " + 
            let result: i32 = unsafe { 
                process_data(input.ptr, output_buffer, input.len) 
             + ";
            
            match result " + 
                0 => {
                    let output_slice: []u8 = unsafe {
                        Asthra_runtime.slice_from_raw_parts(output_buffer, input.len, true, ownership_c)
                     + ";
                    return Result.Ok(output_slice);
                 + ",
                _ => " + 
                    unsafe { libc.free(output_buffer as *mut void)  + ";
                    return Result.Err("Processing failed");
                }
            }
        }
    }
}
```

## Package-Based FFI Organization

With Asthra's **Go-style package system**, you can organize FFI code into dedicated packages for better maintainability and reusability across projects.

### FFI Package Structure

```
crypto-project/
├── Asthra.toml
├── src/
│   └── main.asthra      # package main
├── crypto/
│   ├── openssl.asthra   # package crypto - OpenSSL bindings
│   ├── sodium.asthra    # package crypto - libsodium bindings
│   └── common.asthra    # package crypto - Common crypto utilities
├── database/
│   ├── sqlite.asthra    # package database - SQLite bindings
│   └── postgres.asthra  # package database - PostgreSQL bindings
└── system/
    ├── libc.asthra      # package system - Standard C library
    └── posix.asthra     # package system - POSIX system calls
```

### Example: Crypto Package

```asthra
// crypto/openssl.asthra
package crypto;

// OpenSSL function declarations
extern "openssl" fn EVP_sha256() -> *const void;
extern "openssl" fn EVP_DigestInit_ex(
    ctx: *mut void,
    type_: *const void,
    impl_: *mut void
) -> i32;

#[human_review(high, context = "crypto_ffi")]
pub fn sha256_init() -> Result<*mut void, string> {
    let ctx: *mut void = unsafe { EVP_MD_CTX_new() };
    if ctx.is_null() {
        return Result.Err("Failed to create digest context");
    }
    
    let result: i32 = unsafe {
        EVP_DigestInit_ex(ctx, EVP_sha256(), 0 as *mut void)
    };
    
    match result {
        1 => Result.Ok(ctx),
        _ => {
            unsafe { EVP_MD_CTX_free(ctx) };
            Result.Err("Failed to initialize SHA256")
        }
    }
}
```

```asthra
// crypto/common.asthra
package crypto;

import "system/libc";

pub struct CryptoError {
    pub code: i32,
    pub message: string,
}

pub fn handle_crypto_error(code: i32) -> CryptoError {
    let message: string = match code {
        -1 => "Invalid input parameters",
        -2 => "Memory allocation failed",
        -3 => "Cryptographic operation failed",
        _ => "Unknown crypto error: " + string(code),
    };
    
    return CryptoError {
        code: code,
        message: message,
    };
}

#[transfer_full]
pub fn secure_zero(data: *mut u8, len: usize) -> void {
    unsafe {
        libc.memset(data as *mut void, 0, len);
        // Compiler barrier to prevent optimization
        asm_volatile_memory_barrier();
    }
}
```

### Example: Database Package

```asthra
// database/sqlite.asthra
package database;

extern "sqlite3" fn sqlite3_open(
    filename: *const u8,
    #[transfer_full] ppDb: *mut *mut void
) -> i32;

extern "sqlite3" fn sqlite3_close(db: *mut void) -> i32;
extern "sqlite3" fn sqlite3_errmsg(db: *mut void) -> *const u8;

pub struct SqliteConnection {
    handle: *mut void,
}

#[human_review(medium, context = "database_ffi")]
pub fn open_database(path: string) -> Result<SqliteConnection, string> {
    let c_path: *const u8 = string_to_cstr(path);
    let db_handle: *mut void = 0 as *mut void;
    
    let result: i32 = unsafe {
        sqlite3_open(c_path, &mut db_handle as *mut *mut void)
    };
    
    match result {
        0 => Result.Ok(SqliteConnection { handle: db_handle }),
        _ => {
            let error_msg: *const u8 = unsafe { sqlite3_errmsg(db_handle) };
            let error_str: string = unsafe { cstr_to_string(error_msg) };
            unsafe { sqlite3_close(db_handle) };
            Result.Err("SQLite error: " + error_str)
        }
    }
}
```

### Main Application Integration

```asthra
// src/main.asthra
package main;

import "crypto";
import "database";
import "system";

pub fn main() -> i32 {
    // Use crypto package
    let hash_result: Result<*mut void, string> = crypto.sha256_init();
    let crypto_ctx: *mut void = match hash_result {
        Result.Ok(ctx) => ctx,
        Result.Err(error) => {
            system.log_error("Crypto initialization failed: " + error);
            return 1;
        }
    };
    
    // Use database package
    let db_result: Result<database.SqliteConnection, string> = 
        database.open_database("app.db");
    let db_conn: database.SqliteConnection = match db_result {
        Result.Ok(conn) => conn,
        Result.Err(error) => {
            system.log_error("Database connection failed: " + error);
            return 1;
        }
    };
    
    system.log_info("Application initialized successfully");
    return 0;
}
```

### Package-Based FFI Benefits

1. **Separation of Concerns**: Each package handles one library or domain
2. **Reusability**: FFI packages can be shared across projects
3. **Testing**: Each package can have dedicated test files
4. **Maintenance**: Updates to one library don't affect others
5. **Documentation**: Package-level documentation for each FFI domain

### FFI Package Best Practices

1. **One Library Per Package**: Keep SQLite, OpenSSL, etc. in separate packages
2. **Common Utilities**: Create shared utility functions in each package
3. **Error Handling**: Standardize error types within each package
4. **Memory Management**: Use consistent ownership patterns per package
5. **Documentation**: Document FFI safety requirements per package

**For comprehensive package organization patterns, see [Package Organization](package-organization.md)**

## Next Steps

Now that you understand FFI, explore:

- **[Security Features](security.md)** - Learn about secure FFI practices
- **[Memory Management](memory-management.md)** - Understand how FFI integrates with memory management
- **[Building Projects](building-projects.md)** - See how to configure C library dependencies 
