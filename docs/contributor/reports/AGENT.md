# AGENT.md - Asthra Language Development Guide

## Quick Commands
- `make` - Build compiler with C17 optimizations (default)  
- `make test` - Run all tests  
- `make test-category CATEGORY=<name>` - Run specific test category (ffi, codegen, parser, semantic, concurrency)
- `make format` - Format code with clang-format
- `make analyze` - Run static analysis
- `make clean` - Clean build artifacts
- Single test: Use category system - no individual test runner

## Architecture
- **`src/parser/`** - Lexer/parser with AST generation following PEG grammar
- **`src/analysis/`** - Semantic analysis, type inference, symbol tables  
- **`src/codegen/`** - ELF generation, FFI assembly, optimization pipeline
- **`runtime/`** - Memory management (GC), concurrency primitives, FFI safety
- **`ampu/`** - Rust-based build tool (secondary system)

## Critical Rules  
- **`grammar.txt`** is the ONLY source of truth for language syntax - never implement features not in the PEG grammar
- **NO traits, interfaces, advanced generics, closures, macros, or tuple types** - these don't exist in Asthra
- Always update **`docs/spec.md`** Master Implementation Status when completing features
- Use C17 features with fallbacks: `_Static_assert`, `_Generic`, `atomic_*`, `_Thread_local`
- Follow existing patterns: AsthraResult for errors, 64-byte alignment for performance structs

## Code Style
- Use designated initializers: `(AsthraResult){.tag = ASTHRA_RESULT_OK, .data = value}`
- Error handling via AsthraResult types consistently  
- Memory: atomic operations for ref counting, proper cleanup in error paths
- Testing: Use established test framework in `tests/framework/test_framework.h`
