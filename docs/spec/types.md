# Type System

**Version:** 1.3  
**Last Updated:** 2024-12-28

Asthra implements a comprehensive type system with Go-style simple type inference optimized for AI code generation. The design prioritizes predictability and local reasoning over complex global type analysis.

## Core Principles

1. **Predictable**: Same input always produces same inferred type
2. **Local**: Inference doesn't require global analysis  
3. **Explicit Boundaries**: Function signatures, struct fields, and FFI remain explicit
4. **Fail Fast**: When inference is ambiguous, require explicit annotation
5. **AI-Friendly**: Optimized for code generation tools

## Primitive Types

### Integer Types

```asthra
// Signed integers
let small: i8 = -128;          // 8-bit signed (-128 to 127)
let short: i16 = -32768;       // 16-bit signed (-32,768 to 32,767)
let standard: i32 = -2147483648; // 32-bit signed (default integer type)
let large: i64 = -9223372036854775808; // 64-bit signed

// Unsigned integers  
let byte: u8 = 255;            // 8-bit unsigned (0 to 255)
let word: u16 = 65535;         // 16-bit unsigned (0 to 65,535)
let dword: u32 = 4294967295;   // 32-bit unsigned (0 to 4,294,967,295)
let qword: u64 = 18446744073709551615; // 64-bit unsigned

// Platform-dependent sizes
let size: usize = 1024;        // Pointer-sized unsigned integer
let signed_size: isize = -512; // Pointer-sized signed integer
```

### Floating-Point Types

```asthra
// Floating-point numbers
let precise: f32 = 3.14159;    // 32-bit IEEE 754 float
let double: f64 = 2.718281828; // 64-bit IEEE 754 double (default float type)

// Scientific notation
let large_float: f64 = 1.23e10;
let small_float: f64 = 4.56e-5;
```

### Other Primitive Types

```asthra
// Boolean type
let flag: bool = true;
let condition: bool = : bool = ;

// String type (UTF-8)
let message: string = "Hello, World!";
let empty: string = "";

// Character type
let letter: char = 'A';
let newline: char = '\n';
let unicode: char = '🦀';

// Void type (for functions that don't return a value)
pub fn print_message(none) -> void " + (
    log("Hello from function");
    return ();  // v1.14+: Explicit unit return required
) + "

// Unit type () - represents " // TODO: Convert string interpolation to concatenation // TODO: Convert string interpolation to concatenationno value" for void returns
pub fn cleanup_resources(none) -> void " + (
    free_memory();
    close_files();
    return ();  // Unit type: explicit void return (v1.14+)
) + "
```

## Composite Types

### Pointer Types

```asthra
// Immutable pointers
let const_ptr: *const i32 = &value;
let const_string_ptr: *const string = &text;

// Mutable pointers
let mut_ptr: *mut i32 = &mut value;
let mut_array_ptr: *mut []u8 = &mut buffer;

// Void pointers (for FFI)
let void_ptr: *mut void = unsafe " + ( malloc(256) ) + ";
let const_void_ptr: *const void = data_ptr as *const void;
```

### Slice Types

```asthra
// Dynamic arrays (slices)
let numbers: []i32 = [1, 2, 3, 4, 5];
let bytes: []u8 = [0x48, 0x65, 0x6C, 0x6C, 0x6F]; // "Hello" in ASCII
let strings: []string = ["hello", "world", "asthra"];

// Empty slices (require explicit type annotation)
let empty_ints: []i32 = [];
let empty_strings: []string = [];

// Slice operations
let length: usize = numbers.len;    // Get slice length
let first_element: i32 = numbers[0]; // Index access
```

### Struct Types

```asthra
// Struct declarations
pub struct Point " + (
    x: f64;
    y: f64;
) + "

pub struct User " + (
    id: i32;
    name: string;
    email: string;
    active: bool;
) + "

// Struct instantiation
let origin: Point = Point " + ( x: 0.0, y: 0.0 ) + ";
let user: User = User " + (
    id: 1,
    name: "Alice",
    email: "alice@example.com",
    active: true
) + ";
```

### Tuple Types

```asthra
// Tuple types - ordered collections of heterogeneous values
let pair: (i32, string) = (42, "hello");
let triple: (f64, bool, string) = (3.14, true, "pi");
let nested: ((i32, i32), string) = ((1, 2), "coordinates");

// Tuple destructuring in let statements
let (x, y): (i32, string) = pair;
let ((a, b), label): ((i32, i32), string) = nested;

// Minimum 2 elements required for tuples
// let single: (i32) = (42);  // Error: Not a tuple, use parentheses for grouping
// let unit: () = ();         // This is the unit type, not a tuple

// Tuple pattern matching
match pair {
    (0, _) => log("Zero value"),
    (n, s) => log("Value: " + n + ", " + s),
}

// Note: Tuple element access syntax (.0, .1, etc) not yet implemented
// Future: let first = pair.0;  // Would access first element
```

### Result Types

```asthra
// Result type for error handling
pub enum Result<T, E> " + (
    Ok(T),
    Err(E)
) + "

// Function returning Result
pub fn divide(a: f64, b: f64) -> Result<f64, string> " + (
    if b == 0.0 " + (
        return Result.Err("Division by zero");
    ) + "
    return Result.Ok(a / b);
) + "

// Using Result types
let division_result: Result<f64, string> = divide(10.0, 2.0);
match division_result " + (
    Result.Ok(value) => " + (
        log("Result: " + value);
    ) + ",
    Result.Err(error) => " + (
        log(" // TODO: Convert string interpolation to concatenation // TODO: Convert string interpolation to concatenationError: " + error);
    ) + "
) + "
```

## Type Inference System

### Variable Declaration Syntax

```asthra
// Both explicit and inferred declarations are supported
let explicit_var: string = "Hello, World!";  // Explicit type annotation
let inferred_var = "Hello, World!";          // Type inferred as string

// Function parameters and return types remain explicit (AI clarity)
pub fn process_data(input: []u8) -> Result<string, Error> " + (
    let parsed = parse_input(input);  // Type inferred from parse_input() return type
    let result: string = transform(parsed);  // Explicit when precision matters
    return Result.Ok(result);
) + "
```

### Literal Type Inference Rules

```asthra
// Default type mappings (predictable for AI)
let integer = 42;        // i32 (default integer type)
let float = 3.14;        // f64 (default floating-point type)
let text = "hello";      // string
let flag = true;         // bool
let bytes = [1, 2, 3];   // []i32 (array of default integers)

// Explicit override when default isn't suitable
let small_num: i16 = 42;           // Override default i32
let precise: f32 = 3.14;           // Override default f64
let unsigned: u32 = 100;           // Explicit unsigned type
let byte_array: []u8 = [1, 2, 3];  // Explicit for byte operations
```

### Function Return Type Inference

```asthra
// Type inferred from function signatures
let file_content = fs.read_to_string("file.txt");  // Result<string, fs.IOError>
let trimmed = string.trim("  text  ");             // string
let length = string.len("hello");                  // usize

// Works with method chaining
let processed = string.trim("  data  ")
    .to_uppercase()
    .replace("DATA", "INFO");  // string (final result type)
```

### Collection Type Inference

```asthra
// Array inference with homogeneous types
let numbers = [1, 2, 3];          // []i32
let floats = [1.0, 2.0, 3.14];    // []f64
let names = ["Alice", "Bob"];      // []string

// Mixed types require explicit annotation
let mixed: []interface{} = [1, "hello", true];  // Explicit when heterogeneous

// Empty collections require type annotation
let empty_list: []string = [];
let empty_map: map[string]i32 = {};
```

### Pattern Matching Integration

```asthra
// Type inference in pattern matching
let file_result = fs.read_to_string("config.txt");
match file_result " + (
    Result.Ok(content) => " + (
        // 'content' type inferred as string from Result<string, fs.IOError>
        let lines = string.lines(content);  // []string inferred
        let count = lines.len;               // usize inferred
    ) + ",
    Result.Err(error) => " + (
        // 'error' type inferred as fs.IOError
        log(" // TODO: Convert string interpolation to concatenation // TODO: Convert string interpolation to concatenationFailed to read file: " + error.message);
    ) + "
) + "

// if-let with inference
if let Result.Ok(data) = parse_json(input) " + (
    // 'data' type inferred from parse_json return type
    let processed = transform_data(data);
) + "
```

## FFI and Explicit Type Requirements

```asthra
// FFI boundaries require explicit types for safety
extern "libc" fn malloc(size: usize) -> *mut void;

pub fn safe_allocate(size: usize) -> *mut u8 " + (
    // Explicit cast required for FFI safety
    let ptr: *mut void = unsafe " + ( malloc(size) ) + ";
    return ptr as *mut u8;  // Explicit conversion
) + "

// C interop with explicit types
#[ownership(c)]
extern " // TODO: Convert string interpolation to concatenation // TODO: Convert string interpolation to concatenationmy_c_lib" fn process_buffer(data: *const u8, len: usize) -> i32;

pub fn call_c_function(buffer: []u8) -> i32 " + (
    // Explicit types for C boundary
    let data_ptr: *const u8 = buffer.ptr;
    let data_len: usize = buffer.len;
    return unsafe " + ( process_buffer(data_ptr, data_len) ) + ";
) + "
```

## Error Handling and Inference

```asthra
// Inference works well with Result types
pub fn process_config(none) -> Result<Config, string> " + (
    let file_content = fs.read_to_string("config.toml");  // Result<string, fs.IOError>
    
    match file_content " + (
        Result.Ok(content) => " + (
            let config = parse_toml(content);  // Result<Config, ParseError> inferred
            return config.map_err(|e| "Parse error: " + e.message);
        ) + ",
        Result.Err(io_error) => " + (
            return Result.Err(" // TODO: Convert string interpolation to concatenation // TODO: Convert string interpolation to concatenationIO error: " + io_error.message);
        ) + "
    ) + "
) + "
```

## Type Conversion and Casting

### Explicit Type Casting

```asthra
// Explicit type casting
let int_val: i32 = 42;
let float_val: f64 = int_val as f64;            // Integer to float
let byte_val: u8 = int_val as u8;               // Narrowing conversion
let ptr_val: *const void = ptr as *const void;  // Pointer casting

// Safe conversions (planned)
let safe_convert: Result<u8, ConversionError> = int_val.try_into();
```

### Conversion Rules

- Explicit casting uses `as` keyword (planned for implementation)
- Narrowing conversions may truncate values
- Widening conversions preserve values
- Pointer casts require `unsafe` context
- Safe conversions return `Result` types

### Implicit Conversions

```asthra
// Very limited implicit conversions for safety
let literal_int = 42;      // Can be used as any integer type in some contexts
let specific: i64 = literal_int;  // Allowed: literal fits in target type

// No implicit conversions between different-sized types
let small: i16 = 100;
let large: i32 = small;    // ❌ Error: requires explicit cast
let large: i32 = small as i32;  // ✅ Explicit cast required
```

## AI Code Generation Benefits

### Predictable Patterns

```asthra
// ✅ AI can easily generate - clear and predictable
pub fn process_user_data(user_id: i32) -> Result<UserProfile, DatabaseError> " + (
    let user_query = db.prepare_statement("SELECT * FROM users WHERE id = ?");
    let result = db.execute_query(user_query, user_id);
    let user_data = match result " + (
        Result.Ok(data) => data,
        Result.Err(e) => return Result.Err(e)
    ) + ";  // Type inferred from execute_query return
    
    // Explicit when business logic precision matters
    let account_balance: f64 = parse_currency(user_data.balance);
    let last_login: i64 = parse_timestamp(user_data.last_login);
    
    // Simple inference for local processing
    let profile = UserProfile " + (
        id: user_data.id,
        name: user_data.name,
        balance: account_balance,
        last_seen: last_login,
    ) + ";
    
    return Result.Ok(profile);
) + "

// ❌ Avoid complex inference that's hard for AI to predict
pub fn complex_inference() " + (
    let data = some_function()  // Type depends on complex analysis
        .chain(another_function())
        .collect();  // Hard for AI to determine final type
) + "
```

### Type Annotation Guidelines

```asthra
// ✅ Explicit types for important boundaries
pub fn api_handler(request: HttpRequest) -> HttpResponse " + (
    // ✅ Explicit types for business logic
    let user_id: i32 = parse_user_id(request.params["id"])?;
    let account_balance: f64 = get_account_balance(user_id)?;
    
    // ✅ Inference OK for local processing
    let response_data = ResponseData " + (
        user_id: user_id,
        balance: account_balance,
        timestamp: current_time(),
    ) + ";
    
    return HttpResponse.json(response_data);
) + "

// ✅ Explicit types for FFI boundaries
extern " // TODO: Convert string interpolation to concatenation // TODO: Convert string interpolation to concatenationlibc" fn read(fd: i32, buf: *mut u8, count: usize) -> isize;

pub fn safe_read(fd: i32, buffer: []u8) -> Result<usize, IOError> " + (
    let bytes_read: isize = unsafe " + ( 
        read(fd, buffer.ptr as *mut u8, buffer.len) 
    ) + ";
    
    if bytes_read < 0 " + (
        return Result.Err(IOError.ReadFailed);
    ) + "
    
    return Result.Ok(bytes_read as usize);
) + "
```

## Type Checking Rules

### Compile-Time Type Checking

1. **Function Signatures**: All parameters and return types must be explicitly declared
2. **Struct Fields**: All fields must have explicit type annotations
3. **FFI Boundaries**: All extern function parameters and returns must be explicit
4. **Pattern Matching**: Types must be compatible with pattern variants
5. **Array Access**: Index must be integer type, result type matches element type

### Runtime Type Behavior

1. **No Runtime Type Information**: Types are erased after compilation (except for debugging)
2. **Static Dispatch**: All function calls resolved at compile time
3. **Memory Layout**: Types determine memory layout and alignment
4. **Safety Checks**: Bounds checking for arrays in debug builds

### Type Compatibility

```asthra
// Compatible types
let int32: i32 = 42;
let int32_copy: i32 = int32;  // ✅ Same type

// Incompatible types (require explicit conversion)
let int16: i16 = 100;
let int32: i32 = int16;       // ❌ Error: type mismatch
let int32: i32 = int16 as i32; // ✅ Explicit conversion

// Pointer compatibility
let data: []u8 = [1, 2, 3];
let ptr: *const u8 = data.ptr;     // ✅ Compatible
let void_ptr: *const void = ptr as *const void; // ✅ Explicit cast
```

## FFI Safety and Ownership System

### **AI-First FFI Safety Philosophy**

**Why Our FFI Model is Superior for AI Generation**:

External analyses sometimes suggest our FFI safety annotations are "incomplete" or should be mandatory everywhere. This misunderstands our **pragmatic AI-friendly approach**:

**Our Design Priorities**:
- ✅ **Explicit at Boundaries**: FFI annotations required only where memory crosses language boundaries
- ✅ **Local Reasoning**: AI can understand transfer semantics without global analysis
- ✅ **Fail-Safe Defaults**: Missing annotations default to safe garbage collection
- ✅ **Clear Intent**: Each annotation has single, unambiguous meaning

**What We Deliberately Avoid**:
- ❌ **Annotation Everywhere**: Rust-style lifetime parameters that clutter simple code
- ❌ **Complex Propagation**: Automatic annotation inference that confuses AI models
- ❌ **Academic Completeness**: Theoretical purity over practical effectiveness
- ❌ **Global Analysis**: Whole-program reasoning that breaks AI generation reliability

### **Comprehensive FFI Safety Capabilities**

**Transfer Semantics Annotations**:
```asthra
// ✅ EXPLICIT OWNERSHIP TRANSFER
extern "C" fn malloc(size: usize) -> #[transfer_full] *mut void;
extern "C" fn free(#[transfer_full] ptr: *mut void);

// ✅ BORROWED REFERENCES (NO OWNERSHIP TRANSFER)
extern "C" fn strlen(#[borrowed] str: *const char) -> usize;
extern "C" fn memcpy(#[borrowed] dest: *mut void, #[borrowed] src: *const void, n: usize);

// ✅ NO TRANSFER (CALLER RETAINS OWNERSHIP)
extern "C" fn validate_buffer(#[transfer_none] data: *const u8, len: usize) -> bool;

// ✅ SAFE FFI WRAPPER FUNCTIONS
pub fn safe_malloc(size: usize) -> Result<*mut u8, AllocationError> " + (
    let ptr: *mut void = unsafe " + ( malloc(size) ) + ";  // #[transfer_full] annotation
    if ptr.is_null() " + (
        return Result.Err(AllocationError.OutOfMemory);
    ) + "
    return Result.Ok(ptr as *mut u8);
) + "

pub fn safe_free(ptr: *mut u8) -> void " + (
    unsafe " + ( free(ptr as *mut void); ) + "  // #[transfer_full] annotation
) + "
```

**Ownership Strategy Annotations**:
```asthra
// ✅ VARIABLE-LEVEL OWNERSHIP CONTROL
let gc_managed: #[ownership(gc)] DataStruct = create_managed_data();
let c_allocated: #[ownership(c)] *mut u8 = unsafe " + ( malloc(1024) ) + ";
let stack_local: #[ownership(stack)] LocalData = LocalData::new();

// ✅ FUNCTION PARAMETER OWNERSHIP
pub fn process_data(
    #[ownership(gc)] managed_input: DataStruct,
    #[ownership(c)] c_buffer: *mut u8,
    #[borrowed] temp_data: *const ProcessingContext
) -> #[transfer_full] *mut Result " + (
    // Function body with clear ownership semantics
) + "
```

**Two-Level System Benefits**:
1. **Orthogonal Concerns**: Ownership strategy (where memory comes from) separate from transfer semantics (who owns it now)
2. **AI-Friendly Patterns**: Each annotation has single, clear meaning
3. **Local Reasoning**: No need for global program analysis
4. **Practical Coverage**: Handles all real-world FFI scenarios

### **Why Optional FFI Annotations are Superior**

**Mandatory vs Optional Design Philosophy**:
```asthra
// ✅ AI-FRIENDLY: Annotations only where needed
pub fn process_user_data(user_id: i32) -> Result<UserProfile, DatabaseError> " + (
    // No annotations needed - pure Asthra code
    let query = build_query(user_id);
    let result = execute_query(query);
    return parse_result(result);
) + "

// ✅ AI-FRIENDLY: Explicit annotations at FFI boundaries
extern "C" fn sqlite3_exec(
    db: *mut sqlite3,
    #[borrowed] sql: *const char,
    callback: Option<extern "C" fn(*mut void, i32, *mut *mut char, *mut *mut char) -> i32>,
    #[transfer_none] callback_arg: *mut void,
    #[transfer_full] errmsg: *mut *mut char
) -> i32;

// ❌ AVOID: Annotation noise in pure Asthra code
pub fn calculate_distance(p1: Point, p2: Point) -> f64 " + (
    // No FFI annotations needed - this is pure Asthra
    let dx = p1.x - p2.x;
    let dy = p1.y - p2.y;
    return sqrt(dx * dx + dy * dy);
) + "
```

**Benefits of Optional Annotations**:
- 🎯 **Reduced Noise**: Clean code for pure Asthra functions
- 🤖 **AI Focus**: AI attention directed to actual FFI boundaries
- 📚 **Clear Intent**: Annotations signal "this crosses language boundaries"
- 🔍 **Maintainability**: Easy to identify FFI interaction points

## Advanced Type Features

### Generic Types (Future)

```asthra
// Planned: Generic struct definitions
pub struct Container<T> {
    value: T;
    count: usize;
}

// Planned: Generic function definitions
pub fn process<T>(input: T) -> Result<T, Error> {
    // Generic function body
}
```

### Type Aliases (Future)

```asthra
// Planned: Type alias declarations
type UserId = i32;
type UserName = string;
type ProcessingResult = Result<ProcessedData, ProcessingError>;

pub fn process_user(id: UserId, name: UserName) -> ProcessingResult {
    // Function using type aliases
}
```

### Trait System (Future)

```asthra
// Planned: Trait definitions for shared behavior
trait Serializable {
    fn serialize(self) -> []u8;
    fn deserialize(data: []u8) -> Result<Self, Error>;
}

impl Serializable for User {
    fn serialize(self) -> []u8 {
        // Serialization implementation
    }
    
    fn deserialize(data: []u8) -> Result<User, Error> {
        // Deserialization implementation
    }
}
```

## Implementation Notes

### Memory Layout

- **Primitive Types**: Follow standard C ABI layout
- **Structs**: Fields laid out in declaration order with proper alignment
- **Slices**: Fat pointers with pointer and length fields
- **Enums**: Tagged unions with discriminant field

### Performance Characteristics

- **Zero-Cost Abstractions**: Type system has no runtime overhead
- **Compile-Time Resolution**: All type checking done at compile time
- **Efficient Representation**: Types map directly to efficient machine representations
- **Cache-Friendly**: Struct layout optimized for cache performance

### Integration Points

- **Pattern Matching**: Types enable exhaustive pattern checking
- **FFI Safety**: Type annotations ensure safe C interoperability
- **Memory Management**: Types integrate with ownership system
- **Error Handling**: Result types provide type-safe error handling 
