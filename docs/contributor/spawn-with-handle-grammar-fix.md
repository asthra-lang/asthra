# spawn_with_handle Grammar Fix

## Summary
Fixed grammar inconsistency where `spawn_with_handle` appeared both as a statement and as a unary expression operator, despite only being implemented as a statement.

## Changes Made

### 1. Grammar Updates
- Removed `spawn_with_handle` from the Unary expression rule (line 124)
- Updated SpawnStmt rule to correctly reflect the implemented syntax (line 97)

### 2. Grammar Corrections

#### Before:
```
SpawnStmt <- ('spawn' / 'spawn_with_handle') PostfixExpr ';'
Unary     <- ('await' PostfixExpr) / ('spawn_with_handle' PostfixExpr) / (UnaryPrefix PostfixExpr)
```

#### After:
```
SpawnStmt <- 'spawn' PostfixExpr ';' / 'spawn_with_handle' SimpleIdent '=' PostfixExpr ';'
Unary     <- ('await' PostfixExpr) / (UnaryPrefix PostfixExpr)
```

## Rationale

1. **Implementation Reality**: The parser implementation expects `spawn_with_handle handle = function();` syntax, not the PostfixExpr form
2. **Semantic Clarity**: spawn_with_handle is a statement that assigns a handle, not an expression that returns a value
3. **Consistency**: This aligns the grammar with the actual implementation

## Usage Examples

### Correct Usage (Statements Only)
```asthra
// Regular spawn - fire and forget
spawn worker_task();

// Spawn with handle - capture task handle for later await
spawn_with_handle handle = compute_task();
let result = await handle;
```

### Incorrect Usage (Not Allowed)
```asthra
// ❌ Cannot use spawn_with_handle as expression
let x = spawn_with_handle worker();  // ERROR

// ❌ Cannot use in arithmetic
let y = 5 + spawn_with_handle compute();  // ERROR

// ❌ Cannot use as function argument
process(spawn_with_handle task());  // ERROR
```

## Benefits

1. **Eliminates Confusion**: Clear distinction between statements and expressions
2. **Matches Implementation**: Grammar now accurately describes what the parser accepts
3. **Better Error Messages**: Parser can give clearer errors when spawn_with_handle is misused
4. **AI-Friendly**: Removes ambiguity about where spawn_with_handle can be used