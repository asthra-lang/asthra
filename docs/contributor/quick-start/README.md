# Quick Start Guide

**Get contributing to Asthra in under 5 minutes!**

This guide gets you from zero to your first successful build and test run as quickly as possible. For complete setup instructions, see [Development Environment Setup](development-setup.md).

## Prerequisites Check

Before starting, verify you have:

```bash
# Check C17 compiler
clang --version    # Should be 6.0+ (preferred)
gcc --version      # Should be 7.0+ (alternative)

# Check Make
make --version     # Should be 4.0+

# Check Git
git --version      # Any recent version
```

**Platform Support:**
- ✅ **macOS**: Primary development platform
- ✅ **Linux**: Full support (Ubuntu, Debian, CentOS, Arch)
- ✅ **Windows**: Via MinGW or WSL2

## 5-Minute Setup

### Step 1: Clone Repository (30 seconds)

```bash
git clone https://github.com/asthra-lang/asthra.git
cd asthra
```

### Step 2: Build Compiler (2-3 minutes)

```bash
# Parallel build (recommended)
make -j$(nproc)

# Single-threaded build (if parallel fails)
make
```

**What's happening:**
- Compiling lexer, parser, semantic analysis, and code generation
- Building runtime system and standard library
- Creating development tools and test framework

**Expected output:**
```
Building Asthra Compiler v2.0.0...
[1/4] Compiling lexer and parser...
[2/4] Compiling semantic analysis...
[3/4] Compiling code generation...
[4/4] Linking asthra compiler...
Build complete: bin/asthra
```

### Step 3: Run Basic Tests (1 minute)

```bash
make test-basic
```

**Expected output:**
```
Running basic functionality tests...
✅ Lexer tests: 4/4 passed
✅ Parser tests: 3/3 passed  
✅ Semantic tests: 5/5 passed
✅ Integration tests: 3/3 passed
All basic tests passed!
```

### Step 4: Verify Installation (10 seconds)

```bash
./bin/asthra --version
```

**Expected output:**
```
Asthra Compiler v2.0.0
Build: production-ready
Features: AI-integration, concurrency, FFI, memory-safety
Platform: darwin-arm64 (or your platform)
```

## Success! 🎉

If you see the version output above, you've successfully:

- ✅ **Built the complete Asthra compiler** from source
- ✅ **Verified all core components** are working
- ✅ **Confirmed your development environment** is ready
- ✅ **Ready to make your first contribution**

## What's Next?

Choose your path based on your goals:

### 🚀 **Make Your First Contribution** (30 minutes)
→ **[First Contribution Walkthrough](first-contribution.md)**
- Find a good first issue
- Make a simple but meaningful change
- Submit your first pull request

### ⚡ **Understand the Codebase** (1-2 hours)
→ **[Development Environment Setup](development-setup.md)**
- Complete IDE configuration
- Learn the build system
- Understand the architecture

### 🔧 **Start Developing Features** (Ongoing)
→ **[../HANDBOOK.md#development-workflows](../HANDBOOK.md#development-workflows)**
- Learn development workflows
- Understand testing strategy
- Start contributing features

## Quick Commands Reference

```bash
# Essential build commands
make                    # Build everything
make clean             # Clean and rebuild
make test-curated      # Run high-reliability tests

# Development builds
make debug             # Debug build with symbols
make asan              # Memory safety checking
make release           # Optimized production build

# Specific testing
make test-lexer        # Test lexical analysis
make test-parser       # Test parsing
make test-semantic     # Test semantic analysis
make test-codegen      # Test code generation

# Get help
make help              # Show all available targets
make help-test         # Show testing options
```

## Troubleshooting

### Build Fails

**Missing compiler:**
```bash
# macOS
brew install llvm

# Linux (Ubuntu/Debian)
sudo apt-get install clang-12

# Set compiler
export CC=clang
```

**Missing dependencies:**
```bash
# Install development tools
make install-deps      # If available
```

**Parallel build issues:**
```bash
# Try single-threaded build
make clean && make -j1
```

### Tests Fail

**Environment issues:**
```bash
# Check compiler version
clang --version        # Needs C17 support

# Clean rebuild
make clean && make && make test-basic
```

**Platform-specific issues:**
```bash
# macOS: Install Xcode command line tools
xcode-select --install

# Linux: Install build essentials
sudo apt-get install build-essential

# Windows: Use WSL2 or MinGW
```

### Version Command Fails

**Binary not found:**
```bash
# Check if build completed
ls -la bin/asthra

# Check permissions
chmod +x bin/asthra
```

**Library issues:**
```bash
# Check dependencies
ldd bin/asthra          # Linux
otool -L bin/asthra     # macOS
```

## Getting Help

**Quick Questions:**
- Check [Common Issues](../troubleshooting/common-issues.md)
- Search [GitHub Issues](https://github.com/asthra-lang/asthra/issues)

**Real-time Help:**
- Discord: [Asthra Development](https://discord.gg/asthra-dev)
- GitHub Discussions: [Development Category](https://github.com/asthra-lang/asthra/discussions)

**Detailed Guides:**
- [Complete Development Setup](development-setup.md)
- [Troubleshooting Guide](../troubleshooting/README.md)
- [Contributor Handbook](../HANDBOOK.md)

---

**🎯 Goal Achieved: You're ready to contribute to Asthra!**

*Next: [Make your first contribution](first-contribution.md) or [set up your complete development environment](development-setup.md)* 