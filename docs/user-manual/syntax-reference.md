# Syntax Reference

This comprehensive reference documents all syntax elements in the Asthra programming language, organized by category for quick lookup and systematic learning.

## Table of Contents

- [Keywords](#keywords)
- [Operators](#operators)  
- [Type Syntax](#type-syntax)
- [Literals](#literals)
- [Pattern Matching](#pattern-matching)
- [Annotations](#annotations)
- [Comments](#comments)
- [Predeclared Identifiers](#predeclared-identifiers)
- [Expression Grouping](#expression-grouping)

## Keywords

### Core Language Keywords

#### `let` - Variable Declaration
Declares immutable variables (default) or mutable variables with explicit `mut` modifier.

```asthra
let x = 42;           // Immutable variable
let mut y = 10;       // Mutable variable
let name: string = "Alice";  // With explicit type
```

#### `const` - Compile-Time Constants
Declares compile-time evaluated constants that must be known at compilation.

```asthra
const PI: f64 = 3.14159;
const MAX_SIZE: usize = 1024;
const GREETING: string = "Hello, World!";
```

#### `fn` - Function Declaration
Defines functions with parameters, return types, and visibility modifiers.

```asthra
pub fn add(a: i32, b: i32) -> i32 {
    return a + b;
}

priv fn helper() -> none {
    // Private helper function
}
```

#### `struct` - Structure Definition
Defines composite data types with named fields and optional methods.

```asthra
pub struct Point {
    pub x: f64,
    pub y: f64,
}

impl Point {
    pub fn distance(&self, other: &Point) -> f64 {
        // Implementation
    }
}
```

#### `enum` - Enumeration Definition
Defines variant types with optional associated data, enabling pattern matching.

```asthra
pub enum Result<T, E> {
    Ok(T),
    Err(E),
}

pub enum Status {
    Active,
    Inactive,
    Pending,
}
```

#### `impl` - Implementation Block
Implements methods and associated functions for types.

```asthra
impl Point {
    pub fn new(x: f64, y: f64) -> Point {
        return Point { x: x, y: y };
    }
    
    pub fn distance(&self) -> f64 {
        return (self.x * self.x + self.y * self.y).sqrt();
    }
}
```

#### `match` - Pattern Matching
Provides exhaustive pattern matching with compile-time completeness checking.

```asthra
let result = match status {
    Status.Active => "System is running",
    Status.Inactive => "System is stopped", 
    Status.Pending => "System is starting",
};
```

#### `if` - Conditional Statements
Standard conditional execution with optional else branches.

```asthra
if x > 0 {
    print("Positive");
} else if x < 0 {
    print("Negative");
} else {
    print("Zero");
}
```

#### `for` - Iteration
Iterator-based loops for collections and ranges.

```asthra
for item in collection {
    process(item);
}

for i in 0..10 {
    print(i);
}
```

#### `while` - Conditional Loops
Loops that continue while a condition remains true.

```asthra
while x < 100 {
    x = x * 2;
}
```

#### `return` - Function Return
Explicitly returns values from functions.

```asthra
fn double(x: i32) -> i32 {
    return x * 2;
}
```

#### `use` - Import Declarations
Imports modules, types, and functions from other packages.

```asthra
use std::collections::HashMap;
use json::{parse, stringify};
use super::utils::helper_function;
```

#### `pub` - Public Visibility
Makes declarations visible outside their containing module.

```asthra
pub struct PublicStruct { /* ... */ }
pub fn public_function() { /* ... */ }
pub const PUBLIC_CONSTANT: i32 = 42;
```

#### `priv` - Private Visibility
Explicitly marks declarations as private to their containing module (default visibility).

```asthra
priv struct InternalStruct { /* ... */ }
priv fn internal_helper() { /* ... */ }
```

#### `self` - Instance Reference
References the current instance in method implementations.

```asthra
impl Point {
    pub fn translate(&mut self, dx: f64, dy: f64) {
        self.x = self.x + dx;
        self.y = self.y + dy;
    }
}
```

#### `super` - Parent Module Reference
References the parent module in import paths.

```asthra
use super::common::utilities;
use super::super::root_module::types;
```

#### `spawn` - Concurrent Task Creation
Creates new concurrent tasks that run independently.

```asthra
spawn {
    expensive_computation();
};

let handle = spawn_with_handle {
    return long_running_task();
};
```

#### `spawn_with_handle` - Task Creation with Handle
Creates concurrent tasks returning a handle for result retrieval and coordination.

```asthra
let handle = spawn_with_handle {
    return compute_result();
};

let result = handle.join();  // Wait for completion and get result
```

**Key Features:**
- **Result Retrieval**: Get return values from spawned tasks
- **Task Coordination**: Wait for task completion with `.join()`
- **Error Handling**: Propagate panics and errors from spawned tasks
- **Resource Management**: Automatic cleanup when handle is dropped

**Usage Patterns:**
```asthra
// Basic task with result
let computation = spawn_with_handle {
    return fibonacci(40);
};
let result = computation.join();

// Multiple parallel tasks
let task1 = spawn_with_handle { return process_data_chunk(chunk1); };
let task2 = spawn_with_handle { return process_data_chunk(chunk2); };
let results = [task1.join(), task2.join()];

// Task coordination with timeout
let handle = spawn_with_handle { return network_request(); };
match handle.try_join_timeout(Duration.from_seconds(5)) {
    Some(result) => handle_result(result),
    None => handle_timeout(),
}
```

#### `channel` - Communication Primitives
Creates channels for message passing between concurrent tasks.

```asthra
let (sender, receiver) = channel<i32>();
sender.send(42);
let value = receiver.recv();
```

#### `select` - Channel Selection
Non-deterministic selection over multiple channel operations.

```asthra
select {
    msg = receiver1.recv() => handle_message(msg),
    _ = receiver2.recv() => handle_other(),
    default => handle_no_messages(),
}
```

#### `unsafe` - Unsafe Operations
Marks blocks containing operations that bypass Asthra's safety guarantees.

```asthra
unsafe {
    let raw_ptr = get_raw_pointer();
    *raw_ptr = 42;
}
```

#### `extern` - Foreign Function Interface
Declares external functions and interfaces with other programming languages, primarily C.

```asthra
extern "C" {
    fn malloc(size: usize) -> *mut void;
    fn free(ptr: *mut void);
    fn strlen(s: *const char) -> usize;
}
```

**Key Features:**
- **C Compatibility**: Primary support for C function interfaces
- **Memory Safety**: Requires `unsafe` blocks for raw pointer operations
- **Type Mapping**: Automatic conversion between Asthra and C types where safe
- **Library Integration**: Enable use of existing C libraries and system APIs

**Usage Patterns:**
```asthra
// System library integration
extern "C" {
    fn getpid() -> i32;
    fn sleep(seconds: u32) -> i32;
}

// Custom C library binding
extern "C" {
    fn custom_hash(data: *const u8, len: usize) -> u64;
    fn initialize_library() -> i32;
    fn cleanup_library();
}

// Usage in unsafe context
unsafe {
    let pid = getpid();
    let result = initialize_library();
}
```

#### `in` - Iterator Operator
Used in for loops to iterate over collections, ranges, and iterables.

```asthra
for item in collection {
    process(item);
}

for (key, value) in hash_map {
    print(key, value);
}

for i in 0..100 {
    computation(i);
}
```

**Key Features:**
- **Universal Iteration**: Works with arrays, slices, hash maps, ranges, and custom iterables
- **Destructuring Support**: Automatic unpacking of tuples and structured data
- **Memory Efficient**: Lazy evaluation for ranges and large collections
- **Type Safety**: Compile-time verification of iterator compatibility

**Usage Patterns:**
```asthra
// Array iteration
let numbers = [1, 2, 3, 4, 5];
for num in numbers {
    print(num);
}

// Range iteration
for i in 0..10 {          // 0 to 9 (exclusive end)
    print(i);
}

for i in 0..=10 {         // 0 to 10 (inclusive end)
    print(i);
}

// Hash map iteration with destructuring
let scores: HashMap<string, i32> = create_scores();
for (name, score) in scores {
    print(name + ": " + score.to_string());
}

// Custom iterator implementation
for result in api_results {
    match result {
        Ok(data) => process(data),
        Err(error) => log_error(error),
    }
}
```

### Boolean Literals

#### `true` - Boolean True Value
Represents the boolean true value in logical expressions and conditions.

```asthra
let is_active: bool = true;
let condition = x > 0 && true;

if true {
    print("This always executes");
}
```

#### `false` - Boolean False Value  
Represents the boolean false value in logical expressions and conditions.

```asthra
let is_disabled: bool = false;
let condition = x < 0 || false;

while false {
    print("This never executes");
}
```

**Key Features:**
- **Type Safety**: Distinct boolean type, no implicit conversion from integers
- **Logical Operations**: Full support for `&&`, `||`, `!` operators
- **Pattern Matching**: Can be used in match expressions for flow control
- **Memory Efficient**: Single byte representation

**Usage Patterns:**
```asthra
// Boolean variables
let debug_mode: bool = true;
let production_ready: bool = false;

// Conditional logic
let result = if is_valid && true {
    "Success"
} else {
    "Failure"
};

// Pattern matching
match status_flag {
    true => activate_system(),
    false => shutdown_system(),
}

// Function returns
fn is_prime(n: i32) -> bool {
    if n < 2 { return false; }
    // ... implementation
    return true;
}
```

### Memory Management Keywords

#### `mut` - Mutability Modifier
Explicitly marks local variables as mutable (variables are immutable by default).

```asthra
let mut counter = 0;
counter = counter + 1;  // Valid with mut

let immutable = 42;
// immutable = 43;      // Compile error: cannot assign to immutable variable
```

**Key Features:**
- **Explicit Mutability**: Variables immutable by default, `mut` required for changes
- **Local Scope Only**: Only applies to local variables, not function parameters
- **Memory Safety**: Prevents accidental mutations while allowing intentional ones
- **Smart Compiler**: Automatic optimization converts value operations to in-place when beneficial

### Reserved Keywords

#### `none` - Structural Absence Marker
Represents absence in structural contexts (parameters, content, etc.).

```asthra
fn no_params() -> i32 { return 42; }          // Empty parameter list
struct Empty { none }                          // Empty struct content
let empty_array: [none]i32 = [];              // Empty array
```

#### `void` - Type Absence
Represents the absence of a return type (functions that don't return values).

```asthra
fn print_message(msg: string) -> void {
    print(msg);
    // No return statement needed
}
```

## Operators

### Arithmetic Operators

| Operator | Name | Example | Description |
|----------|------|---------|-------------|
| `+` | Addition | `a + b` | Adds two values |
| `-` | Subtraction | `a - b` | Subtracts right from left |
| `*` | Multiplication | `a * b` | Multiplies two values |
| `/` | Division | `a / b` | Divides left by right |
| `%` | Modulo | `a % b` | Remainder of division |

### Comparison Operators

| Operator | Name | Example | Description |
|----------|------|---------|-------------|
| `==` | Equal | `a == b` | Tests equality |
| `!=` | Not Equal | `a != b` | Tests inequality |
| `<` | Less Than | `a < b` | Tests if left is smaller |
| `<=` | Less Equal | `a <= b` | Tests if left is smaller or equal |
| `>` | Greater Than | `a > b` | Tests if left is larger |
| `>=` | Greater Equal | `a >= b` | Tests if left is larger or equal |

### Logical Operators

| Operator | Name | Example | Description |
|----------|------|---------|-------------|
| `&&` | Logical AND | `a && b` | True if both operands are true |
| `\|\|` | Logical OR | `a \|\| b` | True if either operand is true |
| `!` | Logical NOT | `!a` | Inverts boolean value |

### Assignment Operators

| Operator | Name | Example | Description |
|----------|------|---------|-------------|
| `=` | Assignment | `a = b` | Assigns value to variable |
| `+=` | Add Assign | `a += b` | Equivalent to `a = a + b` |
| `-=` | Subtract Assign | `a -= b` | Equivalent to `a = a - b` |
| `*=` | Multiply Assign | `a *= b` | Equivalent to `a = a * b` |
| `/=` | Divide Assign | `a /= b` | Equivalent to `a = a / b` |
| `%=` | Modulo Assign | `a %= b` | Equivalent to `a = a % b` |

### Memory Operators

| Operator | Name | Example | Description |
|----------|------|---------|-------------|
| `&` | Reference | `&variable` | Creates reference to variable |
| `*` | Dereference | `*pointer` | Accesses value at pointer |

### Access Operators

| Operator | Name | Example | Description |
|----------|------|---------|-------------|
| `.` | Field Access | `object.field` | Accesses struct field |
| `::` | Associated Access | `Type::function()` | Calls associated function |
| `[]` | Index Access | `array[index]` | Accesses array element |

### Range Operators

| Operator | Name | Example | Description |
|----------|------|---------|-------------|
| `..` | Exclusive Range | `0..10` | Range from 0 to 9 |
| `..=` | Inclusive Range | `0..=10` | Range from 0 to 10 |

## Type Syntax

### Generic Type Syntax
Generic types enable code reuse with type parameters.

```asthra
// Generic struct definition
struct Container<T> {
    pub value: T,
}

// Generic function definition
fn process<T, U>(input: T) -> U {
    // Implementation
}

// Generic enum definition
enum Result<T, E> {
    Ok(T),
    Err(E),
}
```

**Usage Examples:**
```asthra
// Instantiation with explicit types
let container: Container<i32> = Container { value: 42 };
let result: Result<string, Error> = Result.Ok("success");

// Type inference
let auto_container = Container { value: "hello" };  // Container<string>
let numbers = Vec.new<i32>();  // Vec<i32>

// Generic constraints
fn serialize<T: Serializable>(data: T) -> string {
    return data.to_json();
}
```

### Pointer Types
Explicit pointer types for unsafe memory operations.

#### Mutable Pointers
```asthra
*mut T    // Mutable pointer to type T
```

#### Immutable Pointers  
```asthra
*const T  // Immutable pointer to type T
```

**Usage Examples:**
```asthra
unsafe {
    let mut value = 42;
    let mut_ptr: *mut i32 = &mut value;
    let const_ptr: *const i32 = &value;
    
    *mut_ptr = 100;  // Modify through mutable pointer
    let read_value = *const_ptr;  // Read through immutable pointer
}
```

### Array and Slice Types

#### Fixed-Size Arrays
```asthra
[N]T      // Array of N elements of type T
```

#### Dynamic Slices
```asthra
[]T       // Slice of type T (reference to contiguous elements)
```

**Usage Examples:**
```asthra
let fixed_array: [5]i32 = [1, 2, 3, 4, 5];
let slice: []i32 = &fixed_array[1..4];  // Elements 1, 2, 3
let dynamic_slice = Vec.new<i32>().as_slice();
```

### Function Types
First-class function types for callbacks and higher-order functions.

```asthra
fn(T, U) -> V    // Function taking T and U, returning V
fn() -> T        // Function taking no parameters, returning T
fn(T) -> none    // Function taking T, returning nothing
```

**Usage Examples:**
```asthra
let callback: fn(i32) -> string = int_to_string;
let processor: fn([]u8, fn(string) -> none) -> bool = process_data;

fn higher_order(f: fn(i32) -> i32, x: i32) -> i32 {
    return f(x * 2);
}
```

### Reference Types
Safe borrowing for memory-efficient operations.

```asthra
&T        // Immutable reference to T
&mut T    // Mutable reference to T
```

**Usage Examples:**
```asthra
fn read_data(data: &string) {  // Borrow without taking ownership
    print(data.length());
}

fn modify_data(data: &mut Vec<i32>) {  // Mutable borrow for modification
    data.push(42);
}
```

## Literals

### Integer Literals

#### Decimal Literals
```asthra
42        // i32 (default)
42i8      // 8-bit signed integer
42u32     // 32-bit unsigned integer
42i64     // 64-bit signed integer
```

#### Hexadecimal Literals
```asthra
0x2A      // Hexadecimal (42 in decimal)
0xFF      // 255 in decimal
0x1A2B    // Multi-digit hex
```

#### Binary Literals
```asthra
0b101010  // Binary (42 in decimal)
0b1111    // 15 in decimal
```

#### Octal Literals
```asthra
0o52      // Octal (42 in decimal)
0o755     // 493 in decimal (common file permissions)
0o17      // 15 in decimal
```

### Floating-Point Literals
```asthra
3.14      // f64 (default)
3.14f32   // 32-bit float
2.5e10    // Scientific notation (2.5 × 10^10)
1.5E-4    // Scientific notation (1.5 × 10^-4)
```

### Character Literals
Single Unicode characters enclosed in single quotes.

```asthra
'A'       // ASCII character
'α'       // Unicode character
'🚀'      // Unicode emoji
```

#### Character Escape Sequences
```asthra
'\n'      // Newline
'\t'      // Tab
'\r'      // Carriage return
'\\'      // Backslash
'\''      // Single quote
'\"'      // Double quote
'\0'      // Null character
'\x41'    // Hex escape (ASCII 'A')
'\u{1F680}' // Unicode escape (🚀)
```

### String Literals

#### Regular String Literals
```asthra
"Hello, World!"
"String with \"escaped quotes\""
"Multi-line strings are\nsupported with \\n"
```

#### Raw String Literals
Multi-line strings without escape sequence processing.

```asthra
r"""
Raw string content
No escape processing: \n \t \"
Perfect for: SQL, configs, documentation
"""
```

#### Processed String Literals  
Multi-line strings with escape sequence processing.

```asthra
"""
Processed multi-line string
Escape sequences work: \n \t \"
Line 1
Line 2
Line 3
"""
```

**Usage Examples:**
```asthra
// SQL queries
let query = r"""
SELECT users.name, posts.title
FROM users
JOIN posts ON users.id = posts.user_id
WHERE users.active = true
ORDER BY posts.created_at DESC;
""";

// Configuration templates
let nginx_config = r"""
server {
    listen 80;
    server_name example.com;
    root /var/www/html;
    
    location / {
        try_files $uri $uri/ =404;
    }
}
""";

// Documentation with formatting
let help_text = """
Usage: myapp [OPTIONS] <FILE>

Options:
  -h, --help     Show this help message
  -v, --version  Show version info
  
Examples:
  myapp input.txt
  myapp --verbose data.json
""";
```

### Boolean Literals
```asthra
true      // Boolean true
false     // Boolean false
```

### Array Literals
```asthra
[1, 2, 3, 4, 5]           // Array of integers
["a", "b", "c"]           // Array of strings
[true, false, true]       // Array of booleans
[]                        // Empty array (type inferred)
```

#### Repeated Element Syntax
```asthra
[0; 10]                   // Array of 10 zeros
["default"; 5]            // Array of 5 "default" strings
[Point.new(0, 0); 3]      // Array of 3 default points
```

## Pattern Matching

### Match Expressions
Comprehensive pattern matching with exhaustiveness checking.

#### Basic Patterns
```asthra
match value {
    1 => "one",
    2 => "two", 
    3 => "three",
    _ => "other",  // Wildcard pattern
}
```

#### Enum Pattern Matching
```asthra
match result {
    Result.Ok(value) => process_success(value),
    Result.Err(error) => handle_error(error),
}
```

#### Guard Patterns
```asthra
match number {
    x if x > 0 => "positive",
    x if x < 0 => "negative", 
    _ => "zero",
}
```

#### Destructuring Patterns
```asthra
match point {
    Point { x: 0, y: 0 } => "origin",
    Point { x: 0, y } => "on y-axis: " + y.to_string(),
    Point { x, y: 0 } => "on x-axis: " + x.to_string(),
    Point { x, y } => "point: (" + x.to_string() + ", " + y.to_string() + ")",
}
```

### Pattern Operators

| Operator | Name | Example | Description |
|----------|------|---------|-------------|
| `=>` | Match Arrow | `pattern => expression` | Maps pattern to result expression |
| `_` | Wildcard | `_ => default` | Matches any value (catch-all) |
| `\|` | Pattern OR | `1 \| 2 \| 3 => "small"` | Matches any of multiple patterns |

### If-Let Patterns
Conditional pattern matching for single patterns.

```asthra
if let Result.Ok(value) = operation() {
    print("Success: " + value.to_string());
} else {
    print("Operation failed");
}

if let Some(item) = optional_value {
    process(item);
}
```

## Annotations

### Observability Annotations

#### `#[deterministic]`
Marks functions as deterministic (same inputs always produce same outputs).

```asthra
#[deterministic]
fn calculate_hash(input: string) -> u64 {
    // Pure function - no side effects
}
```

#### `#[non_deterministic]` 
Marks functions with non-deterministic behavior (randomness, I/O, time-dependent).

```asthra
#[non_deterministic]
fn get_random_number() -> i32 {
    // Uses random number generator
}

#[non_deterministic] 
fn current_timestamp() -> u64 {
    // Time-dependent function
}
```

#### `#[performance_critical]`
Indicates functions requiring optimal performance.

```asthra
#[performance_critical]
fn inner_loop_computation(data: []f64) -> f64 {
    // Hot path - optimize aggressively
}
```

#### `#[memory_intensive]`
Marks functions that allocate significant memory.

```asthra
#[memory_intensive]
fn load_large_dataset(file: string) -> Vec<Record> {
    // Allocates substantial memory
}
```

### Safety Annotations

#### `#[unsafe_operation]`
Documents unsafe operations and their safety requirements.

```asthra
#[unsafe_operation("requires valid pointer and proper alignment")]
unsafe fn write_raw_memory(ptr: *mut u8, value: u8) {
    *ptr = value;
}
```

#### `#[thread_safe]`
Indicates thread-safe functions and data structures.

```asthra
#[thread_safe]
fn atomic_increment(counter: &AtomicI32) -> i32 {
    return counter.fetch_add(1, Ordering.SeqCst);
}
```

### Development Annotations

#### `#[deprecated]`
Marks deprecated functions with migration guidance.

```asthra
#[deprecated("use new_function() instead")]
fn old_function() -> i32 {
    // Legacy implementation
}
```

#### `#[todo]`
Indicates incomplete implementations.

```asthra
#[todo("implement error handling")]
fn partial_function() -> Result<string, Error> {
    // Incomplete implementation
}
```

#### `#[optimize]`
Requests compiler optimization for specific functions.

```asthra
#[optimize("aggressive")]
fn computational_kernel(data: []f64) -> f64 {
    // CPU-intensive computation
}
```

## Comments

### Line Comments
Single-line comments using double slashes.

```asthra
// This is a line comment
let x = 42;  // Comments can appear at end of line

// Multiple line comments
// can be written like this
```

### Block Comments
Multi-line comments using `/* */` syntax with nesting support.

```asthra
/*
This is a block comment
spanning multiple lines
*/

/*
Block comments can be /* nested */ inside
other block comments for documentation
*/
```

### Documentation Comments
Special comments for API documentation generation.

```asthra
/// This function calculates the square of a number
/// 
/// # Arguments
/// * `x` - The number to square
/// 
/// # Returns
/// The square of the input number
/// 
/// # Example
/// ```
/// let result = square(4);
/// assert_eq!(result, 16);
/// ```
pub fn square(x: i32) -> i32 {
    return x * x;
}
```

## Predeclared Identifiers

Asthra provides built-in functions that are automatically available in all packages without imports.

### `range` - Sequence Generation
Generates integer sequences for iteration, commonly used with for loops.

```asthra
// Single parameter: generates [0, 1, 2, ..., end-1]
for i in range(5) {
    print(i);  // Prints 0, 1, 2, 3, 4
}

// Two parameters: generates [start, start+1, ..., end-1]
for i in range(2, 8) {
    print(i);  // Prints 2, 3, 4, 5, 6, 7
}
```

**Function Signatures:**
```asthra
range(end: i32) -> []i32         // Range from 0 to end-1
range(start: i32, end: i32) -> []i32  // Range from start to end-1
```

**Key Features:**
- **Zero-based**: Single parameter starts from 0
- **Exclusive end**: Upper bound is not included
- **Empty ranges**: Returns empty slice if start >= end
- **Immediate availability**: No import required

**Usage Examples:**
```asthra
// Basic iteration
for i in range(10) {
    process_item(i);
}

// Custom range
for page in range(1, 11) {
    fetch_page(page);  // Pages 1 through 10
}

// Array indexing
let data = [10, 20, 30, 40, 50];
for i in range(0, data.len()) {
    print("Index " + i.to_string() + ": " + data[i].to_string());
}

// Storing range result
let numbers: []i32 = range(5);  // [0, 1, 2, 3, 4]
let subset: []i32 = range(10, 15);  // [10, 11, 12, 13, 14]
```

### `log` - Debug Logging
Outputs debug messages to standard error for development and debugging.

```asthra
log("Application started");
log("Processing item: " + item_id.to_string());
log("Error: " + error_message);
```

**Function Signature:**
```asthra
log(message: string) -> void
```

**Key Features:**
- **Debug output**: Writes to stderr, not stdout
- **Thread-safe**: Safe for concurrent use
- **Development aid**: Typically disabled in release builds
- **No formatting**: Use string concatenation for complex messages

## Expression Grouping

### Parentheses
Control operator precedence and create sub-expressions.

```asthra
let result = (a + b) * (c - d);
let condition = (x > 0) && (y < 100);
let complex = ((a * b) + c) / ((d - e) * f);
```

### Operator Precedence
From highest to lowest precedence:

1. **Primary**: `()`, `[]`, `.`, `::`
2. **Unary**: `!`, `-`, `*`, `&`
3. **Multiplicative**: `*`, `/`, `%`
4. **Additive**: `+`, `-`
5. **Comparison**: `<`, `<=`, `>`, `>=`
6. **Equality**: `==`, `!=`
7. **Logical AND**: `&&`
8. **Logical OR**: `||`
9. **Assignment**: `=`, `+=`, `-=`, `*=`, `/=`, `%=`

### Block Expressions
Blocks that evaluate to their final expression.

```asthra
let result = {
    let x = compute_base();
    let y = compute_modifier();
    x * y + 10  // Final expression (no semicolon)
};
```

---

*This syntax reference covers all major elements of the Asthra programming language. For detailed examples and usage patterns, see the [Language Fundamentals](language-fundamentals.md) guide.* 
