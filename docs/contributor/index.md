---
layout: page
title: Contributor Guide
permalink: /docs/contributor/
---

# Asthra Contributor Guide

**Version:** 1.0  
**Date:** January 2025  
**Status:** Active Development  

Welcome to the **Asthra Contributor Guide**! This comprehensive resource will help you contribute to the Asthra programming language compiler and ecosystem, from your first contribution to becoming a core maintainer.

## 🚀 What is Asthra?

Asthra is a modern systems programming language designed specifically for **AI-friendly code generation** while maintaining the performance and safety requirements of systems programming. Built through AI-assisted development, Asthra represents a new paradigm in programming language design.

### What Makes Asthra Special
- **AI-First Design**: Every language feature optimized for AI code generation
- **Memory Safety**: Four-zone memory model with automatic safety guarantees
- **Concurrency**: Three-tier concurrency system from deterministic to advanced
- **FFI Safety**: Automatic C interoperability with transfer semantics
- **Zero-Cost Abstractions**: High-level features with C-level performance

---

## 📊 Project Status

Asthra is in **active development** with a production-ready compiler featuring:
- ✅ Complete compilation pipeline (Lexer → Parser → Semantic → CodeGen)
- ✅ 176 tests across 30 categories with 100% success rate
- ✅ Full AI integration (annotations, linting, API)
- ✅ Advanced features (concurrency, FFI, memory management, OOP)
- ✅ Rich standard library (JSON, HTTP, channels, coordination)

---

## 🎯 How You Can Contribute

### Code Contributions
- **Compiler improvements** and bug fixes
- **New language features** and optimizations
- **Standard library modules** and utilities
- **Testing and quality assurance** improvements

### Documentation
- **User guides** and tutorials
- **API documentation** and references
- **Architecture documentation** and design docs
- **Translation** and localization

### Community
- **Issue triage** and user support
- **Code review** and mentoring
- **Community building** and outreach
- **Educational content** creation

---

## 📚 Contributor Resources

### Getting Started

#### [Quick Start Guide](/docs/contributor/quick-start/)
Get up and running with Asthra development in under 5 minutes.

#### [Developer Handbook](/docs/contributor/HANDBOOK/)
Comprehensive guide covering everything from setup to advanced development.

### Development Workflows

#### [Development Workflows](/docs/contributor/workflows/)
Build processes, testing procedures, and release management.

#### [Troubleshooting Guide](/docs/contributor/troubleshooting/)
Common issues, solutions, and debugging techniques.

### Technical Reference

#### [Architecture Reference](/docs/contributor/reference/)
Detailed technical documentation of compiler architecture and design.

#### [Development Guides](/docs/contributor/guides/)
In-depth guides for specific development tasks and features.

---

## 🚀 Quick Start (5 Minutes)

### Prerequisites
- **C17 Compiler**: Clang 6+
- **Make**: GNU Make 4.0+
- **Git**: For version control
- **Platform**: macOS, Linux, or Windows (with MinGW)

### Setup Commands

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

---

## 🎓 Learning Paths

Choose your starting point based on your experience and goals:

### 🚀 Beginner Path
Perfect for your first contribution to Asthra:
1. **[Quick Start](/docs/contributor/quick-start/)** - 5-minute setup
2. **[First Contribution](/docs/contributor/quick-start/)** - Step-by-step walkthrough
3. **[Development Environment](/docs/contributor/quick-start/)** - Complete setup guide

### ⚡ Intermediate Path
For understanding compiler architecture:
1. **[Developer Handbook](/docs/contributor/HANDBOOK/)** - Comprehensive guide
2. **[Architecture Reference](/docs/contributor/reference/)** - Technical deep-dive
3. **[Development Workflows](/docs/contributor/workflows/)** - Build and test processes

### 🔧 Advanced Path
For adding new language features:
1. **[Advanced Development](/docs/contributor/guides/)** - Feature development
2. **[Compiler Internals](/docs/contributor/reference/)** - Deep technical knowledge
3. **[Performance Optimization](/docs/contributor/guides/)** - Advanced techniques

### 🏆 Expert Path
For core maintainer responsibilities:
1. **[Maintainer Guide](/docs/contributor/reference/)** - Leadership responsibilities
2. **[Release Management](/docs/contributor/workflows/)** - Release processes
3. **[Community Leadership](/docs/contributor/guides/)** - Community building

---

## 🔧 Development Environment

### Recommended Setup

**IDE Configuration:**
```bash
# VS Code (Recommended)
code --install-extension ms-vscode.cpptools
code --install-extension ms-vscode.makefile-tools

# Configure for Asthra development
cp .vscode/settings.json.template .vscode/settings.json
```

**Build Commands:**
```bash
# Core build commands
make                    # Build everything
make clean             # Clean build artifacts
make -j8               # Parallel build (8 jobs)

# Development builds
make debug             # Debug build with symbols
make asan              # AddressSanitizer build
make release           # Optimized release build

# Testing
make test              # Run all tests
make test-lexer        # Run specific category tests
make test-curated      # Run high-reliability tests
```

---

## 🤝 Community & Support

### Getting Help
- **[GitHub Discussions](https://github.com/asthra-lang/asthra/discussions)** - Ask questions and share ideas
- **[GitHub Issues](https://github.com/asthra-lang/asthra/issues)** - Report bugs and request features
- **[Discord Server](https://discord.gg/asthra)** - Real-time chat with contributors

### Code of Conduct
We are committed to providing a welcoming and inclusive environment for all contributors. Please read our [Code of Conduct](https://github.com/asthra-lang/asthra/blob/main/CODE_OF_CONDUCT.md) before participating.

### Recognition
We value all contributions and maintain a [Contributors Hall of Fame](https://github.com/asthra-lang/asthra/blob/main/CONTRIBUTORS.md) to recognize community members who help make Asthra better.

---

*Join us in building the future of AI-friendly programming languages! Every contribution, no matter how small, helps make Asthra better for developers worldwide.* 