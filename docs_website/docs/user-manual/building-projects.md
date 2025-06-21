# Building Projects

This guide covers Asthra's project configuration, build system, and dependency management for creating robust applications.

## Enhanced Project Configuration (`Asthra.toml`)

The `Asthra.toml` file is the central configuration for Asthra projects, supporting comprehensive build settings, feature flags, and dependency management.

```toml
&#91;project&#93;
name = "my_asthra_app"
version = "1.0.0"
description = "A sample Asthra application"
authors = &#91;"Your Name <your.email@example.com>"&#93;
license = "MIT"

&#91;build&#93;
target = "x86_64-linux"
optimization_level = 2
output_name = "my_app"

&#91;features&#93;
debug_mode = true
replay_recording = false
pattern_matching_checks = true
string_operation_validation = true
slice_bounds_checking = true
constant_time_verification = true

&#91;dependencies&#93;
liboqs = " +  version = "0.8", headers = &#91;"oqs/oqs.h"&#93;, libs = &#91;"oqs"&#93;  + "
openssl = "3.0", headers = &#91;"openssl.h"&#93;, libs = &#91;"ssl", "crypto"&#93; }
sqlite3 = " +  version = "3.40", headers = &#91;"sqlite3.h"&#93;, libs = &#91;"sqlite3"&#93;  + "

&#91;dev_dependencies&#93;
test_framework = "1.0", headers = &#91;"test.h"&#93;, libs = &#91;"test"&#93; }

&#91;logging&#93;
default_level = "INFO"
categories = &#91;"DEBUG", "INFO", "WARNING", "ERROR", "VALIDATION_ERROR", "PROCESSING_ERROR"&#93;
output_format = "structured"

&#91;security&#93;
enable_constant_time_checks = true
enable_memory_sanitizer = false
enable_address_sanitizer = false
secure_memory_clearing = true

&#91;ffi&#93;
default_ownership = "borrowed"
enable_variadic_support = true
strict_type_checking = true
```

### Project Structure

A typical Asthra project follows this structure:

```
my_asthra_app/
├── Asthra.toml          # Project configuration
├── src/
│   ├── main.asthra      # Main application entry point
│   ├── lib.asthra       # Library code
│   └── modules/
│       ├── crypto.asthra
│       └── network.asthra
├── tests/
│   ├── unit_tests.asthra
│   └── integration_tests.asthra
├── docs/
│   └── README.md
├── build/               # Build artifacts (generated)
└── deps/                # Dependencies (generated)
```

### Package-Based Project Structure

Asthra's **Go-style package system** organizes code into directory-based packages. Here are recommended structures for different project sizes:

#### Small Project (Single Package)
```
calculator/
├── Asthra.toml
└── src/
    ├── main.asthra        # package main
    ├── operations.asthra  # package main
    └── utils.asthra       # package main
```

#### Medium Project (Multi-Package)
```
web-service/
├── Asthra.toml
├── src/
│   └── main.asthra      # package main
├── handlers/
│   ├── user.asthra      # package handlers
│   ├── auth.asthra      # package handlers
│   └── api.asthra       # package handlers
├── models/
│   ├── user.asthra      # package models
│   └── session.asthra   # package models
├── database/
│   ├── connection.asthra # package database
│   └── queries.asthra   # package database
├── utils/
│   ├── logging.asthra   # package utils
│   └── config.asthra    # package utils
└── tests/
    ├── handlers_test.asthra # package handlers_test
    ├── models_test.asthra   # package models_test
    └── integration_test.asthra # package integration_test
```

#### Package Declaration Rules for Build System

The build system automatically detects packages based on directory structure:

1\. **Each directory = one package**: All `.asthra` files in a directory must declare the same package name
2\. **Package name must match directory**: Directory `models/` should contain `package models;`
3\. **Test files**: Use `_test.asthra` suffix for test files (e.g., `user_test.asthra`)
4\. **Main package**: The `src/` directory typically contains the `main` package

#### Build System Integration

The package system integrates seamlessly with the build system:

```bash
# Build all packages in project
asthra build

# Build specific package
asthra build --package models

# Build with package dependency order
asthra build --check-deps

# List all packages in project
asthra list-packages

# Show package dependency graph
asthra deps-graph
```

**For complete package organization guidance, see &#91;Package Organization&#93;(package-organization.md)**

## Build Commands

Asthra provides a comprehensive set of build commands for different development scenarios:

### Basic Build Commands

```bash
# Standard build
asthra build

# Build with specific target
asthra build --target x86_64-linux

# Build with optimization
asthra build --release

# Build with debugging information
asthra build --debug
```

### Enhanced Build Options

```bash
# Build with enhanced debugging
asthra build --debug --enable-pattern-matching-checks

# Build with replay recording
asthra build --replay=record

# Build with comprehensive validation
asthra build --enable-all-checks

# Build with security features
asthra build --security-mode=strict

# Build with specific optimization level
asthra build --opt-level=3
```

### Development Builds

```bash
# Fast development build (minimal optimization)
asthra build --dev

# Build with hot reload support
asthra build --watch

# Build with verbose output
asthra build --verbose

# Build with timing information
asthra build --time
```

### Cross-Compilation

```bash
# Build for different architectures
asthra build --target x86_64-windows
asthra build --target aarch64-linux
asthra build --target x86_64-macos

# List available targets
asthra build --list-targets
```

## Package Management

Asthra's package management system handles both Asthra libraries and C library dependencies with comprehensive FFI support.

### Adding Dependencies

```bash
# Add C library dependency
asthra add liboqs --headers="oqs/oqs.h" --libs="oqs"

# Add C library with specific version
asthra add openssl --version="3.0" --headers="openssl/ssl.h,openssl/crypto.h" --libs="ssl,crypto"

# Add Asthra library dependency
asthra add asthra_crypto --version="1.2.0"

# Add development dependency
asthra add test_framework --dev --version="1.0"
```

### Managing Dependencies

```bash
# Update all dependencies
asthra update

# Update specific dependency
asthra update openssl

# Remove dependency
asthra remove old_library

# List all dependencies
asthra list

# Show dependency tree
asthra tree
```

### Dependency Resolution

```bash
# Verify FFI compatibility
asthra check-ffi

# Resolve dependency conflicts
asthra resolve

# Clean dependency cache
asthra clean-deps

# Rebuild dependencies
asthra rebuild-deps
```

## Advanced Build Configuration

### Custom Build Scripts

Create a `build.asthra` file for custom build logic:

```asthra
// build.asthra - Custom build script
package build;

pub fn pre_build() -&gt; Result&lt;void, string&gt; {log("BUILD", "Running pre-build steps");
    
    // Generate code from templates
    let codegen_result: Result&lt;void, string&gt; = generate_ffi_bindings();
    match codegen_result " + 
        Result.Err(error) =&gt; return Result.Err("Code generation failed: " + error),
        Result.Ok(_) =&gt; log("BUILD", "Code generation completed")
     + "
    
    return Result.Ok(void);
 + "

pub fn post_build() -&gt; Result&lt;void, string&gt; "BUILD", "Running post-build steps");
    
    // Copy resources
    let copy_result: Result&lt;void, string&gt; = copy_resources();
    match copy_result " + 
        Result.Err(error) =&gt; return Result.Err("Resource copying failed: " + error),
        Result.Ok(_) =&gt; log("BUILD", "Resources copied successfully")
     + "
    
    return Result.Ok(void);
}
```

### Conditional Compilation

Use feature flags for conditional compilation:

```asthra
// Conditional compilation based on features
#&#91;cfg(feature = "debug_mode")&#93;
pub fn debug_function() -&gt; void {log("DEBUG", "Debug mode is enabled");
 + "

#&#91;cfg(feature = "crypto_support")&#93;
pub fn crypto_function() -&gt; Result&lt;&#91;&#93;u8, string&gt; {return generate_crypto_key(32);
 + "

#&#91;cfg(not(feature = "minimal_build"))&#93;
pub fn full_feature_function() -&gt; Result&lt;void, string&gt; {// Full feature implementation
    return Result.Ok(void);
 + "
```

### Build Profiles

Define different build profiles in `Asthra.toml`:

```toml
&#91;profiles.dev&#93;
optimization_level = 0
debug_info = true
assertions = true
bounds_checking = true

&#91;profiles.release&#93;
optimization_level = 3
debug_info = false
assertions = false
bounds_checking = false
lto = true

&#91;profiles.security&#93;
optimization_level = 2
debug_info = true
constant_time_checks = true
memory_sanitizer = true
secure_compilation = true

&#91;profiles.embedded&#93;
optimization_level = "size"
no_std = true
minimal_runtime = true
static_linking = true
```

Use profiles with:

```bash
asthra build --profile dev
asthra build --profile release
asthra build --profile security
asthra build --profile embedded
```

## Testing Integration

### Test Configuration

```toml
&#91;test&#93;
framework = "asthra_test"
parallel = true
timeout = 30
coverage = true

&#91;test.unit&#93;
pattern = "tests/unit_*.asthra"
timeout = 10

&#91;test.integration&#93;
pattern = "tests/integration_*.asthra"
timeout = 60
requires_network = true
```

### Running Tests

```bash
# Run all tests
asthra test

# Run specific test file
asthra test tests/unit_crypto.asthra

# Run tests with coverage
asthra test --coverage

# Run tests in parallel
asthra test --parallel

# Run tests with verbose output
asthra test --verbose
```

### Test Example

```asthra
// tests/unit_crypto.asthra
package test_crypto;

import "test_framework";
import "../src/crypto";

pub fn test_key_generation() -&gt; Result&lt;void, string&gt; {let key_result: Result&lt;&#91;&#93;u8, string&gt; = crypto.generate_key(32);
    
    match key_result " + 
        Result.Ok(key) =&gt; {
            test_framework.assert_eq(key.len, 32, "Key should be 32 bytes");
            test_framework.assert_ne(key, &#91;0; 32&#93;, "Key should not be all zeros");
            return Result.Ok(void);
         + ",
        Result.Err(error) =&gt; "Key generation failed: " + error);
         + "
    }
}

pub fn test_secure_compare() -&gt; Result&lt;void, string&gt; {let key1: &#91;&#93;u8 = &#91;1, 2, 3, 4&#93;;
    let key2: &#91;&#93;u8 = &#91;1, 2, 3, 4&#93;;
    let key3: &#91;&#93;u8 = &#91;1, 2, 3, 5&#93;;
    
    let compare1: Result&lt;bool, string&gt; = crypto.secure_compare(key1, key2);
    let compare2: Result&lt;bool, string&gt; = crypto.secure_compare(key1, key3);
    
    match (compare1, compare2) {
        (Result.Ok(true), Result.Ok(false)) =&gt; {
            return Result.Ok(void);
         + ",
        _ =&gt; " + 
            return Result.Err("Secure compare test failed");
         + "
    }
}
```

## Deployment and Distribution

### Creating Releases

```bash
# Build release version
asthra build --release

# Create distribution package
asthra package

# Create installer
asthra package --installer

# Create static binary
asthra build --release --static
```

### Docker Integration

Create a `Dockerfile` for containerized deployment:

```dockerfile
FROM asthra:1.2-alpine

WORKDIR /app
COPY Asthra.toml .
COPY src/ src/

RUN asthra build --release --static

FROM alpine:latest
RUN apk --no-cache add ca-certificates
WORKDIR /root/
COPY --from=0 /app/build/my_app .
CMD &#91;"./my_app"&#93;
```

### CI/CD Integration

Example GitHub Actions workflow:

```yaml
# .github/workflows/build.yml
name: Build and Test

on: &#91;push, pull_request&#93;

jobs:
  build:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v2
    
    - name: Install Asthra
      run: |
        curl -L https://example.com/asthra/install.sh | sh
        echo "$HOME/.asthra/bin" &gt;&gt; $GITHUB_PATH
    
    - name: Build
      run: asthra build --release
    
    - name: Test
      run: asthra test --coverage
    
    - name: Security Check
      run: asthra build --profile security
    
    - name: Package
      run: asthra package
```

## Annotation Processing in Build System

The Asthra build system can process annotations for various purposes including documentation generation, static analysis, and review workflows.

### Build Configuration for Annotations

```toml
# Asthra.toml
&#91;build&#93;
process_annotations = true
generate_review_reports = true
fail_on_high_priority_review = false

&#91;annotations&#93;
human_review_output = "review-report.json"
security_analysis = true
generate_documentation = true

&#91;review_workflow&#93;
high_priority_blocking = true
medium_priority_warning = true
context_specific_routing = true
```

### Review Report Generation

The build system can automatically extract and process human review annotations:

```bash
# Generate review report
asthra build --generate-review-report

# Extract specific priority levels
asthra analyze --annotations --priority high,medium

# Context-specific analysis
asthra analyze --annotations --context crypto,security
```

### CI/CD Integration

```yaml
# .github/workflows/review.yml
name: Code Review Analysis

on: &#91;push, pull_request&#93;

jobs:
  review-analysis:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Install Asthra
        run: |
          curl -L https://releases.asthra-lang.org/latest/asthra-linux.tar.gz | tar -xz
          sudo mv asthra /usr/local/bin/
      
      - name: Extract Review Annotations
        run: |
          asthra analyze --annotations --output review-report.json
          
      - name: Check High Priority Items
        run: |
          if asthra analyze --annotations --priority high --count &gt; 0; then
            echo "High priority review items found - manual review required"
            exit 1
          fi
          
      - name: Generate Review Report
        run: |
          asthra review-report --format markdown --output review-summary.md
          
      - name: Upload Review Report
        uses: actions/upload-artifact@v3
        with:
          name: review-report
          path: review-summary.md
```

### Integration with Development Tools

- **IDE Plugins**: Highlight annotated functions with priority indicators
- **Static Analysis**: Process annotations for security and safety analysis
- **Documentation Generation**: Include annotation metadata in generated docs
- **Code Review Tools**: Automatically flag high-priority items for human review

## Performance Optimization

### Build-Time Optimization

```bash
# Link-time optimization
asthra build --release --lto

# Profile-guided optimization
asthra build --release --pgo

# Size optimization
asthra build --release --optimize-size

# Speed optimization
asthra build --release --optimize-speed
```

### Dependency Optimization

```toml
&#91;optimization&#93;
strip_unused_code = true
merge_duplicate_strings = true
optimize_ffi_calls = true
inline_small_functions = true

&#91;linking&#93;
static_linking = false
dynamic_linking = true
strip_symbols = true
compress_binary = true
```

## Troubleshooting Build Issues

### Common Build Problems

```bash
# Clean build artifacts
asthra clean

# Rebuild everything from scratch
asthra clean &amp;&amp; asthra build

# Verbose build output
asthra build --verbose

# Check dependency issues
asthra check-deps

# Verify FFI bindings
asthra check-ffi --verbose
```

### Debug Build Issues

```bash
# Build with maximum debugging
asthra build --debug --verbose --keep-temps

# Check for missing dependencies
asthra build --check-missing

# Validate project configuration
asthra validate

# Show build environment
asthra env
```

### Error Handling in Build Scripts

```asthra
pub fn robust_build_step() -&gt; Result&lt;void, string&gt; {let step_result: Result&lt;void, string&gt; = perform_build_step();
    
    match step_result {
        Result.Ok(_) =&gt; {
            log("BUILD", "Build step completed successfully");
            return Result.Ok(void);
         + ",
        Result.Err(error) =&gt; "BUILD_ERROR", "Build step failed: " + error + "");
            
            // Attempt recovery
            let recovery_result: Result&lt;void, string&gt; = attempt_recovery();
            match recovery_result " + 
                Result.Ok(_) =&gt; {
                    log("BUILD", "Recovery successful, retrying build step");
                    return perform_build_step();
                 + ",
                Result.Err(recovery_error) =&gt; "Build failed and recovery failed: " + recovery_error);
                }
            }
        }
    }
}
```

## Best Practices

### Project Organization

1\. **Use clear directory structure** - Separate source, tests, and documentation
2\. **Configure appropriate build profiles** - Different settings for dev/release/security
3\. **Manage dependencies carefully** - Pin versions for reproducible builds
4\. **Use feature flags** - Enable conditional compilation for different targets
5\. **Document build requirements** - Clear instructions for setting up the build environment

### Build Performance

1\. **Use incremental builds** - Only rebuild changed components
2\. **Optimize dependency resolution** - Cache dependencies when possible
3\. **Parallel compilation** - Use multiple cores for faster builds
4\. **Profile build times** - Identify and optimize slow build steps
5\. **Clean builds regularly** - Prevent stale artifacts from causing issues

### Security Considerations

1\. **Verify dependency integrity** - Check checksums and signatures
2\. **Use security build profiles** - Enable additional security checks
3\. **Audit dependencies regularly** - Check for known vulnerabilities
4\. **Secure build environment** - Use trusted build systems and containers
5\. **Sign release artifacts** - Provide authenticity verification

## Next Steps

Now that you understand building projects, explore:

- **&#91;Advanced Topics&#93;(advanced-topics.md)** - Learn about AI-assisted development workflows
- **&#91;Security Features&#93;(security.md)** - Understand security considerations in builds
- **&#91;C Interoperability&#93;(ffi-interop.md)** - Configure C library dependencies effectively 
