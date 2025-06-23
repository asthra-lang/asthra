# Asthra Grammar vs Implementation Gap Analysis

**Generated Date**: January 2025  
**Grammar Version**: Current (grammar.txt)  
**Purpose**: Document features defined in grammar but not fully implemented

## Executive Summary

This report identifies features that are defined in Asthra's PEG grammar but are either not implemented, partially implemented, or have implementation issues. This analysis was conducted while implementing logical operators (`&&` and `||`) and discovering various implementation gaps.

## Categories of Gaps

### 1. Grammar-Defined But Not Fully Implemented

These features exist in grammar.txt but have implementation limitations:

#### Tuple Element Access
- **Grammar**: Lines 130-131: `TupleIndex <- [0-9]+` and `FieldOrIndex <- SimpleIdent / TupleIndex`
- **Issue**: Edge cases like `.0.1` can be misparsed as float `0.1`
- **Workaround**: Parser has special handling for float-like tuple access
- **Impact**: Nested tuple access may require parentheses

#### Const Expression Evaluation ✅ COMPLETED
- **Grammar**: Lines 33-35: `ConstExpr <- Literal / SimpleIdent / BinaryConstExpr / UnaryConstExpr / SizeOf`
- **Status**: ✅ FULLY IMPLEMENTED (discovered during investigation)
- **Parser**: Complete support in `grammar_toplevel_const.c`
- **Evaluator**: All operators implemented in `const_evaluator.c`
- **Test Coverage**: test_const_expressions.c verifies all operators work
- **Limitation**: Const values not emitted in generated C code (compile-time only)
- **Example**: `priv const SIZE: i32 = 10 * 4;` works perfectly at compile time

#### Ownership Tags on Variables ✅ COMPLETED
- **Grammar**: Line 89: `VarDecl <- 'let' MutModifier? SimpleIdent ':' Type OwnershipTag? '=' Expr ';'`
- **Grammar**: Line 26: `OwnershipTag <- '@ownership(' ('gc'|'c'|'pinned') ')'`
- **Status**: FULLY IMPLEMENTED
- **Implementation PR**: feat/ownership-tags-on-variables
- **Test Coverage**: Comprehensive test suite in test_ownership_tags.c
- **Documentation**: See docs/contributor/ownership-tags-implementation.md

### 2. Implemented but Under-tested Features

These appear to work but may have limited test coverage:

#### Full Slice Syntax ✅ TESTED
- **Grammar**: Lines 134-137: `SliceBounds` with patterns: `start:end`, `start:`, `:end`, `:`
- **Status**: ✅ FULLY TESTED (all patterns work correctly)
- **Parser**: Complete implementation in `grammar_expressions_unary.c`
- **Test Coverage**: Comprehensive test suite in `test_slice_syntax_comprehensive.c`
- **Documentation**: See `docs/contributor/slice-syntax-testing-report.md`
- **Test Branch**: test/full-slice-syntax-coverage

#### Raw Multi-line Strings ✅ TESTED
- **Grammar**: Line 230: `RawString <- 'r"""' RawContent* '"""'`
- **Status**: ✅ FULLY IMPLEMENTED AND TESTED
- **Lexer**: Complete implementation in `lexer_scan_strings.c`
- **Feature**: No escape processing - preserves all characters literally
- **Test Coverage**: Comprehensive test suite in `test_raw_multiline_strings.c`
- **Documentation**: See `docs/contributor/raw-multiline-strings-testing.md`
- **Test Branch**: test/raw-multiline-strings

#### SizeOf Expression ✅ VERIFIED
- **Grammar**: Line 142: `SizeOf <- 'sizeof' '(' Type ')'`
- **Status**: ✅ FULLY IMPLEMENTED AND TESTED
- **Parser**: Complete implementation in `grammar_expressions_special.c`
- **Semantic**: Returns `usize` type (see `semantic_binary_unary.c` line 284)
- **Test Coverage**: Comprehensive tests in `test_sizeof_operator.c` and `test_sizeof_operator_semantic.c`
- **Documentation**: See `docs/contributor/sizeof-operator-verification.md`
- **Test Branch**: test/sizeof-operator-verification

### 3. Design Issues in Grammar

These are grammar features that may need reconsideration:

#### spawn_with_handle as Unary Operator ✅ FIXED
- **Grammar**: Line 124: Previously had `spawn_with_handle` as unary operator
- **Grammar**: Line 97: Also appeared as statement
- **Issue**: Treating spawn_with_handle as both unary operator and statement was confusing
- **Resolution**: Removed from unary operators, fixed statement syntax to match implementation
- **Fix PR**: fix/spawn-with-handle-grammar-issue

### 4. Additional Grammar Features Needing Implementation Verification

#### Pattern Matching Completeness
- **Grammar**: Lines 104-110: Full pattern system including `TuplePattern`, `EnumPattern`, `StructPattern`
- **Status**: Basic patterns work, but complex nested patterns may have gaps
- **Example**: `let (a, (b, c)) = nested_tuple;` - nested tuple patterns

#### Complex Pattern Matching
- **Grammar**: Lines 104-110: Nested patterns, enum patterns with extraction
- **Status**: Needs comprehensive testing of edge cases
- **Examples**: Nested tuple patterns, struct patterns with type parameters

#### Field Visibility in Structs ⚠️ PARTIALLY IMPLEMENTED
- **Grammar**: Line 46: `StructField <- VisibilityModifier? SimpleIdent ':' Type`
- **Status**: ⚠️ PARSED BUT NOT ENFORCED
- **Parser**: Fully implemented - correctly extracts `pub`/`priv` modifiers
- **Semantic**: NOT enforced - has TODO comment in `semantic_structs_visibility.c`
- **Issue**: All fields are effectively public regardless of modifier
- **Test Coverage**: Parser tests pass, semantic tests document lack of enforcement
- **Documentation**: See `docs/contributor/struct-field-visibility-analysis.md`
- **Test Branch**: test/struct-field-visibility

#### Repeated Array Elements ✅ TESTED
- **Grammar**: Line 155: `RepeatedElements <- ConstExpr ';' ConstExpr` (e.g., `[0; 100]`)
- **Status**: ✅ FULLY IMPLEMENTED AND TESTED
- **Parser**: Complete implementation in `grammar_arrays.c` with special marker
- **Semantic**: Full type checking and const validation
- **Codegen**: Generates efficient initialization code
- **Test Coverage**: Comprehensive tests in `test_repeated_array_elements.c` and `test_repeated_array_semantic.c`
- **Documentation**: See `docs/contributor/repeated-array-elements-testing.md`
- **Test Branch**: test/repeated-array-elements

#### Associated Function Calls with Generic Types ❌ NOT INTEGRATED
- **Grammar**: Lines 139-140: `AssociatedFuncCall <- (SimpleIdent / GenericType) '::' SimpleIdent '(' ArgList ')'`
- **Example**: `Vec<i32>::new()` - generic type before `::`
- **Status**: ❌ IMPLEMENTATION EXISTS BUT NOT INTEGRATED
- **Parser**: Implementation in `grammar_generics.c` but function never called
- **Issue**: `parse_identifier_with_generics()` exists but not connected to main parser
- **Impact**: Generic type associated functions completely non-functional
- **Test Coverage**: All tests fail - created `test_associated_func_generic_types.c`
- **Documentation**: See `docs/contributor/associated-func-generic-types-analysis.md`
- **Test Branch**: test/associated-function-generic-types

### 5. Edge Case Issues Discovered

During implementation, these specific issues were found:

#### Generic Type Parsing in Expressions
- **Issue**: Parser tried to parse `x < 20` as generic type `x<20>`
- **Root Cause**: Primary expression parser was checking for generics in all contexts
- **Fix Applied**: Removed generic parsing from expression contexts
- **Lesson**: Context-sensitive parsing needed - generics should only be parsed in type contexts

#### Logical NOT Operator ✅ VERIFIED
- **Grammar**: Line 126: `LogicalPrefix <- ('!' / '-' / '~')`
- **Status**: ✅ FULLY IMPLEMENTED AND TESTED
- **Lexer**: Complete implementation with `TOKEN_LOGICAL_NOT`
- **Parser**: Full support in `grammar_expressions_unary.c`
- **Semantic**: Type checking enforces boolean operands
- **Test Coverage**: Comprehensive tests in `test_unary_operator_restrictions.c` and `test_expression_validation_unary.c`
- **Documentation**: See `docs/contributor/logical-not-operator-verification.md`
- **Test Branch**: test/logical-not-operator

#### None Marker Usage
- **Grammar**: Multiple uses of 'none' marker (lines 44, 50, 65, 67, 107, 146, 154, 194)
- **Purpose**: Explicit marker for empty content instead of allowing implicit empty
- **Example**: `struct Empty { none }`, `fn foo(none) -> void`
- **Status**: Unique design choice that improves AI generation clarity

## Recommendations

### High Priority
1. **Enforce** struct field visibility in semantic analyzer (currently parsed but not enforced)
2. **Fix integration** of associated function calls with generic types
3. **Test** complex pattern matching scenarios (nested patterns)

### Medium Priority
1. **Document** the 'none' marker usage clearly for users
2. **Improve** error messages for generic type parsing in wrong contexts
3. **Add tests** for edge cases in tuple element access

### Low Priority
1. **Consider** if double logical NOT (`!!x`) should be allowed with a warning
2. **Document** all verified features in user manual

## Summary of Grammar-Defined Features with Implementation Gaps

### Definitely Missing/Incomplete
1. **Complex tuple access patterns** (nested access like .0.1) - Lines 130-131

### Needs Verification
1. **Raw multi-line strings** (`r"""..."""`) - Line 230
2. **Repeated array elements** (`[0; 100]`) - Line 155
3. **Struct field visibility** - Line 46
4. **Complex pattern matching** (nested patterns) - Lines 104-110

### Design Issues in Grammar
1. **'none' marker** - unique design requiring explicit empty markers - Multiple lines

## Key Implementation Insights

1. **Context-Sensitive Parsing**: The generic type parsing issue showed that some grammar constructs need context-aware parsing. `Type<T>` should only be parsed in type contexts, not in expressions.

2. **Grammar vs Semantics**: The grammar allows many constructs that may be restricted at the semantic level (e.g., ownership tags on variables exist in grammar but parser TODO indicates they're not implemented).

3. **AI-Friendly Design**: The 'none' marker and explicit visibility modifiers show Asthra's commitment to eliminating ambiguity for AI code generation.

## Conclusion

Most of Asthra's grammar is well-implemented. The gaps are primarily in:
- Advanced const expressions
- Edge cases in tuple/slice syntax  
- Features that may have been deferred (ownership tags on variables)

The language successfully implements its core mission of being a safe, AI-friendly systems language with the essential features working correctly.