# Type System

**Version:** 1.3  
**Last Updated:** 2024-12-28

Asthra implements a comprehensive type system with Go-style simple type inference optimized for AI code generation. The design prioritizes predictability and local reasoning over complex global type analysis.

## Core Principles

1\. **Predictable**: Same input always produces same inferred type
2\. **Local**: Inference doesn't require global analysis  
3\. **Explicit Boundaries**: Function signatures, struct fields, and FFI remain explicit
4\. **Fail Fast**: When inference is ambiguous, require explicit annotation
5\. **AI-Friendly**: Optimized for code generation tools

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
pub fn print_message(none) -&gt; void &#123;
    log("Hello from function");
    return ();  // v1.14+: Explicit unit return required
&#125;

// Unit type () - represents " // TODO: Convert string interpolation to concatenation // TODO: Convert string interpolation to concatenationno value" for void returns
pub fn cleanup_resources(none) -&gt; void &#123;
    free_memory();
    close_files();
    return ();  // Unit type: explicit void return (v1.14+)
&#125;
```

## Composite Types

### Pointer Types

```asthra
// Immutable pointers
let const_ptr: *const i32 = &value;
let const_string_ptr: *const string = &text;

// Mutable pointers
let mut_ptr: *mut i32 = &amp;mut value;
let mut_array_ptr: *mut &#91;&#93;u8 = &amp;mut buffer;

// Void pointers (for FFI)
let void_ptr: *mut void = unsafe &#123; malloc(256) &#125;;
let const_void_ptr: *const void = data_ptr as *const void;
```

### Slice Types

```asthra
// Dynamic arrays (slices)
let numbers: &#91;&#93;i32 = &#91;1, 2, 3, 4, 5&#93;;
let bytes: &#91;&#93;u8 = &#91;0x48, 0x65, 0x6C, 0x6C, 0x6F&#93;; // "Hello" in ASCII
let strings: &#91;&#93;string = &#91;"hello", "world", "asthra"&#93;;

// Empty slices (require explicit type annotation)
let empty_ints: &#91;&#93;i32 = &#91;&#93;;
let empty_strings: &#91;&#93;string = &#91;&#93;;

// Slice operations
let length: usize = numbers.len;    // Get slice length
let first_element: i32 = numbers&#91;0&#93;; // Index access
```

### Struct Types

```asthra
// Struct declarations
pub struct Point &#123;
    x: f64;
    y: f64;
&#125;

pub struct User &#123;
    id: i32;
    name: string;
    email: string;
    active: bool;
&#125;

// Struct instantiation
let origin: Point = Point &#123; x: 0.0, y: 0.0 &#125;;
let user: User = User &#123;
    id: 1,
    name: "Alice",
    email: "alice@example.com",
    active: true
&#125;;
```

### Result Types

```asthra
// Result type for error handling
pub enum Result&lt;T, E&gt; &#123;
    Ok(T),
    Err(E)
&#125;

// Function returning Result
pub fn divide(a: f64, b: f64) -&gt; Result&lt;f64, string&gt; &#123;
    if b == 0.0 &#123;
        return Result.Err("Division by zero");
    &#125;
    return Result.Ok(a / b);
&#125;

// Using Result types
let division_result: Result&lt;f64, string&gt; = divide(10.0, 2.0);
match division_result &#123;
    Result.Ok(value) =&gt; &#123;
        log("Result: " + value);
    &#125;,
    Result.Err(error) =&gt; &#123;
        log(" // TODO: Convert string interpolation to concatenation // TODO: Convert string interpolation to concatenationError: " + error);
    &#125;
&#125;
```

## Type Inference System

### Variable Declaration Syntax

```asthra
// Both explicit and inferred declarations are supported
let explicit_var: string = "Hello, World!";  // Explicit type annotation
let inferred_var = "Hello, World!";          // Type inferred as string

// Function parameters and return types remain explicit (AI clarity)
pub fn process_data(input: &#91;&#93;u8) -&gt; Result&lt;string, Error&gt; &#123;
    let parsed = parse_input(input);  // Type inferred from parse_input() return type
    let result: string = transform(parsed);  // Explicit when precision matters
    return Result.Ok(result);
&#125;
```

### Literal Type Inference Rules

```asthra
// Default type mappings (predictable for AI)
let integer = 42;        // i32 (default integer type)
let float = 3.14;        // f64 (default floating-point type)
let text = "hello";      // string
let flag = true;         // bool
let bytes = &#91;1, 2, 3&#93;;   // &#91;&#93;i32 (array of default integers)

// Explicit override when default isn't suitable
let small_num: i16 = 42;           // Override default i32
let precise: f32 = 3.14;           // Override default f64
let unsigned: u32 = 100;           // Explicit unsigned type
let byte_array: &#91;&#93;u8 = &#91;1, 2, 3&#93;;  // Explicit for byte operations
```

### Function Return Type Inference

```asthra
// Type inferred from function signatures
let file_content = fs.read_to_string("file.txt");  // Result&lt;string, fs.IOError&gt;
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
let numbers = &#91;1, 2, 3&#93;;          // &#91;&#93;i32
let floats = &#91;1.0, 2.0, 3.14&#93;;    // &#91;&#93;f64
let names = &#91;"Alice", "Bob"&#93;;      // &#91;&#93;string

// Mixed types require explicit annotation
let mixed: &#91;&#93;interface{} = &#91;1, "hello", true&#93;;  // Explicit when heterogeneous

// Empty collections require type annotation
let empty_list: &#91;&#93;string = &#91;&#93;;
let empty_map: map&#91;string&#93;i32 = {};
```

### Pattern Matching Integration

```asthra
// Type inference in pattern matching
let file_result = fs.read_to_string("config.txt");
match file_result &#123;
    Result.Ok(content) =&gt; &#123;
        // 'content' type inferred as string from Result&lt;string, fs.IOError&gt;
        let lines = string.lines(content);  // &#91;&#93;string inferred
        let count = lines.len;               // usize inferred
    &#125;,
    Result.Err(error) =&gt; &#123;
        // 'error' type inferred as fs.IOError
        log(" // TODO: Convert string interpolation to concatenation // TODO: Convert string interpolation to concatenationFailed to read file: " + error.message);
    &#125;
&#125;

// if-let with inference
if let Result.Ok(data) = parse_json(input) &#123;
    // 'data' type inferred from parse_json return type
    let processed = transform_data(data);
&#125;
```

## FFI and Explicit Type Requirements

```asthra
// FFI boundaries require explicit types for safety
extern "libc" fn malloc(size: usize) -&gt; *mut void;

pub fn safe_allocate(size: usize) -&gt; *mut u8 &#123;
    // Explicit cast required for FFI safety
    let ptr: *mut void = unsafe &#123; malloc(size) &#125;;
    return ptr as *mut u8;  // Explicit conversion
&#125;

// C interop with explicit types
#&#91;ownership(c)&#93;
extern " // TODO: Convert string interpolation to concatenation // TODO: Convert string interpolation to concatenationmy_c_lib" fn process_buffer(data: *const u8, len: usize) -&gt; i32;

pub fn call_c_function(buffer: &#91;&#93;u8) -&gt; i32 &#123;
    // Explicit types for C boundary
    let data_ptr: *const u8 = buffer.ptr;
    let data_len: usize = buffer.len;
    return unsafe &#123; process_buffer(data_ptr, data_len) &#125;;
&#125;
```

## Error Handling and Inference

```asthra
// Inference works well with Result types
pub fn process_config(none) -&gt; Result&lt;Config, string&gt; &#123;
    let file_content = fs.read_to_string("config.toml");  // Result&lt;string, fs.IOError&gt;
    
    match file_content &#123;
        Result.Ok(content) =&gt; &#123;
            let config = parse_toml(content);  // Result&lt;Config, ParseError&gt; inferred
            return config.map_err(|e| "Parse error: " + e.message);
        &#125;,
        Result.Err(io_error) =&gt; &#123;
            return Result.Err(" // TODO: Convert string interpolation to concatenation // TODO: Convert string interpolation to concatenationIO error: " + io_error.message);
        &#125;
    &#125;
&#125;
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
let safe_convert: Result&lt;u8, ConversionError&gt; = int_val.try_into();
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
pub fn process_user_data(user_id: i32) -&gt; Result&lt;UserProfile, DatabaseError&gt; &#123;
    let user_query = db.prepare_statement("SELECT * FROM users WHERE id = ?");
    let result = db.execute_query(user_query, user_id);
    let user_data = match result &#123;
        Result.Ok(data) =&gt; data,
        Result.Err(e) =&gt; return Result.Err(e)
    &#125;;  // Type inferred from execute_query return
    
    // Explicit when business logic precision matters
    let account_balance: f64 = parse_currency(user_data.balance);
    let last_login: i64 = parse_timestamp(user_data.last_login);
    
    // Simple inference for local processing
    let profile = UserProfile &#123;
        id: user_data.id,
        name: user_data.name,
        balance: account_balance,
        last_seen: last_login,
    &#125;;
    
    return Result.Ok(profile);
&#125;

// ❌ Avoid complex inference that's hard for AI to predict
pub fn complex_inference() &#123;
    let data = some_function()  // Type depends on complex analysis
        .chain(another_function())
        .collect();  // Hard for AI to determine final type
&#125;
```

### Type Annotation Guidelines

```asthra
// ✅ Explicit types for important boundaries
pub fn api_handler(request: HttpRequest) -&gt; HttpResponse &#123;
    // ✅ Explicit types for business logic
    let user_id: i32 = parse_user_id(request.params&#91;"id"&#93;)?;
    let account_balance: f64 = get_account_balance(user_id)?;
    
    // ✅ Inference OK for local processing
    let response_data = ResponseData &#123;
        user_id: user_id,
        balance: account_balance,
        timestamp: current_time(),
    &#125;;
    
    return HttpResponse.json(response_data);
&#125;

// ✅ Explicit types for FFI boundaries
extern " // TODO: Convert string interpolation to concatenation // TODO: Convert string interpolation to concatenationlibc" fn read(fd: i32, buf: *mut u8, count: usize) -&gt; isize;

pub fn safe_read(fd: i32, buffer: &#91;&#93;u8) -&gt; Result&lt;usize, IOError&gt; &#123;
    let bytes_read: isize = unsafe &#123; 
        read(fd, buffer.ptr as *mut u8, buffer.len) 
    &#125;;
    
    if bytes_read &lt; 0 &#123;
        return Result.Err(IOError.ReadFailed);
    &#125;
    
    return Result.Ok(bytes_read as usize);
&#125;
```

## Type Checking Rules

### Compile-Time Type Checking

1\. **Function Signatures**: All parameters and return types must be explicitly declared
2\. **Struct Fields**: All fields must have explicit type annotations
3\. **FFI Boundaries**: All extern function parameters and returns must be explicit
4\. **Pattern Matching**: Types must be compatible with pattern variants
5\. **Array Access**: Index must be integer type, result type matches element type

### Runtime Type Behavior

1\. **No Runtime Type Information**: Types are erased after compilation (except for debugging)
2\. **Static Dispatch**: All function calls resolved at compile time
3\. **Memory Layout**: Types determine memory layout and alignment
4\. **Safety Checks**: Bounds checking for arrays in debug builds

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
let data: &#91;&#93;u8 = &#91;1, 2, 3&#93;;
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
extern "C" fn malloc(size: usize) -&gt; #&#91;transfer_full&#93; *mut void;
extern "C" fn free(#&#91;transfer_full&#93; ptr: *mut void);

// ✅ BORROWED REFERENCES (NO OWNERSHIP TRANSFER)
extern "C" fn strlen(#&#91;borrowed&#93; str: *const char) -&gt; usize;
extern "C" fn memcpy(#&#91;borrowed&#93; dest: *mut void, #&#91;borrowed&#93; src: *const void, n: usize);

// ✅ NO TRANSFER (CALLER RETAINS OWNERSHIP)
extern "C" fn validate_buffer(#&#91;transfer_none&#93; data: *const u8, len: usize) -&gt; bool;

// ✅ SAFE FFI WRAPPER FUNCTIONS
pub fn safe_malloc(size: usize) -&gt; Result&lt;*mut u8, AllocationError&gt; &#123;
    let ptr: *mut void = unsafe &#123; malloc(size) &#125;;  // #&#91;transfer_full&#93; annotation
    if ptr.is_null() &#123;
        return Result.Err(AllocationError.OutOfMemory);
    &#125;
    return Result.Ok(ptr as *mut u8);
&#125;

pub fn safe_free(ptr: *mut u8) -&gt; void &#123;
    unsafe &#123; free(ptr as *mut void); &#125;  // #&#91;transfer_full&#93; annotation
&#125;
```

**Ownership Strategy Annotations**:
```asthra
// ✅ VARIABLE-LEVEL OWNERSHIP CONTROL
let gc_managed: #&#91;ownership(gc)&#93; DataStruct = create_managed_data();
let c_allocated: #&#91;ownership(c)&#93; *mut u8 = unsafe &#123; malloc(1024) &#125;;
let stack_local: #&#91;ownership(stack)&#93; LocalData = LocalData::new();

// ✅ FUNCTION PARAMETER OWNERSHIP
pub fn process_data(
    #&#91;ownership(gc)&#93; managed_input: DataStruct,
    #&#91;ownership(c)&#93; c_buffer: *mut u8,
    #&#91;borrowed&#93; temp_data: *const ProcessingContext
) -&gt; #&#91;transfer_full&#93; *mut Result &#123;
    // Function body with clear ownership semantics
&#125;
```

**Two-Level System Benefits**:
1\. **Orthogonal Concerns**: Ownership strategy (where memory comes from) separate from transfer semantics (who owns it now)
2\. **AI-Friendly Patterns**: Each annotation has single, clear meaning
3\. **Local Reasoning**: No need for global program analysis
4\. **Practical Coverage**: Handles all real-world FFI scenarios

### **Why Optional FFI Annotations are Superior**

**Mandatory vs Optional Design Philosophy**:
```asthra
// ✅ AI-FRIENDLY: Annotations only where needed
pub fn process_user_data(user_id: i32) -&gt; Result&lt;UserProfile, DatabaseError&gt; &#123;
    // No annotations needed - pure Asthra code
    let query = build_query(user_id);
    let result = execute_query(query);
    return parse_result(result);
&#125;

// ✅ AI-FRIENDLY: Explicit annotations at FFI boundaries
extern "C" fn sqlite3_exec(
    db: *mut sqlite3,
    #&#91;borrowed&#93; sql: *const char,
    callback: Option&lt;extern "C" fn(*mut void, i32, *mut *mut char, *mut *mut char) -&gt; i32>,
    #&#91;transfer_none&#93; callback_arg: *mut void,
    #&#91;transfer_full&#93; errmsg: *mut *mut char
) -&gt; i32;

// ❌ AVOID: Annotation noise in pure Asthra code
pub fn calculate_distance(p1: Point, p2: Point) -&gt; f64 &#123;
    // No FFI annotations needed - this is pure Asthra
    let dx = p1.x - p2.x;
    let dy = p1.y - p2.y;
    return sqrt(dx * dx + dy * dy);
&#125;
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
pub fn process<T>(input: T) -&gt; Result&lt;T, Error&gt; {
    // Generic function body
}
```

### Type Aliases (Future)

```asthra
// Planned: Type alias declarations
type UserId = i32;
type UserName = string;
type ProcessingResult = Result&lt;ProcessedData, ProcessingError&gt;;

pub fn process_user(id: UserId, name: UserName) -&gt; ProcessingResult {
    // Function using type aliases
}
```

### Trait System (Future)

```asthra
// Planned: Trait definitions for shared behavior
trait Serializable {
    fn serialize(self) -&gt; &#91;&#93;u8;
    fn deserialize(data: &#91;&#93;u8) -&gt; Result&lt;Self, Error&gt;;
}

impl Serializable for User {
    fn serialize(self) -&gt; &#91;&#93;u8 {
        // Serialization implementation
    }
    
    fn deserialize(data: &#91;&#93;u8) -&gt; Result&lt;User, Error&gt; {
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
