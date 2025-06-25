# Asthra Compiler Platform Support Architecture

**Version:** 1.0  
**Date:** January 7, 2025  
**Status:** Complete  

## Table of Contents

1. [Overview](#overview)
2. [Cross-Platform Design](#cross-platform-design)
3. [Target Architecture Support](#target-architecture-support)
4. [Operating System Integration](#operating-system-integration)
5. [Toolchain Integration](#toolchain-integration)
6. [Platform-Specific Optimizations](#platform-specific-optimizations)
7. [Porting Guide](#porting-guide)
8. [Testing and Validation](#testing-and-validation)

## Overview

Asthra is designed as a cross-platform programming language with first-class support for multiple target architectures and operating systems. This document describes the platform abstraction layer, target-specific implementations, and strategies for maintaining portability while leveraging platform-specific optimizations.

### Supported Platforms

#### Primary Targets (Tier 1)
- **x86_64**: Linux, macOS, Windows
- **ARM64**: Linux, macOS, iOS (planned)
- **WASM32**: Browser and WASI environments

#### Secondary Targets (Tier 2)
- **ARM32**: Linux, Android (planned)
- **RISC-V**: Linux (experimental)

#### Development Platforms
- **macOS**: Primary development platform with Clang
- **Linux**: CI/CD and production deployment
- **Windows**: Cross-compilation and testing

### Platform Support Matrix

| Platform | Compilation | Runtime | FFI | Concurrency | Status |
|----------|-------------|---------|-----|-------------|--------|
| x86_64/Linux | ✅ | ✅ | ✅ | ✅ | Production |
| x86_64/macOS | ✅ | ✅ | ✅ | ✅ | Production |
| x86_64/Windows | ✅ | ✅ | ✅ | ⚠️ | Beta |
| ARM64/Linux | ✅ | ✅ | ✅ | ✅ | Production |
| ARM64/macOS | ✅ | ✅ | ✅ | ✅ | Production |
| WASM32/Browser | ✅ | ✅ | ⚠️ | ⚠️ | Beta |
| WASM32/WASI | ✅ | ✅ | ✅ | ⚠️ | Beta |

## Cross-Platform Design

### Platform Abstraction Layer

Asthra uses a comprehensive platform abstraction layer to isolate platform-specific code:

```c
// Platform abstraction interface
typedef struct PlatformInterface {
    // Memory management
    void* (*alloc)(size_t size);
    void (*free)(void* ptr);
    void* (*realloc)(void* ptr, size_t size);
    
    // File system operations
    int (*file_open)(const char* path, int flags);
    ssize_t (*file_read)(int fd, void* buf, size_t count);
    ssize_t (*file_write)(int fd, const void* buf, size_t count);
    int (*file_close)(int fd);
    
    // Threading primitives
    int (*thread_create)(thread_t* thread, void* (*start)(void*), void* arg);
    int (*thread_join)(thread_t thread, void** retval);
    int (*mutex_init)(mutex_t* mutex);
    int (*mutex_lock)(mutex_t* mutex);
    int (*mutex_unlock)(mutex_t* mutex);
    
    // Time and timing
    uint64_t (*get_time_ns)(void);
    void (*sleep_ms)(uint32_t ms);
    
    // Dynamic loading
    void* (*dlopen)(const char* filename, int flags);
    void* (*dlsym)(void* handle, const char* symbol);
    int (*dlclose)(void* handle);
} PlatformInterface;
```

### Platform Detection

The Asthra compiler uses platform detection macros defined in `src/platform.h` to enable conditional compilation and platform-specific behavior:

```c
// Platform detection macros (from platform.h)
#if defined(_WIN32) || defined(_WIN64)
    #define ASTHRA_PLATFORM_WINDOWS 1
    #define ASTHRA_PLATFORM_UNIX 0
    #define ASTHRA_PLATFORM_MACOS 0
    #define ASTHRA_PLATFORM_LINUX 0
    #define ASTHRA_PLATFORM_NAME "Windows"
#elif defined(__APPLE__) && defined(__MACH__)
    #define ASTHRA_PLATFORM_WINDOWS 0
    #define ASTHRA_PLATFORM_UNIX 1
    #define ASTHRA_PLATFORM_MACOS 1
    #define ASTHRA_PLATFORM_LINUX 0
    #define ASTHRA_PLATFORM_NAME "macOS"
#elif defined(__linux__)
    #define ASTHRA_PLATFORM_WINDOWS 0
    #define ASTHRA_PLATFORM_UNIX 1
    #define ASTHRA_PLATFORM_MACOS 0
    #define ASTHRA_PLATFORM_LINUX 1
    #define ASTHRA_PLATFORM_NAME "Linux"
#endif

// Compiler detection
#if defined(_MSC_VER)
    #define ASTHRA_COMPILER_MSVC 1
    #define ASTHRA_COMPILER_CLANG 0
#elif defined(__clang__)
    #define ASTHRA_COMPILER_MSVC 0
    #define ASTHRA_COMPILER_CLANG 1
#endif

// Architecture detection
#if defined(__x86_64__) || defined(_M_X64)
    #define ASTHRA_ARCH_X64 1
    #define ASTHRA_ARCH_NAME "x64"
#elif defined(__i386__) || defined(_M_IX86)
    #define ASTHRA_ARCH_X86 1
    #define ASTHRA_ARCH_NAME "x86"
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define ASTHRA_ARCH_ARM64 1
    #define ASTHRA_ARCH_NAME "arm64"
#elif defined(__arm__) || defined(_M_ARM)
    #define ASTHRA_ARCH_ARM 1
    #define ASTHRA_ARCH_NAME "arm"
#endif
```

### Platform-Specific Code Patterns

When writing platform-specific code, always use the platform detection macros:

```c
// Example: Platform-specific header includes
#include "platform.h"

#if ASTHRA_PLATFORM_UNIX
#include <sys/wait.h>
#include <unistd.h>
#endif

#if ASTHRA_PLATFORM_WINDOWS
#include <windows.h>
#endif

// Example: Handling system() return values portably
int execute_command(const char* cmd) {
    int result = system(cmd);
    
    if (result == -1) {
        return -1; // Failed to execute
    }
    
#if ASTHRA_PLATFORM_UNIX
    // On Unix, use WIFEXITED/WEXITSTATUS macros
    if (WIFEXITED(result) && WEXITSTATUS(result) == 0) {
        return 0; // Success
    }
    return WEXITSTATUS(result);
#else
    // On Windows, system() returns the exit code directly
    return result;
#endif
}
```

## Target Architecture Support

### x86_64 Implementation

The x86_64 backend provides comprehensive support for Intel and AMD processors with SIMD optimizations, atomic operations, and efficient calling conventions.

### ARM64 Implementation

ARM64 support targets modern ARM processors including Apple Silicon with NEON SIMD, crypto extensions, and hardware security features.

### WASM32 Implementation

WebAssembly support enables running Asthra in browsers and WASI environments with linear memory model and type safety.

## Operating System Integration

### Linux Support

Linux is the primary target platform with full feature support including mmap, pthreads, and dynamic loading.

### macOS Support

macOS support leverages Darwin-specific features including Mach kernel APIs and high-resolution timing.

### Windows Support

Windows support uses Win32 APIs for memory management, threading, and dynamic loading.

## Toolchain Integration

### Compiler Integration

Asthra integrates with C compilers (primarily Clang) with platform-specific optimizations and cross-compilation support.

### Linker Integration

Platform-specific linking strategies optimize for each target while maintaining compatibility.

## Platform-Specific Optimizations

### SIMD Optimizations

Platform-specific SIMD implementations leverage AVX2 on x86_64, NEON on ARM64, and fallbacks for other architectures.

### Atomic Operations

Efficient atomic operations use platform-specific instructions and memory models.

### Memory Management

Platform-optimized memory allocation uses mmap on Unix systems and VirtualAlloc on Windows.

## Porting Guide

### Adding a New Architecture

Step-by-step guide for adding support for new target architectures including code generation and runtime support.

### Adding a New Operating System

Guidelines for implementing platform abstraction layer for new operating systems.

## Common Platform Compatibility Issues

### POSIX vs Windows Differences

When writing code that uses system APIs, be aware of these common differences:

1. **Process execution and return values**
   - POSIX: `system()` returns a value that must be decoded with `WIFEXITED()` and `WEXITSTATUS()`
   - Windows: `system()` returns the exit code directly
   - Always use platform guards when handling system call returns

2. **Header files**
   - POSIX-specific headers like `<sys/wait.h>`, `<unistd.h>` must be guarded
   - Windows-specific headers like `<windows.h>` must be guarded
   - Always include `platform.h` before platform-specific headers

3. **File I/O return value checking**
   - Functions like `fread()` must have their return values checked
   - Use proper error handling for all I/O operations

### Required Compiler Flags

Different compilers require different warning flags:

- **Clang**: `-Wall -Werror`
- **MSVC**: `/W4 /WX`

### Best Practices for Cross-Platform Code

1. **Always check return values**
   ```c
   size_t bytes_read = fread(buffer, 1, size, file);
   if (bytes_read != size) {
       // Handle error
   }
   ```

2. **Use platform macros consistently**
   ```c
   #if ASTHRA_PLATFORM_UNIX
       // Unix-specific code
   #elif ASTHRA_PLATFORM_WINDOWS
       // Windows-specific code
   #else
       #error "Unsupported platform"
   #endif
   ```

3. **Provide fallbacks for platform-specific features**
   ```c
   #if ASTHRA_PLATFORM_UNIX
       // Use Unix-specific feature
   #else
       // Provide alternative implementation
   #endif
   ```

## Testing and Validation

### Platform Test Suite

Comprehensive testing across all supported platforms with feature detection and compatibility validation.

### Continuous Integration

Platform testing in CI/CD with cross-compilation and performance benchmarking. The CI system tests:
- Linux with Clang (strict warnings enabled)
- macOS with Clang
- Windows with MSVC (when available)

## Conclusion

Asthra's platform support architecture provides a solid foundation for cross-platform development while enabling platform-specific optimizations. The abstraction layer isolates platform differences while the modular design allows for easy extension to new targets.

For detailed implementation guidance, see the related architecture documentation:
- [Code Generation Architecture](code-generation.md)
- [Runtime System Architecture](runtime-system.md)
- [Performance and Optimization](performance.md) 