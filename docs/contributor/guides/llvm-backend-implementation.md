# LLVM Backend Implementation Guide

## Overview

This document describes the implementation of the LLVM backend for the Asthra compiler, completed as part of Phase 2 of issue #68.

## Implementation Summary

### 1. Build System Integration

The LLVM backend support was integrated into the CMake build system:

- **`cmake/ThirdParty.cmake`**: Added LLVM detection and configuration
  - Added `ENABLE_LLVM_BACKEND` option (default: OFF)
  - Added `USE_SYSTEM_LLVM` option (default: ON)
  - Implemented LLVM detection via both CMake package and llvm-config
  - Created `llvm::llvm` interface library for linking

- **`src/CMakeLists.txt`**: Added conditional LLVM linking
  - Links LLVM libraries only when `ENABLE_LLVM_BACKEND` is enabled

### 2. LLVM Backend Implementation

The LLVM backend was implemented in `src/codegen/llvm_backend.c`:

#### Key Components:

1. **Initialization**
   - Creates LLVM context, module, and IR builder
   - Sets up target triple for native compilation
   - Caches common LLVM types for efficiency

2. **Type Conversion**
   - Maps Asthra TypeInfo to LLVM types
   - Supports primitives, slices, pointers, functions, structs, enums, and tuples
   - Handles unit type as empty struct

3. **Code Generation**
   - Function declarations and definitions
   - Basic expressions: literals, binary operations
   - Return statements
   - Extensible architecture for additional AST nodes

4. **Output Generation**
   - Supports both text IR (.ll) and bitcode (.bc) formats
   - Module verification before output
   - Statistics tracking

#### Supported Features:
- optimization
- debug-info
- cross-compilation
- bitcode-output
- ir-output
- native-codegen

### 3. Testing Infrastructure

Created comprehensive tests in `tests/codegen/backend/test_llvm_backend.c`:

- Backend creation and initialization
- Feature support queries
- Basic code generation (when LLVM is enabled)
- Output format handling
- Graceful handling when LLVM is not compiled in

### 4. Building with LLVM Support

To enable the LLVM backend:

```bash
cmake -B build -DENABLE_LLVM_BACKEND=ON
cmake --build build
```

The build system will automatically find LLVM using:
1. CMake's `find_package(LLVM)`
2. Fallback to `llvm-config` if CMake package not found

### 5. Using the LLVM Backend

Once built with LLVM support, use the `--emit` flag with the desired format:

```bash
asthra --emit llvm-ir input.as    # Generates input.ll (text IR)
asthra --emit llvm-bc input.as -o output.bc  # Generates bitcode
```

## Architecture Decisions

1. **Conditional Compilation**: All LLVM-specific code is guarded by `#ifdef ASTHRA_ENABLE_LLVM_BACKEND` to allow building without LLVM dependencies.

2. **Backend Interface**: The LLVM backend implements the standard `AsthraBackendOps` interface, ensuring compatibility with the multi-backend architecture.

3. **Type System Integration**: Uses the unified `TypeInfo` structure from the semantic analyzer for type information.

4. **Error Handling**: Provides clear error messages when LLVM is not available or when operations fail.

## Future Enhancements

The following features are partially implemented and can be completed in future phases:

1. **Control Flow**: if/else, while, for loops
2. **Advanced Types**: Full struct and enum support
3. **Pattern Matching**: Match expressions
4. **Runtime Integration**: GC, slice operations, FFI
5. **Optimization Passes**: Integration with LLVM's optimization pipeline
6. **Debug Information**: DWARF debug info generation

## Testing

Run the LLVM backend tests:

```bash
# Run specific LLVM tests
ctest --test-dir build -R test_llvm_backend

# Run all backend tests including LLVM
ctest --test-dir build -L backend
```

## Dependencies

The LLVM backend requires:
- LLVM 18.0 or later
- LLVM-C API headers and libraries

On macOS with Homebrew:
```bash
brew install llvm
```

On Ubuntu/Debian:
```bash
apt-get install llvm-dev
```