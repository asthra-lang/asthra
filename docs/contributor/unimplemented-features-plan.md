# Asthra Unimplemented Features Plan

**Last Updated**: January 2025  
**Status**: Planning Document  
**Grammar Version**: Current (grammar.txt)

## Overview

This document outlines features that are defined in Asthra's grammar but not yet implemented, or would be valuable additions that conform to the existing grammar. All features listed here are compatible with Asthra's design principles and would enhance the language while maintaining AI-generation efficiency.

## Priority 1: Core Language Features

These features are defined in the grammar and are essential for a complete implementation.

### 1.1 Boolean Logical Operators
**Grammar Reference**: Lines 114-115
```asthra
// Currently not working:
if x > 5 && y < 10 { ... }  // Logical AND
if x < 0 || x > 100 { ... }  // Logical OR
```
**Implementation**: Add `&&` and `||` operator support in parser and code generation.

### 1.2 Bitwise Operators
**Grammar Reference**: Lines 116-121
```asthra
// Bitwise operations defined but not implemented:
let flags = a | b;    // Bitwise OR
let mask = a & b;     // Bitwise AND  
let xor = a ^ b;      // Bitwise XOR
let shifted = a << 2; // Left shift
let rshift = a >> 2;  // Right shift
```
**Implementation**: Add bitwise operator support in expression evaluation.

### 1.3 Pattern Matching (Match Statements)
**Grammar Reference**: Lines 98-99, 104-110
```asthra
match value {
    Some(x) => { log("Got value"); },
    None => { log("No value"); }
}
```
**Implementation**: Full pattern matching system with exhaustiveness checking.

### 1.4 For Loops
**Grammar Reference**: Line 94
```asthra
for item in collection {
    log(item);
}
```
**Implementation**: Iterator protocol and for-in loop support.

### 1.5 If-Let Statements
**Grammar Reference**: Line 93
```asthra
if let Some(value) = optional_value {
    // Use value
} else {
    // Handle None case
}
```
**Implementation**: Pattern matching in conditional contexts.

## Priority 2: Type System Features

### 2.1 Tuple Types and Literals
**Grammar Reference**: Lines 72, 105, 152
```asthra
let point: (i32, i32) = (10, 20);
let (x, y) = point;  // Destructuring
```
**Implementation**: Tuple type creation, literals, and pattern matching.

### 2.2 Generic Types
**Grammar Reference**: Lines 54-56, 80
```asthra
struct List<T> {
    items: []T
}

pub fn identity<T>(value: T) -> T {
    return value;
}
```
**Implementation**: Generic type parameters for structs, enums, and functions.

### 2.3 Associated Functions
**Grammar Reference**: Line 139
```asthra
// Static methods
let result = String::from_slice(data);
let value = Option<i32>::default();
```
**Implementation**: Type-associated function syntax and resolution.

### 2.4 Enum Variant Constructors
**Grammar Reference**: Line 141
```asthra
enum Status {
    Ok(i32),
    Error(string)
}

let s = Status.Ok(42);
let e = Status.Error("failed");
```
**Implementation**: Enum variant construction syntax.

## Priority 3: Enhanced Literals and Syntax

### 3.1 Multi-line Strings
**Grammar Reference**: Lines 229-231
```asthra
// Raw multi-line strings (no escape processing)
let query = r"""
    SELECT * FROM users
    WHERE active = true
""";

// Processed multi-line strings (with escapes)
let message = """
    Hello, World!
    This is line 2.\n
    This is line 3.
""";
```
**Implementation**: Multi-line string literal support in lexer.

### 3.2 Numeric Literal Formats
**Grammar Reference**: Lines 239-241
```asthra
let hex = 0xFF;        // Hexadecimal
let binary = 0b1010;   // Binary
let octal = 0o755;     // Octal
```
**Implementation**: Alternative numeric literal parsing.

### 3.3 Slice Operations
**Grammar Reference**: Lines 133-137
```asthra
let subset = array[1:5];     // Slice from index 1 to 5
let tail = array[1:];        // Slice from index 1 to end
let head = array[:5];        // Slice from start to index 5
let copy = array[:];         // Full slice copy
```
**Implementation**: Array/slice subsetting operations.

## Priority 4: Advanced Features

### 4.1 Method Implementation Blocks
**Grammar Reference**: Lines 59-62
```asthra
impl Point {
    pub fn new(x: i32, y: i32) -> Point {
        return Point { x: x, y: y };
    }
    
    pub fn distance(self, other: Point) -> f64 {
        // Calculate distance
    }
}
```
**Implementation**: Full object-oriented programming support.

### 4.2 External Function Interface (FFI)
**Grammar Reference**: Line 42, Lines 27-29
```asthra
extern "C" fn malloc(size: usize) -> #[transfer_full] *mut void;
extern "C" fn free(#[transfer_full] ptr: *mut void);
```
**Implementation**: Complete FFI with ownership annotations.

### 4.3 Annotation System
**Grammar Reference**: Lines 20-29
```asthra
#[human_review(high)]
pub fn critical_function(none) -> void { ... }

#[constant_time]
pub fn crypto_compare(a: []u8, b: []u8) -> bool { ... }

#[ownership(pinned)]
struct DMABuffer { ... }
```
**Implementation**: Full annotation processing and validation.

## Priority 5: Quality of Life Improvements

### 5.1 Additional Predeclared Functions
**Grammar Conforming**: Following existing pattern (lines 198-204)
```asthra
print("Hello");           // Print without newline
println("Hello, World!"); // Print with newline
assert(condition);        // Runtime assertion
debug("value: ", value);  // Debug output
```
**Implementation**: Extend predeclared function set.

### 5.2 Const Expression Evaluation
**Grammar Reference**: Lines 32-35
```asthra
const BUFFER_SIZE: usize = 1024;
const MAX_VALUE: i32 = MIN_VALUE + 100;
const ARRAY_SIZE: usize = sizeof(Header) * 8;
```
**Implementation**: Compile-time constant evaluation.

### 5.3 Type Aliases
**Grammar Conforming**: Would use existing type syntax
```asthra
type NodeId = u64;
type Result<T> = Result<T, Error>;
type Callback = fn(i32) -> void;
```
**Implementation**: Type aliasing for better code organization.

## Implementation Strategy

### Phase 1: Core Operators (1-2 weeks)
- Implement `&&` and `||` operators
- Add bitwise operators (`&`, `|`, `^`, `<<`, `>>`)
- Comprehensive test coverage

### Phase 2: Control Flow (2-3 weeks)
- For loops with iterator support
- If-let pattern matching
- Basic match statements

### Phase 3: Type System (3-4 weeks)
- Tuple types and literals
- Generic type parameters (basic)
- Enum variant constructors

### Phase 4: Advanced Features (4-6 weeks)
- Full pattern matching
- Method implementation blocks
- FFI with annotations
- Associated functions

### Phase 5: Polish (2-3 weeks)
- Multi-line strings
- Numeric literal formats
- Additional predeclared functions
- Slice operations

## Testing Requirements

Each feature implementation must include:
1. Unit tests for parser changes
2. Semantic analysis tests
3. Code generation tests
4. BDD feature files with scenarios
5. Integration tests
6. Documentation updates

## Compatibility Considerations

All features must:
- Maintain backward compatibility
- Follow AI-friendly patterns
- Support deterministic behavior
- Include clear error messages
- Update the Master Implementation Status

## Success Metrics

- 100% grammar compliance
- Zero regression in existing tests
- Clear, predictable syntax for AI generation
- Performance within 10% of equivalent C code
- Comprehensive documentation