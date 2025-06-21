# Asthra Standard Library Documentation

**Version**: 1.2  
**Status**: Complete  

## Overview

The Asthra Standard Library provides essential functionality for Asthra programs, including core runtime operations, system interaction, data structures, algorithms, and I/O operations. All modules are designed with AI-first principles, featuring explicit error handling through `Result<T, E>` types and deterministic behavior.

## Quick Start

```asthra
// Import convention - always use stdlib/ prefix
import "stdlib/asthra/core";
import "stdlib/string";
import "stdlib/datetime/time";

fn main() -> i32 {
    let version = core.runtime_version();
    log("INFO", "Running Asthra " + version);
    return 0;
}
```

## Documentation Structure

### 📚 Core Concepts
- **[Import Convention](01-import-convention.md)** - How to import and use stdlib modules
- **[Error Handling](02-error-handling.md)** - Patterns for robust error handling
- **[Memory Management](03-memory-management.md)** - Memory zones and best practices

### 🔧 Core Runtime Modules (`stdlib/asthra/*`)
- **[core](modules/01-core.md)** - Runtime internals, memory management, type system
- **[panic](modules/02-panic.md)** - Panic handling, debugging, assertions
- **[fs](modules/03-fs.md)** - Filesystem operations, file I/O, path utilities
- **[env](modules/04-env.md)** - Environment variables, process info, system details
- **[collections](modules/05-collections.md)** - Vec, HashMap, LinkedList data structures
- **[math](modules/06-math.md)** - Mathematical functions and constants
- **[random](modules/07-random.md)** - Random number generation and cryptography
- **[process](modules/08-process.md)** - Process creation and management
- **[algorithms](modules/09-algorithms.md)** - Sorting, searching, functional utilities

### 🕒 Date and Time (`stdlib/datetime/*`)
- **[time](modules/10-time.md)** - Duration, timing, deadlines, system time

### 📝 String Processing (`stdlib/string`)
- **[string](modules/11-string.md)** - String manipulation and UTF-8 support

### 📖 Advanced Topics
- **[Best Practices](best-practices.md)** - Recommended patterns and techniques
- **[Performance Guide](performance.md)** - Optimization tips and benchmarks
- **[FFI Integration](ffi-guide.md)** - C interoperability patterns

## Module Status

| Module | Package | Status | Documentation |
|--------|---------|--------|---------------|
| Core | `stdlib::asthra::core` | ✅ Complete | [📖](modules/01-core.md) |
| Panic | `stdlib::asthra::panic` | ✅ Complete | [📖](modules/02-panic.md) |
| Filesystem | `stdlib::asthra::fs` | ✅ Complete | [📖](modules/03-fs.md) |
| Environment | `stdlib::asthra::env` | ✅ Complete | [📖](modules/04-env.md) |
| Collections | `stdlib::asthra::collections` | ✅ Complete | [📖](modules/05-collections.md) |
| Math | `stdlib::asthra::math` | ✅ Complete | [📖](modules/06-math.md) |
| Random | `stdlib::asthra::random` | ✅ Complete | [📖](modules/07-random.md) |
| Process | `stdlib::asthra::process` | ✅ Complete | [📖](modules/08-process.md) |
| Algorithms | `stdlib::asthra::algorithms` | ✅ Complete | [📖](modules/09-algorithms.md) |
| Time | `stdlib::datetime::time` | ✅ Complete | [📖](modules/10-time.md) |
| String | `stdlib::string` | ✅ Complete | [📖](modules/11-string.md) |

## Key Features

### Memory Safety
- **Hybrid GC + Manual Zones**: Choose the right memory management for your use case
- **Ownership Tracking**: Clear ownership semantics for FFI safety
- **Zero Memory Leaks**: Comprehensive testing ensures no memory leaks

### Error Handling
- **Explicit Results**: All fallible operations return `Result<T, E>` types
- **Pattern Matching**: Systematic error handling with match expressions
- **Type Safety**: Compile-time error checking prevents runtime failures

### AI-Native Design
- **Deterministic Behavior**: Predictable execution for reproducible results
- **Machine-Friendly APIs**: Optimized for code generation tools
- **Unambiguous Semantics**: Clear function signatures and behavior

### Cross-Platform Support
- **Consistent APIs**: Same behavior across Windows, macOS, and Linux
- **Platform Detection**: Runtime platform identification and adaptation
- **Native Performance**: Zero-cost abstractions over system calls

## Getting Started

1. **Read the [Import Convention](01-import-convention.md)** to understand how to use stdlib modules
2. **Review [Error Handling](02-error-handling.md)** patterns for robust code
3. **Explore individual modules** starting with [core](modules/01-core.md) for runtime basics
4. **Check [Best Practices](best-practices.md)** for recommended patterns

## Examples

Each module documentation includes comprehensive examples. For complete usage demonstrations, see:

- **Basic Usage**: Simple examples in each module's documentation
- **Integration Patterns**: Cross-module usage in [Best Practices](best-practices.md)
- **Real-World Scenarios**: Practical applications in [Performance Guide](performance.md)

## Contributing

The Asthra Standard Library follows strict quality standards:

- **100% Test Coverage**: All functions must have comprehensive tests
- **Memory Safety**: Zero memory leaks and proper resource management
- **Documentation**: Complete API documentation with examples
- **Performance**: Competitive performance with equivalent libraries

For contribution guidelines, see the main Asthra project documentation. 
