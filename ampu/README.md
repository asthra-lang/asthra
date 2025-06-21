# Ampu - Asthra Build Tool

**Ampu** is the official build tool for the Asthra programming language, providing dependency management, compilation orchestration, and access control enforcement. Similar to how Cargo works for Rust, Ampu handles the complexities of building Asthra projects while maintaining the language's hierarchical import system with strict access controls.

## Features

### ✅ Core Functionality
- **Hierarchical Import System**: Supports `stdlib/*`, `internal/*`, and third-party imports
- **Access Control Enforcement**: Prevents user code from importing internal packages
- **Dependency Management**: Git-based package downloading and caching
- **Build Orchestration**: Calls the existing C-based `asthra` compiler
- **Cross-Platform Support**: Works on Windows, macOS, and Linux

### ✅ Advanced Features
- **Library Storage Strategy**: Efficient management of static/dynamic libraries
- **Incremental Compilation**: Smart caching to avoid unnecessary rebuilds
- **Parallel Compilation**: Multi-threaded builds where dependencies allow
- **Advanced Clean Options**: Selective cleaning of build artifacts
- **Documentation Generation**: HTML and Markdown documentation
- **Dependency Analysis**: Visualization and explanation tools

### ✅ Developer Tools
- **Project Initialization**: Quick setup of new Asthra projects
- **Dependency Vendoring**: Local copying of dependencies
- **Graph Generation**: Dependency visualization in multiple formats
- **Import Validation**: Real-time access control checking

## Installation

Ampu is distributed alongside the Asthra compiler. When you install Asthra, you get both binaries:

```bash
# After installing Asthra, both binaries are available
asthra --version  # C-based compiler
ampu --version    # Rust-based build tool
```

## Quick Start

### Initialize a New Project

```bash
# Create a new binary project
ampu init my-project
cd my-project

# Or create a library project
ampu init my-lib --lib
```

This creates the following structure:
```
my-project/
├── asthra.toml          # Project configuration
├── src/
│   └── main.asthra      # Main source file
└── .gitignore
```

### Project Configuration (`asthra.toml`)

```toml
[package]
name = "my-project"
version = "1.0.0"
asthra_version = "1.0"

[dependencies]
"github.com/asthra-lang/json" = "1.2.0"
"github.com/asthra-lang/http" = "2.1.0"

[dev-dependencies]
"github.com/asthra-lang/test" = "1.0.0"

[build]
target = "native"
optimization = "release"
```

### Basic Commands

```bash
# Build the project
ampu build

# Build and run
ampu run

# Run tests
ampu test

# Check imports without building
ampu check --access-control
```

## Command Reference

### Project Management

```bash
# Initialize new project
ampu init [NAME] [--lib]

# Add dependency
ampu add github.com/user/package [--dev] [--version 1.0.0]

# Remove dependency
ampu remove github.com/user/package [--dev]

# Update dependencies
ampu update [PACKAGE]

# List dependencies
ampu list [--tree]
```

### Building and Testing

```bash
# Build project
ampu build [--release] [--target TARGET] [--jobs N]

# Run project
ampu run [--release] [-- ARGS...]

# Run tests
ampu test [--release] [--no-capture] [TESTNAME]
```

### Cleaning and Maintenance

```bash
# Clean all build artifacts
ampu clean

# Clean specific profile
ampu clean --profile debug
ampu clean --profile release

# Clean only dependencies
ampu clean --deps

# Clean only build cache
ampu clean --cache

# Clean everything
ampu clean --all
```

### Analysis and Documentation

```bash
# Show dependency information
ampu deps [--tree] [--graph] [--format FORMAT]

# Explain why a package is included
ampu why github.com/user/package

# Generate dependency graph
ampu graph [--format dot|json|text] [--output FILE]

# Generate documentation
ampu doc [--open] [--format html|markdown] [--serve] [--port 8080]

# Vendor dependencies locally
ampu vendor
```

## Import System and Access Control

Ampu enforces Asthra's hierarchical import system with strict access controls:

### Import Types

1. **Standard Library** (`stdlib/*`)
   ```asthra
   import "stdlib/string";
   import "stdlib/asthra/core";
   ```

2. **Internal Packages** (`internal/*`) - Restricted access
   ```asthra
   import "internal/runtime/memory_allocator";  // ❌ User code cannot import
   ```

3. **Third-Party Packages**
   ```asthra
   import "github.com/user/package";
   import "gitlab.com/org/project";
   ```

4. **Local Imports**
   ```asthra
   import "./utils";
   import "../common";
   ```

### Access Control Rules

| Importing Package | Can Import |
|-------------------|------------|
| User Code | ✅ stdlib/*, third-party, local |
| User Code | ❌ internal/* |
| Standard Library | ✅ stdlib/*, internal/* |
| Internal Packages | ✅ stdlib/*, internal/* |
| Third-Party | ✅ stdlib/*, third-party, local |
| Third-Party | ❌ internal/* |

## Library Storage Strategy

Ampu uses a sophisticated storage strategy for managing build artifacts:

```
project_root/
├── asthra.toml
├── src/
├── target/                      # Build output directory
│   ├── debug/                   # Debug build artifacts
│   │   ├── deps/                # Dependency libraries
│   │   │   ├── libstdlib_string.a
│   │   │   └── external/        # External C libraries
│   │   ├── build/               # Intermediate build files
│   │   └── main                 # Final executable
│   ├── release/                 # Release build artifacts
│   └── cache/                   # Build cache and metadata
└── .asthra/                     # Global cache
    ├── cache/                   # Downloaded packages
    └── config.toml              # Global configuration
```

### Cross-Platform Library Support

- **Linux**: `.so` (dynamic), `.a` (static)
- **macOS**: `.dylib` (dynamic), `.a` (static)  
- **Windows**: `.dll` (dynamic), `.lib` (static)

## Advanced Features

### Incremental Compilation

Ampu automatically detects file changes and rebuilds only what's necessary:

```bash
# First build - compiles everything
ampu build

# Second build - only rebuilds changed files
ampu build  # Much faster!
```

### Parallel Compilation

Leverage multiple CPU cores for faster builds:

```bash
# Use all available cores
ampu build

# Limit parallel jobs
ampu build --jobs 4
```

### Documentation Generation

Generate beautiful documentation for your project:

```bash
# Generate HTML documentation
ampu doc --format html

# Generate and open in browser
ampu doc --open

# Start documentation server
ampu doc --serve --port 8080

# Generate Markdown documentation
ampu doc --format markdown
```

### Dependency Analysis

Understand your project's dependencies:

```bash
# Show dependency tree
ampu deps --tree

# Generate GraphViz dot file
ampu graph --format dot --output deps.dot

# Generate JSON dependency graph
ampu graph --format json

# Explain why a package is included
ampu why github.com/asthra-lang/json
```

### Vendoring Dependencies

Copy dependencies locally for offline builds:

```bash
# Vendor all dependencies to ./vendor/
ampu vendor

# Now you can build offline
ampu build
```

## Configuration

### Global Configuration (`~/.asthra/config.toml`)

```toml
[cache]
max_size_mb = 1024
max_age_days = 30

[build]
default_jobs = 8
default_target = "native"

[registry]
default = "https://packages.asthra-lang.org"
```

### Project Configuration (`asthra.toml`)

```toml
[package]
name = "my-project"
version = "1.0.0"
asthra_version = "1.0"
description = "A sample Asthra project"
license = "MIT"
authors = ["Your Name <you@example.com>"]

[dependencies]
"github.com/asthra-lang/json" = "1.2.0"

[dev-dependencies]
"github.com/asthra-lang/test" = "1.0.0"

[build]
target = "native"
optimization = "release"
debug_info = true

[workspace]
members = ["sub-project1", "sub-project2"]
```

## Integration with Asthra Compiler

Ampu orchestrates compilation by calling the existing C-based `asthra` compiler:

1. **Dependency Resolution**: Downloads and caches packages
2. **Import Validation**: Enforces access control rules
3. **Build Ordering**: Determines compilation order
4. **Compiler Invocation**: Calls `asthra` with proper arguments
5. **Library Management**: Handles static/dynamic library linking

## Error Handling

Ampu provides clear, actionable error messages:

```bash
$ ampu build
error: Import access violation
  --> src/main.asthra:3:8
   |
 3 | import "internal/runtime/memory_allocator";
   |        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ 
   |
   = note: User code cannot import internal packages
   = help: Use stdlib packages instead: "stdlib/memory"
```

## Performance

- **Fast Incremental Builds**: Only rebuilds changed files
- **Parallel Compilation**: Utilizes multiple CPU cores
- **Smart Caching**: Avoids redundant downloads and compilations
- **Efficient Storage**: Optimized library file organization

## Contributing

Ampu is part of the Asthra project. See the main Asthra repository for contribution guidelines.

## License

Ampu is licensed under the same terms as the Asthra programming language.

---

For more information, visit the [Asthra documentation](https://asthra-lang.org) or run `ampu --help` for command-specific help. 
