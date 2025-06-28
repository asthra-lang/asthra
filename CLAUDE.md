# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**Asthra** is a modern, safe, and performant systems programming language with C23-based compiler infrastructure. It features automatic memory management, strong type safety, built-in concurrency support, and seamless C FFI.

## Essential Commands

### Most Common Commands
```bash
# Build and test workflow
cmake -B build && cmake --build build -j8      # Build compiler (parallel)
cmake --build build --target build-tests       # Build test executables
ctest --test-dir build -L semantic -j8         # Run component tests (fast)
ctest --test-dir build                         # Run all tests

# Code quality
cmake --build build --target format            # Format code
cmake --build build --target analyze           # Static analysis
```

### Quick Testing
```bash
# Component testing: ctest -L <parser|semantic|codegen|etc>
ctest --test-dir build -L semantic -j8         # Test semantic analysis
ctest --test-dir build -R "parser.*expr"       # Test parser expressions
```

**📚 Full Command Reference**: See `docs/contributor/reference/commands-quick-reference.md` for all commands.

**⚠️ Test Build Strategy**: See `docs/contributor/guides/test-build-strategy.md` for critical test building info.

**📚 BDD Testing Guide**: See `docs/contributor/guides/bdd-testing-guide.md` for BDD testing documentation.

## Architecture Overview

### Source Structure (`src/`)
- **`parser/`** - Modular lexer/parser with AST generation following PEG grammar
- **`analysis/`** - Semantic analysis, type inference, symbol tables, memory safety
- **`codegen/`** - LLVM IR backend exclusively (no direct assembly generation), optimization via LLVM passes
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

| Rule | Description | Reference |
|------|-------------|-----------|
| **Design Principles** | Follow all 5 core principles: AI efficiency, safe FFI, determinism, observability, pragmatic evolution | `docs/spec/overview.md` |
| **Grammar Authority** | `grammar.txt` is single source of truth. No traits, interfaces, advanced generics, closures, macros | `grammar.txt` |
| **Implementation Status** | Check/update Master Implementation Status table for all features | `docs/spec/` |
| **Documentation Structure** | Use 5-folder structure: architecture/, contributor/, spec/, stdlib/, user-manual/ | `docs/index.md` |
| **Compiler Phases** | Strict separation: Parser → Semantic → CodeGen. No cross-phase dependencies | Design docs |
| **Expression-Oriented** | All constructs return values. Explicit `return ();` for void. Unit type `()` support | `docs/architecture/expression-oriented-design.md` |
| **Gitignore Compliance** | Check files with `git check-ignore -v`. Use `test_*.c` pattern | `docs/contributor/guides/gitignore-compliance.md` |
| **Temporary Test Files** | ALL temporary test files (source and executables) MUST be saved in a temporary directory (e.g., `/tmp/asthra-tests/` or `mktemp -d`). NEVER create temporary test files in the project directory | N/A |
| **GitHub Access** | Always use the `gh` command when accessing GitHub issues, pull requests, or any GitHub-related information. Never guess or assume issue content - always fetch the actual data using commands like `gh issue view`, `gh pr view`, etc. | N/A |

### C23 Standards
- **Must use** modern C23 features appropriately: `typeof`, `_BitInt`, `#embed`, `_Bool` as `bool`, improved type inference
- **Leverage** C23 improvements: better Unicode support, improved atomics, enhanced preprocessor
- **Requires** Clang/LLVM 18.0 or later with C23 support

### Build System
- **CMake-based build system** with modular configuration
- **Platform support**: macOS (Apple Silicon/Intel), Linux (x86_64/ARM64)
- **Sanitizer support**: AddressSanitizer, ThreadSanitizer, UndefinedBehaviorSanitizer
- **LLVM requirement**: Version 18.0 or later (enforced by CMake)

## Development Workflow

### Before Making Changes
1. **Verify Asthra Design Principles alignment** - Ensure proposed changes support all 5 core principles
2. **Check implementation status** in `docs/spec` Master Implementation Status table
3. **Understand component role** by reading related files and documentation
4. **Verify grammar compliance** if making parser/lexer changes

### Required Testing
1. **Build test executables first**: `cmake --build build --target build-tests` (especially after clean)
2. **Always execute tests from root directory**: All tests must be run from the project root using `ctest` commands
3. **Run full test suite**: `ctest --test-dir build` must pass
4. **Component-specific tests**: Test relevant components with `ctest --test-dir build -L <component-name>` (e.g., `ctest -L semantic`)
   - Use parallel execution for quicker iteration: `ctest --test-dir build -L semantic -j8`
   - Some components have sub-categories: `ctest --test-dir build -R "codegen.*function"`
5. **Performance validation**: Run benchmarks if changes affect performance
6. **Memory safety**: Use sanitizers (build with `-DSANITIZER=Address` or `-DSANITIZER=Thread`) for memory-related changes

**⚠️ Important**: Never use full clean when fixing tests. See `docs/contributor/guides/test-build-strategy.md` for the correct approach.

### Code Quality Standards
- **Follow established patterns** from existing codebase (slice patterns, atomic operations, error handling)
- **Use AsthraResult types** for consistent error handling
- **Implement proper cleanup** in error paths
- **64-byte align** performance-critical data structures

### Test File Naming Convention
- **All test files must use the `test_` prefix** - test files should follow the pattern `test_<name>.c`
- **No trailing `_test` suffix needed** - use `test_semantic.c` instead of `semantic_test.c`

### BDD Testing
- **📚 Full Guide**: See `bdd/BDD_BEST_PRACTICES.md` and `docs/contributor/guides/bdd-testing-guide.md`
- Use Given-When-Then pattern, handle @wip scenarios, register tests in CMakeLists.txt

## Performance Targets
- **Compilation speed**: 15-25% faster than baseline with C23 optimizations
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

## Key Development Guidelines

- **Validate** against 5 Design Principles (see table above)
- **Check** `grammar.txt` before implementing features  
- **Update** Master Implementation Status in `docs/spec/`
- **Test** thoroughly - run full test suite before commits
- **Use** C17 features with fallbacks
- **Prioritize** AI-friendly patterns and local reasoning

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

## Development Workflow Reminders

- After modifying source files: `cmake --build build -j8 && cmake --build build --target build-tests && cmake --build build --target format`
- Use incremental builds (see `docs/contributor/guides/test-build-strategy.md`)
