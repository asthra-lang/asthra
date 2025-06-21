---
layout: page
title: Standard Library
permalink: /docs/stdlib/
---

# Asthra Standard Library

**Version:** 1.2  
**Status:** Complete  

Welcome to the Asthra Standard Library documentation! The stdlib provides essential functionality for Asthra programs, including core runtime operations, system interaction, data structures, algorithms, and I/O operations. All modules are designed with AI-first principles, featuring explicit error handling and deterministic behavior.

## 🚀 Quick Start

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

---

## 📚 Documentation Structure

### Core Concepts

#### [Import Convention](/docs/stdlib/01-import-convention/)
How to import and use stdlib modules with proper naming conventions.

#### [Error Handling](/docs/stdlib/02-error-handling/)
Patterns for robust error handling using `Result<T, E>` types.

#### [Best Practices](/docs/stdlib/best-practices/)
Recommended patterns, techniques, and optimization strategies.

#### [Collections Design](/docs/stdlib/collections_design/)
Comprehensive guide to Asthra's data structures and collection types.

### Core Runtime Modules (`stdlib/asthra/*`)

#### [Core Module](/docs/stdlib/modules/)
Runtime internals, memory management, and type system fundamentals.

#### [Panic Handling](/docs/stdlib/modules/)
Panic handling, debugging utilities, and assertion mechanisms.

#### [Filesystem Operations](/docs/stdlib/modules/)
File I/O, path utilities, and filesystem interaction.

#### [Environment Access](/docs/stdlib/modules/)
Environment variables, process information, and system details.

#### [Collections](/docs/stdlib/modules/)
Vec, HashMap, LinkedList, and other essential data structures.

#### [Mathematics](/docs/stdlib/modules/)
Mathematical functions, constants, and numerical operations.

#### [Random Generation](/docs/stdlib/modules/)
Random number generation and cryptographic utilities.

#### [Process Management](/docs/stdlib/modules/)
Process creation, management, and inter-process communication.

#### [Algorithms](/docs/stdlib/modules/)
Sorting, searching, and functional programming utilities.

### Specialized Modules

#### [Date and Time](/docs/stdlib/modules/)
Duration handling, timing operations, and system time utilities.

#### [String Processing](/docs/stdlib/modules/)
String manipulation, UTF-8 support, and text processing.

---

## ✨ Key Features

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

---

## 📊 Module Status

| Module | Package | Status | Features |
|--------|---------|--------|----------|
| Core | `stdlib::asthra::core` | ✅ Complete | Runtime, memory, types |
| Panic | `stdlib::asthra::panic` | ✅ Complete | Error handling, debugging |
| Filesystem | `stdlib::asthra::fs` | ✅ Complete | File I/O, paths |
| Environment | `stdlib::asthra::env` | ✅ Complete | System info, env vars |
| Collections | `stdlib::asthra::collections` | ✅ Complete | Data structures |
| Math | `stdlib::asthra::math` | ✅ Complete | Mathematical operations |
| Random | `stdlib::asthra::random` | ✅ Complete | RNG, cryptography |
| Process | `stdlib::asthra::process` | ✅ Complete | Process management |
| Algorithms | `stdlib::asthra::algorithms` | ✅ Complete | Sorting, searching |
| Time | `stdlib::datetime::time` | ✅ Complete | Duration, timing |
| String | `stdlib::string` | ✅ Complete | Text processing |

---

## 🎯 Getting Started Guide

### For New Users
1. **Read the [Import Convention](/docs/stdlib/01-import-convention/)** to understand module usage
2. **Review [Error Handling](/docs/stdlib/02-error-handling/)** patterns for robust code
3. **Explore [Collections Design](/docs/stdlib/collections_design/)** for data structures
4. **Check [Best Practices](/docs/stdlib/best-practices/)** for recommended patterns

### For Advanced Users
1. **Study [Core Modules](/docs/stdlib/modules/)** for runtime internals
2. **Learn [Memory Management](/docs/stdlib/best-practices/)** techniques
3. **Explore [Performance Optimization](/docs/stdlib/best-practices/)** strategies
4. **Review [FFI Integration](/docs/stdlib/best-practices/)** patterns

---

## 💡 Examples

Each module includes comprehensive examples and usage patterns:

- **Basic Usage**: Simple examples in each module's documentation
- **Integration Patterns**: Cross-module usage in Best Practices
- **Real-World Scenarios**: Practical applications and performance guides
- **Error Handling**: Robust error management patterns

---

## 🔧 Quality Standards

The Asthra Standard Library maintains strict quality standards:

- ✅ **100% Test Coverage**: All functions have comprehensive tests
- ✅ **Memory Safety**: Zero memory leaks and proper resource management
- ✅ **Complete Documentation**: Full API documentation with examples
- ✅ **High Performance**: Competitive performance with equivalent libraries
- ✅ **AI-Friendly Design**: Optimized for code generation and analysis

---

*The Asthra Standard Library provides the foundation for building robust, high-performance applications with AI-first design principles.* 