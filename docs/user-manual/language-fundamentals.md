# Language Fundamentals

**Implementation Status**: ✅ Complete | 🔧 Partial | ❌ Not Implemented  
**Last Verified**: 2025-01-07  
**Grammar Version**: v1.22  
**Test Coverage**: 95%+

This section covers the core syntax and concepts of the Asthra programming language.

> **📖 Quick Reference**: For a comprehensive lookup guide to all syntax elements (keywords, operators, types, literals), see the [Syntax Reference](syntax-reference.md). This chapter provides practical examples and usage patterns.

## Program Structure

An Asthra program consists of:

* A **package declaration** (required for every `.asthra` file).
* Zero or more **import declarations** (for using other packages).
* Zero or more **top-level declarations** (functions, structs, external function declarations).

### Package System Overview

Asthra uses a **Go-style package system** where:
- Each directory contains exactly one package
- All `.asthra` files in a directory must declare the same package name
- Files in the same package can access each other's private functions and types
- Other packages can only access items marked with `pub`

**For comprehensive package organization guidance, see [Package Organization](package-organization.md)**

### Single-File Program Example

```asthra
package main; // Required package declaration

pub fn main() -> i32 {
    let message: string = "Hello, Asthra!";
    return 0;
}
```

### Multi-Package Program Example

```asthra
// src/main.asthra (package main)
package main;

import "models";    // Import local models package
import "utils";     // Import local utils package
import "stdlib/io"; // Import standard library

pub fn main() -> i32 {
    let user: models.User = models.create_user("Alice", "alice@example.com");
    let is_valid: bool = utils.validate_email(user);
    
    io.println("User validation complete");
    return if is_valid { 0 } else { 1 };
}
```

```asthra
// models/user.asthra (package models)
package models; // All files in models/ directory use this package

pub struct User {
    pub id: i32,        // Public field
    pub name: string,   // Public field
    email: string,      // Private field (package-only access)
}

pub fn create_user(name: string, email: string) -> User {
    return User {
        id: generate_id(),  // Can call private functions in same package
        name: name,
        email: email,
    };
}

fn generate_id() -> i32 {  // Private function (no pub keyword)
    return 42;
}
```

```asthra
// utils/validation.asthra (package utils)
package utils;

import "models";

pub fn validate_email(user: models.User) -> bool {
    // Can access public fields and functions
    let user_id: i32 = user.id;        // ✅ Public field accessible
    // let email: string = user.email;  // ❌ Private field not accessible
    
    return user_id > 0;
}
```

### Import System Examples

```asthra
// Basic package imports
import "models";           // Import local package
import "stdlib/collections"; // Import standard library package

// Import aliases for conflict resolution (v1.22+)
import "graphics/math" as GraphicsMath;
import "physics/math" as PhysicsMath;
import "collections/vector" as Vec;

// Using imported packages and aliases
pub fn use_imported_libraries() -> void {
    // Using basic imports
    let user: models.User = models.create_user("Bob", "bob@example.com");
    
    // Using import aliases
    let distance: f64 = GraphicsMath.calculate_distance(1.0, 2.0);
    let velocity: f64 = PhysicsMath.calculate_velocity(10.0, 5.0);
    let my_vector: Vec.Vector<i32> = Vec.create_vector<i32>();
}
```

## Basic Syntax Rules

* Statements end with a semicolon `;`.
* Blocks are enclosed in curly braces `{}`.
* Comments start with `//` for single-line or `/* */` for block comments.
* Identifiers are alphanumeric (can contain underscores).
* String literals use double quotes `""`.
* **Grammar Disambiguation**: Asthra uses unambiguous parsing rules to eliminate conflicts, ensuring reliable AI code generation.

> **📚 Syntax Elements**: For detailed documentation of comment syntax, operators, and expression grouping, see [Syntax Reference - Comments](syntax-reference.md#comments), [Syntax Reference - Operators](syntax-reference.md#operators), and [Syntax Reference - Expression Grouping](syntax-reference.md#expression-grouping).

### Statement Termination Rules

All statements in Asthra must end with a semicolon (`;`) for explicit statement boundaries:

```asthra
// Variable declarations
let x: i32 = 42;
let mut counter: i32 = 0;

// Function calls  
print("Hello, World!");
process_data(input);

// Assignment statements
counter = counter + 1;
result = calculate(a, b);

// Return statements
return value;
return Result.Ok(42);

// Expression statements
do_work();
update_state();
```

**Semicolon Requirements:**
- **Required**: All declaration, assignment, call, and return statements
- **Not Required**: After block expressions (`if`, `for`, `match`, function bodies)
- **Style**: No trailing semicolons after the last expression in blocks

> **📚 Complete Syntax**: See [Syntax Reference - Expression Grouping](syntax-reference.md#expression-grouping) for operator precedence and statement structure rules.

### Block Structure and Nesting

Asthra uses curly braces `{}` to define code blocks with clear nesting rules:

```asthra
// Function blocks
pub fn example() -> void {
    let x: i32 = 10;  // Statement inside block
    
    // Nested control flow blocks
    if x > 5 {
        print("Greater than 5");
        
        // Deeper nesting
        for i in 0..x {
            print(i.to_string());
        }
    } else {
        print("Less than or equal to 5");
    }
    
    // Match expressions with blocks
    let result = match x {
        0 => "zero",
        1..=10 => "small",
        _ => "large",
    };
}

// Struct implementation blocks
impl Point {
    pub fn new(x: f64, y: f64) -> Point {
        return Point { x: x, y: y };
    }
}
```

**Block Rules:**
- **Opening Brace**: Same line as declaration (`fn example() {`)
- **Closing Brace**: Aligned with declaration indentation  
- **Nesting**: Consistent 4-space indentation per level
- **Empty Blocks**: Use `{}` (not `{ void }` for code blocks)

## Annotations and Semantic Tags

Asthra supports semantic annotations that provide metadata for AI code generation, human review, and compiler optimizations. These annotations are a core feature of Asthra's AI-first design.

### Basic Annotation Syntax
```asthra
#[annotation_name(parameters)]
priv fn example_function(void) -> void " + 
    // Function implementation
 + "
```

### Common Annotations
- `#[human_review(priority)]` - Indicates human review requirements
- `#[ownership(type)]` - Memory management hints  
- `#[constant_time]` - Security-critical timing requirements
- `#[transfer_full]` - FFI ownership transfer semantics

### Annotation Placement
Annotations can be applied to:
- Function declarations
- Struct declarations
- External function declarations
- Method declarations
- Variable declarations (for ownership)

For complete annotation documentation, see [Annotations & Semantic Tags](annotations.md).

## Data Types

Asthra supports fundamental types for native compilation:

### Base Types
* **Basic Types:** `int`, `float`, `bool`, `string`, `void`
* **Explicit Integer Types:** `u8`, `i8`, `u16`, `i16`, `u32`, `i32`, `u64`, `i64`
* **Explicit Float Types:** `f32`, `f64`
* **Architecture Types:** `usize`, `isize` (integer types matching the target architecture's pointer size)

### Numeric Literals (v1.11+)
Asthra supports multiple numeric literal formats for systems programming:

* **Decimal:** `42`, `123` (standard base-10 numbers)
* **Hexadecimal:** `0xFF`, `0x1A2B`, `0xDEADBEEF` (base-16, case-insensitive)
* **Binary:** `0b1010`, `0b11110000` (base-2, case-insensitive)
* **Octal:** `0o755`, `0o644` (base-8, explicit prefix required)

```asthra
priv fn numeric_literal_examples(void) -> void " + 
    // Different representations of the same values
    let decimal_value: u32 = 255;
    let hex_value: u32 = 0xFF;        // Same as 255
    let binary_value: u32 = 0b11111111; // Same as 255
    let octal_value: u32 = 0o377;     // Same as 255
    
    // Systems programming examples
    let memory_mask: u32 = 0xFFFF0000;  // Memory alignment mask
    let permission_bits: u32 = 0o755;   // Unix file permissions
    let bit_flags: u8 = 0b10101010;     // Bit manipulation
    
    // Type inference works with all literal types
    let small_hex: u8 = 0xFF;           // Inferred as u8
    let large_hex: u32 = 0xDEADBEEF;    // Inferred as u32
 + "

### Composite Types
* **Slice Type:** `[]Type` (e.g., `[]int` for a slice of integers). Slices are dynamically sized views over a contiguous block of memory with formalized `SliceHeader<T>` internal structure containing a pointer to the data and a length.
* **Struct Type:** `IDENT` (e.g., `Point` for a struct type named Point).

> **📚 Type Syntax**: For complete documentation of generic types (`<T>`), pointer types (`*mut T`, `*const T`), and array types (`[N]T`), see [Syntax Reference - Type Syntax](syntax-reference.md#type-syntax).

### Array and Struct Initialization Patterns

Asthra provides comprehensive initialization syntax for arrays and structs:

#### Array Initialization
```asthra
// Basic array literals
let numbers: [5]i32 = [1, 2, 3, 4, 5];
let names: [3]string = ["Alice", "Bob", "Charlie"];

// Empty array initialization
let empty_array: [0]i32 = [];
let buffer: [1024]u8 = [0; 1024];  // Repeated element syntax

// Nested array initialization
let matrix: [2][3]i32 = [
    [1, 2, 3],
    [4, 5, 6],
];

// Array of structs
let points: [3]Point = [
    Point { x: 0.0, y: 0.0 },
    Point { x: 1.0, y: 1.0 },
    Point { x: 2.0, y: 2.0 },
];
```

#### Struct Initialization
```asthra
// Basic struct initialization
let point = Point {
    x: 3.14,
    y: 2.71,
};

// Nested struct initialization
let person = Person {
    name: "Alice",
    age: 30,
    address: Address {
        street: "123 Main St",
        city: "Anytown",
        zip: "12345",
    },
};

// Struct with computed values
let config = Configuration {
    port: get_port_from_env(),
    timeout: Duration.from_seconds(30),
    debug: is_development_mode(),
};
```

> **📚 Initialization Examples**: See [Syntax Examples](../examples/syntax-completion-examples.asthra) for comprehensive array and struct initialization patterns, and [Syntax Reference - Literals](syntax-reference.md#literals) for array literal syntax.

### Field Access and Method Chaining

Asthra supports clear field access and method chaining syntax:

```asthra
// Basic field access
let x_coord: f64 = point.x;
let user_name: string = person.name;

// Nested field access
let street: string = person.address.street;
let config_port: u16 = server.config.network.port;

// Method calls
let distance: f64 = point.distance_from_origin();
let processed: string = text.to_uppercase().trim();

// Method chaining
let result: string = input
    .trim()
    .to_lowercase()
    .replace(" ", "_")
    .to_string();

// Mixed field access and method calls
let user_email: string = database
    .get_user(user_id)
    .profile
    .contact_info
    .email
    .to_lowercase();
```

**Access Rules:**
- **Field Access**: `object.field` (direct member access)
- **Method Calls**: `object.method(args)` (function call on object)
- **Chaining**: Left-to-right evaluation, each operation returns new value
- **Type Safety**: All access checked at compile time

### Index Access Syntax

Array and slice indexing with bounds checking:

```asthra
// Array indexing
let first: i32 = numbers[0];
let middle: i32 = numbers[2];
let last: i32 = numbers[numbers.len() - 1];

// Slice indexing  
let substring: string = text[1..5];    // Slice from index 1 to 4
let suffix: string = text[10..];       // From index 10 to end
let prefix: string = text[..5];        // From start to index 4

// Nested array access
let cell: i32 = matrix[1][2];          // Second row, third column
let deep: i32 = data[x][y][z];         // Multi-dimensional access

// Expression-based indexing
let dynamic: i32 = array[compute_index()];
let calculated: i32 = grid[(x + y) % size][z * 2];
```

**Index Rules:**
- **Zero-Based**: Arrays and slices start at index 0
- **Bounds Checking**: Runtime bounds validation for safety
- **Range Syntax**: `start..end` (exclusive), `start..=end` (inclusive)
- **Expression Indices**: Any expression evaluating to valid index type

> **📚 Access Operators**: See [Syntax Reference - Access Operators](syntax-reference.md#access-operators) for complete documentation of `.`, `[]`, and `::` operators.

### Pointer Types
* **Mutable Pointer:** `*mut Type` (e.g., `*mut int`)
* **Immutable Pointer:** `*const Type` (e.g., `*const u8`)

### Result Type
* **Error Handling:** `Result<T, E>` for explicit error handling with pattern matching

## Variables and Mutability

Variables are declared using `let`. Asthra bindings are immutable by default; once a variable is bound to a value, the binding cannot be changed to point to a different value.

Mutability of the *data itself* is managed through pointers, typically `*mut Type`. If a `let` binding holds a `*mut Type`, the data *pointed to* can be modified (usually within an `unsafe` block).

```asthra
priv fn example_variables(void) -> void " + 
    let x: int = 10; // Immutable binding to the value 10
    // x = 20; // ERROR: Cannot re-assign 'x'

    let p_mut: *mut int = unsafe " +  libc.malloc(sizeof(int)) as *mut int  + "; // p_mut holds a mutable pointer
    unsafe " + 
        *p_mut = 20; // Modifying the data pointed to by p_mut
        libc.free(p_mut as *mut void);
     + "

    let message: string = "Hello"; // Immutable string data
    let numbers: []int = [1, 2, 3]; // Slice of integers using array literal syntax
 + "
```

**Type Annotation Requirement (v1.15+)**:
All variable declarations must include explicit type annotations for AI generation reliability and code clarity. This eliminates ambiguity and ensures consistent, self-documenting code.

```asthra
// ✅ REQUIRED: Explicit type annotations
let user_id: i32 = 42;
let username: string = "alice";
let is_active: bool = true;
let scores: []f64 = [95.5, 87.2, 92.1];

// ❌ ERROR: Missing type annotations (not allowed in v1.15+)
// let user_id: i32 = 42;           // Error: Expected ':' after variable name
// let username: String = "alice";     // Error: Expected ':' after variable name
// let is_active: bool = true;       // Error: Expected ':' after variable name
```

## Operators and Expressions

Standard arithmetic, comparison, and logical operators are supported with unambiguous precedence rules.

> **📚 Complete Operator Reference**: For detailed operator tables, precedence hierarchy, and expression grouping rules, see [Syntax Reference - Operators](syntax-reference.md#operators) and [Syntax Reference - Expression Grouping](syntax-reference.md#expression-grouping).

### Arithmetic Operators
* `+` (addition), `-` (subtraction), `*` (multiplication), `/` (division)

### Comparison Operators
* `==` (equal), `!=` (not equal)
* `<` (less than), `>` (greater than)
* `<=` (less than or equal), `>=` (greater than or equal)

### Logical Operators
* `&&` (AND), `||` (OR), `!` (NOT)

### Pointer Operators
* `*` (dereference), `&` (address-of)

### Special Operators
* `sizeof(Type)`: Returns the size of a type in bytes
* `.len`: Accesses the length of a slice (e.g., `my_slice.len`)
* Enum Access: `MyEnum.Variant` (e.g., `Result.Ok`)

```asthra
priv fn example_expressions(void) -> bool " + 
    let a: int = 5;
    let b: int = 10;
    let result: int = (a + b) * 2; // 30 - clear precedence rules
    let is_true: bool = (result > 25) && (a != b); // true && true -> true
    
    let numbers: []int = [10, 20, 30]; // Array literal syntax
    let length_of_numbers: usize = numbers.len; // 3

    let size_of_int_type: usize = sizeof(int);
    return is_true;
 + "
```

## String Operations

Asthra provides comprehensive string handling capabilities with both single-line and multi-line string literals, along with deterministic string operations.

### String Literal Types (v1.23+)

Asthra supports three types of string literals to handle different use cases:

#### Single-line Strings (Traditional)
```asthra
priv fn single_line_examples(void) -> void {
    let simple_message: string = "Hello, Asthra!";
    let path_with_escapes: string = "C:\\Program Files\\MyApp\\config.json";
    let formatted_line: string = "User: john@example.com\nStatus: active\n";
}
```

#### Raw Multi-line Strings (`r"""` syntax)
Raw multi-line strings preserve content exactly as written without processing escape sequences - perfect for templates and configuration:

```asthra
priv fn raw_multiline_examples(void) -> void {
    // SQL query construction - no escape processing needed
    let user_query: string = r"""
        SELECT u.id, u.username, u.email, p.first_name, p.last_name
        FROM users u
        LEFT JOIN profiles p ON u.id = p.user_id
        WHERE u.active = true
        ORDER BY u.created_at DESC
    """;
    
    // Shell script generation
    let install_script: string = r"""
        #!/bin/bash
        set -euo pipefail
        
        echo "Installing application..."
        mkdir -p /opt/myapp
        cp -r ./dist/* /opt/myapp/
        chmod +x /opt/myapp/bin/myapp
        
        systemctl enable myapp
        systemctl start myapp
        echo "Installation complete!"
    """;
    
    // Configuration file templates
    let nginx_config: string = r"""
        server {
            listen 80;
            server_name example.com;
            location / {
                proxy_pass http://127.0.0.1:3000;
                proxy_set_header Host $host;
                proxy_set_header X-Real-IP $remote_addr;
            }
        }
    """;
}
```

#### Processed Multi-line Strings (`"""` syntax)
Processed multi-line strings support escape sequences while maintaining multi-line structure:

```asthra
priv fn processed_multiline_examples(void) -> void {
    // Help documentation with escape sequences
    let help_text: string = """
        Usage: myapp [OPTIONS] <command>
        
        Commands:
            start       Start the application server
            stop        Stop the application server
            status      Show current status
        
        Options:
            -h, --help          Show this help message
            -v, --verbose       Enable verbose output
            -c, --config FILE   Use custom configuration file
        
        Examples:
            myapp start --port 3000
            myapp config set database.host "localhost"
        """;
    
    // Error message templates with formatting
    let error_template: string = """
        ===============================================
        ERROR REPORT - Application Failure
        ===============================================
        
        Timestamp: {timestamp}
        Component: {component}
        Error Code: {error_code}
        Message: {error_message}
        
        Stack Trace:
        {stack_trace}
        
        Please contact support with this information.
        ===============================================
    """;
}
```

### Multi-line String Benefits

Multi-line strings provide significant advantages for systems programming:

- **Template Generation**: Natural syntax for configuration files, scripts, and documentation
- **AI-Friendly**: Clear, predictable patterns that AI models can reliably generate
- **Reduced Complexity**: Eliminates complex escape sequence management in long strings
- **Better Readability**: Content structure is preserved and easy to understand
- **Zero Breaking Changes**: Full backward compatibility with existing single-line strings

### Traditional String Operations

Asthra also provides deterministic string operations using the concatenation operator:

### String Concatenation (`+` operator)

```asthra
priv fn string_concatenation_example(void) -> string " + 
    let worker_id: int = 42;
    let status: string = "active";
    
    // String concatenation - explicit types for clarity
    let message: string = "Worker " + worker_id + " is " + status;
    return message; // "Worker 42 is active
```

### Complex String Building

For complex string formatting, use multiple concatenation operations:

```asthra
priv fn complex_string_example(void) -> string "completed";
    
    // Build complex strings with concatenation - explicit types for clarity
    let detailed_message: string = "Worker " + worker_id + " processed " + task_count + " tasks with status: " + status;
    return detailed_message; // "Worker 42 processed 15 tasks with status: completed"
}
```

String concatenation provides deterministic and predictable behavior for AI code generation.

## Control Flow

### If/Else Statements

Standard conditional execution with clear syntax:

```asthra
priv fn check_number(num: int) -> void " + 
    if num > 0 {
        // Positive number logic
     + " else if num < 0 " + 
        // Negative number logic
     + " else " + 
        // Zero logic
     + "
}
```

### For Loops (Range-based)

Iterates over a numerical range:

```asthra
priv fn count_up_to(n: int) -> void " + 
    for i in range(0, n) { // i from 0 to n-1
        // Loop body
     + "
}
```

### Return Statements

Exits a function, optionally returning a value:

```asthra
priv fn add(a: int, b: int) -> int " + 
    return a + b;
 + "
```

## Functions

Functions are declared with `fn`, followed by name, parameters, return type, and body:

```asthra
priv fn greet(name: string) -> void " +  // void indicates no return value
    unsafe " +  libc.puts(name);  + " // Assuming direct string literal passing for simplicity
 + "

priv fn add(a: int, b: int) -> int " + 
    return a + b;
 + "
```

## Blocks

Code blocks `{}` group statements and define scope. Variables declared within a block are only accessible within that block.

## Main Function Return Types

The `main` function serves as the entry point for Asthra programs and supports multiple return types, each with specific semantics for process exit codes:

### Integer Return (`i32`)
```asthra
pub fn main(none) -> i32 {
    return 0;  // Direct exit code (0 = success)
}
```

**Use Cases**: Command-line tools, build scripts, system utilities
- Return value becomes process exit code directly
- 0 indicates success, non-zero indicates error

### Void Return (`void`)
```asthra
pub fn main(none) -> void {
    print("Hello, Asthra!");
    return ();  // Implicit exit code 0
}
```

**Use Cases**: Simple programs, tutorials, demonstrations
- Always exits with success code (0)
- Uses unit type `()` for return statement

### Result Return (`Result<i32, string>`)
```asthra
pub fn main(none) -> Result<i32, string> {
    match initialize_application() {
        Ok(_) => Result.Ok(0),
        Err(error) => Result.Err("Initialization failed: " + error)
    }
}
```

**Use Cases**: Applications with rich error reporting
- `Result.Ok(code)` exits with specified code
- `Result.Err(message)` prints message to stderr, exits with code 1

### Never Return (`Never`)
```asthra
pub fn main(none) -> Never {
    initialize_service();
    loop {
        handle_requests();  // Infinite service loop
    }
    // Never returns normally
}
```

**Use Cases**: System services, daemon processes, embedded systems
- Function never returns through normal control flow
- Terminates via `panic()`, `exit()`, or infinite loops
- Enables compiler optimizations and exhaustiveness checking

## Enhanced Error Handling with `Result<T,E>` and Pattern Matching

Asthra uses a `Result<T, E>` type for explicit error handling with systematic pattern matching. A function that might fail returns a `Result`, which is either an `Ok(T)` value (success) or an `Err(E)` value (error).

### Basic Result Usage

```asthra
priv fn divide(numerator: float, denominator: float) -> Result<float, string> {if denominator == 0.0 " + 
        return Result.Err("Division by zero!");
     + "
    return Result.Ok(numerator / denominator);
 + "
```

### Pattern Matching with `match` Statements

```asthra
priv fn process_division_with_match(a: float, b: float) -> void "SUCCESS", "Division result: " + value);
        },
        Result.Err(error_message) => " + 
            log("ERROR", "Division failed: " + error_message);
         + "
    }
}
```

### Selective Pattern Matching with `if let`

```asthra
priv fn process_division_with_if_let(a: float, b: float) -> void "SUCCESS", "Got result: " + value);
    } else " + 
        log("ERROR", "Operation failed");
     + "
}
```

### Nested Pattern Matching

```asthra
priv fn complex_error_handling(void) -> Result<string, string> "Success: " + final_value);
                },
                Result.Err(second_error) => " + 
                    return Result.Err("Second operation failed: " + second_error);
                 + "
            }
        },
        Result.Err(first_error) => "First operation failed: " + first_error);
        }
    }
}
```

### Match Statement Limitations

> **Important**: In Asthra, `match` is a **statement**, not an expression. You cannot use `match` in contexts that require expressions (like `return match ...` or `let x = match ...`). For detailed examples and workarounds, see [Match Statement Limitations](../spec/advanced-features.md#match-statement-limitations) in the Advanced Features specification.

## Object-Oriented Programming

✅ **Status: FULLY IMPLEMENTED** - Asthra supports modern object-oriented programming through visibility control, implementation blocks, and instance methods.

Asthra provides complete object-oriented programming capabilities through three key keywords:

- **`pub`**: Controls visibility (public/private) for structs, functions, methods, and fields
- **`impl`**: Defines implementation blocks containing methods for structs
- **`self`**: Special parameter for instance methods that represents the struct instance

### Basic Struct Definition

Define data structures with optional visibility modifiers:

```asthra
// Public struct with mixed field visibility
pub struct Point " + 
    pub x: f64;      // Public field - accessible from other modules
    pub y: f64;      // Public field - accessible from other modules
    private_id: i32; // Private field - internal use only (default visibility)
 + "

// Private struct (default visibility)
struct InternalData " + 
    value: i32;
    state: bool;
 + "
```

### Implementation Blocks

Use `impl` to define methods for structs:

```asthra
impl Point " + 
    // Associated function (constructor) - no self parameter
    pub fn new(x: f64, y: f64) -> Point {
        return Point { 
            x: x, 
            y: y, 
            private_id: 42 
         + ";
    }
    
    // Alternative constructor
    pub fn origin(void) -> Point " + 
        return Point::new(0.0, 0.0);
     + "
    
    // Instance method with self parameter
    pub fn distance_from_origin(self) -> f64 " + 
        return sqrt(self.x * self.x + self.y * self.y);
     + "
    
    // Instance method with additional parameters
    pub fn distance_to(self, other: Point) -> f64 " + 
        let dx: f64 = self.x - other.x;
        let dy: f64 = self.y - other.y;
        return sqrt(dx * dx + dy * dy);
     + "
    
    // Instance method returning modified struct
    pub fn translate(self, dx: f64, dy: f64) -> Point " + 
        return Point::new(self.x + dx, self.y + dy);
     + "
    
    // Private method (default visibility)
    pub fn get_internal_id(self) -> i32 " + 
        return self.private_id;
     + "
}
```

### Method Call Syntax

Asthra uses different syntax for associated functions and instance methods:

```asthra
priv fn demonstrate_method_calls(void) -> void " + 
    // Associated function calls using :: syntax
    let origin: Point = Point::origin(void);
    let point: Point = Point::new(3.0, 4.0);
    
    // Instance method calls using . syntax
    let distance: f64 = point.distance_from_origin(void);
    let distance_between: f64 = point.distance_to(origin);
    
    // Method chaining
    let moved_point: Point = point
        .translate(1.0, 1.0)
        .translate(2.0, -1.0);
    
    // Accessing public fields
    let x_coord: f64 = point.x;  // ✅ Allowed - public field
    let y_coord: f64 = point.y;  // ✅ Allowed - public field
    // let id: i32 = point.private_id;  // ❌ Error - private field
 + "
```

### Visibility Rules

Asthra enforces strict visibility rules at compile time:

#### Default Visibility
All items are **private** by default:
- Structs, functions, methods, and fields
- Only accessible within the same module

#### Public Visibility  
Use `pub` keyword to make items public:
- Accessible from other modules/packages
- Can be applied to structs, functions, methods, and individual fields

```asthra
pub struct Configuration " + 
    pub host: string;        // Public: external access allowed
    pub port: i32;           // Public: external access allowed
    secret_key: []u8;        // Private: internal use only
    debug_mode: bool;        // Private: internal use only
 + "

impl Configuration " + 
    // Public constructor
    pub fn new(host: string, port: i32) -> Configuration {
        return Configuration {
            host: host,
            port: port,
            secret_key: [],
            debug_mode: false
         + ";
    }
    
    // Public getter for private field
    pub fn is_debug_enabled(self) -> bool " + 
        return self.debug_mode;
     + "
    
    // Private method for internal operations
    pub fn initialize_secrets(self) -> Configuration " + 
        // Internal implementation
        return self;
     + "
}
```

### Advanced Object-Oriented Patterns

#### Builder Pattern

Use method chaining for flexible object construction:

```asthra
pub struct HttpClient " + 
    pub timeout: i32;
    pub retries: i32;
    pub base_url: string;
    secure: bool;
 + "

impl HttpClient " + 
    pub fn new(void) -> HttpClient {
        return HttpClient {
            timeout: 30,
            retries: 3,
            base_url: "http://localhost",
            secure: false
         + ";
    }
    
    pub fn with_timeout(self, timeout: i32) -> HttpClient " + 
        return HttpClient " + 
            timeout: timeout,
            retries: self.retries,
            base_url: self.base_url,
            secure: self.secure
         + ";
     + "
    
    pub fn with_retries(self, retries: i32) -> HttpClient " + 
        return HttpClient {
            timeout: self.timeout,
            retries: retries,
            base_url: self.base_url,
            secure: self.secure
         + ";
    }
    
    pub fn secure(self) -> HttpClient " + 
        return HttpClient " + 
            timeout: self.timeout,
            retries: self.retries,
            base_url: self.base_url,
            secure: true
         + ";
     + "
}

// Usage with method chaining
priv fn create_http_client(void) -> HttpClient " + 
    return HttpClient::new(void)
        .with_timeout(60)
        .with_retries(5)
        .secure(void);
 + "
```

#### Encapsulation with Controlled Access

Use private fields with public methods for data protection:

```asthra
pub struct Counter " + 
    value: i32;        // Private: cannot be directly modified
    max_value: i32;    // Private: internal constraint
 + "

impl Counter " + 
    pub fn new(max_value: i32) -> Counter {
        return Counter {
            value: 0,
            max_value: max_value
         + ";
    }
    
    // Controlled access to private field
    pub fn get_value(self) -> i32 " + 
        return self.value;
     + "
    
    // Controlled modification with validation
    pub fn increment(self) -> Result<Counter, string> "Counter at maximum value");
        }
        
        return Result.Ok(Counter " + 
            value: self.value + 1,
            max_value: self.max_value
         + ");
    }
    
    pub fn reset(self) -> Counter " + 
        return Counter " + 
            value: 0,
            max_value: self.max_value
         + ";
     + "
}
```

### Method Types and Self Parameter Rules

#### Associated Functions
- **No `self` parameter**
- Called using `::` syntax on the type name
- Often used as constructors or utility functions
- Cannot access instance data

#### Instance Methods
- **Must have `self` as first parameter**
- Called using `.` syntax on an instance
- Can access and use instance fields
- Can modify the instance by returning a new one

#### Compilation Rules
```asthra
impl Point " + 
    // ✅ Valid: Associated function
    pub fn new(x: f64, y: f64) -> Point { ...  + "
    
    // ✅ Valid: Instance method with self first
    pub fn distance(self) -> f64 " +  ...  + "
    
    // ✅ Valid: Instance method with additional parameters
    pub fn distance_to(self, other: Point) -> f64 " +  ...  + "
    
    // ❌ Error: Associated function cannot have self
    // fn invalid_constructor(self, x: f64) -> Point " +  ...  + "
    
    // ❌ Error: self must be first parameter
    // fn invalid_method(x: f64, self) -> f64 " +  ...  + "
    
    // ❌ Error: multiple self parameters not allowed
    // fn invalid_method2(self, other_self: Point) -> f64 " +  ...  + "
}
```

### Error Handling in Object-Oriented Code

Combine object-oriented patterns with Asthra's Result type for robust error handling:

```asthra
pub struct FileReader " + 
    pub path: string;
    is_open: bool;
 + "

impl FileReader " + 
    pub fn new(path: string) -> FileReader {
        return FileReader {
            path: path,
            is_open: false
         + ";
    }
    
    pub fn open(self) -> Result<FileReader, string> {// Simulate file opening logic
        if self.path == "" " + 
            return Result.Err("Empty file path");
         + "
        
        return Result.Ok(FileReader " + 
            path: self.path,
            is_open: true
         + ");
     + "
    
    pub fn read_contents(self) -> Result<string, string> "File not open");
        }
        
        // Simulate reading file contents
        return Result.Ok("File contents here");
    }
}

// Usage with error handling
pub fn process_file(file_path: string) -> Result<string, string> {let reader: FileReader = FileReader::new(file_path);
    
    let opened_reader: FileReader = match reader.open() {
        Result.Ok(r) => r,
        Result.Err(error) => return Result.Err("Failed to open: " + error)
     + ";
    
    let contents: string = match opened_reader.read_contents() "Failed to read: " + error)
    };
    
    return Result.Ok(contents);
}
```

## Next Steps

Now that you understand the language fundamentals, you can explore:

- **[Memory Management](memory-management.md)** - Learn about Asthra's hybrid GC model
- **[C Interoperability](ffi-interop.md)** - Understand how to work with C libraries
- **[Security Features](security.md)** - Explore Asthra's security-focused features 
