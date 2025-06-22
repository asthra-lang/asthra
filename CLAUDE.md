# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**Asthra** is a modern, safe, and performant systems programming language with C17-based compiler infrastructure. It features automatic memory management, strong type safety, built-in concurrency support, and seamless C FFI.

## Essential Commands

### Build Commands
```bash
cmake -B build && cmake --build build          # Build compiler with optimizations (default)
cmake --build build --target clean             # Clean all build artifacts
cmake -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build    # Build with debug symbols
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build  # Build optimized release
```

### Testing Commands
```bash
ctest --test-dir build                          # Run all tests
ctest --test-dir build --verbose               # Comprehensive test suite with reporting

# IMPORTANT: Build test executables before running tests
cmake --build build --target build-tests       # Build all test executables (required after clean)

# Component-specific testing (component must exist under tests/)
ctest --test-dir build -L <component-name>     # Example: ctest -L semantic, ctest -L parser

# Sub-category testing (available for some components)
ctest --test-dir build -R "codegen.*function"  # Run only function call tests
ctest --test-dir build -R "codegen.*control"   # Run only control flow tests
ctest --test-dir build -R "parser.*expression" # Run only expression parser tests
ctest --test-dir build -R "parser.*statement"  # Run only statement parser tests

# Fast variants (parallel execution for quicker iteration)
ctest --test-dir build -j8                     # Run tests in parallel
ctest --test-dir build -L <component> -j8      # Component tests in parallel

# Test reporting
ctest --test-dir build --output-on-failure     # View test failures with output
```

**⚠️ Test Build Strategy**: See `docs/contributor/guides/test-build-strategy.md` for critical information about test building and common build failures.

### Coverage and Analysis
```bash
cmake --build build --target coverage          # Generate coverage reports
cmake --build build --target coverage-html     # HTML coverage reports
cmake --build build --target analyze           # Run static analysis
cmake --build build --target format            # Format code with clang-format

# Performance testing
cmake --build build --target benchmark         # Run optimized benchmarks
```

### Build Tool Commands (Ampu - Rust-based)
```bash
cd ampu && cargo build              # Build the build tool
./ampu/run_e2e_tests.sh            # End-to-end tests
cargo test --test language_tests   # Language feature tests
```

## Architecture Overview

### Source Structure (`src/`)
- **`parser/`** - Modular lexer/parser with AST generation following PEG grammar
- **`analysis/`** - Semantic analysis, type inference, symbol tables, memory safety
- **`codegen/`** - ELF generation, FFI assembly, optimization pipeline, register allocation
- **`performance/`** - Benchmarking infrastructure with nanosecond precision

### Runtime System (`runtime/`)
- **Modular architecture** with specialized components for memory, concurrency, safety, FFI
- **Memory management** with garbage collection and atomic reference counting
- **Concurrency primitives** with lock-free data structures and thread safety
- **FFI safety** with ownership tracking and bounds checking

### Test Structure (`tests/`)
- **Comprehensive test suite** covering FFI, codegen, parser, semantic analysis, and more
- **Detailed reporting** with CSV export and logs
- **Cross-platform validation** with sanitizer support

## Critical Development Rules

### Asthra Design Principles Compliance (MANDATORY)
- **ALL changes must strictly follow Asthra's 5 Core Design Principles** as defined in `docs/spec/overview.md`:
  1. **Minimal syntax for maximum AI generation efficiency** - Predictable patterns, unambiguous grammar, local reasoning
  2. **Safe C interoperability through explicit annotations** - FFI safety, ownership transfer annotations, unsafe blocks
  3. **Deterministic execution for reproducible builds** - Consistent behavior, reproducible errors, reliable outcomes
  4. **Built-in observability and debugging primitives** - Enhanced diagnostics, precise error reporting, AI-friendly messages
  5. **Pragmatic system evolution** - Backward compatibility, incremental enhancement, essential features first
- **Every implementation must demonstrate alignment** with all 5 principles
- **Reject changes that violate** AI-friendly patterns or introduce complex global analysis requirements

### Grammar Compliance (MANDATORY)
- **`grammar.txt`** is the single source of truth for Asthra language syntax
- **Never** implement language features not defined in the PEG grammar
- **Forbidden features**: traits, interfaces, advanced generics, closures, macros, tuple types
- All parser/lexer changes must strictly conform to the grammar

### Documentation Authority
- **`docs/spec/`** contains the Master Implementation Status table - the single source of truth for feature status
- **Status symbols**: ✅ Fully Implemented, 🔧 Partially Implemented, ❌ Not Yet Implemented, 📋 Planned
- Always update implementation status when making changes

### Documentation Organization (MANDATORY)
- **NEVER place files directly under `docs/`** - All documentation must be organized into appropriate subfolders
- **Use the established 5-folder structure**:
  - `docs/architecture/` - Technical architecture, design diagrams, and system analysis
  - `docs/contributor/` - Development guides, references, workflows, and contribution materials
  - `docs/spec/` - Language specification, grammar, and formal technical specifications
  - `docs/stdlib/` - Standard library documentation and API references
  - `docs/user-manual/` - User-facing guides, tutorials, and language usage documentation
- **Exception**: Only `docs/index.md` (main documentation landing page) is allowed at the root level
- **When adding new documentation**, choose the most appropriate subfolder based on the audience and content type
- **Maintain organized substructure** within each folder (e.g., `guides/`, `reference/`, `api/`, etc.)

### Compiler Architecture Separation of Concerns (MANDATORY)
- **STRICT phase separation** - Parser, Semantic Analysis, and Code Generation must be completely independent
- **Parser responsibilities**: ONLY tokenization, syntax analysis, and AST construction - NO semantic analysis or code generation
- **Semantic Analysis responsibilities**: ONLY type checking, symbol resolution, and semantic validation - NO code generation
- **Code Generation responsibilities**: ONLY takes semantically-analyzed AST and produces executable code
- **FORBIDDEN**: Parser triggering semantic analysis, semantic analysis triggering code generation, shared global state between phases
- **Required**: Each phase must have clean input/output interfaces with no side effects on other phases
- **Testing**: Each phase must be testable in isolation without dependencies on other phases

### Expression-Oriented Language Design (MANDATORY)
- **Asthra is expression-oriented** - Most constructs return values, not just perform actions
- **If-else as expressions** - Control flow constructs return values and have types
- **Explicit returns required** - Void functions must use `return ();` explicitly
- **Unit type support** - `()` represents void/no-value contexts
- **Type inference implications** - Every expression must have a well-defined type
- **Test patterns** - Tests must respect expression semantics, avoid statement-oriented patterns
- **Code generation** - Must handle expressions appearing in statement contexts
- **Documentation**: See `docs/architecture/expression-oriented-design.md` for comprehensive details

### Source File Gitignore Compliance (MANDATORY)
- **CRITICAL: Never create source files (*.c, *.h, tests, docs) that match .gitignore patterns**
- **Why this matters**: Files matching .gitignore patterns will NOT be committed to Git and WILL BE LOST when running `git clean -fdx`
- **Common problematic patterns for source files**:
  - Debug source files: `debug_*.c`, `debug_*.h`, `*_debug.c`, `*_debug.h` - These will be ignored!
  - Test executables: `*_test` (without .c/.h extension) - Never name source files this way!
  - Analysis docs: `*_error_analysis.md`, `*_migration_summary.md` - These are temporary and will be lost!
  - Local notes: `TODO.local.md`, `local-notes.txt` - These are explicitly ignored!
- **Before creating ANY source file or documentation**, verify the filename won't be ignored by Git
- **Safe patterns**: 
  - Test source files: `test_*.c`, `test_*.h` (with test_ prefix)
  - Test files with _test_ in name: `*_test_*.c`, `*_test_*.h` (explicitly allowed in .gitignore)
- **If you create a file that matches .gitignore patterns**, it will be lost and cause confusion

### Temporary Debug Files (ENCOURAGED)
- **DO create temporary debug files using .gitignore patterns** for debugging and analysis
- **Purpose**: These files are meant to be temporary and should not clutter the repository
- **Recommended patterns for temporary files**:
  - Debug files: Use `debug_*.c`, `debug_*.h`, `*_debug.c`, `*_debug.h` for temporary debugging code
  - Analysis docs: Use `*_error_analysis.md`, `*_migration_summary.md` for temporary analysis
  - Test outputs: Use `*_test_output.txt`, `*_test_output.log` for test results
  - Local notes: Use `TODO.local.md`, `local-notes.txt` for personal notes
- **Benefits**: These files won't be accidentally committed and can be cleaned up with `git clean -fdx`
- **Use case**: When debugging or analyzing issues, use these patterns to ensure your temporary work doesn't pollute the repository

### C17 Standards
- **Must use** modern C17 features appropriately: `_Static_assert`, `_Generic`, `<stdatomic.h>`, `_Thread_local`
- **Always provide** fallback implementations for older compilers
- Use feature detection macros for C17 features

### Build System
- **Modular Makefile system** in `make/` directory with platform detection and compiler optimization
- **Platform support**: macOS (Apple Silicon/Intel), Linux (x86_64/ARM64), Windows (MSVC)
- **Sanitizer support**: AddressSanitizer, ThreadSanitizer, UndefinedBehaviorSanitizer

## Development Workflow

### Before Making Changes
1. **Verify Asthra Design Principles alignment** - Ensure proposed changes support all 5 core principles
2. **Check implementation status** in `docs/spec` Master Implementation Status table
3. **Understand component role** by reading related files and documentation
4. **Verify grammar compliance** if making parser/lexer changes

### Required Testing
1. **Build test executables first**: `cmake --build build --target build-tests` (especially after clean)
2. **Run full test suite**: `ctest --test-dir build` must pass
3. **Component-specific tests**: Test relevant components with `ctest --test-dir build -L <component-name>` (e.g., `ctest -L semantic`)
   - Use parallel execution for quicker iteration: `ctest --test-dir build -L semantic -j8`
   - Some components have sub-categories: `ctest --test-dir build -R "codegen.*function"`
4. **Performance validation**: Run benchmarks if changes affect performance
5. **Memory safety**: Use sanitizers (build with `-DSANITIZER=Address` or `-DSANITIZER=Thread`) for memory-related changes

**⚠️ Important**: Never use full clean when fixing tests. See `docs/contributor/guides/test-build-strategy.md` for the correct approach.

### Code Quality Standards
- **Follow established patterns** from existing codebase (slice patterns, atomic operations, error handling)
- **Use AsthraResult types** for consistent error handling
- **Implement proper cleanup** in error paths
- **64-byte align** performance-critical data structures

### Test File Naming Convention
- **All test files must use the `test_` prefix** - test files should follow the pattern `test_<name>.c`
- **No trailing `_test` suffix needed** - use `test_semantic.c` instead of `semantic_test.c`

## Performance Targets
- **Compilation speed**: 15-25% faster than baseline with C17 optimizations
- **Memory efficiency**: 10-20% reduction in memory usage
- **Reference counting**: 50-80% faster with atomic operations
- **Keyword lookup**: ~12.86 nanoseconds average time

## Common Patterns

### Error Handling
```c
// Use AsthraResult types consistently
AsthraResult result = some_operation();
if (result.success) {
    // Handle success
} else {
    // Handle error with proper cleanup
}
```

### Memory Management
```c
// Use atomic operations for reference counting
atomic_fetch_add(&ref_count, 1);

// Cache-aligned structures for performance
struct alignas(64) PerformanceCriticalData {
    // ...
};
```

### Testing
```c
// Use established test framework
#include "test_framework.h"

TEST_CASE("descriptive_test_name") {
    // Test implementation
    ASSERT_EQ(expected, actual);
}
```

### Expression-Oriented Test Patterns
```c
// ✅ CORRECT: Expression-oriented patterns
const char* source = 
    "package test;\n"
    "pub fn main(none) -> void {\n"
    "    let result = if condition { 42 } else { 0 };\n"
    "    return ();\n"  // Explicit return required
    "}\n";

// ❌ INCORRECT: Statement-oriented patterns  
const char* bad_source = 
    "if (condition) { action(); } else { other(); }"  // Missing package, returns, types
```

## Important Files

### Core Documentation
- **`docs/spec/`** - Language specification and Master Implementation Status
- **`grammar.txt`** - PEG grammar (authoritative language syntax)
- **`README.md`** - Project overview and quick start guide
- **`docs/architecture/expression-oriented-design.md`** - Expression-oriented design decisions and code generation impacts
- **`docs/contributor/guides/test-build-strategy.md`** - Critical guide for building and running tests without cascade failures

### Build Configuration
- **`Makefile`** - Main build entry point (includes modular system)
- **`make/`** - Modular build system components
- **`ampu/Cargo.toml`** - Rust-based build tool configuration

### Key Headers
- **`runtime/asthra_runtime.h`** - Core runtime API
- **`src/parser/ast_types.h`** - AST node definitions
- **`tests/framework/test_framework.h`** - Testing infrastructure

## Linting and Formatting

Always run these before committing:
```bash
cmake --build build --target format    # clang-format code formatting
cmake --build build --target analyze   # Static analysis with Clang Static Analyzer
```

## Notes for AI Development

- **ALWAYS validate against Asthra's 5 Design Principles** - Every change must enhance AI generation efficiency, safety, determinism, observability, and pragmatic evolution
- **Never guess** about language features - always check `grammar.txt`
- **Always update** implementation status in `docs/spec/` when completing features
- **Test thoroughly** - this is a systems language where correctness is critical
- **Follow C17 standards** - use modern C features appropriately with fallbacks
- **Maintain cross-platform compatibility** - test on multiple platforms when possible
- **Run comprehensive tests** - ensure all tests pass before committing changes
- **Prioritize AI-friendly patterns** - Local reasoning over global analysis, predictable behavior over complex features
- **Document principle alignment** - Explain how changes support each of the 5 core design principles

## Commit Message Standards

### Conventional Commits (MANDATORY)
- **ALL commits must follow Conventional Commits specification** as defined at https://www.conventionalcommits.org/en/v1.0.0/
- **Required format**: `<type>[optional scope]: <description>`
- **Commit types**:
  - `feat:` - New feature implementation
  - `fix:` - Bug fixes
  - `docs:` - Documentation changes
  - `style:` - Code style/formatting changes (no logic changes)
  - `refactor:` - Code refactoring without feature changes
  - `test:` - Adding or modifying tests
  - `chore:` - Build system, dependency updates, tooling
  - `perf:` - Performance improvements
  - `ci:` - CI/CD configuration changes
- **Breaking changes**: Add `!` after type or use `BREAKING CHANGE:` in footer
- **Examples**:
  - `feat(parser): add support for pattern matching syntax`
  - `fix(semantic): resolve type inference for generic constraints`
  - `docs: update CLAUDE.md with conventional commits rule`
  - `test(codegen): add comprehensive function call tests`
  - `refactor(runtime)!: restructure memory management API`

## Development Memories

- At the end of every source file modification (Changes to *.c, *.h, etc.), run 'cmake --build build -j8 && cmake --build build --target build-tests' and fix errors, if any
- **Test Build Best Practice**: Use incremental builds for test work. See `docs/contributor/guides/test-build-strategy.md` for the proven approach to avoid cascade test failures
