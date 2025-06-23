# Main Function Return Values Architecture

**Status**: ✅ Implemented  
**Last Updated**: 2025-01-07  
**Version**: v1.22  

This document describes the architectural design and implementation of main function return values and process exit codes in the Asthra programming language.

## Overview

Asthra provides flexible main function return types that map to process exit codes, enabling different patterns for application termination and error handling. The architecture supports multiple return type patterns while maintaining consistency with system-level process management.

## Architectural Components

### 1. Compiler Exit Code Management

The Asthra compiler itself uses standard Unix exit code conventions:

```c
// src/cli.h - CLI Options Structure
typedef struct {
    AsthraCompilerOptions compiler_options;
    bool test_mode;
    bool show_version;
    bool show_help;
    int exit_code;  // Compiler's own exit code
} CliOptions;
```

**Exit Code Flow:**
1. CLI argument parsing sets `exit_code` in `CliOptions`
2. Main compiler function (`src/main.c`) returns this exit code
3. Shell receives the exit code for build script integration

### 2. Generated Program Exit Code Mapping

Asthra programs support multiple main function return types, each with specific exit code semantics:

#### Type A: Direct Integer Return (`i32`)
```asthra
pub fn main(none) -> i32 {
    return 0;  // Direct exit code
}
```

**Implementation:**
- Return value becomes process exit code directly
- Range: Typically 0-255 (platform dependent)
- 0 = success, non-zero = error

#### Type B: Void Return (`void`)
```asthra
pub fn main(none) -> void {
    return ();  // Implicit exit code 0
}
```

**Implementation:**
- Generated C code returns 0 from main
- Always indicates successful termination
- Used for simple programs without error conditions

#### Type C: Result Return (`Result<i32, string>`)
```asthra
pub fn main(none) -> Result<i32, string> {
    return Result.Ok(0);        // Success with exit code
    // return Result.Err("error"); // Error with message
}
```

**Implementation:**
- `Result.Ok(code)` → exit with `code`
- `Result.Err(msg)` → print message to stderr, exit with code 1
- Enables descriptive error reporting

#### Type D: Never Return (`Never`)
```asthra
pub fn main(none) -> Never {
    panic("Critical failure");  // Never returns normally
}
```

**Implementation:**
- Function must never return normally
- Always terminates via `panic()`, `exit()`, or infinite loop
- Exit code determined by termination mechanism

### 3. Runtime Termination Mechanisms

#### Panic Function
```c
// runtime/asthra_runtime_core.c
void asthra_panic(const char *message) {
    if (message) {
        fprintf(stderr, "panic: %s\n", message);
        fflush(stderr);
    }
    exit(1);  // Always exits with code 1
}
```

#### Generated Panic Code
```c
// src/compiler/code_generation.c - panic() call generation
fprintf(output, "    fprintf(stderr, \"panic: \");\n");
fprintf(output, "    fprintf(stderr, ");
// ... argument processing ...
fprintf(output, ");\n");
fprintf(output, "    fprintf(stderr, \"\\n\");\n");
fprintf(output, "    exit(1)");  // Hardcoded exit code 1
```

## Platform-Specific Considerations

### Unix/Linux Systems
```c
// src/compiler/pipeline_phases.c - Unix exit code handling
#if ASTHRA_PLATFORM_UNIX
if (WIFEXITED(link_result) && WEXITSTATUS(link_result) == 0) {
    // Extract exit code using WEXITSTATUS macro
    bool success = (WIFEXITED(link_result) && WEXITSTATUS(link_result) == 0);
}
#endif
```

### Windows Systems
```c
// Windows - Direct exit code return
#else
// On Windows, system() returns the exit code directly
if (link_result == 0) {
    // Windows exit codes are returned directly
    bool success = (link_result == 0);
}
#endif
```

## Type System Integration

### Never Type Architecture

The Never type is fully integrated into the type system:

```c
// src/analysis/semantic_types_defs.h
typedef enum {
    PRIMITIVE_VOID,
    // ... other types ...
    PRIMITIVE_NEVER,  // Never type for functions that don't return
    PRIMITIVE_COUNT
} PrimitiveKind;
```

**Never Type Properties:**
- Size: 0 bytes (no values exist)
- Alignment: 1 byte
- Category: `TYPE_PRIMITIVE`
- Semantic: Bottom type (subtype of all types)

### Semantic Analysis

```c
// src/analysis/semantic_control_flow.c - Never type handling
bool is_never_function = (expected_return_type->category == TYPE_PRIMITIVE && 
                         expected_return_type->data.primitive.primitive_kind == PRIMITIVE_NEVER);

if (is_never_function && is_unit_return) {
    // Allow "return ();" in Never-returning functions as placeholder
    // This is technically unreachable code but useful for development
}
```

## Optimization Opportunities

### Dead Code Elimination

The Never type enables advanced optimizations:

```c
// src/codegen/optimizer_never.c
bool optimizer_is_never_returning_function(const TypeDescriptor *func_type) {
    const TypeDescriptor *return_type = func_type->data.function.return_type;
    return (return_type->category == TYPE_PRIMITIVE && 
            return_type->data.primitive.primitive_kind == PRIMITIVE_NEVER);
}
```

**Optimization Passes:**
1. Mark blocks unreachable after Never-returning calls
2. Remove successor edges from Never-calling blocks
3. Add branch prediction hints for Never paths

### Control Flow Analysis

Never-returning functions enable:
- Exhaustiveness checking in match expressions
- Unreachable code warnings
- Branch elimination optimizations

## Testing Framework Integration

The test framework uses standardized exit codes:

```c
// tests/framework/test_suite.c
int asthra_test_suite_run_and_exit(AsthraTestSuite *suite) {
    AsthraTestResult result = asthra_test_suite_run(suite);
    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
```

**Test Exit Code Conventions:**
- 0: All tests passed
- 1: Some tests failed
- Other codes: Framework errors

## BDD Testing Integration

Behavior-driven development tests validate exit codes:

```c
// bdd/steps/common_steps.c
static int execution_exit_code = -1;

void then_exit_code_is(int expected_code) {
    BDD_ASSERT_EQ(execution_exit_code, expected_code);
}
```

## Design Rationale

### Multiple Return Type Support

**Flexibility**: Different patterns suit different application types:
- `i32`: System utilities, command-line tools
- `void`: Simple scripts, demonstrations
- `Result<i32, string>`: Applications with rich error reporting
- `Never`: System services, embedded applications

**Consistency**: All patterns map predictably to process exit codes

**AI Generation**: Clear, unambiguous patterns for AI code generation

### Never Type Integration

**Type Safety**: Prevents accidental use of diverging functions
**Optimization**: Enables dead code elimination and control flow analysis
**System Integration**: Works with C functions that never return (`exit`, `abort`)

## Implementation Status

| Component | Status | Location |
|-----------|--------|----------|
| Compiler Exit Codes | ✅ Complete | `src/cli.c`, `src/main.c` |
| Integer Return Type | ✅ Complete | Code generation |
| Void Return Type | ✅ Complete | Code generation |
| Result Return Type | ✅ Complete | Code generation |
| Never Return Type | ✅ Complete | Type system, semantic analysis |
| Panic Mechanism | ✅ Complete | Runtime, code generation |
| Platform Support | ✅ Complete | Unix, Windows |
| Optimization | ✅ Complete | Never type optimizations |
| Testing Integration | ✅ Complete | Test framework, BDD |

## Future Enhancements

### Planned Features
1. **Signal Handling**: Integration with OS signal handling for graceful shutdown
2. **Exit Code Constants**: Predefined constants for common exit codes
3. **Structured Termination**: Enhanced Result types with structured error codes

### Research Areas
1. **Async Main**: Support for async main functions with event loop integration
2. **Multi-threading**: Main function patterns for multi-threaded applications
3. **Resource Cleanup**: Automatic resource cleanup on termination

## References

- [POSIX Exit Status](https://pubs.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#tag_18_08_02)
- [Never Type in Type Theory](https://en.wikipedia.org/wiki/Bottom_type)
- [Process Exit Codes Best Practices](https://tldp.org/LDP/abs/html/exitcodes.html) 