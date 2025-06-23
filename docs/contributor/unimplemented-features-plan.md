# Asthra If Conditions Implementation Plan

**Last Updated**: January 2025  
**Status**: ✅ COMPLETED - All features implemented  
**Grammar Version**: Current (grammar.txt)  
**Target**: Complete all scenarios in `bdd/features/if_conditions.feature`

## Overview

This document outlines the specific features needed to complete all test scenarios in the if conditions BDD feature file. All features have been successfully implemented.

## Current Status

### ✅ Passing Scenarios (5/5)
1. Simple if condition with true branch
2. Simple if condition with false branch  
3. If-else condition
4. Nested if conditions
5. If condition with complex boolean expression (COMPLETED)

## Implemented Features

### ✅ Boolean Logical Operators (COMPLETED)
**Grammar Reference**: Lines 114-115  
**Implementation Date**: January 2025

The logical operators have been successfully implemented:

#### Logical AND (`&&`)
```asthra
// Grammar: LogicAnd <- BitwiseOr ('&&' BitwiseOr)*
// Now works correctly:
if x < y && y < z {
    log("x < y < z is true");
}
```

#### Logical OR (`||`)
```asthra
// Grammar: LogicOr <- LogicAnd ('||' LogicAnd)*
// Now works correctly:
if x > 5 || y < 10 {
    log("At least one condition is true");
}
```

**Implementation Details**:
1. ✅ Verified `&&` and `||` tokens exist in lexer (TOKEN_LOGICAL_AND, TOKEN_LOGICAL_OR)
2. ✅ Fixed parser issue where generic type parsing interfered with comparison operators
3. ✅ Implemented short-circuit evaluation in code generation
4. ✅ Ensured correct operator precedence (OR has lower precedence than AND)

## Implementation Summary

The implementation was completed successfully with the following key fixes:

### Key Fix: Parser Generic Type Interference
The main issue was in `grammar_expressions_primary.c` where the parser was attempting to parse generic types (e.g., `Type<T>`) when encountering `<` in expression contexts. This caused expressions like `x < 20` to fail with "Expected type" errors.

**Solution**: Removed generic type parsing from primary expression parsing, as generic types should only be parsed in type contexts or struct literal contexts.

### Code Generation Implementation
Added full support for logical operators with short-circuit evaluation:
- **AND (`&&`)**: Evaluates right operand only if left is true
- **OR (`||`)**: Evaluates right operand only if left is false
- Proper label management for control flow
- Correct result register handling

## Testing Strategy

### Unit Tests
```asthra
// Test logical AND truth table
assert((true && true) == true);
assert((true && false) == false);
assert((false && true) == false);
assert((false && false) == false);

// Test logical OR truth table
assert((true || true) == true);
assert((true || false) == true);
assert((false || true) == true);
assert((false || false) == false);

// Test short-circuit evaluation for AND
let mut count: i32 = 0;
pub fn increment(none) -> bool {
    count = count + 1;
    return true;
}

// Should not call increment() due to short-circuit
if false && increment() {
    panic("Should not reach here");
}
assert(count == 0);

// Test short-circuit evaluation for OR
count = 0;
if true || increment() {
    // Should reach here without calling increment()
}
assert(count == 0);

// Test operator precedence
assert((true || false && false) == true);  // OR has lower precedence
assert((false && true || true) == true);   // AND evaluated first
```

### Integration Tests
- All existing if condition tests should continue to pass
- The complex boolean expression scenario should pass

### BDD Tests
After implementation, all 5 scenarios in `bdd/features/if_conditions.feature` should pass.

## Code Examples

### Before (Current State)
```asthra
// These currently fail with parser errors:
if x > 5 || y < 10 { ... }  // "Expected type" error
if x < y && y < z { ... }    // Parser doesn't recognize operators
```

### After (Target State)
```asthra
// All of these should work:
if x > 5 || y < 10 {
    log("At least one condition met");
}

if x >= min && x <= max {
    log("Value in range");
}

// Short-circuit evaluation
if dangerous_condition() && expensive_check() {
    // expensive_check() only called if dangerous_condition() is true
}

// Complex expressions with proper precedence
if (a || b) && (c || d) {
    log("Complex condition met");
}
```

## Success Criteria

1. **All 5 BDD scenarios pass** (currently 4/5)
2. **No regression** in existing tests
3. **Short-circuit evaluation** works correctly
4. **Correct operator precedence** (AND before OR)
5. **Clear error messages** for type mismatches

## Notes

- The operators are already in the grammar, just need implementation
- Short-circuit evaluation is essential for correctness and performance
- Must maintain compatibility with existing expression parsing
- Consider adding parentheses support for explicit precedence control