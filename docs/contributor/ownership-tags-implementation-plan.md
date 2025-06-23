# Ownership Tags on Variables Implementation Plan

**Created Date**: January 2025  
**Status**: Planning Phase  
**Grammar Reference**: Line 89 of grammar.txt  
**Estimated Duration**: 2-3 days

## Overview

This document outlines the implementation plan for ownership tags on variable declarations in Asthra. While ownership tags are already implemented for struct and enum declarations, they are not yet implemented for variables despite being defined in the grammar.

## Grammar Definition

From grammar.txt:
```
VarDecl <- 'let' MutModifier? SimpleIdent ':' Type OwnershipTag? '=' Expr ';'
OwnershipTag <- '#[ownership(' ('gc'|'c'|'pinned') ')]'
```

This allows variables to specify their memory management strategy:
- `gc` - Garbage collected
- `c` - C-style manual management
- `pinned` - Pinned in memory (cannot be moved)

## Current State

### What Works
```asthra
// Ownership tags on structs
#[ownership(c)]
pub struct FFIData {
    ptr: *mut u8,
    len: usize
}

// Ownership tags on enums  
#[ownership(gc)]
pub enum Result<T, E> {
    Ok(T),
    Err(E)
}
```

### What Doesn't Work (But Should)
```asthra
// Variable with ownership tag
let data: Buffer #[ownership(c)] = allocate_buffer(1024);
let cached: String #[ownership(pinned)] = "immovable data";
let result: Result<i32, Error> #[ownership(gc)] = compute();
```

## Implementation Analysis

### Parser Location
- **File**: `src/parser/grammar_statements_core.c`
- **Function**: `parse_var_decl` (line 92)
- **TODO**: Line 207 - "Parse ownership tags if needed in future versions"

### Existing Infrastructure
1. **Annotation Parsing**: Already implemented in `grammar_annotations.c`
   - `parse_annotation()` handles `#[ownership(...)]` syntax
   - `parse_bracketed_annotation()` validates gc/c/pinned values
   
2. **AST Support**: Need to verify if AST_LET_STMT has annotation field
   
3. **Semantic Analysis**: Need to handle ownership tags in variable analysis

## Implementation Plan

### Phase 1: Parser Enhancement (Day 1)

#### 1.1 Update Variable Declaration Parser
```c
// In parse_var_decl() after parsing type (line 206):
ASTNode *ownership_tag = NULL;
if (is_annotation_start(parser)) {
    ownership_tag = parse_annotation(parser);
    if (ownership_tag && 
        (ownership_tag->type != AST_ANNOTATION || 
         strcmp(ownership_tag->data.annotation.name, "ownership") != 0)) {
        report_error(parser, "Only ownership annotations allowed on variables");
        ast_free_node(ownership_tag);
        ownership_tag = NULL;
    }
}
```

#### 1.2 Update AST Structure
- Verify AST_LET_STMT has annotations field
- If not, add it to maintain consistency with struct/enum declarations

### Phase 2: Semantic Analysis (Day 2)

#### 2.1 Update Variable Analysis
- **File**: `src/analysis/semantic_basic_statements.c` (or similar)
- Handle ownership tags during variable declaration analysis
- Validate ownership tag compatibility with type

#### 2.2 Symbol Table Enhancement
- Store ownership information in symbol table
- Use for FFI safety checks and optimization

### Phase 3: Code Generation (Day 3)

#### 3.1 Memory Management Hints
- Use ownership tags to guide memory allocation
- Generate appropriate cleanup code based on ownership

#### 3.2 FFI Integration
- Validate FFI calls respect ownership semantics
- Generate warnings for ownership mismatches

## Test Cases

### Basic Functionality
```asthra
package test;

pub fn test_ownership_tags(none) -> void {
    // Garbage collected by default
    let normal: String = "default gc";
    
    // Explicitly garbage collected
    let gc_data: Buffer #[ownership(gc)] = Buffer::new(100);
    
    // C-style manual management
    let c_buffer: *mut u8 #[ownership(c)] = malloc(1024);
    
    // Pinned in memory
    let pinned_str: String #[ownership(pinned)] = "cannot move";
    
    return ();
}
```

### FFI Safety
```asthra
package ffi_test;

extern "C" fn c_function(data: *mut u8) -> void;

pub fn test_ffi_ownership(none) -> void {
    // This should work - C ownership matches FFI
    let buffer: *mut u8 #[ownership(c)] = malloc(100);
    c_function(buffer);
    
    // This should warn - GC data passed to C
    let gc_buffer: Buffer #[ownership(gc)] = Buffer::new(100);
    c_function(gc_buffer.ptr); // Warning: ownership mismatch
    
    return ();
}
```

### Error Cases
```asthra
// Invalid ownership type
let data: i32 #[ownership(rust)] = 42; // Error: Unknown ownership type

// Multiple ownership tags
let data: String #[ownership(gc)] #[ownership(c)] = "test"; // Error: Multiple ownership tags

// Ownership on wrong annotation position
#[ownership(gc)] let data: String = "test"; // Error: Ownership must come after type
```

## Integration Points

### 1. Type System
- Ownership tags don't change types but affect memory management
- Should be compatible with all types except primitives?

### 2. Pattern Matching
- Ownership tags should be preserved through destructuring
- May need to handle in pattern analysis

### 3. Function Parameters
- Grammar doesn't allow ownership tags on parameters
- Consider if this restriction is intentional

## Success Criteria

1. **Parser accepts** ownership tags on variables per grammar
2. **Semantic analysis** validates ownership types
3. **No regression** in existing tests
4. **FFI safety** improvements with ownership checking
5. **Clear error messages** for invalid ownership usage

## Future Considerations

1. **Inference**: Could infer ownership from usage patterns
2. **Conversions**: Rules for converting between ownership types
3. **Lifetime interaction**: How ownership relates to borrowing
4. **Performance**: Use ownership hints for optimization

## Risks and Mitigation

1. **AST Changes**: May need to modify AST structure
   - Mitigation: Check existing annotation support first
   
2. **Semantic Complexity**: Ownership rules interaction
   - Mitigation: Start simple, expand gradually
   
3. **Code Generation**: May complicate memory management
   - Mitigation: Initially just store info, don't change behavior