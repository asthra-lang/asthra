# Predeclared Identifiers

**Version:** 1.4  
**Last Updated:** 2025-01-21

Following Go's design, Asthra provides **predeclared identifiers** - types and functions that are automatically available in all packages without requiring explicit import statements. These identifiers can be shadowed by user declarations when needed, providing both convenience and flexibility.

## Predeclared Types

### Built-in Generic Types

```asthra
Result<T, E>  // Built-in type for error handling with Ok(T) and Err(E) variants
Option<T>     // Built-in type for nullable values with Some(T) and None variants
```

These types are available without any declaration and support full pattern matching:

```asthra
package main;

pub fn divide(a: i32, b: i32) -> Result<i32, string> {
    if b == 0 {
        return Result.Err("Division by zero");
    }
    return Result.Ok(a / b);
}

pub fn find_value(key: string) -> Option<i32> {
    if key == "answer" {
        return Option.Some(42);
    }
    return Option.None;
}

pub fn main(none) -> void {
    match divide(10, 2) {
        Result.Ok(value) => log("Result: " + value),
        Result.Err(error) => log("Error: " + error)
    }
    return ();
}
```

## Available Predeclared Functions

### Utility Functions

```asthra
log(message: string) -> void              // Debug/diagnostic logging
args() -> []string                        // Access command-line arguments
```

### Range Functions

```asthra
range(end: i32) -> []i32                  // Generate [0, 1, 2, ..., end-1]
range(start: i32, end: i32) -> []i32      // Generate [start, start+1, ..., end-1]
```

### Iterator Functions

```asthra
infinite() -> InfiniteIterator            // Generate an infinite iterator
```

**Note:** Functions using `infinite()` must be annotated with `#[non_deterministic]` as they contain unbounded iteration.

## Automatic Availability

Predeclared functions work immediately without imports:

```asthra
package main;

fn main() -> i32 {
    // Predeclared functions work out of the box
    log("Application starting");
    
    // Access command-line arguments
    let arguments = args();
    for arg in arguments {
        log("Argument: " + arg);
    }
    
    // Predeclared range functions work in for loops
    for i in range(5) {
        log("Iteration: " + string(i));
    }
    
    // Multiple range overloads available
    for i in range(2, 8) {
        log("Range iteration: " + string(i));
    }
    
    return 0;
}

// Example using infinite iterator (requires non-deterministic annotation)
#[non_deterministic]
fn server_loop() -> void {
    for _ in infinite() {
        accept_connection();
        if should_shutdown() {
            break;
        }
    }
}
```

## Shadowing Behavior

Predeclared identifiers can be shadowed by user declarations:

```asthra
package main;

// Shadow the predeclared log function
fn log(level: string, message: string) -> void {
    print("[" + level + "] " + message);
}

// Shadow only the single-parameter range function
fn range(end: i32) -> []i32 {
    // Custom implementation that starts from 1
    return custom_range_from_one(end);
}

fn main() -> i32 {
    // Uses the shadowed (custom) log function
    log("INFO", "Using custom logging");
    
    // Uses the shadowed range(i32) function
    let nums1 = range(5);  // Custom implementation
    
    // Uses predeclared range(i32, i32) - not shadowed
    let nums2 = range(2, 8);  // Still predeclared version
    
    return 0;
}
```

## Benefits for AI Code Generation

Predeclared identifiers provide significant advantages for AI-assisted development:

1. **Immediate Availability:** Common types and functions like `Result<T,E>`, `Option<T>`, `log()`, and `range()` work without setup
2. **Reduced Cognitive Load:** No need to remember import statements for basic utilities or error handling types  
3. **Consistent Patterns:** Same types and functions available in every Asthra program
4. **Flexible Override:** Power users can customize behavior when needed
5. **Go-like Familiarity:** Developers familiar with Go will recognize the pattern
6. **Type-Safe Error Handling:** Built-in `Result<T,E>` ensures consistent error handling patterns across all code

## Type Details

### Result<T, E> Type

```asthra
Result<T, E>  // Generic enum with Ok(T) and Err(E) variants
```

**Purpose:** Provides type-safe error handling without exceptions, ensuring all errors are explicitly handled.

**Variants:**
- `Result.Ok(value: T)` - Success case containing the result value
- `Result.Err(error: E)` - Error case containing error information

**Usage Examples:**
```asthra
// Function returning Result
pub fn parse_number(s: string) -> Result<i32, string> {
    // Implementation that returns Ok or Err
}

// Pattern matching on Result
match parse_number("42") {
    Result.Ok(num) => log("Parsed: " + num),
    Result.Err(err) => log("Error: " + err)
}

// Chaining operations
pub fn calculate(input: string) -> Result<i32, string> {
    match parse_number(input) {
        Result.Ok(n) => {
            if n < 0 {
                return Result.Err("Negative numbers not allowed");
            }
            return Result.Ok(n * 2);
        },
        Result.Err(e) => return Result.Err(e)
    }
}
```

### Option<T> Type

```asthra
Option<T>  // Generic enum with Some(T) and None variants
```

**Purpose:** Represents nullable values without null pointers, preventing null reference errors.

**Variants:**
- `Option.Some(value: T)` - Contains a value
- `Option.None` - Represents absence of value

**Usage Examples:**
```asthra
// Function returning Option
pub fn find_user(id: i32) -> Option<User> {
    // Implementation that returns Some or None
}

// Pattern matching on Option
match find_user(123) {
    Option.Some(user) => log("Found: " + user.name),
    Option.None => log("User not found")
}

// Default values
pub fn get_config_value(key: string) -> i32 {
    match lookup_config(key) {
        Option.Some(value) => return value,
        Option.None => return 0  // Default value
    }
}
```

## Function Details

### log Function

```asthra
fn log(message: string) -> void
```

**Purpose:** Provides basic logging functionality for debugging and diagnostic output.

**Behavior:**
- Outputs message to standard error (stderr)
- Automatically adds timestamp in debug builds
- Thread-safe for concurrent usage
- No-op in release builds unless explicitly enabled

**Examples:**
```asthra
log("Starting application");
log("Processing user ID: " + user_id);
log("Error occurred: " + error_message);

// Custom formatting
log("Values: x=" + x + ", y=" + y);
```

### args Function

```asthra
fn args() -> []string
```

**Purpose:** Access command-line arguments passed to the program.

**Behavior:**
- Returns a slice of strings containing all command-line arguments
- First element (index 0) is the program name/path
- Subsequent elements are the arguments passed by the user
- Returns empty slice if no arguments available
- Thread-safe and can be called multiple times

**Examples:**
```asthra
let arguments = args();

// Check if any arguments were provided
if arguments.len() > 1 {
    log("First argument: " + arguments[1]);
}

// Iterate over all arguments
for arg in arguments {
    log("Argument: " + arg);
}

// Process specific arguments
if arguments.len() >= 3 {
    let input_file = arguments[1];
    let output_file = arguments[2];
    process_files(input_file, output_file);
}
```

### range Functions

#### Single Parameter Range

```asthra
fn range(end: i32) -> []i32
```

**Purpose:** Generate sequence from 0 to end-1 (exclusive).

**Parameters:**
- `end`: Upper bound (exclusive)

**Returns:** Array of integers `[0, 1, 2, ..., end-1]`

**Examples:**
```asthra
let numbers = range(5);        // [0, 1, 2, 3, 4]
let empty = range(0);          // []
let single = range(1);         // [0]

// Common usage in loops
for i in range(10) {
    log("Processing item " + i);
}
```

#### Two Parameter Range

```asthra
fn range(start: i32, end: i32) -> []i32
```

**Purpose:** Generate sequence from start to end-1 (exclusive).

**Parameters:**
- `start`: Lower bound (inclusive)
- `end`: Upper bound (exclusive)

**Returns:** Array of integers `[start, start+1, ..., end-1]`

**Examples:**
```asthra
let numbers = range(2, 8);     // [2, 3, 4, 5, 6, 7]
let descending = range(5, 2);  // [] (empty for invalid range)
let same = range(3, 3);        // [] (empty range)

// Common usage patterns
for i in range(1, 11) {        // 1 to 10 inclusive
    log("Number: " + i);
}

for index in range(0, array.len) {
    process_item(array[index]);
}
```

## Implementation Details

### Symbol Resolution

- **Global Scope:** Predeclared identifiers are resolved at the global scope level
- **Shadowing:** Local declarations naturally shadow predeclared types and functions
- **Type Checking:** Predeclared types and functions have well-defined signatures
- **No Parser Changes:** Predeclared identifiers are regular identifiers
- **Built-in Types:** Result<T,E> and Option<T> are registered as built-in generic enum types during semantic analysis

### Runtime Implementation

- **Built-in Functions:** Implemented in the Asthra runtime system
- **Automatic Linking:** Linked automatically without user configuration
- **Performance:** Optimized implementations with zero overhead
- **Thread Safety:** All predeclared functions are thread-safe

### Compile-Time Behavior

```asthra
// Compiler recognizes predeclared functions
fn example() -> void {
    log("Hello");           // Resolves to predeclared log
    let nums = range(5);    // Resolves to predeclared range
    
    // After shadowing
    fn log(msg: string) -> void { /* custom */ }
    
    log("World");           // Now resolves to local log function
    let more = range(3);    // Still resolves to predeclared range
}
```

## Comparison with Import System

### Predeclared vs. Standard Library

```asthra
// Predeclared - immediately available
fn immediate_usage() -> void {
    log("No import needed");
    let sequence = range(10);
}

// Standard library - requires import
import "stdlib/string";

fn library_usage() -> void {
    let trimmed = string.trim("  hello  ");
}
```

### Benefits of Predeclared Approach

1. **Zero Configuration:** No setup required for basic functionality
2. **AI-Friendly:** Reduces cognitive load for code generation
3. **Consistent Availability:** Same functions in every Asthra program
4. **Gradual Learning:** Beginners can start without understanding imports

### When to Use Each

**Use Predeclared Functions For:**
- Quick debugging and logging
- Simple iteration patterns
- Basic utility operations
- Educational/prototype code

**Use Standard Library For:**
- Complex string operations
- File I/O and networking
- Data structure manipulation
- Production applications

## Advanced Usage Patterns

### Conditional Shadowing

```asthra
package main;

#[cfg(debug)]
fn log(message: string) -> void {
    // Enhanced debug logging
    print("[DEBUG " + timestamp() + "] " + message);
}

fn main() -> i32 {
    // Uses custom log in debug builds, predeclared in release
    log("Application starting");
    return 0;
}
```

### Partial Shadowing

```asthra
package main;

// Shadow only one overload
fn range(end: i32) -> []i32 {
    // Custom implementation
    return custom_range_impl(end);
}

fn main() -> i32 {
    let custom = range(5);      // Uses custom implementation
    let builtin = range(2, 8);  // Uses predeclared two-parameter version
    return 0;
}
```

### Wrapper Functions

```asthra
package main;

// Wrap predeclared function with additional functionality
fn debug_log(category: string, message: string) -> void {
    log("[" + category + "] " + message);  // Uses predeclared log
}

fn main() -> i32 {
    debug_log("INIT", "Starting application");
    return 0;
}
```

## Future Extensions

### Planned Predeclared Types

No additional built-in types are planned. Result<T,E> and Option<T> provide the essential type-safe patterns for error handling and nullable values.

### Planned Predeclared Functions

**Additional Utility Functions:**
```asthra
print(message: string) -> void         // Console output
println(message: string) -> void       // Console output with newline
assert(condition: bool) -> void        // Debug assertions
panic(message: string) -> never        // Program termination
```

**Enhanced Range Functions:**
```asthra
range_step(start: i32, end: i32, step: i32) -> []i32  // Range with custom step
range_f64(start: f64, end: f64, step: f64) -> []f64   // Floating-point ranges
```

### Design Considerations

1. **Minimal Set:** Keep predeclared functions to essential utilities
2. **No Conflicts:** Avoid common user function names
3. **Type Safety:** All predeclared functions must be type-safe
4. **Performance:** Optimize for zero-overhead usage
5. **Consistency:** Maintain consistent naming and behavior patterns

This predeclared identifier system enables Asthra programs to be immediately productive while maintaining the language's core values of explicitness and predictability. 
