# Asthra

A modern, safe, and performant systems programming language with C23-based compiler infrastructure.

## Features

- **Memory Safety**: Automatic memory management with garbage collection and atomic reference counting
- **Type Safety**: Strong static typing with type inference
- **Concurrency**: Built-in concurrency support with lock-free data structures
- **C Interoperability**: Seamless C FFI with safety guarantees
- **Performance**: 15-25% faster compilation, 10-20% memory efficiency improvements

## Quick Start

### Build
```bash
cmake -B build && cmake --build build -j8
```

### Test
```bash
cmake --build build --target build-tests
ctest --test-dir build
```

### Development
```bash
# Format code
cmake --build build --target format

# Static analysis
cmake --build build --target analyze
```

## Architecture

- **Parser**: Modular lexer/parser with AST generation
- **Analysis**: Semantic analysis, type inference, symbol tables
- **Codegen**: ELF generation, optimization pipeline
- **Runtime**: Memory management, concurrency primitives, FFI safety

## Requirements

- LLVM 18.0+
- CMake 3.20+
- Clang (C23 support required)

## Documentation

See `docs/` directory for comprehensive documentation including language specification, contributor guides, and architecture details.
