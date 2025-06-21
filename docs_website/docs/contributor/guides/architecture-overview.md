# Architecture Overview

**High-level guide to understanding the Asthra compiler architecture**

This guide provides a comprehensive overview of the Asthra compiler's architecture, design decisions, and system organization for contributors.

## Overview

- **Purpose**: Understand the overall system architecture and design philosophy
- **Audience**: All contributors, especially those new to the project
- **Prerequisites**: Basic understanding of compilers and software architecture
- **Outcome**: Clear mental model of how Asthra components work together

## System Architecture

### High-Level View

The Asthra compiler is designed as a modular, AI-friendly system that transforms Asthra source code into efficient executables:

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Source Code   │───▶│  Asthra Compiler │───▶│   Executable    │
│   (.asthra)     │    │                 │    │    Binary       │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                              │
                              ▼
                    ┌─────────────────┐
                    │   Generated     │
                    │   C Code        │
                    └─────────────────┘
```

### Core Design Principles

#### 1. AI Generation Efficiency
- **Minimal Syntax**: Reduced cognitive load for AI models
- **Deterministic Parsing**: No ambiguous language constructs
- **Consistent Patterns**: Predictable code generation patterns
- **Clear Error Messages**: Actionable feedback for AI debugging

#### 2. Memory Safety
- **Four-Zone Memory Model**: GC, Manual, Pinned, External zones
- **Ownership Tracking**: Clear ownership semantics
- **Bounds Checking**: Automatic array bounds validation
- **Safe FFI**: Memory-safe C interoperability

#### 3. Modular Architecture
- **Component Isolation**: Clear boundaries between compiler phases
- **Parallel Development**: Independent component development
- **Testability**: Isolated testing of individual components
- **Maintainability**: Focused responsibilities per module

#### 4. Performance
- **Zero-Cost Abstractions**: No runtime overhead for language features
- **Monomorphization**: Specialized code for generic types
- **C Backend**: Leverage mature C compiler optimizations
- **Incremental Compilation**: Fast rebuild cycles

## Compilation Pipeline

### Phase Overview

```mermaid
graph TD
    A&#91;Source Code&#93; --&gt; B&#91;Lexer&#93;
    B --&gt; C&#91;Parser&#93;
    C --&gt; D&#91;Semantic Analysis&#93;
    D --&gt; E&#91;Code Generation&#93;
    E --&gt; F&#91;C Compiler&#93;
    F --&gt; G&#91;Linker&#93;
    G --&gt; H&#91;Executable&#93;
    
    I&#91;Runtime System&#93; --&gt; E
    J&#91;Standard Library&#93; --&gt; E
    K&#91;Platform Layer&#93; --&gt; E
```

### Phase 1: Lexical Analysis

**Purpose**: Convert source text into tokens  
**Location**: `src/parser/lexer_*.c`  
**Key Features**:
- Modular lexer design with specialized scanners
- Support for all literal types (hex, binary, octal, strings)
- Comprehensive error recovery and reporting
- Memory-safe token management

```c
// Example: Lexer processing
Source: "fn main() -&gt; i32 { return 42; }"
Tokens: &#91;FN, IDENTIFIER("main"), LEFT_PAREN, RIGHT_PAREN, 
         ARROW, IDENTIFIER("i32"), LEFT_BRACE, RETURN, 
         NUMBER("42"), SEMICOLON, RIGHT_BRACE, EOF&#93;
```

### Phase 2: Parsing

**Purpose**: Build Abstract Syntax Tree from tokens  
**Location**: `src/parser/grammar_*.c`  
**Key Features**:
- PEG (Parsing Expression Grammar) implementation
- Modular grammar rules split across focused files
- Comprehensive AST node types
- Advanced error recovery with helpful messages

```c
// Example: AST structure for function
ASTNode {
    type: AST_FUNCTION,
    data: {
        function: {
            name: "main",
            parameters: &#91;&#93;,
            return_type: AST_BASE_TYPE("i32"),
            body: AST_BLOCK(&#91;
                AST_RETURN(AST_LITERAL("42"))
            &#93;)
        }
    }
}
```

### Phase 3: Semantic Analysis

**Purpose**: Type checking, symbol resolution, and validation  
**Location**: `src/analysis/semantic_*.c`  
**Key Features**:
- Sophisticated type inference system
- Multi-scope symbol table management
- Annotation validation and conflict detection
- Generic type instantiation

```c
// Example: Symbol table after analysis
SymbolTable {
    symbols: &#91;
        Symbol {
            name: "main",
            type: SYMBOL_FUNCTION,
            type_info: FunctionType {
                parameters: [&#93;,
                return_type: PrimitiveType(I32)
            }
        }
    ]
}
```

### Phase 4: Code Generation

**Purpose**: Generate C code and FFI assembly  
**Location**: `src/codegen/`  
**Key Features**:
- Safe C interoperability through FFI assembly
- Generic instantiation with zero-cost monomorphization
- ELF binary generation for direct execution
- Comprehensive optimization pipeline

```c
// Example: Generated C code
int main(void) {
    return 42;
}
```

## Directory Structure

### Source Code Organization

```
src/
├── parser/              # Lexical analysis and parsing
│   ├── lexer_*.c       # Tokenization components
│   └── grammar_*.c     # Grammar rule implementations
├── analysis/           # Semantic analysis
│   ├── semantic_*.c    # Type checking and validation
│   └── symbol_*.c      # Symbol table management
├── codegen/            # Code generation
│   ├── codegen_*.c     # C code generation
│   └── optimization/   # Optimization passes
├── runtime/            # Runtime system
│   ├── memory/         # Memory management
│   ├── concurrency/    # Threading and synchronization
│   └── stdlib/         # Standard library implementations
├── utils/              # Utility functions
│   ├── json_utils.c    # JSON parsing and generation
│   └── string_utils.c  # String manipulation
└── tools/              # Development tools
    ├── linter.c        # Code linting
    └── formatter.c     # Code formatting
```

### Test Organization

```
tests/
├── lexer/              # Lexer tests
├── parser/             # Parser tests
├── semantic/           # Semantic analysis tests
├── codegen/            # Code generation tests
├── integration/        # End-to-end tests
├── performance/        # Performance benchmarks
└── framework/          # Testing framework
```

### Documentation Structure

```
docs/
├── contributor/        # Contributor documentation
│   ├── guides/        # Development guides
│   ├── workflows/     # Development workflows
│   └── reference/     # Reference materials
├── user-manual/       # User documentation
├── spec/              # Language specification
└── architecture/      # Architecture documentation
```

## Key Components

### Memory Management System

#### Four-Zone Memory Model

```c
typedef enum {
    MEMORY_ZONE_GC,        // Garbage collected
    MEMORY_ZONE_MANUAL,    // Manual management
    MEMORY_ZONE_PINNED,    // Pinned memory
    MEMORY_ZONE_EXTERNAL   // External library memory
} MemoryZone;
```

**GC Zone**: Automatic memory management for most objects  
**Manual Zone**: Explicit control for performance-critical code  
**Pinned Zone**: Memory that won't move (for FFI)  
**External Zone**: Memory managed by external libraries

#### Ownership Tracking

```c
typedef enum {
    OWNERSHIP_OWNED,       // Full ownership
    OWNERSHIP_BORROWED,    // Temporary access
    OWNERSHIP_SHARED,      // Shared ownership
    OWNERSHIP_EXTERNAL     // External ownership
} OwnershipType;
```

### Type System

#### Type Hierarchy

```c
typedef enum {
    TYPE_PRIMITIVE,        // i32, f64, bool, char
    TYPE_POINTER,          // *T, *mut T
    TYPE_ARRAY,            // &#91;T; N&#93;
    TYPE_SLICE,            // &#91;T&#93;
    TYPE_STRUCT,           // struct { ... }
    TYPE_ENUM,             // enum { ... }
    TYPE_FUNCTION,         // fn(T, U) -&gt; V
    TYPE_GENERIC,          // T (generic parameter)
    TYPE_INFERRED          // Type to be inferred
} TypeKind;
```

#### Generic System

```c
// Generic struct definition
struct Vec<T> {
    data: *mut T,
    length: usize,
    capacity: usize,
}

// Monomorphization generates:
// Vec_i32, Vec_f64, Vec_String, etc.
```

### Concurrency Model

#### Three-Tier System

**Tier 1**: `spawn`/`await` for deterministic concurrency
```asthra
let task = spawn {
    compute_heavy_operation()
};
let result = await task;
```

**Tier 2**: Channels and `select` for communication
```asthra
let (sender, receiver) = channel::<i32>();
spawn {
    sender.send(42);
};
let value = receiver.recv();
```

**Tier 3**: External library integration
```asthra
#&#91;external_lib("pthread")&#93;
fn pthread_create(thread: *mut pthread_t, attr: *const pthread_attr_t, 
                 start_routine: fn(*mut void) -&gt; *mut void, 
                 arg: *mut void) -&gt; i32;
```

### FFI System

#### Automatic C Binding Generation

```asthra
// Asthra declaration
#&#91;export_c&#93;
fn calculate(a: i32, b: i32) -&gt; i32 {
    return a + b;
}

// Generated C header
int calculate(int a, int b);

// Generated C implementation
int calculate(int a, int b) {
    return asthra_calculate(a, b);
}
```

#### Transfer Semantics

```asthra
#&#91;transfer_full&#93;    // Caller takes ownership
#&#91;transfer_none&#93;    // Callee retains ownership
#&#91;transfer_container&#93; // Transfer container, not contents
```

## Build System Architecture

### Modular Makefile System

```
Makefile                 # Main entry point
make/
├── paths.mk            # Path definitions
├── compiler.mk         # Compiler settings
├── tests.mk            # Test orchestration
├── utils.mk            # Utility functions
├── third-party.mk      # External dependencies
└── tests/              # Test-specific makefiles
    ├── lexer.mk
    ├── parser.mk
    └── ...
```

### Build Variants

```bash
# Debug build (default)
make

# Release build
make release

# Profile build
make profile

# Coverage build
make coverage

# AddressSanitizer build
make asan

# ThreadSanitizer build
make tsan
```

### Parallel Compilation

```bash
# Parallel compilation
make -j8

# Parallel testing
make test-all -j4

# Component-specific builds
make lexer parser semantic codegen
```

## Platform Support

### Target Architectures

**x86_64**: Primary development and testing platform  
**ARM64**: Full support for Apple Silicon and ARM servers  
**WASM32**: WebAssembly target for browser deployment

### Operating Systems

**macOS**: Primary development platform with full toolchain  
**Linux**: Complete support with GCC/Clang integration  
**Windows**: Cross-compilation support via MinGW

### Toolchain Integration

**Clang**: Preferred compiler with enhanced diagnostics  
**GCC**: Full compatibility with fallback support  
**Make**: Modular build system with parallel compilation  
**Static Analysis**: Integrated linting and safety checks

## Performance Characteristics

### Compilation Speed

- **Modular compilation**: Only recompile changed modules
- **Parallel parsing**: Independent grammar rules
- **Incremental analysis**: Cache semantic analysis results
- **Fast lexing**: Optimized character scanning

### Memory Usage

- **Streaming lexer**: Minimal token buffering
- **AST sharing**: Reference counting for common nodes
- **Symbol interning**: Deduplicated identifier storage
- **Garbage collection**: Automatic memory management during compilation

### Generated Code Quality

- **Zero-cost abstractions**: No runtime overhead for language features
- **Monomorphization**: Specialized code for generic types
- **C compiler optimization**: Leverage LLVM/GCC optimizations
- **FFI efficiency**: Direct C function calls without marshaling overhead

## Development Workflow Integration

### AI-Assisted Development

The architecture is specifically designed to support AI-assisted development:

1\. **Predictable Patterns**: Consistent code organization for AI understanding
2\. **Modular Components**: Clear boundaries for AI-generated code
3\. **Comprehensive Testing**: Validation of AI-generated contributions
4\. **Clear Documentation**: Context for AI code generation

### Continuous Integration

```yaml
# Example CI pipeline
stages:
  - build
  - test
  - analyze
  - deploy

build:
  script:
    - make clean
    - make -j8
    - make test-all

analyze:
  script:
    - make static-analysis
    - make coverage-report
    - make performance-benchmark
```

## Extensibility Points

### Adding New Language Features

1\. **Lexer Extension**: Add new token types
2\. **Grammar Extension**: Add new parsing rules
3\. **Semantic Extension**: Add type checking rules
4\. **Codegen Extension**: Add code generation patterns

### Plugin Architecture

```c
// Plugin interface
typedef struct {
    const char *name;
    const char *version;
    bool (*initialize)(void);
    void (*cleanup)(void);
    void (*process_ast)(ASTNode *node);
} CompilerPlugin;
```

### Custom Optimization Passes

```c
// Optimization pass interface
typedef struct {
    const char *name;
    bool (*should_run)(OptimizationContext *ctx);
    bool (*run_pass)(OptimizationContext *ctx, ASTNode *ast);
    void (*report_statistics)(OptimizationContext *ctx);
} OptimizationPass;
```

## Success Criteria

You understand the Asthra architecture when:

- &#91; &#93; **System Overview**: Can explain the compilation pipeline and major components
- &#91; &#93; **Design Principles**: Understand the AI-friendly design decisions
- &#91; &#93; **Component Interactions**: Know how different parts work together
- &#91; &#93; **Memory Model**: Understand the four-zone memory management system
- &#91; &#93; **Build System**: Can navigate and use the modular build system
- &#91; &#93; **Extensibility**: Know how to add new features and components

## Related Guides

- **&#91;Compiler Development&#93;(compiler-development.md)** - Deep dive into compiler implementation
- **&#91;Development Environment&#93;(development-environment.md)** - Setting up your development environment
- **&#91;Testing Guide&#93;(testing-guide.md)** - Understanding the testing architecture
- **&#91;Code Style Guide&#93;(code-style.md)** - Coding standards and conventions

## Reference Materials

- **&#91;Detailed Architecture Docs&#93;(../reference/architecture/)** - In-depth component documentation
- **&#91;API Reference&#93;(../reference/api/)** - Complete API documentation
- **&#91;Language Specification&#93;(../reference/language-spec.md)** - Asthra language definition
- **&#91;Build System Reference&#93;(../reference/build-system.md)** - Complete build system documentation

---

**🏗️ Architecture Understanding Complete!**

*Understanding the overall architecture provides the foundation for effective contribution to any part of the Asthra compiler system. The modular design enables focused development while maintaining system coherence.* 