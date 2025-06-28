/**
 * FFI Assembly Generator - Test Stub
 * 
 * Minimal stub for ELF writer testing
 */

#ifndef FFI_ASSEMBLY_GENERATOR_H
#define FFI_ASSEMBLY_GENERATOR_H

#include <stdbool.h>

typedef struct FFIAssemblyGenerator {
    void *internal;  // Opaque pointer for test purposes
} FFIAssemblyGenerator;

// Architecture and calling convention constants
typedef enum {
    TARGET_ARCH_X86_64 = 0,
    TARGET_ARCH_ARM64 = 1
} TargetArchitecture;

typedef enum {
    CALLING_CONV_SYSTEM_V_AMD64 = 0,
    CALLING_CONV_MS_X64 = 1,
    CALLING_CONV_AAPCS64 = 2
} CallingConvention;

// Stub functions for testing
static inline FFIAssemblyGenerator* ffi_assembly_generator_create(TargetArchitecture arch, CallingConvention conv) {
    (void)arch;
    (void)conv;
    return NULL;
}

static inline void ffi_assembly_generator_destroy(FFIAssemblyGenerator* gen) {
    (void)gen;
}

#endif // FFI_ASSEMBLY_GENERATOR_H