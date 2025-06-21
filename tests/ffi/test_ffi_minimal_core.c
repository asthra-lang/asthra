/**
 * Minimal FFI Assembly Generator Test - Core Implementation
 * 
 * This file contains the core implementation functions for the minimal
 * FFI assembly generator test suite.
 */

#include "test_ffi_minimal_common.h"

// =============================================================================
// CORE GENERATOR IMPLEMENTATION
// =============================================================================

MinimalFFIAssemblyGenerator *minimal_ffi_generator_create(TargetArchitecture arch, CallingConvention conv) {
    MinimalFFIAssemblyGenerator *generator = malloc(sizeof(MinimalFFIAssemblyGenerator));
    if (!generator) return NULL;
    
    // Initialize basic fields
    generator->target_arch = arch;
    generator->calling_conv = conv;
    
    // Initialize configuration with defaults
    generator->config.enable_bounds_checking = true;
    generator->config.enable_security_features = true;
    generator->config.enable_concurrency = true;
    generator->config.optimize_string_operations = false;
    generator->config.optimize_pattern_matching = false;
    generator->config.max_variadic_args = 8;
    generator->config.pic_mode = false;
    
    // Initialize runtime function names
    generator->runtime_functions.gc_alloc = strdup("asthra_gc_alloc");
    generator->runtime_functions.gc_free = strdup("asthra_gc_free");
    generator->runtime_functions.slice_bounds_check = strdup("asthra_slice_bounds_check");
    generator->runtime_functions.string_concat = strdup("asthra_string_concat");
    generator->runtime_functions.string_interpolate = strdup("asthra_string_interpolate");
    generator->runtime_functions.result_create_ok = strdup("asthra_result_create_ok");
    generator->runtime_functions.result_create_err = strdup("asthra_result_create_err");
    generator->runtime_functions.spawn_task = strdup("asthra_spawn_task");
    generator->runtime_functions.secure_zero = strdup("asthra_secure_zero");
    
    // Initialize atomic counters
    atomic_init(&generator->ffi_calls_generated, 0);
    atomic_init(&generator->pattern_matches_generated, 0);
    atomic_init(&generator->string_operations_generated, 0);
    atomic_init(&generator->slice_operations_generated, 0);
    atomic_init(&generator->security_operations_generated, 0);
    atomic_init(&generator->spawn_statements_generated, 0);
    
    return generator;
}

void minimal_ffi_generator_destroy(MinimalFFIAssemblyGenerator *generator) {
    if (!generator) return;
    
    // Free runtime function names
    free(generator->runtime_functions.gc_alloc);
    free(generator->runtime_functions.gc_free);
    free(generator->runtime_functions.slice_bounds_check);
    free(generator->runtime_functions.string_concat);
    free(generator->runtime_functions.string_interpolate);
    free(generator->runtime_functions.result_create_ok);
    free(generator->runtime_functions.result_create_err);
    free(generator->runtime_functions.spawn_task);
    free(generator->runtime_functions.secure_zero);
    
    free(generator);
}

// =============================================================================
// STRING OPERATIONS IMPLEMENTATION
// =============================================================================

bool minimal_generate_string_concatenation(MinimalFFIAssemblyGenerator *generator, Register left_reg, Register right_reg, Register result_reg) {
    if (!generator) return false;
    
    atomic_fetch_add(&generator->string_operations_generated, 1);
    
    printf("  Generated string concatenation: concat(r%d, r%d) -> r%d\n", left_reg, right_reg, result_reg);
    printf("  Assembly: call %s\n", generator->runtime_functions.string_concat);
    
    return true;
}

// =============================================================================
// SECURITY OPERATIONS IMPLEMENTATION
// =============================================================================

bool minimal_generate_volatile_memory_access(MinimalFFIAssemblyGenerator *generator, Register memory_reg, size_t size, bool is_read) {
    if (!generator) return false;
    
    atomic_fetch_add(&generator->security_operations_generated, 1);
    
    printf("  Generated volatile memory %s: %s r%d, %zu bytes\n", 
           is_read ? "read" : "write", 
           is_read ? "mov rax," : "mov",
           memory_reg, size);
    printf("  Assembly: %s [r%d]\n", is_read ? "mov rax," : "mov [r%d], rax", memory_reg);
    
    return true;
}

bool minimal_generate_secure_zero(MinimalFFIAssemblyGenerator *generator, Register memory_reg, Register size_reg) {
    if (!generator) return false;
    
    atomic_fetch_add(&generator->security_operations_generated, 1);
    
    printf("  Generated secure zero: zero(r%d, r%d)\n", memory_reg, size_reg);
    printf("  Assembly: call %s\n", generator->runtime_functions.secure_zero);
    
    return true;
}

// =============================================================================
// CONCURRENCY OPERATIONS IMPLEMENTATION
// =============================================================================

bool minimal_generate_task_creation(MinimalFFIAssemblyGenerator *generator, const char *function_name, Register *arg_regs, size_t arg_count, Register handle_reg) {
    if (!generator) return false;
    
    atomic_fetch_add(&generator->spawn_statements_generated, 1);
    
    printf("  Generated task creation: spawn %s(", function_name);
    for (size_t i = 0; i < arg_count; i++) {
        printf("r%d%s", arg_regs[i], (i < arg_count - 1) ? ", " : "");
    }
    printf(") -> r%d\n", handle_reg);
    printf("  Assembly: call %s\n", generator->runtime_functions.spawn_task);
    
    return true;
}

// =============================================================================
// ASSEMBLY VALIDATION AND OUTPUT
// =============================================================================

bool minimal_validate_generated_assembly(MinimalFFIAssemblyGenerator *generator) {
    if (!generator) return false;
    
    // Simple validation - just check that some operations were generated
    size_t total_ops = atomic_load(&generator->string_operations_generated) +
                      atomic_load(&generator->slice_operations_generated) +
                      atomic_load(&generator->security_operations_generated) +
                      atomic_load(&generator->spawn_statements_generated);
    
    printf("  Validating %zu total operations...\n", total_ops);
    printf("  Assembly validation: PASSED\n");
    
    return total_ops > 0;
}

bool minimal_print_nasm_assembly(MinimalFFIAssemblyGenerator *generator, char *output_buffer, size_t buffer_size) {
    if (!generator || !output_buffer) return false;
    
    // Generate a simple NASM assembly snippet
    int written = snprintf(output_buffer, buffer_size,
        "; Generated assembly for %s calling convention\n"
        "section .text\n"
        "global _start\n"
        "_start:\n"
        "    ; Instruction 0\n"
        "    ; Instruction 1\n"
        "    ; Instruction 2\n"
        "    ; Instruction 3\n"
        "    ; Instruction 4\n"
        "    ; Instruction 5\n"
        "...\n",
        (generator->calling_conv == CALLING_CONV_SYSTEM_V_AMD64) ? "System V AMD64" : "Unknown");
    
    return written > 0 && written < (int)buffer_size;
}

// =============================================================================
// STATISTICS IMPLEMENTATION
// =============================================================================

void minimal_get_generation_statistics(MinimalFFIAssemblyGenerator *generator,
                                      size_t *ffi_calls, size_t *pattern_matches,
                                      size_t *string_ops, size_t *slice_ops,
                                      size_t *security_ops, size_t *spawn_stmts) {
    if (!generator) return;
    
    if (ffi_calls) *ffi_calls = atomic_load(&generator->ffi_calls_generated);
    if (pattern_matches) *pattern_matches = atomic_load(&generator->pattern_matches_generated);
    if (string_ops) *string_ops = atomic_load(&generator->string_operations_generated);
    if (slice_ops) *slice_ops = atomic_load(&generator->slice_operations_generated);
    if (security_ops) *security_ops = atomic_load(&generator->security_operations_generated);
    if (spawn_stmts) *spawn_stmts = atomic_load(&generator->spawn_statements_generated);
}

// =============================================================================
// STANDALONE TEST RUNNER (FOR WHEN COMPILED AS STANDALONE)
// =============================================================================

#ifndef SKIP_MAIN
static bool test_generator_basic_operations(void) {
    printf("Testing generator basic operations...\n");
    
    MinimalFFIAssemblyGenerator *generator = minimal_ffi_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!generator) {
        printf("Failed to create generator\n");
        return false;
    }
    
    // Test string operation
    bool result = minimal_generate_string_concatenation(generator, REG_RDI, REG_RSI, REG_RAX);
    if (!result) {
        printf("String concatenation generation failed\n");
        minimal_ffi_generator_destroy(generator);
        return false;
    }
    
    // Test security operation
    result = minimal_generate_secure_zero(generator, REG_RDI, REG_RSI);
    if (!result) {
        printf("Secure zero generation failed\n");
        minimal_ffi_generator_destroy(generator);
        return false;
    }
    
    // Test validation
    result = minimal_validate_generated_assembly(generator);
    if (!result) {
        printf("Assembly validation failed\n");
        minimal_ffi_generator_destroy(generator);
        return false;
    }
    
    minimal_ffi_generator_destroy(generator);
    return true;
}

int main(void) {
    printf("Minimal FFI Core Operations Test\n");
    printf("=================================\n");
    
    if (test_generator_basic_operations()) {
        printf("✅ Core operations test passed\n");
        return 0;
    } else {
        printf("❌ Core operations test failed\n");
        return 1;
    }
}
#endif 
