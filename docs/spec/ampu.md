# Ampu Build System

**Version:** 1.3  
**Last Updated:** 2024-12-28

The Asthra Modular Package Utility (`ampu`) is the official build tool and package manager for Asthra. It provides comprehensive project management, dependency resolution, and build orchestration capabilities designed specifically for AI-generated code and safe C interoperability.

## Core Features

```bash
# Project initialization
ampu init my-project          # Create new binary project
ampu init my-lib --lib        # Create new library project

# Build and compilation
ampu build                    # Build current project
ampu build --release          # Release build with optimizations
ampu build --parallel 4       # Parallel compilation with 4 threads

# Dependency management
ampu add github.com/user/repo # Add dependency
ampu update                   # Update all dependencies
ampu list --tree              # Show dependency tree
ampu vendor                   # Vendor dependencies locally

# Project maintenance
ampu clean                    # Clean build artifacts
ampu clean --all              # Clean everything including cache
ampu clean --profile debug    # Clean specific profile

# Documentation and analysis
ampu doc                      # Generate HTML documentation
ampu doc --format markdown    # Generate Markdown docs
ampu graph --format dot       # Generate dependency graph
ampu why package-name         # Explain why package is included

# Quality assurance
ampu check                    # Check code without building
ampu test                     # Run tests
```

## Project Structure

```
my-asthra-project/
├── asthra.toml              # Project manifest
├── src/                     # Source code (package main)
│   ├── main.asthra         # Main entry point
│   ├── types.asthra        # Data structures
│   ├── handlers.asthra     # Request handlers
│   └── utils.asthra        # Utility functions
├── crypto/                  # Crypto package
│   ├── hash.asthra         # Hashing functions
│   ├── encrypt.asthra      # Encryption functions
│   └── keys.asthra         # Key management
├── database/                # Database package
│   ├── connection.asthra   # Connection management
│   ├── queries.asthra      # Query builders
│   └── models.asthra       # Data models
├── tests/                   # Test files
│   ├── main_test.asthra    # Tests for main package
│   ├── crypto_test.asthra  # Black-box crypto tests
│   └── database_test.asthra # Black-box database tests
├── docs/                    # Generated documentation
│   ├── index.html          # HTML documentation
│   └── README.md           # Markdown API reference
├── target/                  # Build output
│   ├── debug/              # Debug builds
│   ├── release/            # Release builds
│   ├── deps/               # Dependency artifacts
│   └── cache/              # Build cache
└── vendor/                  # Vendored dependencies (optional)
```

## Configuration: `asthra.toml`

```toml
[package]
name = "my-project"
version = "1.0.0"
asthra_version = "1.3"
authors = ["AI Assistant <ai@asthra-lang.org>"]
description = "AI-generated Asthra project with Go-style packages"
license = "MIT"

[build]
target = "x86_64-linux"
library_type = "static"        # "static" or "dynamic"
parallel_jobs = 4
incremental = true

# Package organization is automatic - no need to list internal packages
# ampu discovers packages by directory structure:
# - src/ -> package main  
# - crypto/ -> package crypto
# - database/ -> package database

[dependencies]
# Asthra packages
"github.com/asthra-lang/json" = "1.0.0"
"github.com/asthra-lang/http" = "2.0.0"

# C library integration
liboqs = { 
    version = "0.8", 
    headers = ["oqs/oqs.h"], 
    libs = ["oqs"], 
    link_type = "dynamic" 
}
openssl = { 
    version = "3.0", 
    headers = ["openssl.h"], 
    libs = ["ssl", "crypto"], 
    link_type = "static" 
}

[features]
debug_mode = true
replay_recording = false
constant_time = true

[dev-dependencies]
"github.com/asthra-lang/test-framework" = "1.0.0"

[build-dependencies]
"github.com/asthra-lang/codegen" = "1.0.0"
```

## Library Storage Strategy

`ampu` implements a sophisticated library storage system optimized for incremental compilation and cross-platform compatibility:

```
target/
├── debug/                   # Debug profile artifacts
│   ├── libasthraproject.a  # Main library (Unix)
│   ├── asthraproject.lib   # Main library (Windows)
│   └── asthraproject       # Binary executable
├── release/                # Release profile artifacts
├── deps/                   # Dependency libraries
│   ├── libjson.a
│   ├── libhttp.a
│   └── metadata/           # Dependency metadata
├── cache/                  # Incremental compilation cache
│   ├── fingerprints/       # File change detection
│   └── build-stats.json   # Build performance metrics
└── external/               # External C libraries
    ├── include/            # Header files
    └── lib/                # Library files
```

## Incremental Compilation

`ampu` provides intelligent incremental compilation with package-aware optimization:

- **Package-level Change Detection:** SHA-256 fingerprinting of source files within packages
- **Cross-package Dependency Tracking:** Automatic rebuild when imported packages change
- **Package-parallel Builds:** Respects package dependency order while maximizing parallelism
- **Multi-file Package Optimization:** Changes within a package only recompile affected files

```bash
# Build with package-level statistics
ampu build --stats
# Output: 
# Package 'main': Cache hits: 3/4 files (75%), Build time: 1.2s
# Package 'crypto': Cache hits: 2/3 files (67%), Build time: 0.8s
# Package 'database': Cache hits: 2/2 files (100%), Build time: 0.0s
# Total build time: 2.0s

# Build specific package
ampu build --package crypto

# Force clean rebuild of specific package
ampu build --package crypto --no-cache
```

## AI-Optimized Features

`ampu` includes features specifically designed for AI code generation:

- **Deterministic Builds**: Reproducible compilation for AI debugging
- **Structured Error Reporting**: Machine-parseable error messages
- **Build Statistics**: Performance metrics for AI optimization
- **Parallel Safety**: Thread-safe operations for concurrent AI workflows
- **Cache Optimization**: Intelligent caching for iterative AI development

```bash
# AI-friendly error reporting
ampu check --format json
# Output: {"errors": [{"file": "src/main.asthra", "line": 10, "message": "..."}]}

# Build performance analysis
ampu build --profile --output build-profile.json
```

## Standard Library Auto-Discovery

The Asthra build system automatically provides access to the standard library without requiring manual configuration:

- **Automatic Path Resolution:** `stdlib/*` imports are resolved without manual setup
- **Always Available:** Standard library is included in the compiler search path by default
- **No Explicit Configuration:** Developers don't need to specify stdlib locations in `asthra.toml`

```asthra
// These work out of the box - no configuration needed
import "stdlib/string";
import "stdlib/asthra/core";
import "stdlib/collections";
import "stdlib/datetime/time";

fn main() -> i32 {
    // Standard library functions are immediately available
    let version: string = core.runtime_version();
    let trimmed: string = string.trim("  hello world  ");
    return 0;
}
```

This is in contrast to third-party packages which must be explicitly declared in `asthra.toml`:

```toml
[dependencies]
"github.com/user/package" = "1.0.0"  # Required for third-party imports
```

## C Library Integration

### FFI Configuration

```toml
[dependencies]
# Static linking example
openssl = { 
    version = "3.0", 
    headers = ["openssl/ssl.h", "openssl/crypto.h"], 
    libs = ["ssl", "crypto"], 
    link_type = "static",
    lib_dirs = ["/usr/local/lib"],
    include_dirs = ["/usr/local/include"]
}

# Dynamic linking example
liboqs = { 
    version = "0.8", 
    headers = ["oqs/oqs.h"], 
    libs = ["oqs"], 
    link_type = "dynamic",
    pkg_config = "liboqs"
}

# System library with pkg-config
zlib = {
    pkg_config = "zlib",
    link_type = "dynamic"
}
```

### FFI Error Handling

```asthra
// Convert C error codes to contextualized Results
fn c_operation_with_context() -> Result<Data, ErrorWithContext<i32>> {
    let c_result = unsafe { external_c_function() };
    
    if c_result < 0 {
        return Result.Err(ErrorWithContext {
            error: c_result,
            context: "C operation failed with code",
            source_file: __FILE__,
            line: __LINE__
        });
    }
    
    // Process successful result...
    return Result.Ok(process_c_result(c_result));
}
```

## Build System Integration

The `ampu` build system handles multi-file packages automatically:

```bash
# Compile entire project
ampu build

# Compile specific package
ampu build --package crypto

# Run tests for specific package
ampu test crypto

# Generate documentation for all packages
ampu doc

# List all packages in project
ampu list --packages
```

### Build Profiles

```bash
# Development builds (default)
ampu build                    # Debug symbols, no optimization
ampu build --profile dev      # Explicit development profile

# Release builds
ampu build --release          # Optimizations, no debug symbols
ampu build --profile release  # Explicit release profile

# Custom profiles (defined in asthra.toml)
ampu build --profile test     # Testing profile with coverage
ampu build --profile bench    # Benchmarking profile
```

### Parallel Compilation

```bash
# Automatic parallelism based on CPU cores
ampu build --parallel auto

# Explicit thread count
ampu build --parallel 8

# Disable parallelism for debugging
ampu build --parallel 1
```

## Dependency Management

### Adding Dependencies

```bash
# Add latest version
ampu add github.com/user/package

# Add specific version
ampu add github.com/user/package@1.2.3

# Add with features
ampu add github.com/user/package --features json,http

# Add development dependency
ampu add github.com/user/test-utils --dev
```

### Dependency Resolution

```bash
# Update dependencies
ampu update                   # Update all dependencies
ampu update package-name      # Update specific dependency

# Dependency information
ampu list                     # List direct dependencies
ampu list --tree              # Show dependency tree
ampu list --outdated          # Show outdated dependencies

# Dependency graph
ampu graph                    # Generate dependency graph
ampu graph --format svg       # Generate SVG graph
ampu why package-name         # Explain why package is included
```

### Vendoring

```bash
# Vendor all dependencies
ampu vendor

# Vendor specific dependencies
ampu vendor github.com/user/package

# Check vendored dependencies
ampu vendor --check

# Update vendored dependencies
ampu vendor --update
```

## Testing

### Test Organization

```bash
# Run all tests
ampu test

# Run tests for specific package
ampu test crypto

# Run specific test
ampu test crypto::test_sha256

# Run tests with pattern
ampu test --pattern "hash*"
```

### Test Configuration

```toml
[test]
parallel = true
timeout = 30
coverage = true
report_format = "json"

[test.env]
TEST_DATA_DIR = "test-data"
LOG_LEVEL = "debug"
```

## Documentation Generation

### HTML Documentation

```bash
# Generate HTML docs
ampu doc

# Open docs in browser
ampu doc --open

# Generate docs for specific package
ampu doc --package crypto
```

### Markdown Documentation

```bash
# Generate markdown API reference
ampu doc --format markdown

# Generate specific documentation
ampu doc --format markdown --package crypto --output crypto.md
```

## Quality Assurance

### Code Checking

```bash
# Check code without building
ampu check

# Check specific package
ampu check --package crypto

# Check with specific profile
ampu check --profile release
```

### Linting and Formatting

```bash
# Format code
ampu fmt

# Check formatting
ampu fmt --check

# Lint code
ampu lint

# Fix linting issues
ampu lint --fix
```

## Performance and Optimization

### Build Performance

```bash
# Profile build performance
ampu build --profile-build

# Show build statistics
ampu build --stats

# Analyze build times
ampu build --timings
```

### Cache Management

```bash
# Show cache information
ampu cache info

# Clean cache
ampu cache clean

# Validate cache
ampu cache validate
```

## Cross-Platform Support

### Target Platforms

```bash
# List available targets
ampu target list

# Build for specific target
ampu build --target x86_64-windows

# Cross-compilation setup
ampu target add aarch64-linux
```

### Platform-Specific Configuration

```toml
[target.x86_64-linux]
linker = "gcc"

[target.x86_64-windows]
linker = "x86_64-w64-mingw32-gcc"

[target.aarch64-apple-darwin]
linker = "clang"
```

## Integration with Development Tools

### IDE Integration

```bash
# Generate language server information
ampu lsp-info

# Generate compilation database
ampu compile-commands

# Export project information
ampu project-info --format json
```

### CI/CD Integration

```bash
# CI-friendly commands
ampu check --format json       # Machine-readable output
ampu test --format junit       # JUnit test results
ampu build --quiet             # Minimal output
```

## Troubleshooting

### Common Issues

```bash
# Verbose output for debugging
ampu build --verbose

# Very verbose output
ampu build -vv

# Debug build system
ampu build --debug

# Show environment information
ampu env
```

### Cache Issues

```bash
# Clean and rebuild
ampu clean && ampu build

# Reset cache
ampu cache clean --all

# Validate project state
ampu check --all
```

This comprehensive build system ensures that Asthra projects can be efficiently developed, tested, and deployed while maintaining the language's core principles of predictability, safety, and AI-friendliness. 
