# Asthra Compiler Architecture Documentation

## Overview

The Asthra compiler is a production-ready implementation featuring a modular architecture with clear separation of concerns across all compilation phases. The compiler transforms Asthra source code into native executables through a well-defined pipeline: lexical analysis → parsing → semantic analysis → code generation → linking.

## High-Level Architecture

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│  Source Files   │────▶│     Lexer       │────▶│     Parser      │
│   (.asthra)     │     │  (Tokenizer)    │     │ (AST Builder)   │
└─────────────────┘     └─────────────────┘     └─────────────────┘
                                                          │
                                                          ▼
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│   Executable    │◀────│ Code Generator  │◀────│Semantic Analyzer│
│  (ELF Binary)   │     │   (LLVM IR)     │     │ (Type Checker)  │
└─────────────────┘     └─────────────────┘     └─────────────────┘
                                │
                                ▼
                        ┌─────────────────┐
                        │ Runtime System  │
                        │  (libasthra)    │
                        └─────────────────┘
```

## Component Structure

### 1. Lexer (`src/parser/lexer.*`)

The lexer converts source text into a stream of tokens, handling:
- **Keyword recognition** with perfect hash optimization (~12.86 ns lookup)
- **Multi-line string literals** including raw strings (`r"""..."""`)
- **Numeric literals** (decimal, hex, binary, octal)
- **Comment handling** (single-line `//` and multi-line `/* */`)
- **Error recovery** with position tracking

Key files:
- `lexer_core.c` - Main lexer implementation
- `lexer_scan_*.c` - Specialized scanners for different token types
- `keyword.c` - Optimized keyword recognition

### 2. Parser (`src/parser/`)

The parser implements a recursive descent parser following the PEG grammar strictly:
- **Grammar-driven parsing** - All productions follow `grammar.txt` v1.25
- **AST construction** with reference counting for memory efficiency
- **Error recovery** with synchronized parsing contexts
- **Modular grammar** split by language constructs

Key components:
- `parser_core.c` - Main parser driver and state management
- `grammar_toplevel.c` - Program structure parsing
- `grammar_expressions*.c` - Expression parsing with precedence
- `grammar_statements*.c` - Statement parsing
- `ast_node_creation.c` - AST node factory with atomic refcounting

### 3. Semantic Analysis (`src/analysis/`)

The semantic analyzer performs comprehensive program validation:
- **Type system** with inference and checking
- **Symbol resolution** with hierarchical scoping
- **Memory safety analysis** including ownership tracking
- **FFI validation** with annotation processing
- **Const evaluation** for compile-time constants

Key modules:
- `semantic_core.c` - Analyzer lifecycle and coordination
- `semantic_types*.c` - Type system implementation
- `semantic_symbols*.c` - Symbol table management
- `semantic_expressions.c` - Expression analysis and type checking
- `semantic_ffi.c` - Foreign function interface validation
- `const_evaluator.c` - Compile-time constant evaluation

### 4. Code Generation (`src/codegen/`)

The code generator exclusively uses LLVM IR backend:
- **LLVM IR generation** for all target platforms
- **LLVM optimization passes** instead of custom optimizers
- **LLVM's native code generation** for x86_64, ARM64, and other targets
- **FFI marshaling** through LLVM intrinsics
- **Generic instantiation** for type parameters

Key components:
- `backend_interface.c` - LLVM backend integration
- `llvm_backend.c` - LLVM IR generation
- `generic_instantiation*.c` - Monomorphization
- FFI support through LLVM's foreign function capabilities

### 5. Runtime System (`runtime/`)

The runtime provides essential services:
- **Memory management** - GC with reference counting
- **Concurrency** - Tasks and threads with synchronization
- **String/slice operations** - Safe manipulation
- **FFI safety** - Ownership tracking and validation
- **Error handling** - Result types and propagation

Key components:
- `asthra_runtime_modular.h` - Unified runtime interface
- `memory/` - Memory management subsystem
- `concurrency/` - Threading and task system
- `safety/` - Runtime safety checks
- `ffi/` - FFI runtime support

## Key Design Patterns

### 1. Modular Architecture

Each major component is split into focused modules:
```c
// Example: Semantic analysis modular structure
semantic_analyzer.h          // Main header includes all modules
├── semantic_core.*         // Lifecycle and coordination
├── semantic_types.*        // Type system
├── semantic_symbols.*      // Symbol tables
├── semantic_expressions.*  // Expression analysis
└── semantic_statements.*   // Statement analysis
```

### 2. C17 Modernization

The codebase uses modern C17 features with fallbacks:
```c
// Atomic reference counting
#if ASTHRA_HAS_ATOMICS
    atomic_fetch_add(&node->ref_count, 1);
#else
    __sync_fetch_and_add(&node->ref_count, 1);
#endif

// Static assertions
_Static_assert(sizeof(size_t) >= 4, "size_t must be at least 32-bit");

// Designated initializers
ParserConfig config = {
    .strict_mode = true,
    .max_errors = 100,
    .enable_recovery = true
};
```

### 3. Error Handling Pattern

Consistent error handling using Result types:
```c
typedef struct {
    bool success;
    union {
        void *value;
        AsthraError error;
    };
} AsthraResult;

// Usage
AsthraResult result = parse_expression(parser);
if (!result.success) {
    handle_error(result.error);
    return NULL;
}
```

### 4. Memory Management

Reference counting with atomic operations:
```c
ASTNode *ast_retain_node(ASTNode *node) {
    if (!node) return NULL;
    atomic_fetch_add(&node->ref_count, 1);
    return node;
}

void ast_release_node(ASTNode *node) {
    if (!node) return;
    if (atomic_fetch_sub(&node->ref_count, 1) == 1) {
        ast_free_node(node);
    }
}
```

## Build System

The build system uses a modular Makefile architecture:

### Makefile Structure
```
Makefile                    // Main entry point
make/
├── platform.mk            // Platform detection
├── compiler.mk            // Compiler configuration
├── paths.mk              // Directory structure
├── rules.mk              // Compilation rules
├── targets.mk            // Build targets
├── tests.mk              // Test targets
└── sanitizers.mk         // Memory sanitizers
```

### Key Features
- **Cross-platform support** - macOS, Linux, Windows
- **Category-based testing** - 31 test categories
- **Sanitizer integration** - ASan, TSan, UBSan
- **Coverage analysis** - gcov/lcov support
- **Parallel builds** - Automatic job detection

## Testing Infrastructure

### Test Framework (`tests/framework/`)
- **C17 modernized** with atomic statistics
- **Category-based** organization
- **Comprehensive coverage** - 130+ tests
- **Performance benchmarking** built-in

### Test Categories
1. **Parser tests** - Grammar compliance
2. **Semantic tests** - Type checking
3. **Codegen tests** - Assembly generation
4. **FFI tests** - C interoperability
5. **Concurrency tests** - Thread safety
6. **Memory tests** - Leak detection
7. **Integration tests** - End-to-end

## Performance Characteristics

### Compilation Speed
- **15-25% faster** than baseline with C17 optimizations
- **Keyword lookup** - ~12.86 nanoseconds average
- **Memory efficiency** - 10-20% reduction in usage

### Runtime Performance
- **Reference counting** - 50-80% faster with atomics
- **Concurrency** - Lock-free data structures
- **FFI calls** - Minimal overhead marshaling

## Extension Points

### Adding Language Features
1. Update `grammar.txt` with new syntax
2. Add AST node types in `ast_types.h`
3. Implement parser production in `grammar_*.c`
4. Add semantic analysis in `semantic_*.c`
5. Implement code generation in `code_generator_*.c`
6. Add tests in appropriate category

### Adding Optimization Passes
1. Define pass in `optimizer_passes.h`
2. Implement in `optimizer_passes.c`
3. Register in optimization pipeline
4. Add performance benchmarks

### Supporting New Platforms
1. Update `make/platform.mk` for detection
2. Add architecture in `code_generator_types.h`
3. Implement in platform-specific codegen
4. Update ELF writer for new format
5. Add platform tests

## Development Workflow

### Building the Compiler
```bash
make clean        # Clean build artifacts
make              # Build with optimizations
make debug        # Build with debug symbols
make test         # Run all tests
make coverage     # Generate coverage report
```

### Running Tests
```bash
# Run specific category
make test-category CATEGORY=parser

# Run with sanitizers
make asan-test    # Address sanitizer
make tsan-test    # Thread sanitizer

# Generate reports
make test-report
```

### Using Ampu Build Tool
```bash
ampu init my-project      # Create new project
ampu build               # Build project
ampu test               # Run tests
ampu run                # Build and execute
```

## Debugging Guide

### Parser Debugging
- Set `PARSER_DEBUG=1` environment variable
- Use `parser_set_config()` for strict mode
- Check AST with `ast_print_tree()`

### Semantic Analysis Debugging
- Enable `SEMANTIC_DEBUG` in compilation
- Use symbol table dumps
- Check type resolution traces

### Code Generation Debugging
- Generate assembly comments with config
- Use `objdump` on generated ELF
- Enable instruction traces

## Summary

The Asthra compiler represents a modern, production-ready implementation with:
- **Strict grammar compliance** ensuring predictable parsing
- **Comprehensive type safety** with inference
- **Efficient code generation** targeting native platforms
- **Robust runtime system** with memory safety
- **Extensive testing** validating all components

The modular architecture enables easy maintenance and extension while C17 modernization provides performance benefits. The compiler successfully achieves its design goals of being both AI-friendly for code generation and efficient for production use.