# Current Failing BDD Scenarios - Phase 4

**Generated**: 2025-07-01  
**Total Failing Scenarios**: 74 out of 362 tests

## Critical Failures (Compiler Crashes) 🔴

### sizeof_operator (4 crashes)
- **Sizeof integer types** - Segfault: `Both operands to ICmp instruction are not of the same type!`
- **Sizeof pointer types** - Segfault: Type mismatch i8 vs i32
- **Sizeof struct types** - Segfault: `icmp sge i8 %size_point1, i8 8` type mismatch
- **Sizeof array types** - Runtime failure (compiles but wrong result)

### pointer_operations (1 crash)
- **Pointer function parameter** - Segfault: `Call parameter type does not match function signature`

### slice_operations (1 crash)
- **Slice access indexing** - Segfault: `GEP base pointer is not a vector`

### predeclared_identifiers (1 crash)
- **Args function** - Bus error at runtime when calling `args()`

## Parser Errors 🟡

### spawn_statements (3 failures)
- **Spawn with method call** - Error: `expected '(' but found '.'` (spawn manager.process(42))
- **Spawn error non-function** - Error: `expected '(' but found ';'` (spawn value;)
- **Spawn error missing semicolon** - Working as expected (validation test)

### spawn_with_handle (3 failures)
- **Spawn handle with custom type** - Error: `Expected struct name`
- **Spawn handle error missing handle** - Parser error before semantic check
- **Spawn handle error type mismatch** - Parser error before semantic check

### slice_operations (23 failures)
- All slice creation tests fail with: `expected ';' but found 'as'`
- Parser doesn't recognize slice syntax like `arr[0:3]`

### return_statements (1 failure)
- **Early return conditional** - Error: `Expected type` at line 4:16

## Runtime Failures 🟠

### await_expressions (7 failures)
- **Basic await expression** - Wrong exit code (expected 42)
- **Await with arithmetic** - Wrong exit code (expected 31)
- **Multiple await expressions** - Wrong exit code (expected 27)
- **Await void function** - Wrong exit code
- **Await in expressions** - Semantic error: `Left operand has no type information`
- **Await error non-handle** - Should fail compilation but succeeds

### unsafe_blocks (5 failures)
- **Basic unsafe block** - Wrong exit code (expected 42, got 0)
- **Unsafe pointer operations** - Wrong exit code (expected 100, got 0)
- **Unsafe mutable pointer** - Wrong exit code
- **Nested unsafe blocks** - Wrong exit code
- **Unsafe with struct** - Wrong exit code

### pointer_operations (5 failures)
- **Basic address-of operator** - Wrong exit code
- **Mutable pointer operations** - Wrong exit code
- **Pointer to struct** - Wrong exit code
- **Pointer error dereference unsafe** - Should fail but succeeds

### associated_functions (8 failures)
- **Basic associated function** - Wrong exit code (expected 8)
- **Associated function multiple params** - Wrong exit code (expected 14)
- **Associated function custom return** - Wrong exit code
- **Multiple associated functions** - Wrong exit code
- **Associated function in expressions** - Semantic error: no type information
- **Associated function error undefined** - Should fail but succeeds

### predeclared_identifiers (6 failures)
- **Range start and end** - Error: `Function 'range' expects 1 arguments, got 2`
- **Log function basic** - Wrong exit code
- **Panic function** - Wrong exit code
- **Exit function** - Wrong exit code
- **Infinite iterator error** - Should fail but succeeds
- **Shadowing log function** - Correctly fails (can't shadow predeclared)

## Summary by Category

| Category | Count | Examples |
|----------|-------|----------|
| **Compiler Crashes** | 7 | sizeof operations, pointer params, slices |
| **Parser Errors** | 30 | spawn syntax, slice syntax, return statements |
| **Runtime Wrong Result** | 35 | await, unsafe, pointers, associated functions |
| **Missing Validation** | 2 | await non-handle, undefined functions |

## Most Urgent Fixes

1. **Fix LLVM type mismatches in sizeof** - Causing segfaults
2. **Fix pointer parameter codegen** - Causing segfaults  
3. **Implement await/unsafe runtime** - Features compile but don't work
4. **Add slice syntax to parser** - Currently not recognized
5. **Fix args() function** - Causes bus errors