# Main Function Return Values - Complete Documentation Summary

**Status**: ✅ Fully Documented  
**Last Updated**: 2025-01-07  
**Coverage**: Architecture, Specification, User Manual

This document provides a comprehensive overview of how main function return values and process exit codes are documented across the Asthra documentation suite.

## Documentation Structure

### 1. Architecture Documentation
**Location**: [`docs/architecture/main-function-return-values.md`](architecture/main-function-return-values.md)

**Focus**: Technical implementation details, compiler internals, and system-level architecture

**Key Topics**:
- Compiler exit code management (`CliOptions` structure)
- Generated program exit code mapping for all return types
- Runtime termination mechanisms (`asthra_panic`, generated panic code)
- Platform-specific considerations (Unix/Linux vs Windows)
- Type system integration (Never type implementation)
- Optimization opportunities (dead code elimination, control flow analysis)
- Testing framework integration (BDD, unit tests)

**Target Audience**: Compiler developers, contributors, system architects

### 2. Specification Documentation
**Location**: [`docs/spec/overview.md`](spec/overview.md) and [`docs/spec/types.md`](spec/types.md)

**Focus**: Formal language specification and type system definition

**Key Topics**:
- Complete specification of all four return types (`i32`, `void`, `Result<i32, string>`, `Never`)
- Semantic definitions and behavior guarantees
- AI generation patterns for each return type
- Platform compatibility requirements
- Implementation status and testing coverage
- Never type integration in type system with main function examples

**Target Audience**: Language implementers, AI model trainers, specification writers

### 3. User Manual Documentation
**Location**: [`docs/user-manual/getting-started.md`](user-manual/getting-started.md) and [`docs/user-manual/language-fundamentals.md`](user-manual/language-fundamentals.md)

**Focus**: Practical usage guidance and developer experience

**Key Topics**:
- Comprehensive examples for each return type
- Use case recommendations and best practices
- Migration patterns between return types
- Real-world application scenarios
- Choosing the right return type for different project types

**Target Audience**: Asthra developers, application programmers, tutorial users

## Complete Feature Matrix

| Return Type | Spec Status | Architecture Status | User Manual Status | Examples | Use Cases |
|-------------|-------------|--------------------|--------------------|----------|-----------|
| `i32` | ✅ Complete | ✅ Complete | ✅ Complete | ✅ Multiple | Command-line tools, utilities |
| `void` | ✅ Complete | ✅ Complete | ✅ Complete | ✅ Multiple | Simple programs, demos |
| `Result<i32, string>` | ✅ Complete | ✅ Complete | ✅ Complete | ✅ Multiple | Complex applications |
| `Never` | ✅ Complete | ✅ Complete | ✅ Complete | ✅ Multiple | Services, embedded systems |

## Cross-Reference Guide

### For Compiler Development
1. **Start with**: [Architecture documentation](architecture/main-function-return-values.md)
2. **Reference**: [Type system specification](spec/types.md#never-type-in-main-functions)
3. **Validate with**: User manual examples for testing

### For Language Learning
1. **Start with**: [Getting Started guide](user-manual/getting-started.md#main-function-return-types)
2. **Deep dive**: [Language Fundamentals](user-manual/language-fundamentals.md#main-function-return-types)
3. **Reference**: [Specification overview](spec/overview.md#main-function-return-types---complete-specification-)

### For AI Model Training
1. **Patterns**: [Specification AI generation patterns](spec/overview.md#ai-generation-patterns)
2. **Examples**: [User manual comprehensive examples](user-manual/getting-started.md#main-function-return-types)
3. **Implementation**: [Architecture technical details](architecture/main-function-return-values.md)

## Code Examples Summary

### Basic Patterns
```asthra
// Integer return - direct exit code
pub fn main(none) -> i32 {
    return 0;  // Success
}

// Void return - implicit success
pub fn main(none) -> void {
    return ();  // Always successful
}

// Result return - rich error reporting
pub fn main(none) -> Result<i32, string> {
    return Result.Ok(0);  // Or Result.Err("message")
}

// Never return - continuous operation
pub fn main(none) -> Never {
    loop { handle_requests(); }  // Never returns
}
```

### Advanced Usage Patterns
```asthra
// Command-line tool with exit codes
pub fn main(none) -> i32 {
    match validate_arguments() {
        ValidationResult.Success => 0,
        ValidationResult.Warning => 1,
        ValidationResult.Error => 2,
        ValidationResult.Critical => 3
    }
}

// Application with detailed error reporting
pub fn main(none) -> Result<i32, string> {
    let config = match load_config() {
        Ok(c) => c,
        Err(e) => return Result.Err("Config error: " + e)
    };
    
    match run_application(config) {
        Ok(exit_code) => Result.Ok(exit_code),
        Err(e) => Result.Err("Runtime error: " + e)
    }
}

// System service with infinite loop
pub fn main(none) -> Never {
    initialize_service();
    loop {
        match handle_request() {
            Ok(_) => continue,
            Err(critical) => panic("Service failure: " + critical)
        }
    }
}
```

## Implementation Status

### Compiler Support
- ✅ **Exit Code Generation**: All return types generate correct exit codes
- ✅ **Type Checking**: Full semantic analysis for all return types
- ✅ **Code Generation**: C code generation for all patterns
- ✅ **Platform Support**: Unix/Linux and Windows compatibility

### Runtime Support
- ✅ **Panic Mechanism**: `asthra_panic()` function with exit code 1
- ✅ **Result Handling**: Automatic stderr output and exit code mapping
- ✅ **Never Type**: Full type system integration with optimizations

### Testing Coverage
- ✅ **Unit Tests**: All return types covered in test suite
- ✅ **BDD Tests**: Behavior validation for exit codes
- ✅ **Integration Tests**: End-to-end validation of generated programs
- ✅ **Platform Tests**: Cross-platform exit code validation

## Best Practices Summary

### Choosing Return Types
1. **`void`**: Simple programs without error conditions
2. **`i32`**: Standard Unix-style tools and utilities
3. **`Result<i32, string>`**: Applications needing descriptive errors
4. **`Never`**: Long-running services and embedded systems

### Migration Strategies
- Start with `void` for prototypes
- Upgrade to `i32` when basic error handling is needed
- Move to `Result<i32, string>` for production applications
- Use `Never` for services that run indefinitely

### AI Generation Guidelines
- All return types have clear, unambiguous patterns
- Examples provided for each pattern type
- Semantic meaning is consistent across all documentation
- Error handling patterns are well-defined and predictable

## Future Enhancements

### Planned Documentation Updates
1. **Interactive Examples**: Web-based code examples with live compilation
2. **Video Tutorials**: Recorded explanations of each return type
3. **Migration Guides**: Detailed guides for converting between return types
4. **Performance Analysis**: Benchmarks and optimization guides

### Language Evolution
1. **Signal Handling**: Integration with OS signal handling
2. **Async Main**: Support for async main functions
3. **Structured Exit Codes**: Enhanced Result types with structured error codes

## Conclusion

The main function return value system in Asthra is now comprehensively documented across all three documentation areas:

- **Architecture documentation** provides the technical foundation for implementers
- **Specification documentation** defines the formal language behavior
- **User manual documentation** guides practical usage and best practices

This multi-layered approach ensures that all stakeholders - from compiler developers to application programmers to AI model trainers - have access to the appropriate level of detail for their needs.

The documentation is complete, consistent, and ready for production use. 