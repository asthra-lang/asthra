# Type System

**Version:** 1.6  
**Last Updated:** 2025-01-20  
**Change:** Promoted Option<T> to built-in type status alongside Result<T,E>

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
let condition: bool = false;

// String type (UTF-8)
let message: string = "Hello, World!";
let empty: string = "";

// Character type
let letter: char = 'A';
let newline: char = '\n';
let unicode: char = '🦀';

// Void type (for functions that don't return a value)
pub fn print_message(none) -> void {
    log("Hello from function");
    return ();  // v1.14+: Explicit unit return required
}

// Unit type () - represents "no value" for void returns
pub fn cleanup_resources(none) -> void {
    free_memory();
    close_files();
    return ();  // Unit type: explicit void return (v1.14+)
}
```

## Composite Types

### Tuple Types

Asthra provides comprehensive tuple type support for ordered collections of heterogeneous values with full pattern matching integration.

#### Basic Tuple Syntax

```asthra
// Tuple types - minimum 2 elements required
let pair: (i32, string) = (42, "hello");
let triple: (f64, bool, string) = (3.14, true, "pi");
let coords: (i32, i32) = (10, 20);

// Nested tuples
let nested: ((i32, i32), string) = ((1, 2), "coordinates");
let complex: (Result<i32, string>, Option<string>, bool) = 
    (Result.Ok(42), Option.Some("data"), true);

// Tuple with different types
let mixed: (u8, f64, bool, string) = (255, 3.14159, false, "mixed");
```

#### Tuple Pattern Matching

```asthra
// Destructuring in let statements
let (x, y): (i32, string) = pair;
let ((a, b), label): ((i32, i32), string) = nested;

// Pattern matching in match expressions
match pair {
    (0, _) => log("Zero value"),
    (n, s) => log("Value: " + n + ", " + s),
}

// Tuple patterns in function parameters
pub fn process_coordinate_pair(coords: (i32, i32)) -> f64 {
    let (x, y) = coords;
    return sqrt((x * x + y * y) as f64);
}

// Complex tuple pattern matching
pub fn handle_complex_tuple(data: (Result<i32, string>, Option<string>, bool)) -> string {
    match data {
        (Result.Ok(value), Option.Some(label), true) => {
            return "Success: " + label + " = " + value;
        },
        (Result.Err(error), _, _) => {
            return "Error: " + error;
        },
        (Result.Ok(value), Option.None, _) => {
            return "Unlabeled value: " + value;
        },
        (Result.Ok(value), Option.Some(label), false) => {
            return "Disabled: " + label + " = " + value;
        }
    }
}
```

#### Tuple Element Access (Planned)

```asthra
// Future: Direct element access by index
// let first = pair.0;   // Access first element
// let second = pair.1;  // Access second element
// let third = triple.2; // Access third element
```

#### Tuple Type Rules

1. **Minimum Elements**: Tuples must have at least 2 elements
2. **Type Safety**: All elements are statically typed
3. **Immutability**: Tuple elements follow standard mutability rules
4. **Pattern Matching**: Full destructuring support in all pattern contexts
5. **Nested Support**: Tuples can contain other tuples and complex types

### Never Type

The Never type represents computations that never return normally, enabling better control flow analysis and optimization.

#### Never Type Syntax

```asthra
// Functions that never return
pub fn panic(message: string) -> Never {
    log("PANIC: " + message);
    exit(1);
    // Compiler knows this function diverges
}

pub fn infinite_loop() -> Never {
    loop {
        process_events();
    }
    // Never reaches here
}

// External functions that never return
extern "C" fn exit(code: i32) -> Never;
extern "C" fn abort() -> Never;
```

#### Never Type in Complex Expressions

```asthra
// Never type as error branch in Result
pub fn safe_operation() -> Result<i32, Never> {
    // This operation cannot fail
    return Result.Ok(42);
}

// Never type in match expressions
pub fn handle_critical_error(error: CriticalError) -> i32 {
    match error.severity {
        Severity.Warning => return 0,
        Severity.Error => return -1,
        Severity.Fatal => panic("Fatal error occurred"), // Never returns
    }
    // Compiler knows all paths are covered
}

// Never type in function parameters (theoretical)
pub fn unreachable_function(never_param: Never) -> i32 {
    return 42;  // This function can never be called
}
```

#### Never Type Properties

1. **Bottom Type**: Never is compatible with any type (subtype of all types)
2. **Control Flow**: Enables exhaustiveness checking in match expressions
3. **Optimization**: Compiler can eliminate unreachable code after Never
4. **Type Safety**: Prevents accidental use of diverging functions
5. **FFI Integration**: Works with C functions that never return

### Fixed-Size Arrays

Asthra supports fixed-size arrays with compile-time known dimensions for performance-critical applications.

#### Fixed Array Syntax

```asthra
// Basic fixed-size array types
let buffer: [256]u8 = [0; 256];        // 256-byte buffer initialized to zero
let matrix: [3][4]i32 = [[0; 4]; 3];   // 3x4 matrix of integers
let lookup: [16]string = [""; 16];     // Array of 16 empty strings

// Const expressions for array sizes
const BUFFER_SIZE: usize = 1024;
const DOUBLE_SIZE: usize = BUFFER_SIZE * 2;

let large_buffer: [BUFFER_SIZE]u8 = [0; BUFFER_SIZE];
let huge_buffer: [DOUBLE_SIZE]u8 = [0; DOUBLE_SIZE];

// Complex element types
let points: [10]Point = [Point::default(); 10];
let results: [5]Result<i32, string> = [Result.Err("uninitialized"); 5];
```

#### Fixed Array Initialization

```asthra
// Repeated element syntax (Rust-style)
let zeros: [100]i32 = [0; 100];
let ones: [50]f64 = [1.0; 50];
let flags: [8]bool = [false; 8];

// Explicit element list (for small arrays)
let primes: [5]i32 = [2, 3, 5, 7, 11];
let colors: [3]string = ["red", "green", "blue"];

// Multi-dimensional initialization
let identity_matrix: [3][3]f64 = [
    [1.0, 0.0, 0.0],
    [0.0, 1.0, 0.0],
    [0.0, 0.0, 1.0]
];
```

#### Fixed Array Operations

```asthra
// Array indexing
let first_element: i32 = primes[0];
let last_element: i32 = primes[4];

// Matrix element access
let matrix_element: f64 = identity_matrix[1][2];

// Array slicing (creates slice from array)
let array_slice: []i32 = primes[:];      // Full array as slice
let partial_slice: []i32 = primes[1:4];  // Elements 1-3

// Array length (compile-time constant)
const PRIME_COUNT: usize = primes.len;   // Evaluates to 5
```

#### Fixed Array Type Rules

1. **Compile-Time Size**: Array size must be a compile-time constant
2. **Positive Size**: Array size must be greater than zero
3. **Stack Allocation**: Fixed arrays are typically stack-allocated
4. **Type Safety**: Element type is enforced throughout
5. **Memory Layout**: Elements are stored contiguously in memory
6. **FFI Compatible**: Fixed arrays map directly to C arrays

### Pointer Types

```asthra
// Immutable pointers
let const_ptr: *const i32 = &value;
let const_string_ptr: *const string = &text;

// Mutable pointers
let mut_ptr: *mut i32 = &mut value;
let mut_array_ptr: *mut []u8 = &mut buffer;

// Void pointers (for FFI)
let void_ptr: *mut void = unsafe { malloc(256) };
let const_void_ptr: *const void = data_ptr as *const void;
```

### Slice Types

```asthra
// Dynamic arrays (slices)
let numbers: []i32 = [1, 2, 3, 4, 5];
let bytes: []u8 = [0x48, 0x65, 0x6C, 0x6C, 0x6F]; // "Hello" in ASCII
let strings: []string = ["hello", "world", "asthra"];

// Empty slices (require explicit type annotation)
let empty_ints: []i32 = [none];
let empty_strings: []string = [none];

// Slice operations
let length: usize = numbers.len;    // Get slice length
let first_element: i32 = numbers[0]; // Index access

// Creating slices from fixed arrays
let fixed_array: [5]i32 = [1, 2, 3, 4, 5];
let full_slice: []i32 = fixed_array[:];     // Full array as slice
let partial: []i32 = fixed_array[1:4];      // Elements 1-3
```

### Struct Types

```asthra
// Struct declarations
pub struct Point {
    x: f64,
    y: f64,
}

pub struct User {
    id: i32,
    name: string,
    email: string,
    active: bool,
}

// Struct instantiation
let origin: Point = Point { x: 0.0, y: 0.0 };
let user: User = User {
    id: 1,
    name: "Alice",
    email: "alice@example.com",
    active: true
};
```

### Enum Types

Asthra provides comprehensive enum support with pattern matching integration and advanced variant types.

#### Basic Enum Declaration

```asthra
// Simple enum variants
pub enum Status {
    Pending,
    Processing,
    Complete,
    Failed
}

// Enum with associated data
pub enum Message {
    Text(string),
    Number(i32),
    Coordinate { x: i32, y: i32 },
    Quit
}
```

#### Built-in Generic Types

```asthra
// Result type for error handling (built-in language type)
Result<T, E>  // Built-in generic type with Ok(T) and Err(E) variants

// Option type for nullable values (built-in language type)  
Option<T>     // Built-in generic type with Some(T) and None variants

// Custom generic enums
pub enum Tree<T> {
    Leaf(T),
    Branch { left: Box<Tree<T>>, right: Box<Tree<T>> }
}
```

#### Enum Pattern Matching

```asthra
// Basic enum pattern matching
pub fn handle_status(status: Status) -> string {
    match status {
        Status.Pending => return "Waiting...",
        Status.Processing => return "Working...",
        Status.Complete => return "Done!",
        Status.Failed => return "Error occurred"
    }
}

// Pattern matching with data extraction
pub fn process_message(msg: Message) -> string {
    match msg {
        Message.Text(content) => return "Text: " + content,
        Message.Number(value) => return "Number: " + value,
        Message.Coordinate { x, y } => return "Point: (" + x + ", " + y + ")",
        Message.Quit => return "Goodbye!"
    }
}

// Result type usage
pub fn divide(a: f64, b: f64) -> Result<f64, string> {
    if b == 0.0 {
        return Result.Err("Division by zero");
    }
    return Result.Ok(a / b);
}

// Using Result types (built-in type)
let division_result: Result<f64, string> = divide(10.0, 2.0);
match division_result {
    Result.Ok(value) => {
        log("Result: " + value);
    },
    Result.Err(error) => {
        log("Error: " + error);
    }
}

// Using Option types (built-in type)
pub fn find_user(id: i32) -> Option<User> {
    if user_exists(id) {
        return Option.Some(get_user(id));
    }
    return Option.None;
}

let user_option: Option<User> = find_user(123);
match user_option {
    Option.Some(user) => {
        log("Found user: " + user.name);
    },
    Option.None => {
        log("User not found");
    }
}

// Generic enum pattern matching
pub fn unwrap_result<T, E>(result: Result<T, E>) -> T {
    match result {
        Result.Ok(value) => return value,
        Result.Err(error) => panic("Result contained error")
    }
}
```

#### Enum Construction

```asthra
// Simple variant construction
let status: Status = Status.Pending;

// Variant with associated data
let text_msg: Message = Message.Text("Hello, World!");
let coord_msg: Message = Message.Coordinate { x: 10, y: 20 };

// Built-in type construction
let success: Result<i32, string> = Result.Ok(42);
let failure: Result<i32, string> = Result.Err("Something went wrong");
let some_value: Option<string> = Option.Some("data");
let no_value: Option<string> = Option.None;
```

#### Enum Type Properties

1. **Tagged Unions**: Enums are implemented as tagged unions in C
2. **Type Safety**: Compiler ensures exhaustive pattern matching
3. **Memory Efficiency**: Optimal memory layout for variants
4. **Generic Support**: Full generic type parameter support
5. **Pattern Integration**: Deep integration with pattern matching system

## Generic Type System

### Generic Structs

```asthra
// Generic struct declaration
pub struct Vec<T> {
    data: *mut T,
    len: usize,
    capacity: usize,
}

// Multiple type parameters
pub struct HashMap<K, V> {
    buckets: *mut Bucket<K, V>,
    size: usize,
    capacity: usize,
}

// Generic instantiation
let numbers: Vec<i32> = Vec::new();
let strings: Vec<string> = Vec::new();
let map: HashMap<string, i32> = HashMap::new();

// Generic struct literals with type arguments
let point_data: Container<Point> = Container { 
    data: Point { x: 1.0, y: 2.0 },
    metadata: "point_data"
};
```

### Generic Enums

```asthra
// Generic instantiation
let maybe_number: Option<i32> = Option.Some(42);
let file_result: Result<string, IOError> = read_file("config.txt");
```

### Type Parameter Constraints

```asthra
// Basic type parameters (no constraints in v1.0)
pub struct Container<T> {
    value: T,
}

// Future: Type parameter constraints (planned)
// pub struct SortedList<T: Comparable> {
//     items: Vec<T>,
// }
```

### Generic Functions

```asthra
// Generic function declaration
pub fn identity<T>(value: T) -> T {
    return value;
}

// Multiple type parameters
pub fn pair<T, U>(first: T, second: U) -> (T, U) {
    return (first, second);
}

// Using generic functions
let number: i32 = identity(42);
let text: string = identity("hello");
let tuple: (i32, string) = pair(42, "hello");
```

### Generic Type Instantiation

```asthra
// Monomorphization - each generic use creates specialized code
let int_vec: Vec<i32> = Vec::new();     // Creates Vec_i32 in C code
let str_vec: Vec<string> = Vec::new();  // Creates Vec_string in C code

// Generic with multiple type parameters
let user_map: HashMap<string, User> = HashMap::new();  // Creates HashMap_string_User

// Nested generics
let vec_of_results: Vec<Result<i32, string>> = Vec::new();
let optional_map: Option<HashMap<string, i32>> = Option.Some(HashMap::new());
```

### Generic Implementation (impl blocks)

```asthra
// Generic implementation blocks
impl<T> Vec<T> {
    pub fn new() -> Vec<T> {
        return Vec {
            data: null as *mut T,
            len: 0,
            capacity: 0
        };
    }
    
    pub fn push(mut self, item: T) -> Vec<T> {
        // Implementation details...
        return self;
    }
    
    pub fn get(self, index: usize) -> Option<T> {
        if index < self.len {
            return Option.Some(unsafe { *self.data.offset(index) });
        }
        return Option.None;
    }
}

// Multiple type parameter implementation
impl<K, V> HashMap<K, V> {
    pub fn new() -> HashMap<K, V> {
        return HashMap {
            buckets: null as *mut Bucket<K, V>,
            size: 0,
            capacity: 16
        };
    }
    
    pub fn insert(mut self, key: K, value: V) -> HashMap<K, V> {
        // Implementation details...
        return self;
    }
}
```

## Type Inference System

### Variable Declaration Syntax

```asthra
// Both explicit and inferred declarations are supported
let explicit_var: string = "Hello, World!";  // Explicit type annotation
let inferred_var = "Hello, World!";          // Type inferred as string

// Function parameters and return types remain explicit (AI clarity)
pub fn process_data(input: []u8) -> Result<string, Error> {
    let parsed = parse_input(input);  // Type inferred from parse_input() return type
    let result: string = transform(parsed);  // Explicit when precision matters
    return Result.Ok(result);
}
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
let empty_list: []string = [none];
let empty_map: map[string]i32 = {};
```

### Pattern Matching Integration

```asthra
// Type inference in pattern matching
let file_result = fs.read_to_string("config.txt");
match file_result {
    Result.Ok(content) => {
        // 'content' type inferred as string from Result<string, fs.IOError>
        let lines = string.lines(content);  // []string inferred
        let count = lines.len;               // usize inferred
    },
    Result.Err(error) => {
        // 'error' type inferred as fs.IOError
        log("Failed to read file: " + error.message);
    }
}

// if-let with inference
if let Result.Ok(data) = parse_json(input) {
    // 'data' type inferred from parse_json return type
    let processed = transform_data(data);
}

// Tuple destructuring with inference
let coordinates = (10, 20);
let (x, y) = coordinates;  // x: i32, y: i32 inferred

// Struct pattern matching with inference
match user {
    User { name, age, .. } => {
        // name: string, age: i32 inferred from User struct
        log("User " + name + " is " + age + " years old");
    }
}
```

## Memory Management Types

### Memory Zones

Asthra provides a four-zone memory management system:

```asthra
// Memory zone annotations
#[ownership(gc)]     // Garbage collected (default)
pub struct GCData {
    value: i32,
}

#[ownership(c)]      // C-managed memory
pub struct CData {
    ptr: *mut void,
}

#[ownership(pinned)] // Pinned memory (no GC movement)
pub struct PinnedData {
    buffer: []u8,
}

// Stack allocation (automatic)
pub fn stack_example() -> void {
    let local_data: i32 = 42;  // Stack allocated
    return ();
}
```

### Ownership Transfer Types

```asthra
// FFI ownership transfer annotations
extern "libc" fn malloc(size: usize) -> #[transfer_full] *mut void;
extern "libc" fn free(#[transfer_full] ptr: *mut void) -> void;

// Borrowed references
extern "crypto" fn hash_data(#[borrowed] data: *const u8, len: usize) -> u64;

// No transfer (caller retains ownership)
extern "utils" fn validate_data(#[transfer_none] data: *const u8) -> bool;
```

### Memory Zone Semantics

```asthra
// GC Zone - automatic memory management
#[ownership(gc)]
pub struct UserData {
    name: string,        // GC-managed string
    scores: []i32,       // GC-managed array
    metadata: UserMeta,  // GC-managed struct
}

// C Zone - manual memory management
#[ownership(c)]
extern "libc" fn malloc(size: usize) -> *mut void;

#[ownership(c)]
pub struct CBuffer {
    data: *mut u8,       // C-managed pointer
    size: usize,         // Size tracking
    capacity: usize,     // Capacity tracking
}

// Pinned Zone - GC-observed but not moved
#[ownership(pinned)]
extern "hardware" fn get_dma_buffer() -> *mut u8;

#[ownership(pinned)]
pub struct HardwareBuffer {
    dma_ptr: *mut u8,    // Hardware-controlled memory
    interrupt_flag: bool, // Status flag
}

// Stack Zone - automatic scope-based cleanup
pub fn process_local_data() -> i32 {
    let stack_array: [1024]u8 = [0; 1024];  // Stack-allocated
    let local_count: i32 = 0;                // Stack-allocated
    
    // ... processing ...
    
    return local_count;  // Stack variables automatically cleaned up
}
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
```

## FFI and Explicit Type Requirements

```asthra
// FFI boundaries require explicit types for safety
extern "libc" fn malloc(size: usize) -> *mut void;

pub fn safe_allocate(size: usize) -> *mut u8 {
    // Explicit cast required for FFI safety
    let ptr: *mut void = unsafe { malloc(size) };
    return ptr as *mut u8;  // Explicit conversion
}

// C interop with explicit types
#[ownership(c)]
extern "my_c_lib" fn process_buffer(data: *const u8, len: usize) -> i32;

pub fn call_c_function(buffer: []u8) -> i32 {
    // Explicit types for C boundary
    let data_ptr: *const u8 = buffer.ptr;
    let data_len: usize = buffer.len;
    return unsafe { process_buffer(data_ptr, data_len) };
}
```

## Error Handling and Inference

```asthra
// Inference works well with Result types
pub fn process_config(none) -> Result<Config, string> {
    let file_content = fs.read_to_string("config.toml");  // Result<string, fs.IOError>
    
    match file_content {
        Result.Ok(content) => {
            let config = parse_toml(content);  // Result<Config, ParseError> inferred
            return config.map_err(|e| "Parse error: " + e.message);
        },
        Result.Err(io_error) => {
            return Result.Err("IO error: " + io_error.message);
        }
    }
}
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
pub fn process_user_data(user_id: i32) -> Result<UserProfile, DatabaseError> {
    let user_query = db.prepare_statement("SELECT * FROM users WHERE id = ?");
    let result = db.execute_query(user_query, user_id);
    let user_data = match result {
        Result.Ok(data) => data,
        Result.Err(e) => return Result.Err(e)
    };  // Type inferred from execute_query return
    
    // Explicit when business logic precision matters
    let account_balance: f64 = parse_currency(user_data.balance);
    let last_login: i64 = parse_timestamp(user_data.last_login);
    
    // Simple inference for local processing
    let profile = UserProfile {
        id: user_data.id,
        name: user_data.name,
        balance: account_balance,
        last_seen: last_login,
    };
    
    return Result.Ok(profile);
}

// ❌ Avoid complex inference that's hard for AI to predict
pub fn complex_inference() {
    let data = some_function()  // Type depends on complex analysis
        .chain(another_function())
        .collect();  // Hard for AI to determine final type
}
```

### Type Annotation Guidelines

```asthra
// ✅ Explicit types for important boundaries
pub fn api_handler(request: HttpRequest) -> HttpResponse {
    // ✅ Explicit types for business logic
    let user_id: i32 = parse_user_id(request.params["id"])?;
    let account_balance: f64 = get_account_balance(user_id)?;
    
    // ✅ Inference OK for local processing
    let response_data = ResponseData {
        user_id: user_id,
        balance: account_balance,
        timestamp: current_time(),
    };
    
    return HttpResponse.json(response_data);
}

// ✅ Explicit types for FFI boundaries
extern "libc" fn read(fd: i32, buf: *mut u8, count: usize) -> isize;

pub fn safe_read(fd: i32, buffer: []u8) -> Result<usize, IOError> {
    let bytes_read: isize = unsafe { 
        read(fd, buffer.ptr as *mut u8, buffer.len) 
    };
    
    if bytes_read < 0 {
        return Result.Err(IOError.ReadFailed);
    }
    
    return Result.Ok(bytes_read as usize);
}
```

## Type Checking Rules

### Compile-Time Type Checking

1. **Function Signatures**: All parameters and return types must be explicitly declared
2. **Struct Fields**: All fields must have explicit type annotations
3. **FFI Boundaries**: All extern function parameters and returns must be explicit
4. **Pattern Matching**: Types must be compatible with pattern variants
5. **Array Access**: Index must be integer type, result type matches element type
6. **Generic Instantiation**: Type arguments must match parameter count and constraints

### Runtime Type Behavior

1. **No Runtime Type Information**: Types are erased after compilation (except for debugging)
2. **Static Dispatch**: All function calls resolved at compile time
3. **Memory Layout**: Types determine memory layout and alignment
4. **Safety Checks**: Bounds checking for arrays in debug builds
5. **Generic Monomorphization**: Each generic instantiation creates separate code

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

// Generic type compatibility
let vec_i32: Vec<i32> = Vec::new();
let vec_u32: Vec<u32> = Vec::new();
// vec_i32 = vec_u32;  // ❌ Error: Vec<i32> != Vec<u32>

// Tuple compatibility
let pair1: (i32, string) = (42, "hello");
let pair2: (i32, string) = pair1;  // ✅ Same tuple type
// let pair3: (string, i32) = pair1;  // ❌ Error: different order
```

## Advanced Type Features

### Type Aliases (Planned)

```asthra
// Future: Type aliases for complex types
// type UserId = i32;
// type UserMap = HashMap<UserId, User>;
// type Result<T> = Result<T, Error>;  // Generic type alias

// Using type aliases
// let user_id: UserId = 12345;
// let users: UserMap = HashMap::new();
// let operation_result: Result<string> = process_data();
```

### Associated Types (Planned)

```asthra
// Future: Associated types in traits
// trait Iterator {
//     type Item;
//     fn next(self) -> Option<Self::Item>;
// }
```

### Higher-Ranked Types (Future)

```asthra
// Future: Higher-ranked types for advanced generics
// fn for_all<F>(f: F) where F: for<T> Fn(T) -> T {
//     // Function that works with any type T
// }
```

## Implementation Notes

### Memory Layout

- **Primitive Types**: Follow standard C ABI layout
- **Structs**: Fields laid out in declaration order with proper alignment
- **Tuples**: Elements stored contiguously with proper alignment
- **Fixed Arrays**: Elements stored contiguously in declaration order
- **Slices**: Fat pointers with pointer and length fields
- **Enums**: Tagged unions with discriminant field

### Performance Characteristics

- **Zero-Cost Abstractions**: Type system has no runtime overhead
- **Compile-Time Resolution**: All type checking done at compile time
- **Efficient Representation**: Types map directly to efficient machine representations
- **Cache-Friendly**: Struct layout optimized for cache performance
- **Monomorphization**: Generic types specialized for each use

### Integration Points

- **Pattern Matching**: Types enable exhaustive pattern checking
- **FFI Safety**: Type annotations ensure safe C interoperability
- **Memory Management**: Types integrate with ownership system
- **Error Handling**: Result types provide type-safe error handling
- **Concurrency**: Types work seamlessly with spawn/await system
