# Asthra If Conditions Implementation Plan

**Last Updated**: January 2025  
**Status**: Implementation Plan for BDD If Conditions  
**Grammar Version**: Current (grammar.txt)  
**Target**: Complete all scenarios in `bdd/features/if_conditions.feature`

## Overview

This document outlines the specific features needed to complete all test scenarios in the if conditions BDD feature file. Currently, 4 out of 6 scenarios pass. This plan focuses on implementing the remaining features to achieve 100% test coverage.

## Current Status

### ✅ Passing Scenarios (4/6)
1. Simple if condition with true branch
2. Simple if condition with false branch  
3. If-else condition
4. Nested if conditions

### ❌ Failing Scenarios (2/6)
1. If condition with expression result (@wip)
2. If condition with complex boolean expression (@wip)

## Required Features

### 1. Boolean Logical Operators
**Grammar Reference**: Lines 114-115  
**Required For**: "If condition with complex boolean expression" scenario

#### 1.1 Logical AND (`&&`)
```asthra
// Currently fails:
if x < y && y < z {
    log("x < y < z is true");
}
```

#### 1.2 Logical OR (`||`)
```asthra
// Currently fails:
if x > 5 || y < 10 {
    log("At least one condition is true");
}
```

**Implementation Tasks**:
- Add `&&` and `||` tokens to lexer
- Update parser to handle LogicAnd and LogicOr productions
- Implement short-circuit evaluation in code generation
- Add operator precedence handling

### 2. If-Else as Expressions
**Grammar Analysis**: While not explicitly in expression grammar, needed for value context  
**Required For**: "If condition with expression result" scenario

```asthra
// Currently fails:
let result: i32 = if true { 42 } else { 0 };
```

**Implementation Tasks**:
- Extend expression grammar to include if-else expressions
- Ensure type checking for both branches (must return same type)
- Generate appropriate code for expression context vs statement context
- Handle unit type `()` for void branches

## Implementation Plan

### Phase 1: Logical Operators (1 week)

#### Week 1 Tasks:
1. **Day 1-2: Lexer Updates**
   - Add `&&` and `||` token types
   - Update token classification
   - Add tests for new tokens

2. **Day 3-4: Parser Updates**
   - Implement LogicOr and LogicAnd parsing
   - Ensure correct precedence (OR lower than AND)
   - Add parser tests

3. **Day 5-7: Code Generation**
   - Implement short-circuit evaluation
   - Generate correct assembly/IR for logical operations
   - Add code generation tests

### Phase 2: If Expressions (1 week)

#### Week 2 Tasks:
1. **Day 1-2: Grammar Extension**
   - Add if-else to expression grammar
   - Update parser to handle if in expression context
   - Ensure both branches are required for expressions

2. **Day 3-4: Type Checking**
   - Verify both branches return compatible types
   - Handle type inference for if expressions
   - Add semantic analysis tests

3. **Day 5-7: Code Generation**
   - Generate code that produces values
   - Handle assignment from if expressions
   - Complete BDD test validation

## Testing Strategy

### Unit Tests
```asthra
// Test logical AND
assert((true && true) == true);
assert((true && false) == false);
assert((false && true) == false);
assert((false && false) == false);

// Test logical OR
assert((true || true) == true);
assert((true || false) == true);
assert((false || true) == true);
assert((false || false) == false);

// Test short-circuit evaluation
let mut x: i32 = 0;
if false && (x = 1) == 1 {
    panic("Should not execute");
}
assert(x == 0);  // x unchanged due to short-circuit

// Test if expressions
let a: i32 = if true { 10 } else { 20 };
assert(a == 10);
```

### BDD Tests
All scenarios in `bdd/features/if_conditions.feature` should pass after implementation.

## Success Criteria

1. **All 6 BDD scenarios pass** (currently 4/6)
2. **No regression** in existing tests
3. **Performance**: Logical operations compile to efficient machine code
4. **Error messages**: Clear errors for type mismatches in if expressions

## Code Examples

### Before (Current State)
```asthra
// These currently fail:
let result: i32 = if true { 42 } else { 0 };  // Parser error
if x > 5 || y < 10 { ... }  // Parser error  
if x < y && y < z { ... }   // Parser error
```

### After (Target State)
```asthra
// All of these should work:
let result: i32 = if condition { 42 } else { 0 };
let message: string = if success { "OK" } else { "Error" };

if x > 5 || y < 10 {
    log("At least one condition met");
}

if x >= min && x <= max {
    log("Value in range");
}
```

## Notes

- Short-circuit evaluation is essential for `&&` and `||`
- If expressions must have `else` branch (no implicit unit type)
- Both branches must have compatible types
- Consider future extension to support `if let` expressions