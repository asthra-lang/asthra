# AI Code Generation Guidelines

**Version:** 1.0  
**Created:** 2024-12-30  
**Target Audience:** AI Models, Code Generation Tools, Developers using AI assistance  

## Overview

This guide provides specific patterns and best practices for generating Asthra code using AI models. Asthra is designed with AI generation efficiency as a core principle, offering familiar syntax patterns that work reliably with modern AI code generators.

## Core Design Philosophy for AI Generation

Asthra follows these principles to maximize AI generation effectiveness:

1\. **Minimal Syntax for Maximum AI Generation Efficiency**: Simple, predictable patterns that AI models can reliably learn and reproduce
2\. **Familiar Patterns**: Syntax closely matches Go, Rust, and C patterns that AI models are trained on
3\. **Deterministic Execution**: Consistent behavior and error messages across all platforms
4\. **Pragmatic Gradualism**: Essential features with clear complexity boundaries

## Import Statement Generation

### ✅ AI-Friendly Import Patterns

**Standard Library Imports (Always Safe):**
```asthra
import "stdlib/string";
import "stdlib/io";
import "stdlib/collections";
import "stdlib/collections/hashmap";
import "stdlib/math";
import "stdlib/time";
import "stdlib/os";
import "stdlib/fs";
import "stdlib/net";
import "stdlib/crypto";
```

**GitHub/External Package Imports:**
```asthra
import "github.com/asthra-lang/json";
import "github.com/user/package";
import "gitlab.com/organization/library";
import "bitbucket.org/team/utility";
```

**Local Relative Imports:**
```asthra
import "./utils";
import "./models/user";
import "../common/types";
import "../shared/constants";
```

**Import with Aliases:**
```asthra
import "stdlib/collections/hashmap" as HashMap;
import "github.com/vendor/very-long-package-name" as ShortName;
import "./utils/string_helpers" as Helpers;
```

### ❌ Avoid These Import Patterns

**Internal Package Access (Will Cause Build Errors):**
```asthra
// ❌ Don't generate these - user code cannot access internal packages
import "internal/runtime/memory";
import "internal/compiler/parser";
import "internal/stdlib/implementation";
```

**Malformed GitHub Paths:**
```asthra
// ❌ Missing repository name
import "github.com/user";

// ❌ Missing domain extension
import "github/user/repo";

// ❌ Invalid domain
import "githubcom/user/repo";
```

**Invalid Path Formats:**
```asthra
// ❌ Empty import path
import "";

// ❌ Spaces in path (use quotes properly)
import "std lib/string";

// ❌ Missing quotes (not string literal)
import stdlib/string;

// ❌ Complex expressions (imports must be string literals)
import ("stdlib/string");
```

**Incorrect Local Path Syntax:**
```asthra
// ❌ Missing relative path prefix
import "utils";              // Use "./utils" instead
import "common/types";       // Use "./common/types" or "../common/types"

// ❌ Absolute paths (not supported)
import "/usr/local/lib/package";
import "C:\\Windows\\package";
```

### Import Error Recovery Guidelines

When an AI-generated import fails, apply these recovery patterns:

**1. Check Import Path Format:**
```asthra
// If you get: "Invalid import path format"
// Check for proper string literal syntax and valid characters

// ❌ Problem
import invalid-path;

// ✅ Solution  
import "valid/path";
```

**2. Fix GitHub Repository Paths:**
```asthra
// If you get: "GitHub imports require format 'github.com/user/repository'"
// Ensure three components: domain, user, repository

// ❌ Problem
import "github.com/user";

// ✅ Solution
import "github.com/user/repository";
```

**3. Replace Internal Package Access:**
```asthra
// If you get: "Cannot access internal package from user code"
// Replace with stdlib or external alternatives

// ❌ Problem
import "internal/runtime/memory";

// ✅ Solution
import "stdlib/memory";
// or
import "github.com/memory-lib/safe-allocator";
```

**4. Add Local Path Prefixes:**
```asthra
// If you get: "Local imports must use './' or '../' prefix"
// Add appropriate relative path prefix

// ❌ Problem
import "utils";

// ✅ Solution
import "./utils";     // Same directory
import "../utils";    // Parent directory
```

## Language Fundamentals for AI Generation

### Package Declarations

**Always start Asthra files with a package declaration:**
```asthra
package main;        // For executable programs
package utils;       // For library packages
package models;      // For data structure packages
```

### Function Definitions

**Use clear, predictable function syntax:**
```asthra
// Basic function
pub fn add(a: i32, b: i32) -&gt; i32 " + 
    return a + b;
 + "

// Public function (accessible from other packages)
pub fn calculate(input: f64) -&gt; f64 " + 
    return input * 2.0;
 + "

// Function with multiple parameters
pub fn process_data(name: string, age: i32, active: bool) -&gt; string " + 
    return name + " is " + age.to_string();
 + "
```

### Type Declarations

**Use familiar struct and enum patterns:**
```asthra
// Simple struct
struct Point " + 
    x: f64;
    y: f64;
 + "

// Public struct with mixed visibility
pub struct User " + 
    pub name: string;       // Public field
    pub email: string;      // Public field
    internal_id: i32;       // Private field
 + "

// Enum with variants
enum Status " + 
    Active,
    Inactive,
    Pending,
 + "

// Generic enum (like Rust's Option/Result)
enum Option&lt;T&gt; " + 
    Some(T),
    None,
 + "
```

### Variable Declarations

**Use explicit type annotations when helpful:**
```asthra
// Local variables
let name: string = "Alice";
let age: i32 = 30;
let active: bool = true;

// Type inference (when type is obvious)
let point: auto = Point " +  x: 1.0, y: 2.0  + ";
let status: auto = Status.Active;

// Mutable variables
let mut counter: i32 = 0;
counter = counter + 1;
```

## Memory Management Patterns

### Ownership and Borrowing

**Use ownership annotations when working with memory:**
```asthra
// Owned data
pub fn process_owned(#&#91;owned&#93; data: Vec<i32>) -&gt; i32 " + 
    return data.len();
 + "

// Borrowed data
pub fn process_borrowed(#&#91;borrowed&#93; data: &amp;Vec<i32>) -&gt; i32 " + 
    return data.len();
 + "

// Move semantics
let data: auto = vec!&#91;1, 2, 3&#93;;
let result: String = process_owned();  // data is moved, no longer accessible
```

### Slice Operations

**Use safe slice patterns:**
```asthra
// Safe slice creation
let numbers: Vec<i32> = vec!&#91;1, 2, 3, 4, 5&#93;;
let slice: &amp;&#91;i32&#93; = &amp;numbers&#91;1..4&#93;;  // Elements &#91;2, 3, 4&#93;

// Slice with bounds checking
if index &lt; numbers.len() " + 
    let element: String = numbers&#91;index&#93;;
 + "

// Safe slice operations
pub fn sum_slice(data: &amp;&#91;i32&#93;) -&gt; i32 " + 
    let mut total = 0;
    for element in data " + 
        total = total + element;
     + "
    return total;
 + "
```

## Error Handling Patterns

### Result Type Usage

**Use Result&lt;T, E&gt; for error-prone operations:**
```asthra
// Function that can fail
pub fn divide(a: f64, b: f64) -&gt; Result&lt;f64, string&gt; " + if b == 0.0 " + 
        return Result.Err("Division by zero");
     + "
    return Result.Ok(a / b);
 + "

// Error handling with pattern matching
let result: String = divide();
match result "Result.to_string(&#125;
    Result.Err(error) =&gt; " + 
        println("Error: " + , error);
     + "
}
```

### Option Type Usage

**Use Option&lt;T&gt; for nullable values:**
```asthra
// Function that might not return a value
pub fn find_user(id: i32) -&gt; Option<User> " + 
    if id &gt; 0 " + 
        return Option.Some(User " +  name: "Alice", email: "alice@example.com);
     + "
    return Option.None;
 + "

// Option handling
let user_option: String = find_user();
match user_option " + 
    Option.Some(user) =&gt; " + 
        println("Found user.to_string(&#125;
```

## Concurrency Patterns

### Tier 1 Concurrency (Deterministic)

**Use basic spawn and await for simple concurrency:**
```asthra
// Basic task spawning
spawn " + 
    println("Running in background");
    let result: String = expensive_computation();
 + "

// Spawn with handle for result retrieval
let handle: auto = spawn_with_handle " + 
    return calculate_value(42);
 + ";

// Await the result
let value: auto = await handle;
```

### Tier 2 Concurrency (Non-deterministic)

**Require #&#91;non_deterministic&#93; annotation for advanced patterns:**
```asthra
// Advanced concurrency requires annotation
#&#91;non_deterministic&#93;
pub fn complex_coordination(none) -&gt; void "stdlib/concurrent/channels";
    import "stdlib/concurrent/coordination";
    
    let channel: auto = Channel.create<i32>(100);
    let barrier: String = Barrier.create(3);
    
    // Complex coordination logic
}
```

## FFI (Foreign Function Interface) Patterns

### Safe FFI with Annotations

**Use transfer annotations for FFI functions:**
```asthra
// C function import with transfer semantics
extern "C" " + 
    #&#91;transfer_none&#93;
    pub fn c_function_borrowed(#&#91;borrowed&#93; data: *const u8) -&gt; i32;
    
    #&#91;transfer_full&#93;  
    pub fn c_function_owned() -&gt; *mut u8;
    
    #&#91;transfer_container&#93;
    pub fn c_function_container(#&#91;owned&#93; input: *mut Container) -&gt; *mut Container;
 + "

// Safe FFI usage
pub fn use_c_functions() -&gt; void " + let data: String = allocate_data();
    let result: String = c_function_borrowed();  // data still owned by Asthra
    
    let owned_ptr: String = c_function_owned();       // Asthra takes ownership
    free_c_memory(owned_ptr);                 // Asthra responsible for cleanup
 + "
```

## Common AI Generation Mistakes to Avoid

### 1. Mixing Language Syntax

```asthra
// ❌ Don't mix Rust/Go/C syntax unnecessarily
func main() " +           // Go-style function keyword
    let x: i32 = 5;    // Rust-style variable declaration
    printf("%d", x);   // C-style print function
 + "

// ✅ Use consistent Asthra syntax
pub fn main() -&gt; void " {, x);
}
```

### 2. Incorrect Annotation Usage

```asthra
// ❌ Don't use multiple conflicting annotations
#&#91;transfer_full&#93; #&#91;transfer_none&#93;
pub fn conflicting_annotations(none) -&gt; void {+ "

// ✅ Use single appropriate annotation
#&#91;transfer_full&#93;
pub fn consistent_annotation(none) -&gt; void " + + "
```

### 3. Invalid Pattern Matching

```asthra
// ❌ Don't use incomplete pattern matching
match value " + 
    Some(x) =&gt; " +  println(" {, x);  + "
    // Missing None case - compilation error
 + "

// ✅ Use exhaustive pattern matching
match value " + 
    Some(x) =&gt; { println(, x);  + "
    None =&gt; " +  println("No value");  + "
}
```

### 4. Improper Memory Management

```asthra
// ❌ Don't ignore ownership rules
let data: auto = vec!&#91;1, 2, 3&#93;;
process_owned(data);      // data is moved here
println(, data.len()); // Error: use after move

// ✅ Understand ownership transfer
let data: auto = vec!&#91;1, 2, 3&#93;;
process_borrowed(&amp;data);   // data is borrowed, still accessible
println(, data.len()); // OK: data still owned by current scope
```

## Validation and Testing

### Use Build Tools for Validation

**Always validate AI-generated code:**
```bash
# Validate import statements and dependencies
ampu validate

# Build with error checking
ampu build

# Run tests to verify functionality  
ampu test
```

### AI Code Generation Testing

**Test AI-generated code systematically:**
1\. **Syntax Validation**: Ensure code parses correctly
2\. **Type Checking**: Verify all types are valid and consistent
3\. **Import Resolution**: Confirm all imports resolve successfully
4\. **Memory Safety**: Check ownership and borrowing rules
5\. **Functionality**: Test that generated code produces expected behavior

## Best Practices Summary

### Do:
- ✅ Use string literals for all import paths
- ✅ Follow stdlib/github.com/local path conventions
- ✅ Include package declarations at file start
- ✅ Use explicit type annotations when helpful
- ✅ Apply appropriate ownership annotations for FFI
- ✅ Handle errors with Result&lt;T, E&gt; and Option&lt;T&gt;
- ✅ Use exhaustive pattern matching
- ✅ Validate generated code with ampu tools

### Don't:
- ❌ Generate imports to internal packages from user code
- ❌ Use malformed GitHub repository paths
- ❌ Mix conflicting annotation styles
- ❌ Ignore memory ownership rules
- ❌ Create incomplete pattern matches
- ❌ Generate complex expressions in import statements
- ❌ Assume syntax from other languages will work identically

## Conclusion

Asthra is designed to be AI-friendly while maintaining safety and expressiveness. By following these guidelines, AI models can generate reliable, safe, and idiomatic Asthra code that compiles successfully and runs correctly.

For additional guidance, refer to:
- &#91;Language Fundamentals&#93;(./language-fundamentals.md)
- &#91;Memory Management&#93;(./memory-management.md)
- &#91;FFI Interop&#93;(./ffi-interop.md)
- &#91;Building Projects&#93;(./building-projects.md) 
