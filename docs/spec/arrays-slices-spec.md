# Arrays and Slices Specification

This document specifies the syntax and semantics of arrays and slices in the Asthra programming language.

## Overview

Asthra provides two primary aggregate data types for sequences of elements:
- **Arrays**: Fixed-size sequences with size known at compile time
- **Slices**: Dynamically-sized views into contiguous sequences

## Fixed-Size Arrays

### Type Syntax

```
ArrayType := '[' ConstExpr ']' Type
```

Where:
- `ConstExpr` is a compile-time constant expression that evaluates to a positive integer
- `Type` is any valid Asthra type

Examples:
```asthra
[10]i32          // Array of 10 32-bit integers
[256]u8          // Array of 256 unsigned bytes
[CONST_SIZE]f32  // Array with size from constant
[5][3]bool       // Array of 5 arrays of 3 booleans (multi-dimensional)
```

### Array Literals

#### Repeated Element Syntax

```
ArrayLiteral := '[' Expr ';' ConstExpr ']'
```

Creates an array where all elements are initialized to the same value.

```asthra
[0; 10]      // Array of 10 zeros
[true; 5]    // Array of 5 true values
[1.0; 100]   // Array of 100 1.0 values
```

#### Element List Syntax

```
ArrayLiteral := '[' ExprList? ']'
```

Creates an array with explicitly listed elements.

```asthra
[1, 2, 3, 4, 5]         // Array of 5 integers
[]                      // Empty array (size inferred from context)
[1.0, 2.0, 3.0]        // Array of 3 floats
```

### Semantic Rules

1. **Size Constraints**:
   - Array size must be a compile-time constant expression
   - Size must evaluate to a positive integer
   - Size of 0 is implementation-defined (may be disallowed)

2. **Type Checking**:
   - For repeated element syntax: `[value; count]`
     - The array type is `[count]T` where `T` is the type of `value`
   - For element list syntax: `[e1, e2, ..., en]`
     - All elements must have compatible types
     - The common type is determined through type unification

3. **Memory Layout**:
   - Arrays are stored contiguously in memory
   - Multi-dimensional arrays use row-major order

## Slices

### Type Syntax

```
SliceType := '[]' Type
```

Examples:
```asthra
[]i32       // Slice of 32-bit integers
[]u8        // Slice of bytes
[][]f32     // Slice of slices of floats
```

### Slice Creation

#### Slicing Syntax

```
SliceExpr := Expr '[' SliceRange ']'
SliceRange := Expr? ':' Expr?
            | ':'
```

Where the base expression must be:
- An array
- Another slice
- A type that implements slicing

Semantics:
- `arr[start:end]`: Elements from `start` (inclusive) to `end` (exclusive)
- `arr[:end]`: Elements from beginning to `end` (exclusive)
- `arr[start:]`: Elements from `start` (inclusive) to end
- `arr[:]`: All elements

### Semantic Rules

1. **Index Validation**:
   - Indices must be of integer type
   - Start index must be ≤ end index
   - Indices must be within bounds (0 ≤ index ≤ length)

2. **Type Preservation**:
   - Slicing an `[N]T` produces `[]T`
   - Slicing a `[]T` produces `[]T`

3. **Memory Semantics**:
   - Slices are views into existing data (no copying)
   - Slices consist of a pointer and length
   - Modifying slice elements modifies the underlying data

## Indexing

### Syntax

```
IndexExpr := Expr '[' Expr ']'
```

### Semantic Rules

1. **Base Type**:
   - Must be an array, slice, or pointer type
   - For pointers, requires unsafe context

2. **Index Type**:
   - Must be an integer type
   - Must be within bounds (0 ≤ index < length)

3. **Result Type**:
   - Indexing `[N]T` yields `T`
   - Indexing `[]T` yields `T`
   - Result is an lvalue (can be assigned to)

## Type Compatibility

### Array Compatibility

Two array types `[N]T` and `[M]U` are compatible if:
- `N == M` (same size)
- `T` and `U` are compatible types

### Slice Compatibility

Two slice types `[]T` and `[]U` are compatible if:
- `T` and `U` are compatible types

### Array to Slice Conversion

An array `[N]T` can be implicitly converted to `[]T` through:
- Slice syntax: `arr[:]`
- Function parameter passing (if parameter expects slice)

## Compile-Time Evaluation

### Constant Expressions

The following must be compile-time constants:
- Array sizes in type declarations
- Count in repeated element syntax `[value; count]`

### Constant Folding

The compiler performs constant folding for:
- Arithmetic expressions: `[10 + 5]i32`
- Constant references: `[SIZE * 2]u8`
- Const function calls (when implemented)

## Code Generation

### Memory Layout

1. **Arrays**:
   ```
   [N]T occupies: N * sizeof(T) bytes
   Alignment: alignof(T)
   ```

2. **Slices**:
   ```
   []T occupies: 2 * sizeof(pointer) bytes
   Layout: { ptr: *T, len: usize }
   ```

### Operations

1. **Array Initialization**:
   - Repeated element: Generate loop or use memset optimization
   - Element list: Generate sequential stores

2. **Slice Creation**:
   - Calculate base pointer: `base + start * sizeof(T)`
   - Calculate length: `end - start`
   - Create slice descriptor

3. **Indexing**:
   - Arrays: `base + index * sizeof(T)`
   - Slices: `ptr + index * sizeof(T)` with bounds check

## Safety Considerations

1. **Bounds Checking**:
   - Debug mode: Runtime bounds checks on all operations
   - Release mode: Bounds checks may be elided if proven safe

2. **Lifetime Tracking**:
   - Slices must not outlive their underlying data
   - Compiler tracks lifetimes to prevent use-after-free

3. **Overflow Protection**:
   - Size calculations check for overflow
   - Large arrays may require heap allocation

## Grammar Summary

```
ArrayType    := '[' ConstExpr ']' Type
SliceType    := '[]' Type
ArrayLiteral := '[' Expr ';' ConstExpr ']'
              | '[' ExprList? ']'
SliceExpr    := Expr '[' SliceRange ']'
SliceRange   := Expr? ':' Expr? | ':'
IndexExpr    := Expr '[' Expr ']'
```

## Implementation Status

| Feature | Status | Notes |
|---------|---------|-------|
| Fixed-size array types `[N]T` | ✅ Implemented | Full type checking |
| Repeated element syntax `[v; n]` | ✅ Implemented | With const validation |
| Slice types `[]T` | ✅ Implemented | Fat pointer representation |
| Slice operations `[s:e]` | ✅ Implemented | All variants supported |
| Array indexing | ✅ Implemented | With bounds checking |
| Multi-dimensional arrays | ✅ Implemented | Row-major layout |
| Array to slice conversion | ✅ Implemented | Via slice syntax |
| Runtime bounds checking | ❌ Not implemented | Planned for Phase 5 |
| Slice iterators | ❌ Not implemented | Future enhancement |