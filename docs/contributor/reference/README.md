# Reference Materials

**Comprehensive reference documentation for Asthra contributors**

This section provides detailed technical reference materials for all aspects of Asthra development. For getting started guides, see the [Contributor Handbook](../HANDBOOK.md).

## Table of Contents

1. [Architecture Reference](#architecture-reference)
2. [API Documentation](#api-documentation)
3. [Build System Reference](#build-system-reference)
4. [Testing Framework](#testing-framework)
5. [Development Tools](#development-tools)
6. [Language Specifications](#language-specifications)
7. [Coding Standards](#coding-standards)

## Architecture Reference

### Core Architecture
- **[Compiler Architecture](../../architecture/compiler-architecture.md)** - Complete compilation pipeline
- **[Directory Structure](architecture/directory-structure.md)** - Codebase organization
- **[Component Overview](architecture/components.md)** - Major system components
- **[Data Flow](architecture/data-flow.md)** - Information flow through compiler

### Detailed Components
- **[Lexer Architecture](architecture/lexer.md)** - Tokenization and scanning
- **[Parser Architecture](architecture/parser.md)** - Grammar and AST construction
- **[Semantic Analysis](architecture/semantic.md)** - Type checking and validation
- **[Code Generation](architecture/codegen.md)** - C code and binary generation

### Advanced Topics
- **[Memory Management](architecture/memory.md)** - Memory safety and ownership
- **[Concurrency Model](architecture/concurrency.md)** - Three-tier concurrency system
- **[FFI System](architecture/ffi.md)** - Foreign function interface
- **[AI Integration](architecture/ai-integration.md)** - AI-friendly design patterns

## API Documentation

### Core APIs
- **[Parser API](api/parser.md)** - Parsing functions and data structures
- **[Semantic API](api/semantic.md)** - Semantic analysis interfaces
- **[CodeGen API](api/codegen.md)** - Code generation functions
- **[Runtime API](api/runtime.md)** - Runtime system interfaces

### Utility APIs
- **[AST API](api/ast.md)** - Abstract syntax tree manipulation
- **[Symbol Table API](api/symbol-table.md)** - Symbol management
- **[Error Handling API](api/error-handling.md)** - Error reporting system
- **[Memory API](api/memory.md)** - Memory management utilities

### Testing APIs
- **[Test Framework API](api/test-framework.md)** - Testing infrastructure
- **[Mock APIs](api/mocks.md)** - Testing utilities and mocks
- **[Assertion API](api/assertions.md)** - Test assertion functions

## Build System Reference

### Core Build System
- **[Makefile Reference](build-system/makefile.md)** - Main build configuration
- **[Modular System](build-system/modular.md)** - Modular makefile architecture
- **[Build Targets](build-system/targets.md)** - Available build targets
- **[Platform Support](build-system/platforms.md)** - Platform-specific configuration

### Advanced Build Features
- **[Sanitizers](build-system/sanitizers.md)** - Memory and thread safety checking
- **[Coverage](build-system/coverage.md)** - Code coverage analysis
- **[Profiling](build-system/profiling.md)** - Performance profiling
- **[Static Analysis](build-system/static-analysis.md)** - Code quality tools

### Third-Party Integration
- **[Dependencies](build-system/dependencies.md)** - External library management
- **[Package Management](build-system/packages.md)** - System package integration
- **[Cross-Compilation](build-system/cross-compilation.md)** - Multi-platform builds

## Testing Framework

### Framework Overview
- **[Testing Philosophy](testing/philosophy.md)** - Testing approach and principles
- **[Test Categories](testing/categories.md)** - Complete test category guide
- **[Test Structure](testing/structure.md)** - Test organization and layout
- **[Writing Tests](testing/writing-tests.md)** - Test development guide

### Test Categories
- **[Unit Tests](testing/unit-tests.md)** - Component-level testing
- **[Integration Tests](testing/integration-tests.md)** - End-to-end testing
- **[Performance Tests](testing/performance-tests.md)** - Benchmark testing
- **[Regression Tests](testing/regression-tests.md)** - Bug prevention testing

### Advanced Testing
- **[Memory Testing](testing/memory-testing.md)** - Memory safety validation
- **[Concurrency Testing](testing/concurrency-testing.md)** - Thread safety testing
- **[Fuzzing](testing/fuzzing.md)** - Automated test generation
- **[CI/CD Integration](testing/ci-cd.md)** - Continuous integration

## Development Tools

### Core Tools
- **[AST Visualizer](tools/ast-visualizer.md)** - AST debugging and visualization
- **[Grammar Validator](tools/grammar-validator.md)** - Grammar compliance checking
- **[Symbol Inspector](tools/symbol-inspector.md)** - Symbol table debugging
- **[Code Formatter](tools/formatter.md)** - Code style enforcement

### Analysis Tools
- **[Static Analyzer](tools/static-analyzer.md)** - Code quality analysis
- **[Performance Profiler](tools/profiler.md)** - Performance analysis
- **[Memory Debugger](tools/memory-debugger.md)** - Memory issue detection
- **[Coverage Analyzer](tools/coverage.md)** - Test coverage analysis

### Development Utilities
- **[Build Scripts](tools/build-scripts.md)** - Automation scripts
- **[Test Runners](tools/test-runners.md)** - Test execution utilities
- **[Documentation Generators](tools/doc-generators.md)** - Documentation tools
- **[Release Tools](tools/release-tools.md)** - Release management

## Language Specifications

### Grammar and Syntax
- **[Grammar Specification](language/grammar.md)** - Complete PEG grammar
- **[Syntax Reference](language/syntax.md)** - Language syntax guide
- **[Lexical Analysis](language/lexical.md)** - Token specification
- **[Parsing Rules](language/parsing.md)** - Parser implementation guide

### Type System
- **[Type System](language/types.md)** - Complete type system reference
- **[Memory Model](language/memory-model.md)** - Four-zone memory system
- **[Ownership](language/ownership.md)** - Ownership and borrowing
- **[Generics](language/generics.md)** - Generic type system

### Advanced Features
- **[Concurrency](language/concurrency.md)** - Concurrency primitives
- **[Pattern Matching](language/patterns.md)** - Pattern matching system
- **[FFI](language/ffi.md)** - Foreign function interface
- **[Annotations](language/annotations.md)** - Annotation system

## Coding Standards

### Code Style
- **[C Style Guide](standards/c-style.md)** - C code formatting and style
- **[Naming Conventions](standards/naming.md)** - Identifier naming rules
- **[File Organization](standards/file-organization.md)** - Source file structure
- **[Documentation Standards](standards/documentation.md)** - Code documentation

### Quality Standards
- **[Code Review Guidelines](standards/code-review.md)** - Review process and criteria
- **[Testing Standards](standards/testing.md)** - Test quality requirements
- **[Performance Guidelines](standards/performance.md)** - Performance considerations
- **[Security Guidelines](standards/security.md)** - Security best practices

### Project Standards
- **[Git Workflow](standards/git-workflow.md)** - Version control practices
- **[Issue Management](standards/issues.md)** - Issue tracking and management
- **[Release Process](standards/releases.md)** - Release management process
- **[Contribution Guidelines](standards/contributions.md)** - Contribution requirements

## Quick Reference Cards

### Cheat Sheets
- **[Build Commands](quick-ref/build-commands.md)** - Essential build commands
- **[Test Commands](quick-ref/test-commands.md)** - Testing command reference
- **[Debug Commands](quick-ref/debug-commands.md)** - Debugging utilities
- **[Git Commands](quick-ref/git-commands.md)** - Version control commands

### API Quick Reference
- **[Parser Functions](quick-ref/parser-functions.md)** - Key parser functions
- **[Semantic Functions](quick-ref/semantic-functions.md)** - Semantic analysis functions
- **[AST Functions](quick-ref/ast-functions.md)** - AST manipulation functions
- **[Test Functions](quick-ref/test-functions.md)** - Testing framework functions

### Configuration Reference
- **[Environment Variables](quick-ref/environment.md)** - Development environment variables
- **[Makefile Variables](quick-ref/makefile-vars.md)** - Build system variables
- **[Compiler Flags](quick-ref/compiler-flags.md)** - Compilation options
- **[Test Configuration](quick-ref/test-config.md)** - Test system configuration

## Navigation Tips

### Finding Information Quickly

**By Component:**
- **Lexer**: [Architecture](architecture/lexer.md) → [API](api/parser.md) → [Tests](testing/categories.md#lexer-tests)
- **Parser**: [Architecture](architecture/parser.md) → [API](api/parser.md) → [Grammar](language/grammar.md)
- **Semantic**: [Architecture](architecture/semantic.md) → [API](api/semantic.md) → [Types](language/types.md)
- **CodeGen**: [Architecture](architecture/codegen.md) → [API](api/codegen.md) → [FFI](language/ffi.md)

**By Task:**
- **Adding Features**: [Architecture](architecture/) → [API](api/) → [Testing](testing/writing-tests.md)
- **Fixing Bugs**: [Debugging](tools/memory-debugger.md) → [Testing](testing/regression-tests.md) → [Standards](standards/code-review.md)
- **Performance**: [Profiling](tools/profiler.md) → [Guidelines](standards/performance.md) → [Testing](testing/performance-tests.md)
- **Documentation**: [Standards](standards/documentation.md) → [Tools](tools/doc-generators.md) → [Style](standards/c-style.md)

**By Experience Level:**
- **Beginner**: [Quick Reference](quick-ref/) → [API](api/) → [Examples](../HANDBOOK.md#your-first-contribution)
- **Intermediate**: [Architecture](architecture/) → [Testing](testing/) → [Tools](tools/)
- **Advanced**: [Language Specs](language/) → [Standards](standards/) → [Build System](build-system/)

### Search Tips

**Finding Functions:**
```bash
# Search API documentation
grep -r "function_name" docs/contributor/reference/api/

# Search implementation
grep -r "function_name" src/
```

**Finding Examples:**
```bash
# Search test files for usage examples
grep -r "pattern" tests/

# Search documentation for examples
grep -r "example" docs/contributor/reference/
```

**Finding Configuration:**
```bash
# Search build system documentation
grep -r "variable_name" docs/contributor/reference/build-system/

# Search quick reference
grep -r "command" docs/contributor/reference/quick-ref/
```

## Contributing to Reference Materials

### Adding New Documentation

1. **Choose the Right Section**
   - Architecture: System design and component interaction
   - API: Function interfaces and data structures
   - Build System: Build configuration and tools
   - Testing: Test framework and methodologies
   - Tools: Development utilities and scripts
   - Language: Language specification and features
   - Standards: Code quality and process guidelines

2. **Follow Documentation Standards**
   - Use clear, concise language
   - Include practical examples
   - Provide cross-references
   - Test all code examples
   - Follow [Documentation Standards](standards/documentation.md)

3. **Update Navigation**
   - Add entries to this index
   - Update related cross-references
   - Add to quick reference if applicable

### Maintaining Documentation

- **Regular Reviews**: Documentation should be reviewed quarterly
- **Version Updates**: Update when language or tools change
- **Example Validation**: Verify all examples work with current codebase
- **Link Checking**: Ensure all internal links are valid

---

**📚 Complete Reference Library**

*This reference section provides comprehensive technical documentation for all aspects of Asthra development. For getting started, see the [Contributor Handbook](../HANDBOOK.md).* 