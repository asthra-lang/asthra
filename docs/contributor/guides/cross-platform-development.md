# Cross-Platform Development Guide

**Version:** 1.1  
**Date:** January 28, 2025  
**Status:** Active  

## Overview

This guide provides practical instructions for writing cross-platform code in the Asthra compiler. It covers common pitfalls, platform-specific APIs, and best practices for ensuring code works correctly on macOS and Linux.

## Quick Reference

### Platform Detection Macros

Always include `platform.h` first:
```c
#include "platform.h"  // or "../platform.h" depending on location
```

Available macros:
- `ASTHRA_PLATFORM_UNIX` - True for Linux and macOS
- `ASTHRA_PLATFORM_LINUX` - True for Linux only
- `ASTHRA_PLATFORM_MACOS` - True for macOS only

### Common Patterns

#### 1. Platform-Specific Headers

```c
#include "platform.h"

// Unix headers are always available
#include <sys/wait.h>
#include <unistd.h>
```

#### 2. Handling system() Calls

```c
int result = system(command);
if (result == -1) {
    // Failed to execute
    return -1;
}

// Unix: check exit status
if (!WIFEXITED(result) || WEXITSTATUS(result) != 0) {
    // Command failed
    return -1;
}
```

#### 3. Checking I/O Return Values

Always check return values from I/O functions:

```c
// Reading files
size_t bytes_read = fread(buffer, 1, size, file);
if (bytes_read != size) {
    // Handle error
    if (feof(file)) {
        // End of file reached
    } else if (ferror(file)) {
        // Read error occurred
    }
    return -1;
}

// Writing files
size_t bytes_written = fwrite(buffer, 1, size, file);
if (bytes_written != size) {
    // Handle write error
    return -1;
}
```

## Common Issues and Solutions

### Issue 1: Unused Return Value Warnings

**Problem**: Clang with `-Wunused-result` warns about unchecked return values.

**Solution**: Always check return values from system functions:
```c
// Bad
system("mkdir -p output");

// Good
int result = system("mkdir -p output");
if (result != 0) {
    fprintf(stderr, "Failed to create directory\n");
}
```

### Issue 2: POSIX-Specific Headers

**Problem**: Some headers are specific to certain Unix variants.

**Solution**: Use platform guards when needed:
```c
#if ASTHRA_PLATFORM_LINUX
// Linux-specific headers
#include <linux/limits.h>
#endif
```

### Issue 3: Platform-Specific System Calls

**Problem**: Some system calls vary between macOS and Linux.

**Solution**: Provide platform-specific implementations:
```c
#if ASTHRA_PLATFORM_MACOS
    // macOS-specific implementation
    void* mem = mmap(NULL, size, PROT_READ | PROT_WRITE, 
                     MAP_PRIVATE | MAP_ANON, -1, 0);
#elif ASTHRA_PLATFORM_LINUX
    // Linux-specific implementation
    void* mem = mmap(NULL, size, PROT_READ | PROT_WRITE, 
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif
```

## Testing Cross-Platform Code

### Local Testing

1. **On Linux**: Test with Clang
   ```bash
   # Clang with strict warnings
   CC=clang CFLAGS="-Wall -Werror" make
   ```

2. **On macOS**: Test with Apple Clang
   ```bash
   # Apple Clang with strict warnings
   CC=clang CFLAGS="-Wall -Werror" make
   ```

### CI/CD Testing

The GitHub Actions CI tests on:
- Linux with Clang (Ubuntu, strict warnings)
- macOS with Clang (Apple Silicon and Intel)

## Checklist for New Code

Before submitting platform-specific code:

- [ ] Include `platform.h` before any platform-specific headers
- [ ] Guard all POSIX headers with `#if ASTHRA_PLATFORM_UNIX`
- [ ] Check all `fread()`/`fwrite()` return values
- [ ] Handle `system()` return values correctly for each platform
- [ ] Provide fallbacks for platform-specific features
- [ ] Test with `-Wall -Werror` on available platforms
- [ ] Document any platform-specific behavior

## Examples from the Codebase

### Good Example: compilation_pipeline.c

```c
#include "platform.h"
#include <sys/wait.h>

// ... later in code ...

int result = system(compile_cmd);
if (result == -1) {
    printf("Error: Failed to execute compile command\n");
    return -1;
}

// Unix-specific exit status checking
if (!WIFEXITED(result) || WEXITSTATUS(result) != 0) {
    printf("Error: Compilation failed\n");
    return -1;
}
```

## Additional Resources

- [Platform Support Reference](../reference/platform-support.md) - Detailed platform architecture
- [src/platform.h](../../../src/platform.h) - Platform detection header
- [GitHub Actions Workflows](../../../.github/workflows/) - CI configuration

## Getting Help

If you encounter platform-specific issues:

1. Check existing code for similar patterns
2. Consult the platform support documentation
3. Ask in the project discussions
4. File an issue with platform details