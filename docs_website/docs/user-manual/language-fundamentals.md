# Language Fundamentals

**Implementation Status**: ✅ Complete | 🔧 Partial | ❌ Not Implemented  
**Last Verified**: 2025-01-07  
**Grammar Version**: v1.22  
**Test Coverage**: 95%+

This section covers the core syntax and concepts of the Asthra programming language.

&gt; **📖 Quick Reference**: For a comprehensive lookup guide to all syntax elements (keywords, operators, types, literals), see the &#91;Syntax Reference&#93;(syntax-reference.md). This chapter provides practical examples and usage patterns.

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

**For comprehensive package organization guidance, see &#91;Package Organization&#93;(package-organization.md)**

### Single-File Program Example

```asthra
package main; // Required package declaration

pub fn main() -&gt; i32 {
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

pub fn main() -&gt; i32 {
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

pub fn create_user(name: string, email: string) -&gt; User {
    return User {
        id: generate_id(),  // Can call private functions in same package
        name: name,
        email: email,
    };
}

fn generate_id() -&gt; i32 {  // Private function (no pub keyword)
    return 42;
}
```

```asthra
// utils/validation.asthra (package utils)
package utils;

import "models";

pub fn validate_email(user: models.User) -&gt; bool {
    // Can access public fields and functions
    let user_id: i32 = user.id;        // ✅ Public field accessible
    // let email: string = user.email;  // ❌ Private field not accessible
    
    return user_id &gt; 0;
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
pub fn use_imported_libraries() -&gt; void {
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

&gt; **📚 Syntax Elements**: For detailed documentation of comment syntax, operators, and expression grouping, see &#91;Syntax Reference - Comments&#93;(syntax-reference.md#comments), &#91;Syntax Reference - Operators&#93;(syntax-reference.md#operators), and &#91;Syntax Reference - Expression Grouping&#93;(syntax-reference.md#expression-grouping).

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

&gt; **📚 Complete Syntax**: See &#91;Syntax Reference - Expression Grouping&#93;(syntax-reference.md#expression-grouping) for operator precedence and statement structure rules.

### Block Structure and Nesting

Asthra uses curly braces `{}` to define code blocks with clear nesting rules:

```asthra
// Function blocks
pub fn example() -&gt; void {
    let x: i32 = 10;  // Statement inside block
    
    // Nested control flow blocks
    if x &gt; 5 {
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
        0 =&gt; "zero",
        1..=10 =&gt; "small",
        _ =&gt; "large",
    };
}

// Struct implementation blocks
impl Point {
    pub fn new(x: f64, y: f64) -&gt; Point {
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
#&#91;annotation_name(parameters)&#93;
priv fn example_function(void) -&gt; void " + 
    // Function implementation
 + "
```

### Common Annotations
- `#&#91;human_review(priority)&#93;` - Indicates human review requirements
- `#&#91;ownership(type)&#93;` - Memory management hints  
- `#&#91;constant_time&#93;` - Security-critical timing requirements
- `#&#91;transfer_full&#93;` - FFI ownership transfer semantics

### Annotation Placement
Annotations can be applied to:
- Function declarations
- Struct declarations
- External function declarations
- Method declarations
- Variable declarations (for ownership)

For complete annotation documentation, see &#91;Annotations &amp; Semantic Tags&#93;(annotations.md).

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
priv fn numeric_literal_examples(void) -&gt; void " + 
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
* **Slice Type:** `&#91;&#93;Type` (e.g., `&#91;&#93;int` for a slice of integers). Slices are dynamically sized views over a contiguous block of memory with formalized `SliceHeader<T>` internal structure containing a pointer to the data and a length.
* **Struct Type:** `IDENT` (e.g., `Point` for a struct type named Point).

&gt; **📚 Type Syntax**: For complete documentation of generic types (`<T>`), pointer types (`*mut T`, `*const T`), and array types (`&#91;N&#93;T`), see &#91;Syntax Reference - Type Syntax&#93;(syntax-reference.md#type-syntax).

### Array and Struct Initialization Patterns

Asthra provides comprehensive initialization syntax for arrays and structs:

#### Array Initialization
```asthra
// Basic array literals
let numbers: &#91;5&#93;i32 = &#91;1, 2, 3, 4, 5&#93;;
let names: &#91;3&#93;string = &#91;"Alice", "Bob", "Charlie"&#93;;

// Empty array initialization
let empty_array: &#91;0&#93;i32 = &#91;&#93;;
let buffer: &#91;1024&#93;u8 = &#91;0; 1024&#93;;  // Repeated element syntax

// Nested array initialization
let matrix: &#91;2&#93;&#91;3&#93;i32 = &#91;
    [1, 2, 3&#93;,
    &#91;4, 5, 6&#93;,
];

// Array of structs
let points: &#91;3&#93;Point = &#91;
    Point { x: 0.0, y: 0.0 },
    Point { x: 1.0, y: 1.0 },
    Point { x: 2.0, y: 2.0 },
&#93;;
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

&gt; **📚 Initialization Examples**: See &#91;Syntax Examples&#93;(../examples/syntax-completion-examples.asthra) for comprehensive array and struct initialization patterns, and &#91;Syntax Reference - Literals&#93;(syntax-reference.md#literals) for array literal syntax.

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
let first: i32 = numbers&#91;0&#93;;
let middle: i32 = numbers&#91;2&#93;;
let last: i32 = numbers&#91;numbers.len() - 1&#93;;

// Slice indexing  
let substring: string = text&#91;1..5&#93;;    // Slice from index 1 to 4
let suffix: string = text&#91;10..&#93;;       // From index 10 to end
let prefix: string = text&#91;..5&#93;;        // From start to index 4

// Nested array access
let cell: i32 = matrix&#91;1&#93;&#91;2&#93;;          // Second row, third column
let deep: i32 = data&#91;x&#93;&#91;y&#93;&#91;z&#93;;         // Multi-dimensional access

// Expression-based indexing
let dynamic: i32 = array&#91;compute_index()&#93;;
let calculated: i32 = grid&#91;(x + y) % size&#93;&#91;z * 2&#93;;
```

**Index Rules:**
- **Zero-Based**: Arrays and slices start at index 0
- **Bounds Checking**: Runtime bounds validation for safety
- **Range Syntax**: `start..end` (exclusive), `start..=end` (inclusive)
- **Expression Indices**: Any expression evaluating to valid index type

&gt; **📚 Access Operators**: See &#91;Syntax Reference - Access Operators&#93;(syntax-reference.md#access-operators) for complete documentation of `.`, `&#91;&#93;`, and `::` operators.

### Pointer Types
* **Mutable Pointer:** `*mut Type` (e.g., `*mut int`)
* **Immutable Pointer:** `*const Type` (e.g., `*const u8`)

### Result Type
* **Error Handling:** `Result&lt;T, E&gt;` for explicit error handling with pattern matching

## Variables and Mutability

Variables are declared using `let`. Asthra bindings are immutable by default; once a variable is bound to a value, the binding cannot be changed to point to a different value.

Mutability of the *data itself* is managed through pointers, typically `*mut Type`. If a `let` binding holds a `*mut Type`, the data *pointed to* can be modified (usually within an `unsafe` block).

```asthra
priv fn example_variables(void) -&gt; void " + 
    let x: int = 10; // Immutable binding to the value 10
    // x = 20; // ERROR: Cannot re-assign 'x'

    let p_mut: *mut int = unsafe " +  libc.malloc(sizeof(int)) as *mut int  + "; // p_mut holds a mutable pointer
    unsafe " + 
        *p_mut = 20; // Modifying the data pointed to by p_mut
        libc.free(p_mut as *mut void);
     + "

    let message: string = "Hello"; // Immutable string data
    let numbers: &#91;&#93;int = &#91;1, 2, 3&#93;; // Slice of integers using array literal syntax
 + "
```

**Type Annotation Requirement (v1.15+)**:
All variable declarations must include explicit type annotations for AI generation reliability and code clarity. This eliminates ambiguity and ensures consistent, self-documenting code.

```asthra
// ✅ REQUIRED: Explicit type annotations
let user_id: i32 = 42;
let username: string = "alice";
let is_active: bool = true;
let scores: &#91;&#93;f64 = &#91;95.5, 87.2, 92.1&#93;;

// ❌ ERROR: Missing type annotations (not allowed in v1.15+)
// let user_id: i32 = 42;           // Error: Expected ':' after variable name
// let username: String = "alice";     // Error: Expected ':' after variable name
// let is_active: bool = true;       // Error: Expected ':' after variable name
```

## Operators and Expressions

Standard arithmetic, comparison, and logical operators are supported with unambiguous precedence rules.

&gt; **📚 Complete Operator Reference**: For detailed operator tables, precedence hierarchy, and expression grouping rules, see &#91;Syntax Reference - Operators&#93;(syntax-reference.md#operators) and &#91;Syntax Reference - Expression Grouping&#93;(syntax-reference.md#expression-grouping).

### Arithmetic Operators
* `+` (addition), `-` (subtraction), `*` (multiplication), `/` (division)

### Comparison Operators
* `==` (equal), `!=` (not equal)
* `&lt;` (less than), `&gt;` (greater than)
* `&lt;=` (less than or equal), `&gt;=` (greater than or equal)

### Logical Operators
* `&amp;&amp;` (AND), `||` (OR), `!` (NOT)

### Pointer Operators
* `*` (dereference), `&amp;` (address-of)

### Special Operators
* `sizeof(Type)`: Returns the size of a type in bytes
* `.len`: Accesses the length of a slice (e.g., `my_slice.len`)
* Enum Access: `MyEnum.Variant` (e.g., `Result.Ok`)

```asthra
priv fn example_expressions(void) -&gt; bool " + 
    let a: int = 5;
    let b: int = 10;
    let result: int = (a + b) * 2; // 30 - clear precedence rules
    let is_true: bool = (result &gt; 25) &amp;&amp; (a != b); // true &amp;&amp; true -&gt; true
    
    let numbers: &#91;&#93;int = &#91;10, 20, 30&#93;; // Array literal syntax
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
priv fn single_line_examples(void) -&gt; void {
    let simple_message: string = "Hello, Asthra!";
    let path_with_escapes: string = "C:\\Program Files\\MyApp\\config.json";
    let formatted_line: string = "User: john@example.com\nStatus: active\n";
}
```

#### Raw Multi-line Strings (`r"""` syntax)
Raw multi-line strings preserve content exactly as written without processing escape sequences - perfect for templates and configuration:

```asthra
priv fn raw_multiline_examples(void) -&gt; void {
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
priv fn processed_multiline_examples(void) -&gt; void {
    // Help documentation with escape sequences
    let help_text: string = """
        Usage: myapp &#91;OPTIONS&#93; <command>
        
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
priv fn string_concatenation_example(void) -&gt; string " + 
    let worker_id: int = 42;
    let status: string = "active";
    
    // String concatenation - explicit types for clarity
    let message: string = "Worker " + worker_id + " is " + status;
    return message; // "Worker 42 is active
```

### Complex String Building

For complex string formatting, use multiple concatenation operations:

```asthra
priv fn complex_string_example(void) -&gt; string "completed";
    
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
priv fn check_number(num: int) -&gt; void " + 
    if num &gt; 0 {
        // Positive number logic
     + " else if num &lt; 0 " + 
        // Negative number logic
     + " else " + 
        // Zero logic
     + "
}
```

### For Loops (Range-based)

Iterates over a numerical range:

```asthra
priv fn count_up_to(n: int) -&gt; void " + 
    for i in range(0, n) { // i from 0 to n-1
        // Loop body
     + "
}
```

### Return Statements

Exits a function, optionally returning a value:

```asthra
priv fn add(a: int, b: int) -&gt; int " + 
    return a + b;
 + "
```

## Functions

Functions are declared with `fn`, followed by name, parameters, return type, and body:

```asthra
priv fn greet(name: string) -&gt; void " +  // void indicates no return value
    unsafe " +  libc.puts(name);  + " // Assuming direct string literal passing for simplicity
 + "

priv fn add(a: int, b: int) -&gt; int " + 
    return a + b;
 + "
```

## Blocks

Code blocks `{}` group statements and define scope. Variables declared within a block are only accessible within that block.

## Enhanced Error Handling with `Result&lt;T,E&gt;` and Pattern Matching

Asthra uses a `Result&lt;T, E&gt;` type for explicit error handling with systematic pattern matching. A function that might fail returns a `Result`, which is either an `Ok(T)` value (success) or an `Err(E)` value (error).

### Basic Result Usage

```asthra
priv fn divide(numerator: float, denominator: float) -&gt; Result&lt;float, string&gt; {if denominator == 0.0 " + 
        return Result.Err("Division by zero!");
     + "
    return Result.Ok(numerator / denominator);
 + "
```

### Pattern Matching with `match` Statements

```asthra
priv fn process_division_with_match(a: float, b: float) -&gt; void "SUCCESS", "Division result: " + value);
        },
        Result.Err(error_message) =&gt; " + 
            log("ERROR", "Division failed: " + error_message);
         + "
    }
}
```

### Selective Pattern Matching with `if let`

```asthra
priv fn process_division_with_if_let(a: float, b: float) -&gt; void "SUCCESS", "Got result: " + value);
    } else " + 
        log("ERROR", "Operation failed");
     + "
}
```

### Nested Pattern Matching

```asthra
priv fn complex_error_handling(void) -&gt; Result&lt;string, string&gt; "Success: " + final_value);
                },
                Result.Err(second_error) =&gt; " + 
                    return Result.Err("Second operation failed: " + second_error);
                 + "
            }
        },
        Result.Err(first_error) =&gt; "First operation failed: " + first_error);
        }
    }
}
```

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
    pub fn new(x: f64, y: f64) -&gt; Point {
        return Point { 
            x: x, 
            y: y, 
            private_id: 42 
         + ";
    }
    
    // Alternative constructor
    pub fn origin(void) -&gt; Point " + 
        return Point::new(0.0, 0.0);
     + "
    
    // Instance method with self parameter
    pub fn distance_from_origin(self) -&gt; f64 " + 
        return sqrt(self.x * self.x + self.y * self.y);
     + "
    
    // Instance method with additional parameters
    pub fn distance_to(self, other: Point) -&gt; f64 " + 
        let dx: f64 = self.x - other.x;
        let dy: f64 = self.y - other.y;
        return sqrt(dx * dx + dy * dy);
     + "
    
    // Instance method returning modified struct
    pub fn translate(self, dx: f64, dy: f64) -&gt; Point " + 
        return Point::new(self.x + dx, self.y + dy);
     + "
    
    // Private method (default visibility)
    pub fn get_internal_id(self) -&gt; i32 " + 
        return self.private_id;
     + "
}
```

### Method Call Syntax

Asthra uses different syntax for associated functions and instance methods:

```asthra
priv fn demonstrate_method_calls(void) -&gt; void " + 
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
    secret_key: &#91;&#93;u8;        // Private: internal use only
    debug_mode: bool;        // Private: internal use only
 + "

impl Configuration " + 
    // Public constructor
    pub fn new(host: string, port: i32) -&gt; Configuration {
        return Configuration {
            host: host,
            port: port,
            secret_key: &#91;&#93;,
            debug_mode: false
         + ";
    }
    
    // Public getter for private field
    pub fn is_debug_enabled(self) -&gt; bool " + 
        return self.debug_mode;
     + "
    
    // Private method for internal operations
    pub fn initialize_secrets(self) -&gt; Configuration " + 
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
    pub fn new(void) -&gt; HttpClient {
        return HttpClient {
            timeout: 30,
            retries: 3,
            base_url: "http://localhost",
            secure: false
         + ";
    }
    
    pub fn with_timeout(self, timeout: i32) -&gt; HttpClient " + 
        return HttpClient " + 
            timeout: timeout,
            retries: self.retries,
            base_url: self.base_url,
            secure: self.secure
         + ";
     + "
    
    pub fn with_retries(self, retries: i32) -&gt; HttpClient " + 
        return HttpClient {
            timeout: self.timeout,
            retries: retries,
            base_url: self.base_url,
            secure: self.secure
         + ";
    }
    
    pub fn secure(self) -&gt; HttpClient " + 
        return HttpClient " + 
            timeout: self.timeout,
            retries: self.retries,
            base_url: self.base_url,
            secure: true
         + ";
     + "
}

// Usage with method chaining
priv fn create_http_client(void) -&gt; HttpClient " + 
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
    pub fn new(max_value: i32) -&gt; Counter {
        return Counter {
            value: 0,
            max_value: max_value
         + ";
    }
    
    // Controlled access to private field
    pub fn get_value(self) -&gt; i32 " + 
        return self.value;
     + "
    
    // Controlled modification with validation
    pub fn increment(self) -&gt; Result&lt;Counter, string&gt; "Counter at maximum value");
        }
        
        return Result.Ok(Counter " + 
            value: self.value + 1,
            max_value: self.max_value
         + ");
    }
    
    pub fn reset(self) -&gt; Counter " + 
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
    pub fn new(x: f64, y: f64) -&gt; Point { ...  + "
    
    // ✅ Valid: Instance method with self first
    pub fn distance(self) -&gt; f64 " +  ...  + "
    
    // ✅ Valid: Instance method with additional parameters
    pub fn distance_to(self, other: Point) -&gt; f64 " +  ...  + "
    
    // ❌ Error: Associated function cannot have self
    // fn invalid_constructor(self, x: f64) -&gt; Point " +  ...  + "
    
    // ❌ Error: self must be first parameter
    // fn invalid_method(x: f64, self) -&gt; f64 " +  ...  + "
    
    // ❌ Error: multiple self parameters not allowed
    // fn invalid_method2(self, other_self: Point) -&gt; f64 " +  ...  + "
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
    pub fn new(path: string) -&gt; FileReader {
        return FileReader {
            path: path,
            is_open: false
         + ";
    }
    
    pub fn open(self) -&gt; Result&lt;FileReader, string&gt; {// Simulate file opening logic
        if self.path == "" " + 
            return Result.Err("Empty file path");
         + "
        
        return Result.Ok(FileReader " + 
            path: self.path,
            is_open: true
         + ");
     + "
    
    pub fn read_contents(self) -&gt; Result&lt;string, string&gt; "File not open");
        }
        
        // Simulate reading file contents
        return Result.Ok("File contents here");
    }
}

// Usage with error handling
pub fn process_file(file_path: string) -&gt; Result&lt;string, string&gt; {let reader: FileReader = FileReader::new(file_path);
    
    let opened_reader: FileReader = match reader.open() {
        Result.Ok(r) =&gt; r,
        Result.Err(error) =&gt; return Result.Err("Failed to open: " + error)
     + ";
    
    let contents: string = match opened_reader.read_contents() "Failed to read: " + error)
    };
    
    return Result.Ok(contents);
}
```

## Next Steps

Now that you understand the language fundamentals, you can explore:

- **&#91;Memory Management&#93;(memory-management.md)** - Learn about Asthra's hybrid GC model
- **&#91;C Interoperability&#93;(ffi-interop.md)** - Understand how to work with C libraries
- **&#91;Security Features&#93;(security.md)** - Explore Asthra's security-focused features 
