# Asthra Contributor Handbook

**Version:** 1.0  
**Date:** January 2025  
**Status:** Active Development  

Welcome to the **Asthra Contributor Handbook** - your comprehensive guide to contributing to the Asthra programming language compiler and ecosystem. This handbook will take you from your first contribution to becoming a core maintainer, with practical guidance, real examples, and clear workflows.

## Table of Contents

1\. &#91;Welcome to Asthra Development&#93;(#welcome-to-asthra-development)
2\. &#91;Quick Start (5-minute setup)&#93;(#quick-start)
3\. &#91;Development Environment Setup&#93;(#development-environment-setup)
4\. &#91;Your First Contribution&#93;(#your-first-contribution)
5\. &#91;Development Workflows&#93;(#development-workflows)
6\. &#91;Testing &amp; Quality Assurance&#93;(#testing--quality-assurance)
7\. &#91;Code Review Process&#93;(#code-review-process)
8\. &#91;Advanced Topics&#93;(#advanced-topics)
9\. &#91;Troubleshooting&#93;(#troubleshooting)
10\. &#91;Reference Materials&#93;(#reference-materials)
11\. &#91;Community &amp; Support&#93;(#community--support)

## Progressive Learning Path

Choose your starting point based on your experience and goals:

- **🚀 &#91;Beginner&#93;(#beginner-path)**: First contribution in 30 minutes
- **⚡ &#91;Intermediate&#93;(#intermediate-path)**: Understanding compiler architecture  
- **🔧 &#91;Advanced&#93;(#advanced-path)**: Adding new language features
- **🏆 &#91;Expert&#93;(#expert-path)**: Core maintainer responsibilities

---

## Welcome to Asthra Development

Asthra is a modern systems programming language designed specifically for **AI-friendly code generation** while maintaining the performance and safety requirements of systems programming. Built through AI-assisted development, Asthra represents a new paradigm in programming language design.

### What Makes Asthra Special

- **AI-First Design**: Every language feature optimized for AI code generation
- **Memory Safety**: Four-zone memory model with automatic safety guarantees
- **Concurrency**: Three-tier concurrency system from deterministic to advanced
- **FFI Safety**: Automatic C interoperability with transfer semantics
- **Zero-Cost Abstractions**: High-level features with C-level performance

### Project Status

Asthra is in **active development** with a production-ready compiler featuring:
- ✅ Complete compilation pipeline (Lexer → Parser → Semantic → CodeGen)
- ✅ 176 tests across 30 categories with 100% success rate
- ✅ Full AI integration (annotations, linting, API)
- ✅ Advanced features (concurrency, FFI, memory management, OOP)
- ✅ Rich standard library (JSON, HTTP, channels, coordination)

### How You Can Contribute

**Code Contributions:**
- Compiler improvements and bug fixes
- New language features and optimizations
- Standard library modules
- Testing and quality assurance

**Documentation:**
- User guides and tutorials
- API documentation
- Architecture documentation
- Translation and localization

**Community:**
- Issue triage and support
- Code review and mentoring
- Community building and outreach
- Educational content creation

---

## Quick Start

Get up and running with Asthra development in under 5 minutes.

### Prerequisites

- **C17 Compiler**: Clang 6+ (preferred) or GCC 7+
- **Make**: GNU Make 4.0+
- **Git**: For version control
- **Platform**: macOS, Linux, or Windows (with MinGW)

### 5-Minute Setup

```bash
# 1. Clone the repository
git clone https://github.com/asthra-lang/asthra.git
cd asthra

# 2. Build the compiler (2-3 minutes)
make -j$(nproc)

# 3. Run basic tests (1 minute)
make test-basic

# 4. Verify installation
./bin/asthra --version
```

**Expected Output:**
```
Asthra Compiler v2.0.0
Build: production-ready
Features: AI-integration, concurrency, FFI, memory-safety
```

### Your First Build Success

If you see the version output above, congratulations! You've successfully:
- ✅ Built the complete Asthra compiler
- ✅ Verified all core components work
- ✅ Confirmed your development environment is ready

**Next Steps:**
- 🚀 **New to Asthra?** → Continue to &#91;Your First Contribution&#93;(#your-first-contribution)
- ⚡ **Want to understand the code?** → Jump to &#91;Understanding the Codebase&#93;(#understanding-the-codebase)
- 🔧 **Ready to add features?** → See &#91;Development Workflows&#93;(#development-workflows)

### Quick Links

- **&#91;5-minute setup guide&#93;(quick-start/README.md)** - Detailed setup instructions
- **&#91;First contribution walkthrough&#93;(quick-start/first-contribution.md)** - Step-by-step first PR
- **&#91;Development environment guide&#93;(quick-start/development-setup.md)** - Complete environment setup

---

## Development Environment Setup

### Recommended Development Setup

**IDE Configuration:**
```bash
# VS Code (Recommended)
code --install-extension ms-vscode.cpptools
code --install-extension ms-vscode.makefile-tools

# Configure for Asthra development
cp .vscode/settings.json.template .vscode/settings.json
```

**Development Tools:**
```bash
# macOS
brew install llvm cppcheck valgrind

# Linux (Ubuntu/Debian)
sudo apt-get install clang-12 cppcheck valgrind

# Set preferred compiler
export CC=clang
export CXX=clang++
```

### Build System Overview

Asthra uses a **modern CMake build system** with excellent cross-platform support:

```bash
# Core build commands
cmake -B build          # Configure build
cmake --build build     # Build everything
cmake --build build -j8 # Parallel build (8 jobs)

# Development builds
cmake -B build -DCMAKE_BUILD_TYPE=Debug    # Debug build with symbols
cmake -B build -DENABLE_ASAN=ON            # AddressSanitizer build
cmake -B build -DCMAKE_BUILD_TYPE=Release  # Optimized release build

# Testing
ctest --test-dir build                      # Run all tests
ctest --test-dir build -L lexer             # Run specific category tests
cmake --build build --target test-quick    # Run high-reliability tests
```

### Environment Variables

```bash
# Compiler selection
export CC=clang
export CXX=clang++

# Debug options
export ASTHRA_DEBUG=1
export ASTHRA_VERBOSE=1

# Test configuration
export ASTHRA_TEST_TIMEOUT=30
export ASTHRA_TEST_PARALLEL=8
```

---

## Understanding the Codebase

For a comprehensive overview of the Asthra compiler's architecture, including its design principles, compilation pipeline, key components, and data flow, please refer to the dedicated &#91;Architecture Overview&#93;(reference/architecture.md) guide in the Reference Materials section.

### Key Components

**Parser (`src/parser/`):**
- `lexer_*.c` - Tokenization and scanning
- `grammar_*.c` - PEG grammar implementation
- `ast_*.c` - AST node management
- `token.c` - Token definitions
- `keyword.c` - Keyword recognition

**Semantic Analysis (`src/analysis/`):**
- `semantic_*.c` - Type checking and validation
- `symbol_*.c` - Symbol table management
- `type_*.c` - Type system implementation

**Code Generation (`src/codegen/`):**
- `ffi_*.c` - FFI assembly generation
- `generic_*.c` - Generic instantiation
- `elf_*.c` - Direct binary generation

### Design Principles

1\. **AI Generation Efficiency**
   - Minimal, unambiguous syntax
   - Deterministic parsing behavior
   - Consistent patterns across features

2\. **Memory Safety**
   - Four-zone memory model (GC, Manual, Pinned, External)
   - Ownership tracking with transfer semantics
   - Bounds checking for all operations

3\. **Modular Architecture**
   - Clear component boundaries
   - Parallel development support
   - Comprehensive testing at all levels

---

## Your First Contribution

### Finding Your First Issue

**Good First Issues:**
- Documentation improvements
- Test case additions
- Error message enhancements
- Example program creation

**Where to Look:**
```bash
# GitHub labels
good-first-issue        # Beginner-friendly issues
documentation          # Documentation improvements
testing                # Test-related work
help-wanted            # Community help needed
```

### Step-by-Step First Contribution

#### 1. Choose an Issue
```bash
# Example: Improve error message for undefined variables
# Issue: "Make undefined variable error more helpful"
```

#### 2. Fork and Clone
```bash
# Fork on GitHub, then:
git clone https://github.com/asthra-lang/asthra.git
cd asthra
git remote add upstream https://github.com/asthra-lang/asthra.git
```

#### 3. Create Feature Branch
```bash
git checkout -b improve-undefined-variable-error
```

#### 4. Make Your Change
```c
// Example: src/analysis/semantic_core.c
// Before:
fprintf(stderr, "Error: undefined variable\n");

// After:
fprintf(stderr, "Error: undefined variable '%s' at line %d\n", 
        var_name, line_number);
fprintf(stderr, "Hint: Did you mean '%s'? (similar variable found)\n", 
        suggested_name);
```

#### 5. Test Your Change
```bash
# Build with your changes
make clean &amp;&amp; make

# Run relevant tests
make test-semantic

# Test manually
echo 'fn main() { let x = undefined_var; }' &gt; test.asthra
./bin/asthra test.asthra
```

#### 6. Commit and Push
```bash
git add src/analysis/semantic_core.c
git commit -m "Improve undefined variable error messages

- Add variable name and line number to error
- Suggest similar variable names when available
- Enhance user experience for common typos

Fixes #123"

git push origin improve-undefined-variable-error
```

#### 7. Create Pull Request
- Go to GitHub and create PR
- Use the provided PR template
- Link to the issue you're fixing
- Describe your changes clearly

### First Contribution Checklist

- &#91; &#93; Issue selected and understood
- &#91; &#93; Fork created and cloned
- &#91; &#93; Feature branch created
- &#91; &#93; Changes implemented
- &#91; &#93; Tests pass (`make test-relevant-category`)
- &#91; &#93; Code follows style guidelines
- &#91; &#93; Commit message follows conventions
- &#91; &#93; PR created with clear description

---

## Development Workflows

### Adding New Features

#### 1. Feature Planning
```bash
# Create feature branch
git checkout -b feature/new-language-feature

# Document the feature
echo "# New Feature: &#91;Feature Name&#93;" &gt; docs/features/new-feature.md
```

#### 2. Implementation Phases
**Phase 1: Grammar &amp; Parser**
```bash
# Update grammar
vim grammar.txt

# Implement parser rules
vim src/parser/grammar_*.c

# Test parsing
make test-parser
```

**Phase 2: Semantic Analysis**
```bash
# Add semantic validation
vim src/analysis/semantic_*.c

# Test semantic analysis
make test-semantic
```

**Phase 3: Code Generation**
```bash
# Implement code generation
vim src/codegen/*.c

# Test code generation
make test-codegen
```

**Phase 4: Integration &amp; Testing**
```bash
# Add comprehensive tests
vim tests/*/test_new_feature.c

# Run full test suite
make test-all
```

### Bug Fixing Workflow

#### 1. Issue Investigation
```bash
# Reproduce the bug
./bin/asthra problematic_file.asthra

# Enable debug output
export ASTHRA_DEBUG=1
./bin/asthra problematic_file.asthra

# Use debugging tools
gdb ./bin/asthra
(gdb) run problematic_file.asthra
```

#### 2. Root Cause Analysis
```bash
# Check relevant test categories
make test-lexer test-parser test-semantic test-codegen

# Use static analysis
make static-analysis

# Memory debugging
make asan
./bin/asthra problematic_file.asthra
```

#### 3. Fix Implementation
```bash
# Create fix branch
git checkout -b fix/issue-description

# Implement fix
vim src/relevant/component.c

# Add regression test
vim tests/relevant/test_regression.c
```

#### 4. Validation
```bash
# Verify fix works
./bin/asthra problematic_file.asthra

# Ensure no regressions
make test-all

# Performance check
make benchmark
```

### Performance Optimization

#### 1. Profiling
```bash
# Build with profiling
make profile

# Run performance tests
make test-performance

# Generate profile data
./bin/asthra large_program.asthra
gprof ./bin/asthra gmon.out &gt; profile.txt
```

#### 2. Optimization Implementation
```bash
# Identify bottlenecks
vim profile.txt

# Implement optimizations
vim src/relevant/component.c

# Validate improvements
make benchmark-before-after
```

---

## Testing &amp; Quality Assurance

### Testing Philosophy

Asthra maintains **100% test success rate** across all categories through:
- **Comprehensive Coverage**: 176 tests across 30 categories
- **Quality Focus**: Every test must pass consistently
- **Regression Prevention**: New tests for every bug fix
- **Performance Validation**: Benchmark tests for critical paths

### Test Categories

```bash
# Core compiler components
make test-lexer          # Lexical analysis
make test-parser         # Parsing and AST
make test-semantic       # Type checking and validation
make test-codegen        # Code generation

# Language features
make test-concurrency    # Concurrency features
make test-ffi           # Foreign function interface
make test-memory        # Memory management
make test-patterns      # Pattern matching

# Integration and quality
make test-integration   # End-to-end testing
make test-performance   # Performance benchmarks
make test-security      # Security validation
```

### Writing Effective Tests

#### Test Structure
```c
#include "../framework/test_framework.h"

TEST_CASE("descriptive_test_name") {
    // Setup
    TestContext *ctx = create_test_context();
    
    // Execute
    Result result = function_under_test(input);
    
    // Verify
    ASSERT_TRUE(result.success);
    ASSERT_EQUAL_STR(result.output, "expected_output");
    
    // Cleanup
    cleanup_test_context(ctx);
}
```

#### Test Best Practices
- **Descriptive Names**: Test names should explain what they verify
- **Single Responsibility**: Each test should verify one specific behavior
- **Comprehensive Coverage**: Test both success and failure cases
- **Resource Cleanup**: Always clean up allocated resources
- **Deterministic**: Tests should produce consistent results

### Running Tests

```bash
# Quick validation
make test-curated        # High-reliability tests

# Category-specific
make test-lexer         # Specific component
make test-all-sans-performance  # All except performance

# Comprehensive
make test-all           # Complete test suite

# Debug failing tests
make test-semantic VERBOSE=1
```

---

## Code Review Process

### Preparing for Review

#### Pre-Review Checklist
- &#91; &#93; All tests pass (`make test-all`)
- &#91; &#93; Code follows style guidelines
- &#91; &#93; Documentation updated
- &#91; &#93; Commit messages are clear
- &#91; &#93; No debug code or temporary changes
- &#91; &#93; Performance impact considered

#### Code Quality Standards
```c
// Good: Clear, documented function
/**
 * Parse a function declaration from the token stream.
 * 
 * @param parser The parser context
 * @param visibility The function visibility (pub/priv)
 * @return AST node for the function, or NULL on error
 */
ASTNode* parse_function_declaration(Parser* parser, Visibility visibility) {
    // Implementation with clear error handling
}

// Bad: Unclear, undocumented function
ASTNode* parse_func(Parser* p, int v) {
    // Implementation without error handling
}
```

### Review Process

#### 1. Automated Checks
- **Build Verification**: All configurations must build
- **Test Validation**: All tests must pass
- **Static Analysis**: No new warnings or errors
- **Performance**: No significant regressions

#### 2. Human Review
- **Code Quality**: Readability, maintainability, correctness
- **Architecture**: Fits with overall design
- **Testing**: Adequate test coverage
- **Documentation**: Clear and complete

#### 3. Feedback Integration
```bash
# Address review feedback
git checkout feature-branch
# Make requested changes
git add .
git commit -m "Address review feedback: improve error handling"
git push origin feature-branch
```

### Review Guidelines

**For Authors:**
- Respond to all feedback
- Ask questions if feedback is unclear
- Make requested changes promptly
- Test changes thoroughly

**For Reviewers:**
- Be constructive and specific
- Explain the reasoning behind suggestions
- Acknowledge good practices
- Focus on code quality and maintainability

---

## Advanced Topics

Asthra is designed for **AI-assisted development**. Here are proven patterns:

- &#91;AI Collaboration Patterns&#93;(guides/ai-collaboration.md)
- &#91;Performance Optimization Techniques&#93;(guides/performance-optimization.md)
- &#91;Extending the Build System&#93;(guides/extending-the-build-system.md)
- &#91;Adding New Platforms&#93;(guides/extending-the-build-system.md#3-adding-new-platforms)

---

## Troubleshooting

### Common Build Issues

#### Compilation Errors
```bash
# Missing dependencies
make clean &amp;&amp; make -j1    # Single-threaded for clearer errors

# Compiler version issues
clang --version           # Verify C17 support
export CC=clang          # Set preferred compiler

# Missing headers
make install-deps        # Install development dependencies
```

#### Link Errors
```bash
# Missing object files
make clean &amp;&amp; make       # Full rebuild

# Symbol conflicts
nm build/src/parser/*.o | grep conflicting_symbol

# Library issues
ldd bin/asthra          # Check library dependencies
```

### Test Failures

#### Debugging Test Failures
```bash
# Run specific failing test
make test-semantic VERBOSE=1

# Debug with GDB
gdb tests/semantic/test_semantic_main
(gdb) run

# Memory debugging
make asan
make test-semantic
```

#### Common Test Issues
- **Environment**: Wrong compiler or missing tools
- **Timing**: Race conditions in concurrent tests
- **Platform**: Platform-specific behavior differences
- **Resources**: Insufficient memory or disk space

### Performance Issues

#### Slow Compilation
```bash
# Parallel build
make -j$(nproc)

# Incremental builds
make                     # Only rebuild changed files

# Profile compilation
time make clean &amp;&amp; time make
```

#### Runtime Performance
```bash
# Profile generated code
make profile
./bin/asthra program.asthra
gprof ./bin/asthra

# Memory usage
valgrind --tool=massif ./bin/asthra program.asthra
```

### Getting Help

#### Self-Help Resources
- **&#91;Troubleshooting Guide&#93;(troubleshooting/README.md)** - Comprehensive problem-solving
- **&#91;Common Issues&#93;(troubleshooting/common-issues.md)** - Frequently encountered problems
- **&#91;Build Problems&#93;(troubleshooting/build-problems.md)** - Build system issues
- **&#91;Test Failures&#93;(troubleshooting/test-failures.md)** - Test debugging guide

#### Community Support
- **GitHub Issues**: Report bugs and ask questions
- **Discussions**: General development discussion
- **Discord/Slack**: Real-time community chat
- **Email**: Direct contact with maintainers

### Reporting Issues

**Include This Information:**
```bash
# System information
uname -a
clang --version
make --version

# Environment
env | grep ASTHRA
env | grep CC

# Build information
make clean &amp;&amp; make -j1 2>&amp;1 | tee build.log

# Error reproduction
./bin/asthra problematic_file.asthra 2>&amp;1 | tee error.log
```

**Template for Bug Reports:**
```markdown
## Environment
- OS: &#91;macOS 13.0 / Ubuntu 22.04 / etc.&#93;
- Compiler: &#91;clang 14.0.0&#93;
- Make: &#91;GNU Make 4.3&#93;

## Problem Description
&#91;Clear description of the issue&#93;

## Steps to Reproduce
1\. &#91;First step&#93;
2\. &#91;Second step&#93;
3\. &#91;Error occurs&#93;

## Expected Behavior
&#91;What should happen&#93;

## Actual Behavior
&#91;What actually happens&#93;

## Error Output
```
&#91;Paste error messages here&#93;
```

## Additional Context
&#91;Any other relevant information&#93;
```

---

## Reference Materials

### Architecture Documentation
- **&#91;Compiler Architecture&#93;(reference/architecture.md)** - Complete architecture overview
- **&#91;Directory Structure&#93;(reference/directory-structure.md)** - Codebase organization
- **&#91;Build System&#93;(reference/build-system.md)** - Build system reference
- **&#91;Lexer and Parser&#93;(reference/lexer-parser.md)** - Tokenization and AST construction
- **&#91;Semantic Analysis&#93;(reference/semantic-analysis.md)** - Type checking and validation
- **&#91;Code Generation&#93;(reference/code-generation.md)** - C code and binary generation
- **&#91;Runtime System&#93;(reference/runtime-system.md)** - Runtime system implementation
- **&#91;Performance and Optimization&#93;(reference/performance.md)** - Compiler performance and optimization
- **&#91;Platform Support&#93;(reference/platform-support.md)** - Cross-platform support
- **&#91;Extensibility&#93;(reference/extensibility.md)** - Extending the compiler

### API Documentation
- **&#91;Parser API&#93;(reference/api/parser.md)** - Parser component APIs
- **&#91;Semantic API&#93;(reference/api/semantic.md)** - Semantic analysis APIs
- **&#91;CodeGen API&#93;(reference/api/codegen.md)** - Code generation APIs
- **&#91;Runtime API&#93;(reference/api/runtime.md)** - Runtime system interfaces
- **&#91;AST API&#93;(reference/api/ast.md)** - Abstract syntax tree manipulation
- **&#91;Symbol Table API&#93;(reference/api/symbol-table.md)** - Symbol management
- **&#91;Error Handling API&#93;(reference/api/error-handling.md)** - Error reporting system
- **&#91;Memory API&#93;(reference/api/memory.md)** - Memory management utilities

### Development Tools
- **&#91;AST Visualizer&#93;(reference/tools/ast-visualizer.md)** - AST debugging tool
- **&#91;Grammar Validator&#93;(reference/tools/grammar-validator.md)** - Grammar validation
- **&#91;Symbol Inspector&#93;(reference/tools/symbol-inspector.md)** - Symbol table debugging

### Testing Framework
- **&#91;Testing Guide&#93;(reference/testing/framework.md)** - Testing framework reference
- **&#91;Test Categories&#93;(reference/testing/categories.md)** - Complete test category guide
- **&#91;Writing Tests&#93;(reference/testing/writing-tests.md)** - Test development guide

---

## Community &amp; Support

### Communication Channels

**GitHub:**
- **Issues**: Bug reports and feature requests
- **Discussions**: General development discussion
- **Pull Requests**: Code contributions and reviews

**Real-time Chat:**
- **Discord**: Community chat and quick questions
- **Slack**: Development team coordination

**Email:**
- **Mailing List**: Development announcements
- **Direct Contact**: Maintainer communication

### Contributing Guidelines

**Code Contributions:**
- Follow the &#91;development workflows&#93;(#development-workflows)
- Ensure all tests pass
- Include comprehensive documentation
- Respond to review feedback promptly

**Documentation:**
- Use clear, concise language
- Include practical examples
- Maintain consistency with existing docs
- Test all instructions and code examples

**Community Participation:**
- Be respectful and inclusive
- Help newcomers get started
- Share knowledge and experience
- Provide constructive feedback

### Recognition and Growth

**Contribution Levels:**
- **Contributor**: Made meaningful contributions
- **Regular Contributor**: Consistent, quality contributions
- **Core Contributor**: Significant architectural contributions
- **Maintainer**: Project leadership and decision-making

**Growth Opportunities:**
- **Mentoring**: Help new contributors get started
- **Leadership**: Lead feature development or initiatives
- **Speaking**: Present at conferences and meetups
- **Writing**: Create educational content and tutorials

---

## Learning Paths

### Beginner Path
**Goal**: Make your first meaningful contribution

1\. **Setup** (15 minutes)
   - &#91;Quick Start&#93;(#quick-start) - Get Asthra building
   - &#91;Development Environment&#93;(#development-environment-setup) - Configure tools

2\. **Understanding** (30 minutes)
   - &#91;Codebase Overview&#93;(#understanding-the-codebase) - Learn the architecture
   - &#91;First Contribution&#93;(#your-first-contribution) - Make your first PR

3\. **Practice** (Ongoing)
   - Fix documentation issues
   - Add test cases
   - Improve error messages

### Intermediate Path
**Goal**: Understand compiler architecture and contribute features

1\. **Deep Dive** (2-3 hours)
   - Study &#91;Architecture Documentation&#93;(reference/architecture.md)
   - Understand compilation pipeline
   - Learn testing framework

2\. **Feature Work** (Ongoing)
   - Implement small language features
   - Add standard library functions
   - Optimize existing code

3\. **Quality** (Ongoing)
   - Write comprehensive tests
   - Improve documentation
   - Participate in code reviews

### Advanced Path
**Goal**: Add major language features and architectural improvements

1\. **Expertise** (Ongoing)
   - Master all compiler phases
   - Understand performance characteristics
   - Learn AI collaboration patterns

2\. **Leadership** (Ongoing)
   - Design new language features
   - Mentor other contributors
   - Make architectural decisions

3\. **Innovation** (Ongoing)
   - Research new techniques
   - Implement cutting-edge features
   - Contribute to language evolution

### Expert Path
**Goal**: Core maintainer and project leadership

1\. **Mastery** (Ongoing)
   - Complete understanding of all systems
   - Performance optimization expertise
   - Security and safety analysis

2\. **Leadership** (Ongoing)
   - Project roadmap planning
   - Community building
   - Strategic decision making

3\. **Vision** (Ongoing)
   - Language design evolution
   - Ecosystem development
   - Industry collaboration

---

## Quick Reference

### Essential Commands
```bash
# Build and test
make &amp;&amp; make test-curated

# Development builds
make debug asan release

# Specific testing
make test-lexer test-parser test-semantic test-codegen

# Performance
make benchmark profile

# Help
make help help-test help-tools
```

### Key Files
- `src/parser/` - Lexer and parser implementation
- `src/analysis/` - Semantic analysis and type checking
- `src/codegen/` - Code generation and optimization
- `tests/` - Comprehensive test suite
- `docs/` - Documentation and specifications
- `make/` - Modular build system

### Getting Help
- **Quick Questions**: GitHub Discussions
- **Bug Reports**: GitHub Issues
- **Real-time Chat**: Discord/Slack
- **Documentation**: This handbook and reference materials

---

**Welcome to the Asthra community! We're excited to have you contribute to the future of AI-friendly programming languages.**

*This handbook is a living document. If you find areas for improvement, please contribute updates through the normal PR process.* 