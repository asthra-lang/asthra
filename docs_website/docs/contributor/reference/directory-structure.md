# Asthra Compiler Directory Structure

**Version:** 1.0  
**Date:** January 2025  
**Status:** Active Development  

## Overview

The Asthra compiler codebase is organized using a modular architecture that separates concerns and enables parallel development. This document explains the purpose and organization of each directory and the naming conventions used throughout the project.

## Root Directory Structure

```
asthra/
├── src/                    # Compiler source code
├── tests/                  # Comprehensive test suite
├── docs/                   # Documentation and specifications
├── make/                   # Modular build system
├── tools/                  # Development and build tools
├── runtime/                # Runtime system implementation
├── stdlib/                 # Standard library (Asthra code)
├── examples/               # Example programs and tutorials
├── scripts/                # Automation and utility scripts
├── build/                  # Build artifacts and object files
└── Makefile               # Main build entry point
```

## Source Code Organization (`src/`)

The compiler source code is organized by compilation phase and functionality:

```
src/
├── parser/                 # Lexical analysis and parsing
│   ├── lexer_*.c          # Lexer implementation
│   ├── grammar_*.c        # Grammar rule implementations
│   ├── ast_*.c            # AST node management
│   ├── token.c            # Token definitions and utilities
│   └── keyword.c          # Keyword recognition
├── analysis/              # Semantic analysis and type checking
│   ├── semantic_*.c       # Semantic analysis modules
│   ├── type_*.c           # Type system implementation
│   └── symbol_*.c         # Symbol table management
├── codegen/               # Code generation and optimization
│   ├── ffi_*.c            # FFI assembly generation
│   ├── generic_*.c        # Generic instantiation
│   ├── elf_*.c            # ELF binary generation
│   └── optimization_*.c   # Optimization passes
├── ast/                   # AST infrastructure
│   ├── ast_nodes.h        # AST node type definitions
│   ├── ast_utility.c      # AST manipulation utilities
│   └── ast_traversal.c    # AST traversal algorithms
├── compiler/              # Main compiler coordination
│   ├── compiler.c         # Main compilation pipeline
│   └── compiler.h         # Compiler interface definitions
├── performance/           # Performance monitoring and optimization
│   ├── profiling.c        # Performance profiling
│   └── benchmarks.c       # Benchmark utilities
├── platform.c             # Platform abstraction layer
├── cli.c                  # Command-line interface
├── main.c                 # Main entry point
└── version.h              # Version information
```

### Parser Directory (`src/parser/`)

The parser directory contains all lexical analysis and parsing functionality:

#### Lexer Components
- **`lexer_core.c`**: Core lexer state management and token generation
- **`lexer_scan.c`**: Character scanning and token recognition
- **`lexer_util.c`**: Utility functions for character classification
- **`token.c`**: Token type definitions and manipulation functions
- **`keyword.c`**: Keyword recognition and reserved word handling

#### Grammar Components
- **`grammar_toplevel_*.c`**: Top-level declarations (functions, structs, enums)
- **`grammar_statements_*.c`**: Statement parsing (control flow, assignments)
- **`grammar_expressions_*.c`**: Expression parsing with precedence
- **`grammar_patterns.c`**: Pattern matching syntax
- **`grammar_annotations.c`**: Annotation parsing and validation

#### AST Components
- **`ast_node_*.c`**: AST node creation and management
- **`ast_destruction_*.c`**: Memory cleanup for AST nodes
- **`ast_utility.c`**: AST manipulation and traversal utilities
- **`ast_traversal.c`**: Generic AST traversal algorithms

### Analysis Directory (`src/analysis/`)

Contains semantic analysis, type checking, and symbol management:

- **`semantic_*.c`**: Modular semantic analysis for different language constructs
- **`type_*.c`**: Type system implementation and inference
- **`symbol_*.c`**: Symbol table management and scope resolution

### Code Generation Directory (`src/codegen/`)

Handles code generation and optimization:

- **`ffi_*.c`**: FFI assembly generation for C interoperability
- **`generic_*.c`**: Generic type instantiation and monomorphization
- **`elf_*.c`**: ELF binary generation for direct execution
- **`optimization_*.c`**: Various optimization passes

## Test Organization (`tests/`)

The test suite is organized by category and component:

```
tests/
├── basic/                  # Basic functionality tests
├── lexer/                  # Lexer-specific tests
├── parser/                 # Parser-specific tests
├── semantic/               # Semantic analysis tests
├── codegen/                # Code generation tests
├── integration/            # End-to-end integration tests
├── performance/            # Performance and benchmark tests
├── concurrency/            # Concurrency feature tests
├── ffi/                    # FFI interoperability tests
├── patterns/               # Pattern matching tests
├── stdlib/                 # Standard library tests
├── framework/              # Test framework infrastructure
├── runtime/                # Runtime system tests
├── security/               # Security and safety tests
├── optimization/           # Optimization pass tests
├── character/              # Character and string tests
├── core/                   # Core language feature tests
├── types/                  # Type system tests
├── memory/                 # Memory management tests
├── errors/                 # Error handling tests
├── annotations/            # Annotation system tests
├── imports/                # Import system tests
├── variants/               # Variant type tests
└── scripts/                # Test automation scripts
```

### Test Naming Conventions

- **`test_<component>_<feature>.c`**: Individual test files
- **`test_<component>_common.{h,c}`**: Shared test utilities
- **`test_<component>_main.c`**: Test suite orchestrators
- **`Makefile.<component>`**: Component-specific build rules

## Documentation Organization (`docs/`)

Documentation is organized by audience and purpose:

```
docs/
├── architecture/           # Compiler architecture documentation
├── spec/                   # Language specification
├── user-manual/            # User-facing documentation
├── guides/                 # Developer guides and tutorials
├── examples/               # Example code and tutorials
├── design/                 # Design documents and rationale
├── stdlib/                 # Standard library documentation
├── ai/                     # AI collaboration documentation
└── archive/                # Historical and deprecated docs
```

## Build System Organization (`make/`)

The build system uses modular Makefiles:

```
make/
├── tests/                  # Test-specific build rules
├── platform.mk            # Platform detection and configuration
├── compiler.mk            # Compiler flags and configuration
├── paths.mk               # Path definitions and file lists
├── rules.mk               # Common build rules
├── targets.mk             # Build targets and recipes
├── utils.mk                # Utility functions
├── sanitizers.mk           # Memory sanitizer configuration
└── coverage.mk            # Code coverage configuration
```

## Runtime System (`runtime/`)

Runtime system components organized by functionality:

```
runtime/
├── core/                   # Core runtime functionality
├── memory/                 # Memory management systems
├── concurrency/            # Concurrency primitives
├── ffi/                    # FFI bridge implementation
├── safety/                 # Safety validation systems
└── platform/               # Platform-specific implementations
```

## File Naming Conventions

### Source Files
- **`<component>_<feature>.c`**: Implementation files
- **`<component>_<feature>.h`**: Header files
- **`<component>_common.{h,c}`**: Shared utilities
- **`<component>_types.h`**: Type definitions
- **`<component>_internal.h`**: Internal implementation details

### Test Files
- **`test_<component>_<feature>.c`**: Test implementations
- **`test_<component>_common.{h,c}`**: Test utilities
- **`test_<component>_main.c`**: Test orchestrators
- **`Makefile.<component>`**: Build configuration

### Documentation Files
- **`README.md`**: Overview and getting started
- **`<topic>.md`**: Specific topic documentation
- **`<FEATURE>_PLAN.md`**: Implementation plans
- **`<FEATURE>_GUIDE.md`**: User guides

## Module Boundaries

### Clear Separation of Concerns

Each directory has a specific responsibility:

1\. **`parser/`**: Converts source text to AST
2\. **`analysis/`**: Validates and analyzes AST
3\. **`codegen/`**: Generates executable code
4\. **`runtime/`**: Provides runtime services
5\. **`tests/`**: Validates all components

### Interface Definitions

Modules communicate through well-defined interfaces:

- **Header files** define public APIs
- **Internal headers** hide implementation details
- **Common utilities** provide shared functionality
- **Type definitions** ensure consistency

### Dependency Management

Dependencies flow in one direction:

```
parser → analysis → codegen → runtime
   ↓        ↓         ↓
 tests    tests    tests
```

## Build Artifacts (`build/`)

Generated files are organized to mirror source structure:

```
build/
├── src/                    # Compiled object files
│   ├── parser/            # Parser object files
│   ├── analysis/          # Analysis object files
│   └── codegen/           # Codegen object files
├── tests/                  # Test executables
├── tools/                  # Built development tools
├── coverage/               # Coverage reports
└── docs/                   # Generated documentation
```

## Development Workflow

### Adding New Components

1\. **Create source files** in appropriate `src/` subdirectory
2\. **Add header files** with public interface definitions
3\. **Create test files** in corresponding `tests/` subdirectory
4\. **Update build system** in `make/` configuration
5\. **Document component** in `docs/` directory

### Modifying Existing Components

1\. **Update implementation** in source files
2\. **Modify tests** to cover new functionality
3\. **Update documentation** to reflect changes
4\. **Verify build system** handles new dependencies

### File Organization Best Practices

- **Keep files focused**: Each file should have a single, clear purpose
- **Use descriptive names**: File names should clearly indicate their contents
- **Maintain consistency**: Follow established naming patterns
- **Document interfaces**: Header files should be self-documenting
- **Minimize dependencies**: Reduce coupling between modules

## Platform-Specific Organization

### Cross-Platform Code
- **Common implementation** in main source files
- **Platform abstraction** in `platform.{h,c}`
- **Conditional compilation** using feature flags

### Platform-Specific Code
- **Platform detection** in build system
- **Conditional includes** for platform-specific headers
- **Runtime platform selection** for optimal performance

## Future Organization

As the project grows, the directory structure may evolve:

### Potential Additions
- **`plugins/`**: Compiler plugin system
- **`backends/`**: Multiple code generation targets
- **`optimization/`**: Advanced optimization passes
- **`analysis/`**: Static analysis tools

### Scalability Considerations
- **Subdirectory limits**: Keep directories manageable (&lt; 20 files)
- **Module boundaries**: Maintain clear separation of concerns
- **Build performance**: Optimize for parallel compilation
- **Documentation**: Keep documentation synchronized with code

This directory structure supports the Asthra compiler's goals of modularity, maintainability, and AI-friendly development while providing clear organization for human contributors. 
