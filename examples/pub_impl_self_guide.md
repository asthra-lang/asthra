# Complete Guide to `pub`, `impl`, and `self` Keywords

**Version:** 1.0  
**Status:** ✅ FULLY IMPLEMENTED  
**Target:** Asthra v1.3+

This guide provides comprehensive documentation for Asthra's object-oriented programming features using the `pub`, `impl`, and `self` keywords.

## Table of Contents

1. [Overview](#overview)
2. [Visibility Control with `pub`](#visibility-control-with-pub)
3. [Implementation Blocks with `impl`](#implementation-blocks-with-impl)
4. [Instance Methods with `self`](#instance-methods-with-self)
5. [Method Call Syntax](#method-call-syntax)
6. [Advanced Patterns](#advanced-patterns)
7. [Best Practices](#best-practices)
8. [Error Handling](#error-handling)

## Overview

Asthra supports modern object-oriented programming through three key keywords:

- **`pub`**: Controls visibility (public/private) for structs, functions, methods, and fields
- **`impl`**: Defines implementation blocks containing methods for structs
- **`self`**: Special parameter for instance methods that represents the struct instance

### Basic Example

```asthra
// Public struct with mixed field visibility
pub struct Point {
    pub x: f64;      // Public field
    pub y: f64;      // Public field
    private_id: i32; // Private field (default visibility)
}

// Implementation block containing methods
impl Point {
    // Associated function (constructor)
    pub fn new(x: f64, y: f64) -> Point {
        return Point { x: x, y: y, private_id: 42 };
    }
    
    // Instance method with self parameter
    pub fn distance_from_origin(self) -> f64 {
        return sqrt(self.x * self.x + self.y * self.y);
    }
}

fn main() {
    // Create instance using associated function
    let point = Point::new(3.0, 4.0);
    
    // Call instance method
    let distance = point.distance_from_origin();
    
    // Access public fields
    let x_value = point.x;  // ✅ Allowed
    // let id = point.private_id;  // ❌ Error: private field
}
```

## Visibility Control with `pub`

### Default Visibility

By default, all items in Asthra are **private**:

```asthra
struct PrivateStruct {    // Private struct
    field: i32;           // Private field
}

fn private_function() {   // Private function
    // ...
}
```

### Public Visibility

Use the `pub` keyword to make items public:

```asthra
pub struct PublicStruct {     // Public struct
    pub public_field: i32;    // Public field
    private_field: i32;       // Private field (default)
}

pub fn public_function() {    // Public function
    // ...
}
```

### Field-Level Visibility

Struct fields can have individual visibility modifiers:

```asthra
pub struct MixedVisibility {
    pub id: i32;              // Public: external access allowed
    pub name: string;         // Public: external access allowed
    internal_state: bool;     // Private: internal use only
    cached_value: f64;        // Private: implementation detail
}

impl MixedVisibility {
    pub fn new(id: i32, name: string) -> MixedVisibility {
        return MixedVisibility {
            id: id,
            name: name,
            internal_state: true,
            cached_value: 0.0
        };
    }
    
    // Public getter for private field
    pub fn is_active(self) -> bool {
        return self.internal_state;
    }
    
    // Private method for internal operations
    fn update_cache(self) -> MixedVisibility {
        return MixedVisibility {
            id: self.id,
            name: self.name,
            internal_state: self.internal_state,
            cached_value: self.calculate_value()
        };
    }
}
```

## Implementation Blocks with `impl`

### Basic Implementation Blocks

Use `impl` to define methods for a struct:

```asthra
struct Rectangle {
    width: f64;
    height: f64;
}

impl Rectangle {
    // Methods go here
}
```

### Associated Functions (Constructors)

Functions without a `self` parameter are **associated functions**:

```asthra
impl Rectangle {
    // Constructor pattern
    pub fn new(width: f64, height: f64) -> Rectangle {
        return Rectangle { width: width, height: height };
    }
    
    // Alternative constructors
    pub fn square(size: f64) -> Rectangle {
        return Rectangle::new(size, size);
    }
    
    pub fn zero() -> Rectangle {
        return Rectangle { width: 0.0, height: 0.0 };
    }
}
```

### Instance Methods

Functions with a `self` parameter are **instance methods**:

```asthra
impl Rectangle {
    // Calculate area
    pub fn area(self) -> f64 {
        return self.width * self.height;
    }
    
    // Calculate perimeter
    pub fn perimeter(self) -> f64 {
        return 2.0 * (self.width + self.height);
    }
    
    // Check if square
    pub fn is_square(self) -> bool {
        return self.width == self.height;
    }
}
```

### Method Visibility

Methods can be public or private:

```asthra
impl Rectangle {
    // Public methods - external API
    pub fn area(self) -> f64 {
        return self.width * self.height;
    }
    
    pub fn scale(self, factor: f64) -> Rectangle {
        return Rectangle {
            width: self.width * factor,
            height: self.height * factor
        };
    }
    
    // Private methods - internal implementation
    fn validate_dimensions(self) -> bool {
        return self.width > 0.0 && self.height > 0.0;
    }
    
    fn normalize(self) -> Rectangle {
        if self.width < 0.0 || self.height < 0.0 {
            return Rectangle::zero();
        }
        return self;
    }
}
```

## Instance Methods with `self`

### The `self` Parameter

The `self` parameter must be the first parameter in instance methods:

```asthra
impl Point {
    // ✅ Correct: self is first parameter
    pub fn distance_to(self, other: Point) -> f64 {
        let dx = self.x - other.x;
        let dy = self.y - other.y;
        return sqrt(dx * dx + dy * dy);
    }
    
    // ❌ Error: self must be first parameter
    // fn invalid_method(x: f64, self) -> f64 { ... }
    
    // ❌ Error: multiple self parameters not allowed
    // fn invalid_method2(self, other_self: Point) -> f64 { ... }
}
```

### Accessing Fields Through `self`

Use `self` to access the instance's fields:

```asthra
struct Circle {
    radius: f64;
    center: Point;
}

impl Circle {
    pub fn area(self) -> f64 {
        return 3.14159 * self.radius * self.radius;
    }
    
    pub fn circumference(self) -> f64 {
        return 2.0 * 3.14159 * self.radius;
    }
    
    pub fn contains(self, point: Point) -> bool {
        let distance = self.center.distance_to(point);
        return distance <= self.radius;
    }
}
```

### Returning Self Type

Methods can return the same struct type:

```asthra
impl Point {
    // Return new instance with transformed coordinates
    pub fn translate(self, dx: f64, dy: f64) -> Point {
        return Point {
            x: self.x + dx,
            y: self.y + dy,
            private_id: self.private_id
        };
    }
    
    pub fn scale(self, factor: f64) -> Point {
        return Point {
            x: self.x * factor,
            y: self.y * factor,
            private_id: self.private_id
        };
    }
}
```

## Method Call Syntax

### Associated Function Calls

Use `::` syntax to call associated functions:

```asthra
// Constructor calls
let point = Point::new(1.0, 2.0);
let rect = Rectangle::square(5.0);
let origin = Point::origin();

// Utility functions
let empty_rect = Rectangle::zero();
```

### Instance Method Calls

Use `.` syntax to call instance methods:

```asthra
let point = Point::new(3.0, 4.0);

// Simple method calls
let distance = point.distance_from_origin();
let is_origin = point.is_origin();

// Method calls with parameters
let other_point = Point::new(6.0, 8.0);
let distance_between = point.distance_to(other_point);

// Accessing public fields
let x_coordinate = point.x;
let y_coordinate = point.y;
```

### Method Chaining

Since methods can return new instances, you can chain method calls:

```asthra
let transformed_point = Point::new(1.0, 2.0)
    .translate(5.0, 3.0)    // Move the point
    .scale(2.0)             // Scale coordinates
    .translate(-1.0, -1.0); // Adjust position

let rect = Rectangle::new(10.0, 5.0)
    .scale(1.5)             // Make it bigger
    .normalize();           // Ensure positive dimensions
```

## Advanced Patterns

### Builder Pattern

Use associated functions and method chaining for builder patterns:

```asthra
pub struct Config {
    pub host: string;
    pub port: i32;
    pub timeout: i32;
    pub secure: bool;
}

impl Config {
    pub fn new() -> Config {
        return Config {
            host: "localhost",
            port: 8080,
            timeout: 30,
            secure: false
        };
    }
    
    pub fn with_host(self, host: string) -> Config {
        return Config {
            host: host,
            port: self.port,
            timeout: self.timeout,
            secure: self.secure
        };
    }
    
    pub fn with_port(self, port: i32) -> Config {
        return Config {
            host: self.host,
            port: port,
            timeout: self.timeout,
            secure: self.secure
        };
    }
    
    pub fn with_timeout(self, timeout: i32) -> Config {
        return Config {
            host: self.host,
            port: self.port,
            timeout: timeout,
            secure: self.secure
        };
    }
    
    pub fn secure(self) -> Config {
        return Config {
            host: self.host,
            port: self.port,
            timeout: self.timeout,
            secure: true
        };
    }
}

// Usage
let config = Config::new()
    .with_host("api.example.com")
    .with_port(443)
    .with_timeout(60)
    .secure();
```

### Encapsulation with Private Fields

Use private fields with public methods for controlled access:

```asthra
pub struct BankAccount {
    pub account_number: string;
    balance: f64;                    // Private: no direct access
    transaction_history: [string];  // Private: internal tracking
}

impl BankAccount {
    pub fn new(account_number: string, initial_balance: f64) -> BankAccount {
        return BankAccount {
            account_number: account_number,
            balance: initial_balance,
            transaction_history: []
        };
    }
    
    // Controlled access to balance
    pub fn get_balance(self) -> f64 {
        return self.balance;
    }
    
    // Controlled deposits
    pub fn deposit(self, amount: f64) -> BankAccount {
        if amount <= 0.0 {
            return self;  // Invalid amount, no change
        }
        
        return BankAccount {
            account_number: self.account_number,
            balance: self.balance + amount,
            transaction_history: self.add_transaction("deposit")
        };
    }
    
    // Controlled withdrawals
    pub fn withdraw(self, amount: f64) -> BankAccount {
        if amount <= 0.0 || amount > self.balance {
            return self;  // Invalid withdrawal
        }
        
        return BankAccount {
            account_number: self.account_number,
            balance: self.balance - amount,
            transaction_history: self.add_transaction("withdrawal")
        };
    }
    
    // Private helper method
    fn add_transaction(self, transaction_type: string) -> [string] {
        // Implementation to add transaction to history
        return self.transaction_history;
    }
}
```

### Multiple Constructors

Provide different ways to create instances:

```asthra
pub struct Color {
    pub r: f64;
    pub g: f64;
    pub b: f64;
    pub a: f64;
}

impl Color {
    // RGB constructor
    pub fn rgb(r: f64, g: f64, b: f64) -> Color {
        return Color { r: r, g: g, b: b, a: 1.0 };
    }
    
    // RGBA constructor
    pub fn rgba(r: f64, g: f64, b: f64, a: f64) -> Color {
        return Color { r: r, g: g, b: b, a: a };
    }
    
    // Grayscale constructor
    pub fn gray(value: f64) -> Color {
        return Color { r: value, g: value, b: value, a: 1.0 };
    }
    
    // Predefined colors
    pub fn black() -> Color {
        return Color::rgb(0.0, 0.0, 0.0);
    }
    
    pub fn white() -> Color {
        return Color::rgb(1.0, 1.0, 1.0);
    }
    
    pub fn red() -> Color {
        return Color::rgb(1.0, 0.0, 0.0);
    }
}
```

## Best Practices

### 1. Use Clear Naming Conventions

```asthra
impl Rectangle {
    // ✅ Good: Clear, descriptive names
    pub fn calculate_area(self) -> f64 { ... }
    pub fn get_width(self) -> f64 { ... }
    pub fn set_dimensions(self, width: f64, height: f64) -> Rectangle { ... }
    
    // ❌ Avoid: Unclear, abbreviated names
    // fn calc(self) -> f64 { ... }
    // fn w(self) -> f64 { ... }
    // fn set(self, w: f64, h: f64) -> Rectangle { ... }
}
```

### 2. Group Related Functionality

```asthra
impl Point {
    // Constructors first
    pub fn new(x: f64, y: f64) -> Point { ... }
    pub fn origin() -> Point { ... }
    
    // Basic getters/properties
    pub fn x(self) -> f64 { ... }
    pub fn y(self) -> f64 { ... }
    
    // Mathematical operations
    pub fn distance_to(self, other: Point) -> f64 { ... }
    pub fn distance_from_origin(self) -> f64 { ... }
    
    // Transformations
    pub fn translate(self, dx: f64, dy: f64) -> Point { ... }
    pub fn scale(self, factor: f64) -> Point { ... }
    
    // Utility methods
    pub fn is_origin(self) -> bool { ... }
    pub fn is_equal(self, other: Point) -> bool { ... }
}
```

### 3. Use Appropriate Visibility

```asthra
pub struct Parser {
    pub input: string;           // Public: users need access
    position: i32;               // Private: internal state
    current_token: Token;        // Private: implementation detail
}

impl Parser {
    // Public API
    pub fn new(input: string) -> Parser { ... }
    pub fn parse(self) -> ASTNode { ... }
    pub fn get_position(self) -> i32 { ... }
    
    // Private helpers
    fn advance(self) -> Parser { ... }
    fn peek_token(self) -> Token { ... }
    fn match_keyword(self, keyword: string) -> bool { ... }
}
```

### 4. Provide Meaningful Error Handling

```asthra
pub struct Result<T, E> {
    // Implementation depends on Result type system
}

impl Rectangle {
    pub fn new(width: f64, height: f64) -> Result<Rectangle, string> {
        if width <= 0.0 || height <= 0.0 {
            return Err("Width and height must be positive");
        }
        
        return Ok(Rectangle { width: width, height: height });
    }
    
    pub fn divide(self, factor: f64) -> Result<Rectangle, string> {
        if factor == 0.0 {
            return Err("Cannot divide by zero");
        }
        
        return Ok(Rectangle {
            width: self.width / factor,
            height: self.height / factor
        });
    }
}
```

## Error Handling

### Common Compilation Errors

#### 1. Private Field Access

```asthra
pub struct Point {
    pub x: f64;
    private_y: f64;  // Private field
}

fn test() {
    let p = Point::new(1.0, 2.0);
    let x = p.x;          // ✅ OK: public field
    let y = p.private_y;  // ❌ Error: private field access
}
```

**Error Message:** `Field 'private_y' is private and cannot be accessed`

#### 2. Invalid Self Parameter

```asthra
impl Point {
    // ❌ Error: self not first parameter
    fn invalid_method(x: f64, self) -> f64 {
        return self.x + x;
    }
}
```

**Error Message:** `'self' parameter must be the first parameter`

#### 3. Associated Function with Self

```asthra
impl Point {
    // ❌ Error: associated function cannot have self
    fn new(self, x: f64, y: f64) -> Point {
        return Point { x: x, y: y };
    }
}
```

**Error Message:** `Associated functions cannot have 'self' parameter`

#### 4. Method Call Syntax Errors

```asthra
let point = Point::new(1.0, 2.0);

// ❌ Error: using :: on instance
let distance = point::distance_from_origin();

// ❌ Error: using . on type
let new_point = Point.new(3.0, 4.0);
```

**Error Messages:**
- `Cannot call associated function on instance, use '.'`
- `Cannot call instance method on type, use an instance`

### Debugging Tips

1. **Check visibility modifiers** - ensure public items are marked with `pub`
2. **Verify method call syntax** - use `::` for associated functions, `.` for instance methods
3. **Validate self parameter placement** - `self` must be first in instance methods
4. **Review implementation blocks** - ensure struct exists and is in scope

## Summary

The `pub`, `impl`, and `self` keywords provide a complete object-oriented programming system in Asthra:

- **`pub`** controls access to structs, fields, functions, and methods
- **`impl`** defines implementation blocks containing methods for structs
- **`self`** enables instance methods that operate on struct instances

These features enable:
- Encapsulation through visibility control
- Polymorphism through method definitions
- Clear separation between constructors (associated functions) and instance behavior
- Method chaining and builder patterns
- Type safety with compile-time visibility checking

For more examples and advanced patterns, see the [comprehensive test file](../../test_pub_impl_self_comprehensive.asthra) and the [existing struct methods example](../../examples/struct_methods.asthra). 
