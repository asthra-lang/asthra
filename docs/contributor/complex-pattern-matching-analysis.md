# Complex Pattern Matching Analysis

**Date**: January 2025  
**Feature**: Complex Pattern Matching  
**Grammar**: Lines 103-110  
**Status**: ⚠️ PARTIALLY IMPLEMENTED  
**Last Updated**: January 2025 - Added GitHub issue reference for nested enum patterns

## Executive Summary

Complex pattern matching in Asthra is partially implemented. Basic patterns work well, but there are significant gaps in nested patterns, generic struct patterns, and some semantic validation. The parser successfully handles simple cases but struggles with complex nested patterns and certain syntactic requirements.

## Grammar Definition

```peg
Pattern        <- EnumPattern / StructPattern / TuplePattern / SimpleIdent
TuplePattern   <- '(' Pattern ',' Pattern (',' Pattern)* ')'
EnumPattern    <- SimpleIdent '.' SimpleIdent ('(' PatternArgs ')')?
PatternArgs    <- Pattern (',' Pattern)* / 'none'
StructPattern  <- SimpleIdent TypeArgs? '{' FieldPattern (',' FieldPattern)* '}'
FieldPattern   <- SimpleIdent ':' FieldBinding
FieldBinding   <- SimpleIdent / '_'
```

## Implementation Status

### ✅ Fully Working

1. **Basic Patterns**
   - Simple identifier patterns: `x`, `value`
   - Wildcard patterns: `_`
   - Literal patterns: `42`, `"hello"`, `true`

2. **Basic Tuple Patterns**
   - Simple tuples: `(x, y)`
   - Tuples with wildcards: `(x, _, z)`
   - Unit pattern: `()`

3. **Basic Enum Patterns**
   - Variant without data: `Option.None`
   - Variant with single binding: `Result.Ok(value)`
   - Supports `Result` and `Option` keywords

4. **Basic Struct Patterns**
   - With explicit field binding: `Point { x: px, y: py }`
   - Field ignoring: `Point { x: px, y: _ }`
   - Partial patterns with `..`: `Config { host: h, port: p, .. }`

### ⚠️ Partially Working

1. **Nested Patterns**
   - Nested tuples: `(a, (b, c))` - Parser works, semantic may have issues
   - Mixed patterns: `(Point { x: x1, y: y1 }, Point { x: x2, y: y2 })`
   - Complex nesting depth limited

2. **Struct Patterns with Generics**
   - Pattern parsing: `Container<i32> { value: v, next: n }` - Parser supports
   - Type checking may not fully validate generic constraints

3. **Pattern Exhaustiveness**
   - Basic exhaustiveness checking exists
   - Complex cases may not be fully validated
   - Wildcard `_` properly recognized as catch-all

### ❌ Not Working

1. **Nested Enum Patterns**
   - `Option.Some(Result.Ok(val))` - Grammar doesn't support nested enum extraction
   - Would require pattern arguments to accept full patterns, not just identifiers
   - **GitHub Issue**: [#63](https://github.com/asthra-lang/asthra/issues/63) - Feature request created for this enhancement

2. **Shorthand Struct Patterns**
   - `Point { x, y }` - Not supported, requires explicit binding syntax
   - Parser enforces: `Point { x: x, y: y }`

3. **Guard Conditions**
   - `pattern if condition => ...` - Not implemented in match arms
   - Grammar doesn't include guard syntax

4. **Range Patterns**
   - `1..10 => ...` - Not defined in grammar
   - Would require grammar extension

## Key Implementation Details

### Parser (`grammar_patterns.c`)

1. **Entry Point**: `parse_pattern()` handles all pattern types
2. **Type Disambiguation**: Uses lookahead to determine pattern type
3. **Field Patterns**: Enforces explicit binding syntax (`:` required)
4. **Error Recovery**: Good error messages for common mistakes

### Semantic Analysis Issues

1. **Type Checking**: Enum patterns fail type checking in some contexts
2. **Exhaustiveness**: Basic implementation, complex cases not fully covered
3. **Variable Binding**: Pattern variables properly scoped but may have shadowing issues

## Test Results Summary

| Test Category | Status | Notes |
|--------------|--------|-------|
| Basic Patterns | ✅ Pass | All literal and identifier patterns work |
| Tuple Patterns | ✅ Pass | Including nested tuples |
| Enum Patterns | ⚠️ Partial | Basic extraction works, type checking issues |
| Struct Patterns | ⚠️ Partial | Explicit syntax only, no shorthand |
| Nested Patterns | ⚠️ Partial | Limited depth, some combinations fail |
| Guard Conditions | ❌ Fail | Not implemented |
| If-Let Patterns | ⚠️ Partial | Parsing issues with complex patterns |

## Common Error Messages

1. **"Expected ':' after field name in pattern"**
   - Struct patterns require explicit field binding
   - Use `Point { x: px }` not `Point { x }`

2. **"Cannot match enum pattern against non-enum type"**
   - Type system doesn't properly recognize enum types in all contexts
   - May be a semantic analyzer limitation

3. **"Unsupported statement type: 41"**
   - Match statements not fully integrated in all contexts
   - AST node type 41 is AST_MATCH_STMT

## Design Decisions

1. **Explicit Field Binding**: Requiring `:` in struct patterns improves clarity
2. **No Nested Enum Patterns**: Simplifies parser but limits expressiveness
3. **Type Arguments in Patterns**: Allows matching generic types like `Vec<i32>`

## Recommendations

### High Priority
1. **Fix enum pattern type checking** - Semantic analyzer needs proper enum type recognition
2. **Implement guard conditions** - Essential for conditional pattern matching
3. **Support nested enum patterns** - Common use case for Option/Result chaining
   - See [Issue #63](https://github.com/asthra-lang/asthra/issues/63) for detailed proposal

### Medium Priority
1. **Add shorthand struct patterns** - Quality of life improvement
2. **Improve exhaustiveness checking** - Better coverage of complex cases
3. **Fix if-let parsing issues** - Important for ergonomic code

### Low Priority
1. **Add range patterns** - Nice to have for numeric matching
2. **Support or-patterns** - `pattern1 | pattern2` syntax
3. **Add pattern aliases** - `pattern @ binding` syntax

## Code Examples

### Working Examples
```asthra
// Basic tuple pattern
match pair {
    (x, y) => { return x + y; }
    _ => { return 0; }
}

// Struct pattern with explicit binding
match point {
    Point { x: px, y: py } => { return px * py; }
    _ => { return 0; }
}

// Enum pattern with extraction
match result {
    Result.Ok(value) => { return value; }
    Result.Err(_) => { return -1; }
}
```

### Not Working Examples
```asthra
// Shorthand struct pattern - NOT SUPPORTED
match point {
    Point { x, y } => { return x + y; }  // Error: Expected ':'
}

// Nested enum pattern - NOT SUPPORTED
match opt_result {
    Option.Some(Result.Ok(val)) => { return val; }  // Can't nest
}

// Guard condition - NOT SUPPORTED
match x {
    n if n > 0 => { return n; }  // 'if' not recognized
}
```

## Conclusion

Asthra's pattern matching system provides a solid foundation with good support for basic patterns. However, it lacks some advanced features common in modern languages. The explicit syntax requirements (like mandatory `:` in struct patterns) align with Asthra's design principle of AI-friendly unambiguous syntax, but may feel verbose to developers.

The most critical issues are in the semantic analysis phase rather than parsing, suggesting that the grammar is well-designed but the implementation needs refinement.