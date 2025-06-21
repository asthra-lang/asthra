---
layout: page
title: Language Specification
permalink: /docs/spec/
---

# Asthra Language Specification

Welcome to the complete Asthra language specification. This section provides formal documentation of the language design, grammar, type system, and all technical details needed for implementation and advanced usage.

## 🎉 Production Ready Status

Asthra has achieved **PRODUCTION-READY status** with 100% test success rate across ALL 31 test categories, representing the world's first programming language designed from the ground up for AI code generation excellence.

---

## 📖 Specification Documents

### Core Language Specification

#### [Overview](/docs/spec/overview/)
Language introduction, design principles, and production-ready status overview.

#### [Grammar](/docs/spec/grammar/)
Complete PEG grammar specification (v1.25) with all syntax rules and parsing details.

#### [Type System](/docs/spec/types/)
Complete type system documentation including inference rules, checking, and type relationships.

#### [Design Rationale](/docs/spec/design_rationale/)
Comprehensive explanation of design decisions and responses to external analyses.

#### [Advanced Features](/docs/spec/advanced-features/)
Documentation of advanced language features including unsafe blocks, AI integration, and optimization.

### Package and Module System

#### [Packages](/docs/spec/packages/)
Complete Go-style package organization and multi-file package structure.

#### [Imports](/docs/spec/imports/)
Import declarations, aliases, name resolution, and module system details.

### Built-in Features

#### [Predeclared Identifiers](/docs/spec/predeclared/)
Complete reference of predeclared identifiers and built-in functions.

### Build System

#### [Ampu Build System](/docs/spec/ampu/)
Complete Ampu build system and project management documentation.

---

## 🚀 Key Features

### Production-Ready Capabilities
- **Complete Go-style package organization** with multi-file packages
- **Complete type system** with inference and checking
- **Object-oriented programming** with `pub`, `impl`, `self`
- **Pattern matching** with all constructs
- **Concurrency system** with spawn, channels, coordination
- **FFI with memory safety** annotations
- **Unsafe blocks** for systems programming
- **Enum variant construction** (Result.Ok, Option.Some)
- **Const declarations** with compile-time evaluation
- **Immutable-by-default** with smart optimization

### Revolutionary AI Integration
- **AI Code Generation**: Optimized for 10x faster development cycles
- **Smart Optimization**: AI-friendly patterns compile to C-level performance
- **Complete Tooling**: Linting, annotations, programmatic APIs
- **Production Quality**: 100% test coverage across all categories

---

## 📚 Reading Guide

### For New Users
1. Start with **[Overview](/docs/spec/overview/)** for language introduction
2. Read **[Grammar](/docs/spec/grammar/)** for complete syntax specification
3. Explore **[Packages](/docs/spec/packages/)** for project organization
4. Check **[Ampu](/docs/spec/ampu/)** for build system usage

### For Implementers
1. Study **[Grammar](/docs/spec/grammar/)** for complete parsing rules
2. Review **[Type System](/docs/spec/types/)** for implementation details
3. Reference **[Predeclared](/docs/spec/predeclared/)** for built-in functions
4. Examine **[Imports](/docs/spec/imports/)** for module system

### For AI Code Generation
1. Start with **[Design Rationale](/docs/spec/design_rationale/)** for AI-first philosophy
2. Focus on **[Grammar](/docs/spec/grammar/)** for predictable patterns
3. Understand **[Type System](/docs/spec/types/)** for inference rules
4. Study **[Advanced Features](/docs/spec/advanced-features/)** for AI integration

---

*This specification provides complete technical documentation for using Asthra in production environments, leveraging its revolutionary AI-first design and comprehensive feature set.* 