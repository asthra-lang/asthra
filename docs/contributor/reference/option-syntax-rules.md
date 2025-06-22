# Option Type Syntax Rules

## Overview

This document clarifies the syntax rules for creating and using `Option<T>` values in Asthra. The language enforces explicit enum variant syntax to maintain consistency and prevent ambiguity.

## Creating Option Values

### Correct Syntax

Option values must be created using explicit enum variant syntax:

```asthra
// Creating Some variant
let value: Option<i32> = Option.Some(42);
let name: Option<string> = Option.Some("Alice");

// Creating None variant
let empty: Option<i32> = Option.None;
let missing: Option<string> = Option.None;
```

### Incorrect Syntax

The following syntaxes are **NOT** supported and will result in compilation errors:

```asthra
// ❌ INCORRECT - Standalone Some() is not a built-in function
let value: Option<i32> = Some(42);  // Error: undefined function 'Some'

// ❌ INCORRECT - Standalone None() is not a built-in function
let empty: Option<i32> = None();    // Error: undefined function 'None'
```

## Pattern Matching

In pattern matching contexts, both explicit and implicit syntax are allowed for convenience:

```asthra
// Both syntaxes work in match patterns
match my_option {
    Option.Some(value) => process(value),  // Explicit syntax
    Option.None => default_value()          // Explicit syntax
}

// Implicit syntax also allowed in patterns
match my_option {
    Some(value) => process(value),  // Implicit syntax (allowed)
    None => default_value()          // Implicit syntax (allowed)
}
```

## Struct Field Initialization

When initializing struct fields of Option type, use explicit syntax:

```asthra
struct Config {
    timeout: Option<i32>,
    name: Option<string>
}

// Correct initialization
let config = Config {
    timeout: Option.Some(30),        // ✅ Explicit syntax
    name: Option.None                 // ✅ Explicit syntax
};

// Incorrect initialization
let config = Config {
    timeout: Some(30),               // ❌ Error: undefined function 'Some'
    name: None                       // ❌ Error: 'None' is not a value
};
```

## Function Returns

When returning Option values from functions, use explicit syntax:

```asthra
fn find_user(id: i32) -> Option<User> {
    if id > 0 {
        return Option.Some(User { id: id, name: "User" });  // ✅ Correct
    } else {
        return Option.None;                                  // ✅ Correct
    }
}

// NOT:
fn find_user_wrong(id: i32) -> Option<User> {
    if id > 0 {
        return Some(User { id: id, name: "User" });  // ❌ Error
    } else {
        return None;                                  // ❌ Error
    }
}
```

## Why This Design?

1. **Consistency**: All enum variants use the same `EnumName.VariantName` syntax
2. **Clarity**: Explicit syntax makes it clear that `Some` and `None` are variants of `Option`
3. **No Reserved Names**: `Some` and `None` are not reserved keywords, allowing users to define their own functions with these names if needed
4. **AI Generation**: Explicit syntax is more predictable for AI code generation tools

## Summary

- **Always use** `Option.Some(value)` and `Option.None` when creating Option values
- **Never use** standalone `Some()` or `None()` as they are not built-in functions
- **Pattern matching** allows both explicit and implicit syntax for convenience
- **This applies** to all contexts: variable initialization, struct fields, function returns, etc.