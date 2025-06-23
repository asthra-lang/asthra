# Fix spawn_with_handle Grammar Issue

## Problem Summary
`spawn_with_handle` appears in two conflicting contexts in the grammar:
1. As a statement (line 97) - CORRECT
2. As a unary expression (line 124) - INCORRECT and unimplemented

## Solution Plan

### 1. Update Grammar
- Remove `spawn_with_handle` from the Unary expression rule (line 124)
- Keep it only as a statement form (line 97)
- This aligns with the actual implementation

### 2. Verify Parser Implementation
- Confirm parser only implements spawn_with_handle as a statement
- Ensure no code tries to parse it as an expression

### 3. Update Documentation
- Document that spawn_with_handle is a statement-only construct
- Clarify the syntax: `spawn_with_handle handle = function();`

### 4. Add Tests
- Test that spawn_with_handle works correctly as a statement
- Test that it cannot be used in expression contexts

## Expected Grammar Change

Before:
```
Unary <- ('await' PostfixExpr) / ('spawn_with_handle' PostfixExpr) / (UnaryPrefix PostfixExpr)
```

After:
```
Unary <- ('await' PostfixExpr) / (UnaryPrefix PostfixExpr)
```

This makes the grammar consistent with the implementation and clarifies that:
- `spawn` is a fire-and-forget statement
- `spawn_with_handle` is a statement that captures a handle
- Neither can be used as expressions