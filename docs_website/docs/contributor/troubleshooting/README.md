# Troubleshooting Guide

**Comprehensive problem-solving guide for Asthra development**

This guide helps you diagnose and fix common issues encountered during Asthra development. For quick solutions, check &#91;Common Issues&#93;(common-issues.md).

## Table of Contents

1\. &#91;Quick Diagnosis&#93;(#quick-diagnosis)
2\. &#91;Build Issues&#93;(#build-issues)
3\. &#91;Test Failures&#93;(#test-failures)
4\. &#91;Runtime Problems&#93;(#runtime-problems)
5\. &#91;IDE and Editor Issues&#93;(#ide-and-editor-issues)
6\. &#91;Platform-Specific Issues&#93;(#platform-specific-issues)
7\. &#91;Performance Problems&#93;(#performance-problems)
8\. &#91;Getting Help&#93;(#getting-help)

## Quick Diagnosis

### Health Check Script

Run this comprehensive health check to identify common issues:

```bash
#!/bin/bash
# Save as scripts/health_check.sh

echo "=== Asthra Development Environment Health Check ==="

# Check basic tools
echo "1. Checking basic tools..."
command -v clang &gt;/dev/null 2>&amp;1 &amp;&amp; echo "✅ clang found" || echo "❌ clang missing"
command -v make &gt;/dev/null 2>&amp;1 &amp;&amp; echo "✅ make found" || echo "❌ make missing"
command -v git &gt;/dev/null 2>&amp;1 &amp;&amp; echo "✅ git found" || echo "❌ git missing"

# Check compiler version
echo -e "\n2. Checking compiler version..."
clang --version | head -1

# Check environment variables
echo -e "\n3. Checking environment..."
echo "CC: ${CC:-not set}"
echo "CXX: ${CXX:-not set}"
echo "ASTHRA_DEBUG: ${ASTHRA_DEBUG:-not set}"

# Check build status
echo -e "\n4. Checking build status..."
if &#91; -f "bin/asthra" &#93;; then
    echo "✅ Asthra binary exists"
    ./bin/asthra --version 2>/dev/null &amp;&amp; echo "✅ Binary runs" || echo "❌ Binary fails to run"
else
    echo "❌ Asthra binary missing - run 'make' to build"
fi

# Check test status
echo -e "\n5. Quick test check..."
if make test-basic &gt;/dev/null 2>&1; then
    echo "✅ Basic tests pass"
else
    echo "❌ Basic tests fail"
fi

echo -e "\n=== Health Check Complete ==="
```

### Quick Fixes

**Most Common Issues:**
```bash
# 1. Clean rebuild (fixes 80% of build issues)
make clean &amp;&amp; make -j$(nproc)

# 2. Reset environment (fixes environment issues)
unset ASTHRA_DEBUG ASTHRA_VERBOSE
export CC=clang
export CXX=clang++

# 3. Update dependencies (fixes missing library issues)
# macOS:
brew update &amp;&amp; brew upgrade
# Linux:
sudo apt update &amp;&amp; sudo apt upgrade

# 4. Check disk space (fixes mysterious failures)
df -h .
```

## Build Issues

### Compilation Errors

#### Missing Compiler

**Symptoms:**
```
make: clang: command not found
```

**Solutions:**
```bash
# macOS
brew install llvm
export CC=/opt/homebrew/bin/clang  # Apple Silicon
export CC=/usr/local/bin/clang     # Intel Mac

# Linux (Ubuntu/Debian)
sudo apt install clang-12
sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-12 100

# Linux (CentOS/RHEL)
sudo yum install clang

# Verify installation
clang --version
```

#### Wrong Compiler Version

**Symptoms:**
```
error: unknown argument: '-std=c17'
```

**Solutions:**
```bash
# Check current version
clang --version

# Minimum required: Clang 6.0+ or GCC 7.0+
# Update if necessary:

# macOS
brew upgrade llvm

# Linux
sudo apt install clang-12  # or latest available

# Set specific version
export CC=clang-12
export CXX=clang++-12
```

#### Missing Headers

**Symptoms:**
```
fatal error: 'stdio.h' file not found
fatal error: 'json-c/json.h' file not found
```

**Solutions:**
```bash
# System headers missing
# macOS:
xcode-select --install

# Linux:
sudo apt install libc6-dev build-essential

# Third-party headers missing
# macOS:
brew install json-c pkg-config

# Linux:
sudo apt install libjson-c-dev pkg-config

# Verify pkg-config can find libraries
pkg-config --cflags --libs json-c
```

#### Linker Errors

**Symptoms:**
```
undefined reference to `json_object_new_string'
ld: library not found for -ljson-c
```

**Solutions:**
```bash
# Check library installation
# macOS:
brew list json-c
ls /opt/homebrew/lib/libjson-c*  # Apple Silicon
ls /usr/local/lib/libjson-c*     # Intel Mac

# Linux:
dpkg -l | grep json-c
ldconfig -p | grep json-c

# Install if missing
# macOS:
brew install json-c

# Linux:
sudo apt install libjson-c-dev

# Force system library usage
export JSON_C_SYSTEM=1
make clean &amp;&amp; make
```

### Build System Issues

#### Makefile Errors

**Symptoms:**
```
make: *** No rule to make target 'test-lexer'
make: *** No targets specified and no makefile found
```

**Solutions:**
```bash
# Ensure you're in the project root
pwd  # Should end with 'asthra'
ls Makefile  # Should exist

# Check for corrupted Makefile
git status
git checkout -- Makefile  # Reset if modified

# Update from upstream
git fetch upstream
git merge upstream/main

# Verify make version
make --version  # Should be 4.0+
```

#### Parallel Build Issues

**Symptoms:**
```
make: *** &#91;target&#93; Error 2
# Works with make -j1 but fails with make -j8
```

**Solutions:**
```bash
# Use single-threaded build for debugging
make clean &amp;&amp; make -j1

# Check for race conditions in dependencies
make clean &amp;&amp; make -j1 2>&amp;1 | tee build.log

# Reduce parallelism
make -j4  # Instead of -j8

# Check system resources
free -h  # Memory
df -h    # Disk space
```

## Test Failures

### Environment-Related Test Failures

#### Test Framework Issues

**Symptoms:**
```
test_lexer_main: command not found
Segmentation fault in test runner
```

**Solutions:**
```bash
# Rebuild test framework
make clean-tests &amp;&amp; make test-framework

# Check test binary permissions
ls -la tests/*/test_*_main
chmod +x tests/*/test_*_main

# Run with debug output
export ASTHRA_TEST_VERBOSE=1
make test-lexer

# Check for missing dependencies
ldd tests/lexer/test_lexer_main  # Linux
otool -L tests/lexer/test_lexer_main  # macOS
```

#### Memory-Related Test Failures

**Symptoms:**
```
AddressSanitizer: heap-buffer-overflow
Valgrind: Invalid read/write
```

**Solutions:**
```bash
# Build with AddressSanitizer
make clean &amp;&amp; make asan

# Run specific failing test
export ASAN_OPTIONS=abort_on_error=1:halt_on_error=1
./tests/semantic/test_semantic_main

# Use Valgrind for detailed analysis
make clean &amp;&amp; make debug
valgrind --tool=memcheck --leak-check=full \
         --show-leak-kinds=all --track-origins=yes \
         ./tests/semantic/test_semantic_main

# Check for common issues
# - Uninitialized variables
# - Buffer overflows
# - Memory leaks
# - Use after free
```

### Category-Specific Test Failures

#### Lexer Test Failures

**Common Issues:**
```bash
# Token recognition issues
# Check keyword vs identifier conflicts
grep -r "log" src/parser/keyword.c

# String literal parsing issues
# Check escape sequence handling
./bin/asthra -c 'let s = "test\n";'

# Comment parsing issues
# Check nested comment support
./bin/asthra -c '/* outer /* inner */ */'
```

#### Parser Test Failures

**Common Issues:**
```bash
# Grammar compliance issues
# Check for v1.22 compliance
./scripts/validate_grammar.py examples/

# AST node creation issues
# Enable parser debugging
export ASTHRA_PARSER_DEBUG=1
./bin/asthra test.asthra

# Memory management in parser
make asan &amp;&amp; ./bin/asthra test.asthra
```

#### Semantic Test Failures

**Common Issues:**
```bash
# Symbol table issues
# Check symbol resolution
export ASTHRA_SEMANTIC_DEBUG=1
./bin/asthra test.asthra

# Type checking issues
# Verify type compatibility
./bin/asthra -c 'let x: i32 = "string";'  # Should fail

# Scope management issues
# Check variable scoping
./bin/asthra -c 'fn test() { let x = 1; } let y = x;'  # Should fail
```

## Runtime Problems

### Compiler Crashes

#### Segmentation Faults

**Diagnosis:**
```bash
# Get stack trace with GDB
gdb ./bin/asthra
(gdb) run problematic_file.asthra
(gdb) bt  # Backtrace when it crashes
(gdb) info registers
(gdb) disassemble
```

**Common Causes:**
- Null pointer dereference
- Stack overflow (deep recursion)
- Heap corruption
- Use after free

**Solutions:**
```bash
# Build with debug symbols
make clean &amp;&amp; make debug

# Use AddressSanitizer
make clean &amp;&amp; make asan
./bin/asthra problematic_file.asthra

# Check for stack overflow
ulimit -s unlimited  # Increase stack size
./bin/asthra problematic_file.asthra
```

#### Memory Leaks

**Detection:**
```bash
# Use Valgrind
make clean &amp;&amp; make debug
valgrind --tool=memcheck --leak-check=full ./bin/asthra test.asthra

# Use AddressSanitizer
make clean &amp;&amp; make asan
export ASAN_OPTIONS=detect_leaks=1
./bin/asthra test.asthra
```

**Common Sources:**
- AST nodes not freed
- Symbol table entries not cleaned up
- String allocations not freed
- File handles not closed

### Performance Issues

#### Slow Compilation

**Diagnosis:**
```bash
# Profile compilation
make clean &amp;&amp; make profile
./bin/asthra large_file.asthra
gprof ./bin/asthra gmon.out &gt; profile.txt

# Check for algorithmic issues
# - O(n²) algorithms in parser
# - Excessive memory allocations
# - Inefficient string operations
```

**Solutions:**
```bash
# Optimize build
make clean &amp;&amp; make release

# Check for debug output
unset ASTHRA_DEBUG ASTHRA_VERBOSE

# Profile memory usage
valgrind --tool=massif ./bin/asthra large_file.asthra
ms_print massif.out.*
```

## IDE and Editor Issues

### VS Code Issues

#### IntelliSense Not Working

**Symptoms:**
- No auto-completion
- No error squiggles
- Go to definition doesn't work

**Solutions:**
```bash
# Check C++ extension
code --list-extensions | grep ms-vscode.cpptools

# Install if missing
code --install-extension ms-vscode.cpptools

# Generate compile commands
make clean &amp;&amp; bear -- make -j8

# Reset IntelliSense database
# Ctrl+Shift+P -&gt; "C/C++: Reset IntelliSense Database"

# Check configuration
cat .vscode/settings.json
```

#### Build Tasks Not Working

**Solutions:**
```bash
# Check tasks.json exists
ls .vscode/tasks.json

# Copy from template
cp .vscode/tasks.json.template .vscode/tasks.json

# Test build task
# Ctrl+Shift+P -&gt; "Tasks: Run Task" -&gt; "Build Asthra"
```

### CLion Issues

#### Project Not Loading

**Solutions:**
```bash
# Clear CLion cache
rm -rf .idea/

# Reopen project
# File -&gt; Open -&gt; Select asthra directory

# Check CMake configuration (if using)
ls CMakeLists.txt

# Generate CMakeLists.txt if needed
# See development-setup.md for template
```

## Platform-Specific Issues

### macOS Issues

#### Xcode Command Line Tools

**Symptoms:**
```
xcrun: error: invalid active developer path
clang: command not found
```

**Solutions:**
```bash
# Install/reinstall Xcode tools
xcode-select --install

# Check installation
xcode-select -p

# Reset if corrupted
sudo xcode-select --reset
```

#### Homebrew Issues

**Solutions:**
```bash
# Update Homebrew
brew update &amp;&amp; brew upgrade

# Fix permissions
sudo chown -R $(whoami) /opt/homebrew  # Apple Silicon
sudo chown -R $(whoami) /usr/local     # Intel Mac

# Reinstall problematic packages
brew uninstall llvm &amp;&amp; brew install llvm
```

### Linux Issues

#### Package Manager Issues

**Ubuntu/Debian:**
```bash
# Update package lists
sudo apt update

# Fix broken packages
sudo apt --fix-broken install

# Clean package cache
sudo apt clean &amp;&amp; sudo apt autoclean
```

**CentOS/RHEL:**
```bash
# Update packages
sudo yum update

# Clean cache
sudo yum clean all

# Enable EPEL repository
sudo yum install epel-release
```

#### Library Path Issues

**Solutions:**
```bash
# Update library cache
sudo ldconfig

# Check library paths
echo $LD_LIBRARY_PATH
ldconfig -p | grep json-c

# Add custom library paths
export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"
```

### Windows Issues

#### WSL2 Issues

**Solutions:**
```bash
# Update WSL2
wsl --update

# Check WSL version
wsl --list --verbose

# Restart WSL
wsl --shutdown
wsl

# Fix file permissions
sudo chmod -R 755 /mnt/c/path/to/asthra
```

#### MinGW Issues

**Solutions:**
```bash
# Update MSYS2
pacman -Syu

# Reinstall toolchain
pacman -S mingw-w64-x86_64-toolchain

# Check PATH
echo $PATH | grep mingw64
```

## Performance Problems

### Slow Builds

**Diagnosis:**
```bash
# Time the build
time make clean &amp;&amp; time make -j8

# Check system resources
htop  # CPU and memory usage
iotop # Disk I/O usage

# Check for swap usage
free -h
```

**Solutions:**
```bash
# Use more parallel jobs
make -j$(nproc)

# Use faster storage
# Move project to SSD if on HDD

# Increase memory
# Close other applications
# Add more RAM if possible

# Use ccache for faster rebuilds
# macOS: brew install ccache
# Linux: sudo apt install ccache
export CC="ccache clang"
```

### Slow Tests

**Solutions:**
```bash
# Run subset of tests
make test-curated  # Instead of test-all

# Reduce parallel test execution
export ASTHRA_TEST_PARALLEL=4

# Increase test timeout
export ASTHRA_TEST_TIMEOUT=60

# Skip performance tests
make test-all-sans-performance
```

## Getting Help

### Self-Help Resources

1\. **Check Documentation**
   - &#91;Common Issues&#93;(common-issues.md)
   - &#91;Build Problems&#93;(build-problems.md)
   - &#91;Test Failures&#93;(test-failures.md)
   - &#91;Development Setup&#93;(../quick-start/development-setup.md)

2\. **Search Existing Issues**
   ```bash
   # Search GitHub issues
   # https://github.com/asthra-lang/asthra/issues
   # Use keywords from your error message
   ```

3\. **Enable Debug Output**
   ```bash
   export ASTHRA_DEBUG=1
   export ASTHRA_VERBOSE=1
   ./bin/asthra problematic_file.asthra 2>&amp;1 | tee debug.log
   ```

### Community Support

1\. **GitHub Issues**
   - Report bugs with full details
   - Include error messages and logs
   - Provide minimal reproduction case

2\. **GitHub Discussions**
   - Ask questions about development
   - Share solutions to common problems
   - Get help from community

3\. **Real-time Chat**
   - Discord: Quick questions and help
   - Slack: Development team coordination

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

**🔧 Still having issues?**

*Don't hesitate to ask for help! The Asthra community is here to support you. Check &#91;Common Issues&#93;(common-issues.md) for quick solutions or reach out on GitHub Discussions.* 