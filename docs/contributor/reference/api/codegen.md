# CodeGen API

## Overview

Asthra exclusively uses LLVM IR for all code generation. The old direct assembly and ELF generation APIs have been removed in favor of a unified LLVM backend interface.

### Backend Interface

```c
// Backend Creation and Lifecycle
AsthraBackend *asthra_backend_create_by_type(AsthraBackendType type);
void asthra_backend_destroy(AsthraBackend *backend);

// Backend Types
typedef enum {
    ASTHRA_BACKEND_LLVM_IR,    // LLVM IR generation (only supported backend)
} AsthraBackendType;

// Code Generation
bool asthra_backend_compile_ast(AsthraBackend *backend, ASTNode *ast, const char *output_file);
bool asthra_backend_emit_ir(AsthraBackend *backend, char *buffer, size_t buffer_size);

// Configuration
void asthra_backend_set_optimization_level(AsthraBackend *backend, int level);
void asthra_backend_set_target_triple(AsthraBackend *backend, const char *triple);
```

### LLVM Integration

All code generation is performed through LLVM:
- **Target Support**: x86_64, ARM64, WASM32, and all LLVM-supported targets
- **Optimizations**: LLVM's powerful optimization passes
- **FFI Support**: Through LLVM's foreign function intrinsics
- **Debug Info**: DWARF generation via LLVM

### Migration Notes

The following APIs have been removed:
- Direct ELF generation (`elf_writer_*`)
- Assembly generation (`ffi_assembly_*`)
- Custom optimization passes (`optimizer_*`)
- Register allocation (`register_allocator_*`)

All functionality is now provided through LLVM's comprehensive infrastructure.