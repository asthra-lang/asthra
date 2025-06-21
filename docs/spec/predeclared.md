# Predeclared Identifiers

**Version:** 1.3  
**Last Updated:** 2024-12-28

Following Go's design, Asthra provides **predeclared identifiers** - functions that are automatically available in all packages without requiring explicit import statements. These identifiers can be shadowed by user declarations when needed, providing both convenience and flexibility.

## Available Predeclared Functions

### Utility Functions

```asthra
log(message: string) -> void              // Debug/diagnostic logging
```

### Range Functions

```asthra
range(end: i32) -> []i32                  // Generate [0, 1, 2, ..., end-1]
range(start: i32, end: i32) -> []i32      // Generate [start, start+1, ..., end-1]
```

## Automatic Availability

Predeclared functions work immediately without imports:

```asthra
package main;

fn main() -> i32 {
    // Predeclared functions work out of the box
    log("Application starting");
    
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

1. **Immediate Availability:** Common functions like `log()` and `range()` work without setup
2. **Reduced Cognitive Load:** No need to remember import statements for basic utilities  
3. **Consistent Patterns:** Same functions available in every Asthra program
4. **Flexible Override:** Power users can customize behavior when needed
5. **Go-like Familiarity:** Developers familiar with Go will recognize the pattern

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
- **Shadowing:** Local declarations naturally shadow predeclared functions
- **Type Checking:** Predeclared functions have well-defined type signatures
- **No Parser Changes:** Predeclared identifiers are regular identifiers

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
