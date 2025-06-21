# Asthra Compiler Fixes

This directory contains documentation of fixes applied to the Asthra compiler, including:

## Linker Errors

- [Fixed Linker Errors in test-all](linker_errors_in_test_all.md) - Resolution of linker errors in the test suite due to missing object files and incorrect build dependencies.

## Status of Tests

After applying all fixes, the following tests are now working:

- ✅ `test-ffi-minimal` - Minimal FFI Assembly Generator Test Suite
- ✅ `test-ffi-simple` - Simplified FFI Assembly Generator Test Suite
- ✅ `test-elf-writer` - ELF Writer Test Suite
- ✅ `test-import-aliases` - Import Aliases Test Suite
- ✅ `test-io-module` - I/O Module Test Suite
- ✅ `test-struct-declarations` - Struct Declarations Test Suite
- ✅ `test-struct-field-access` - Struct Field Access Test Suite
- ✅ `test-struct-methods` - Struct Methods Test Suite (with expected error for unimplemented feature)
- ✅ `test-struct-pattern-matching` - Struct Pattern Matching Test Suite
- ✅ `test-type-info-integration` - TypeInfo Integration Test Suite
- ✅ `test-type-inference` - Type Inference Test Suite
- ❌ `test-advanced-concurrency` - Advanced Concurrency Test Suite (still has linker errors)

## Common Issues

- **Missing Include Paths**: Many compilation issues were related to missing `-I.` include path flag
- **Undefined Symbols**: Functions declared in header files but not linked during compilation
- **Dependency Order**: Some source files depend on others and need to be compiled in the correct order
- **Missing Object Files**: Required source files not included in target dependencies 
