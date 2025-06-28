# Windows Platform Support Removal Plan

## Overview
This document outlines the comprehensive plan to remove Windows platform support from the Asthra compiler codebase. The removal will simplify the codebase and reduce maintenance overhead by focusing solely on Unix-based platforms (macOS and Linux).

## Files Affected

### 1. Platform Abstraction Headers
- `runtime/platform/asthra_platform.h` - Core platform detection and Windows-specific types
- `src/platform.h` - Compiler platform detection with Windows macros
- `src/platform.c` - Implementation of platform-specific functions

### 2. Runtime System
- `runtime/platform/asthra_platform_stub.c` - Platform abstraction stub
- `runtime/concurrency/asthra_concurrency_atomics.h` - Windows threading primitives
- `runtime/asthra_tasks_threads.c` - Windows thread handling
- `runtime/asthra_runtime_core.c` - Runtime core with Windows support
- `runtime/stdlib_concurrency_support.c` - Windows concurrency support

### 3. Build System (CMake)
- `CMakeLists.txt` - Main build configuration
- `cmake/Platform.cmake` - Extensive Windows platform detection and configuration
- `cmake/ThirdParty.cmake` - Windows-specific third-party handling
- `cmake/BuildTypes.cmake` - Windows build type configurations
- `cmake/Targets.cmake` - Windows target definitions
- `cmake/Coverage.cmake` - Windows coverage support
- `cmake/Testing.cmake` - Windows test configurations
- `cmake/Documentation.cmake` - Windows documentation generation
- `cmake/Installation.cmake` - Windows installation paths
- `cmake/CheckCoverageThreshold.cmake` - Windows coverage checks

### 4. Compiler Source
- `src/linker/linker_internal.c` - Windows linking support
- `src/linker/object_file_manager.c` - Windows object file handling
- `src/compiler/compilation_multi_file.c` - Windows multi-file compilation
- `src/compiler/compiler_core.c` - Windows compiler core
- `src/compiler/options_validation.c` - Windows path validation
- `src/codegen/llvm_pipeline.c` - Windows LLVM pipeline
- `src/codegen/llvm_utilities.c` - Windows LLVM utilities
- `src/codegen/llvm_process_execution.c` - Windows process execution
- `src/platform/executable_generator.h` - Windows executable generation
- `src/platform/executable_generator.c` - Windows .exe creation
- `src/cli.h` - Windows CLI support
- `src/cli.c` - Windows command-line handling

### 5. Tests
- `tests/codegen/cross_platform/cross_platform_common.h` - Cross-platform test utilities
- `tests/codegen/cross_platform/test_platform_specific_codegen.c` - Windows-specific codegen tests
- `tests/codegen/cross_platform/test_platform_detection.c` - Windows platform detection tests
- `tests/codegen/cross_platform/test_cross_compilation_validation.c` - Windows cross-compilation tests
- `tests/codegen/cross_platform/test_calling_convention_handling.c` - Windows calling convention tests
- `tests/runtime/runtime_test_infrastructure.c` - Windows runtime tests
- `tests/platform/test_executable_generator_comprehensive.c` - Windows executable tests

### 6. CI/CD Workflows
- `.github/workflows/build-and-test.yml` - Windows CI configuration
- `.github/workflows/cross-platform-check.yml` - Windows platform compatibility checks

### 7. Documentation
- `CLAUDE.md` - Windows platform support mention
- `docs/contributor/reference/platform-support.md` - Detailed Windows support documentation
- `docs/contributor/reference/build-system.md` - Windows build system docs
- `docs/contributor/guides/cross-platform-development.md` - Windows development guide
- `docs/contributor/guides/cmake-usage-guide.md` - Windows CMake usage
- `docs/contributor/guides/development-environment.md` - Windows dev environment
- `docs/contributor/quick-start/development-setup.md` - Windows setup instructions
- `docs/architecture/compiler-infrastructure-analysis.md` - Windows compiler analysis
- `docs/architecture/compiler-architecture.md` - Windows architecture notes
- `docs/spec/overview.md` - Windows platform specification
- `docs/spec/ampu.md` - Windows package manager support

## Removal Plan by Phase

### Phase 1: CI/CD and Build Infrastructure
1. Remove Windows CI/CD configurations from GitHub workflows
   - Remove `windows-2022` runner
   - Remove Chocolatey package manager setup
   - Remove Windows-specific LLVM installation steps
   - Remove Windows test matrix entries

2. Clean up CMake build system
   - Remove Windows platform detection in `cmake/Platform.cmake`
   - Remove MSVC and MinGW compiler handling
   - Remove Windows-specific compiler flags and definitions
   - Remove Windows executable extensions (.exe, .dll, .lib)
   - Remove Windows path handling (backslash separators)

### Phase 2: Core Platform Abstraction
1. Update platform detection headers
   - Remove `_WIN32`, `_WIN64`, `_MSC_VER` preprocessor checks
   - Remove Windows-specific type definitions (HANDLE, CRITICAL_SECTION)
   - Remove Windows API includes (windows.h, process.h, io.h, direct.h)

2. Simplify platform-specific code paths
   - Remove Windows branches from conditional compilation
   - Consolidate Unix-only implementations

### Phase 3: Runtime System
1. Remove Windows-specific threading
   - Remove CreateThread, CRITICAL_SECTION, CONDITION_VARIABLE
   - Remove Windows thread local storage
   - Keep only POSIX pthread implementation

2. Remove Windows memory management
   - Remove VirtualAlloc, VirtualProtect calls
   - Remove Windows-specific memory alignment handling
   - Keep only POSIX memory management

3. Remove Windows dynamic loading
   - Remove LoadLibrary/GetProcAddress
   - Keep only dlopen/dlsym implementation

### Phase 4: Compiler and Linker
1. Remove Windows executable generation
   - Remove .exe file generation
   - Remove Windows PE format support
   - Remove Windows resource compilation

2. Clean up object file handling
   - Remove Windows .obj/.lib file support
   - Remove Windows-specific linking flags

3. Update path handling
   - Remove backslash path separator support
   - Remove semicolon environment variable separator
   - Standardize on Unix path conventions

### Phase 5: Test Suite
1. Remove Windows-specific tests
   - Remove or adapt cross-platform tests
   - Remove Windows calling convention tests
   - Remove Windows platform detection tests

2. Update test infrastructure
   - Remove Windows-specific test utilities
   - Update test runners for Unix-only execution

### Phase 6: Documentation
1. Update all documentation
   - Remove Windows installation instructions
   - Remove Windows development guides
   - Update platform support matrix
   - Update build requirements

2. Update code comments
   - Remove Windows-specific implementation notes
   - Update platform assumptions in comments

## Implementation Order

1. **Day 1-2**: CI/CD and Build Infrastructure (Phase 1)
   - Start with GitHub workflows to stop Windows CI immediately
   - Clean up CMake files to remove Windows build support

2. **Day 3-4**: Core Platform Abstraction (Phase 2)
   - Update platform headers
   - Remove Windows-specific includes and types

3. **Day 5-6**: Runtime System (Phase 3)
   - Remove Windows threading and memory management
   - Consolidate to POSIX-only implementations

4. **Day 7-8**: Compiler and Linker (Phase 4)
   - Remove Windows executable generation
   - Clean up path handling

5. **Day 9**: Test Suite (Phase 5)
   - Remove Windows-specific tests
   - Update test infrastructure

6. **Day 10**: Documentation (Phase 6)
   - Update all documentation
   - Final cleanup and verification

## Verification Steps

After each phase:
1. Run full test suite on macOS and Linux
2. Verify successful builds without Windows references
3. Check for any remaining Windows-specific code
4. Update documentation as needed

## Expected Benefits

1. **Reduced Complexity**: ~40+ files simplified or reduced
2. **Faster CI/CD**: No Windows build/test overhead
3. **Simplified Maintenance**: No Windows-specific bugs or updates
4. **Cleaner Codebase**: Removal of platform abstraction layers
5. **Focus**: Better Unix/POSIX optimization opportunities

## Risks and Mitigation

1. **Risk**: Breaking existing Unix functionality
   - **Mitigation**: Comprehensive testing after each phase

2. **Risk**: Missing Windows-specific code in cleanup
   - **Mitigation**: Use grep/search tools to find all occurrences

3. **Risk**: Documentation inconsistencies
   - **Mitigation**: Systematic review of all docs

## Success Criteria

- All tests pass on macOS (Intel/Apple Silicon) and Linux (x86_64/ARM64)
- No Windows-specific code remains in the codebase
- Documentation accurately reflects Unix-only support
- Build system simplified with Windows logic removed
- CI/CD runs faster without Windows jobs