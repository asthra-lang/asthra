# Internal Documentation

This directory contains detailed technical documentation intended primarily for compiler developers and contributors working on the internal implementation of Asthra.

## Contents

### Implementation Specifications

- **[ffi_assembly_generator_v1.2.md](ffi_assembly_generator_v1.2.md)** - Comprehensive assembly generation specification
  - Detailed FFI call generation for System V AMD64 ABI
  - Pattern matching code generation strategies
  - String operation assembly optimization
  - Slice operation implementation details
  - Security feature assembly patterns

- **[build-system.md](build-system.md)** - Multi-Makefile architecture for compiler development
  - Domain-driven build system separation (Main, Safety, Concurrency)
  - Specialized toolchains and sanitizer configurations
  - CI pipeline strategies for different development domains
  - Build system design rationale and integration considerations

### Development Planning and Status

- **[prd.md](prd.md)** - Product Requirements Document with implementation status
  - Current feature implementation status and priorities
  - Technical architecture decisions and design rationale
  - Development timelines and effort estimates
  - Success metrics and performance benchmarks
  - Build system internals and AI-optimized features

- **[stdlib_essential_plan.md](stdlib_essential_plan.md)** - Essential Standard Library Development Plan
  - Comprehensive implementation specification for core/foundation libraries
  - Module structure, priorities, and effort estimates
  - Development timeline and phase breakdown
  - Design principles and architectural decisions
  - Quick reference guide and usage examples

- **[standard_library_classification.md](standard_library_classification.md)** - Hierarchical Standard Library Classification
  - Three-tier classification system (Essential, Intermediate, Advanced)
  - Functional categorization by purpose and domain
  - Comparative analysis with Python and Go standard libraries
  - Development planning framework for library organization

## Target Audience

This documentation is intended for:

- **Compiler Developers** - Those working on the Asthra compiler implementation
- **Contributors** - Advanced contributors working on language features and optimizations
- **Architecture Reviewers** - Those reviewing low-level implementation decisions
- **Performance Engineers** - Those optimizing code generation and runtime performance
- **Project Managers** - Those tracking development progress and technical decisions

## Complexity Level

The documents in this directory assume:

- Deep understanding of assembly language and calling conventions
- Familiarity with compiler implementation techniques
- Knowledge of FFI (Foreign Function Interface) concepts
- Understanding of memory management and garbage collection
- Knowledge of software development project management

## Note

This documentation is highly technical and implementation-specific. For user-facing documentation, language tutorials, or general development guidance, please refer to the main documentation directory. 
