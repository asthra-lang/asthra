# Const Expression Operators Implementation Plan

**Created Date**: January 2025  
**Status**: Planning Phase  
**Grammar Reference**: Lines 33-35 of grammar.txt  
**Estimated Duration**: 1-2 weeks

## Overview

This document outlines the implementation plan for const expression operators in Asthra. Currently, const declarations only support literal values and simple identifiers. The grammar defines support for binary and unary operations on constants, which would enable compile-time evaluation of expressions.

## Current State

### What Works
```asthra
const PI: f64 = 3.14159;           // ✅ Literal values
const MAX_SIZE: i32 = 100;         // ✅ Integer literals
const DEBUG: bool = true;          // ✅ Boolean literals
const NAME: string = "Asthra";     // ✅ String literals
const ALIAS: i32 = MAX_SIZE;       // ✅ Simple identifier references
```

### What Doesn't Work (But Should)
```asthra
const DOUBLE_SIZE: i32 = MAX_SIZE * 2;      // ❌ Binary operations
const NEGATIVE: i32 = -5;                    // ❌ Unary operations
const MASK: u32 = 0xFF << 8;                // ❌ Bitwise operations
const ARRAY_SIZE: i32 = 10 + 20 + 30;       // ❌ Multiple operations
const BUFFER_SIZE: usize = sizeof(MyStruct); // ❌ sizeof in const context
```

## Grammar Definition

From grammar.txt:
```
ConstExpr      <- Literal / SimpleIdent / BinaryConstExpr / UnaryConstExpr / SizeOf
BinaryConstExpr<- ConstExpr BinaryOp ConstExpr
UnaryConstExpr <- UnaryOp ConstExpr
```

This allows recursive const expressions with full operator support.

## Implementation Plan

### Phase 1: Parser Enhancement (Days 1-2)

#### 1.1 Update Const Expression Parser
- **File**: `src/parser/grammar_statements_types.c` (or similar)
- **Current**: Only parses literals and identifiers
- **Change**: Add recursive parsing for BinaryConstExpr and UnaryConstExpr

```c
ASTNode *parse_const_expr(Parser *parser) {
    // Current implementation likely only handles:
    // - Literals
    // - Simple identifiers
    
    // Need to add:
    // - Binary expression parsing with const context flag
    // - Unary expression parsing with const context flag
    // - Ensure only const-safe operations are allowed
}
```

#### 1.2 AST Node Updates
- Ensure AST nodes can be marked as "const context"
- May need to add a flag to track const expressions

### Phase 2: Const Expression Evaluator (Days 3-5)

#### 2.1 Create Const Evaluation Engine
- **New File**: `src/analysis/const_evaluator.c`
- **Purpose**: Evaluate expressions at compile time

```c
typedef struct ConstValue {
    enum {
        CONST_INT,
        CONST_FLOAT,
        CONST_BOOL,
        CONST_STRING,
        CONST_SIZE
    } type;
    union {
        int64_t int_val;
        double float_val;
        bool bool_val;
        char *string_val;
        size_t size_val;
    } value;
} ConstValue;

ConstValue *evaluate_const_expr(SemanticAnalyzer *analyzer, ASTNode *expr);
```

#### 2.2 Supported Operations
Binary operators to implement:
- Arithmetic: `+`, `-`, `*`, `/`, `%`
- Bitwise: `&`, `|`, `^`, `<<`, `>>`
- Comparison: `==`, `!=`, `<`, `<=`, `>`, `>=` (result in bool)
- Logical: `&&`, `||` (with short-circuit evaluation)

Unary operators to implement:
- Arithmetic: `-` (negation)
- Logical: `!` (not)
- Bitwise: `~` (complement)

### Phase 3: Semantic Analysis Integration (Days 6-7)

#### 3.1 Update Semantic Analyzer
- **File**: `src/analysis/semantic_analyzer.c`
- Integrate const evaluator into const declaration processing
- Validate that all referenced identifiers are also const
- Type check const expressions
- Store evaluated const values in symbol table

#### 3.2 Error Handling
Implement clear error messages for:
- Non-const identifiers in const expressions
- Division by zero at compile time
- Type mismatches in const operations
- Overflow/underflow in const arithmetic
- Invalid operations (e.g., string arithmetic)

### Phase 4: Code Generation Updates (Day 8)

#### 4.1 Const Value Emission
- Update code generator to emit pre-calculated const values
- No runtime evaluation needed for const expressions
- May optimize code that uses const values

### Phase 5: Testing (Days 9-10)

#### 5.1 Unit Tests
Create comprehensive tests for:
- Each operator in const context
- Nested const expressions
- Type checking in const expressions
- Error cases

#### 5.2 Integration Tests
```asthra
// test_const_expressions.asthra
package test;

const BASE: i32 = 100;
const DOUBLE: i32 = BASE * 2;
const TRIPLE: i32 = BASE * 3;
const NEGATIVE: i32 = -BASE;
const MASK: u32 = 0xFF << 8;
const COMBINED: i32 = (BASE + 50) * 2 - 10;
const COMPARISON: bool = BASE > 50;
const LOGICAL: bool = true && (BASE > 0);

pub fn main(none) -> void {
    // Verify const values work correctly
    assert(DOUBLE == 200);
    assert(TRIPLE == 300);
    assert(NEGATIVE == -100);
    assert(MASK == 0xFF00);
    assert(COMBINED == 290);
    assert(COMPARISON == true);
    assert(LOGICAL == true);
    
    log("All const expression tests passed!");
    return ();
}
```

## Technical Considerations

### 1. Order of Evaluation
- Must handle dependency ordering (const A depends on const B)
- Detect circular dependencies
- Build dependency graph if needed

### 2. Type Coercion
- Define rules for type promotion in const expressions
- E.g., `i32 + i64` → `i64`
- Maintain type safety

### 3. Overflow Handling
- Decide on overflow behavior (wrap, error, or saturate)
- Consistent with runtime behavior
- Clear error messages

### 4. sizeof Integration
- Grammar includes `SizeOf` in const expressions
- Requires type information at compile time
- May need to defer some evaluations

## Success Criteria

1. **All arithmetic operations** work in const expressions
2. **All bitwise operations** work in const expressions  
3. **Comparison operations** produce const bool values
4. **Logical operations** work with proper short-circuiting
5. **Nested expressions** evaluate correctly
6. **Clear error messages** for invalid const expressions
7. **No performance regression** in compilation
8. **Comprehensive test coverage**

## Risk Mitigation

### Potential Challenges
1. **Circular dependencies** between consts
   - Solution: Dependency graph with cycle detection
   
2. **Complex type inference** in expressions
   - Solution: Explicit type checking before evaluation
   
3. **Parser ambiguity** with regular expressions
   - Solution: Use context flag to distinguish const vs runtime

## Implementation Order

1. **Start simple**: Implement binary arithmetic only
2. **Add unary**: Extend to unary operators
3. **Add bitwise**: Include bitwise operations
4. **Add comparison**: Enable const booleans
5. **Add logical**: Complete with && and ||
6. **Handle edge cases**: sizeof, complex nesting

## Alternative Approaches Considered

1. **Lazy evaluation**: Evaluate consts only when used
   - Rejected: Complicates error reporting
   
2. **AST transformation**: Convert to simpler form first
   - Rejected: Adds complexity without clear benefit
   
3. **External evaluator**: Use existing expression evaluator
   - Rejected: Need tight integration with type system

## Conclusion

Implementing const expression operators will significantly enhance Asthra's compile-time capabilities, allowing more expressive and maintainable code. The implementation is straightforward but requires careful attention to type safety and error handling.

The feature aligns with Asthra's design principles by:
- Providing predictable compile-time behavior
- Enabling better optimization opportunities
- Maintaining clear, unambiguous syntax
- Supporting common systems programming patterns