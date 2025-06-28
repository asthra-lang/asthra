---
layout: page
title: Documentation
permalink: /docs/
---

# Asthra Programming Language Documentation

Welcome to the comprehensive documentation for Asthra! Whether you're just getting started or diving deep into advanced features, you'll find everything you need here.

## 📚 Documentation Sections

### [🚀 User Manual](/docs/user-manual/)
Complete guide for developers using Asthra, covering everything from basic syntax to advanced features.

- **[Getting Started](/docs/user-manual/getting-started/)** - Your first steps with Asthra
- **[Language Fundamentals](/docs/user-manual/language-fundamentals/)** - Core concepts and syntax
- **[Building Projects](/docs/user-manual/building-projects/)** - Project structure and build system
- **[Advanced Topics](/docs/user-manual/advanced-topics/)** - Concurrency, memory management, and more
- **[Examples](/docs/user-manual/examples/)** - Practical code examples and tutorials

### [📖 Language Specification](/docs/spec/)
Formal specification and technical details of the Asthra language.

- **[Overview](/docs/spec/overview/)** - Language design and philosophy
- **[Grammar](/docs/spec/grammar/)** - Complete language grammar
- **[Type System](/docs/spec/types/)** - Type definitions and rules
- **[Package System](/docs/spec/packages/)** - Module and package organization

### [🔧 Standard Library](/docs/stdlib/)
Documentation for Asthra's built-in modules and functions.

- **[Core Modules](/docs/stdlib/modules/)** - Essential built-in functionality
- **[Collections](/docs/stdlib/collections_design/)** - Arrays, maps, and data structures
- **[Error Handling](/docs/stdlib/02-error-handling/)** - Result types and error management
- **[Best Practices](/docs/stdlib/best-practices/)** - Recommended patterns and conventions

### [🤝 Contributor Guide](/docs/contributor/)
Resources for contributing to the Asthra language and ecosystem.

- **[Quick Start](/docs/contributor/quick-start/)** - Get started contributing
- **[Developer Handbook](/docs/contributor/HANDBOOK/)** - Comprehensive contributor guide
- **[Development Workflows](/docs/contributor/workflows/)** - Build, test, and release processes
- **[Reference](/docs/contributor/reference/)** - Technical reference for contributors

---

## Quick Start Guide

### Installation

#### Using the installer script (Recommended)

```bash
curl -sSf https://install.asthra-lang.org | sh
```

#### Package Managers

**Homebrew (macOS/Linux):**
```bash
brew install asthra-lang/tap/asthra
```

**Note:** Asthra is designed for Unix-like systems (macOS and Linux) only.


### Your First Program

Create `hello.asthra`:

```asthra
func main() {
    print("Hello, Asthra!")
}
```

Run it:

```bash
asthra run hello.asthra
```

### Language Basics

```asthra
// Variables
let name = "Asthra"
mut counter = 0

// Functions
func greet(name: String) -> String {
    return "Hello, ${name}!"
}

// Data structures
struct User {
    name: String
    age: Int
}

// Pattern matching
match result {
    Ok(value) => print("Success: ${value}"),
    Err(error) => print("Error: ${error}")
}
```

---

## Getting Help

- **[GitHub Discussions](https://github.com/yourusername/asthra-lang/discussions)** - Ask questions and share ideas
- **[GitHub Issues](https://github.com/yourusername/asthra-lang/issues)** - Report bugs and request features
- **[Discord Server](https://discord.gg/asthra)** - Real-time chat with the community

---

*Happy coding with Asthra! 🚀* 