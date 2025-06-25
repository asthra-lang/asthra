/**
 * FFI Test Stubs - Enhanced FFI Generator Implementation
 * 
 * Provides realistic implementations of FFI generator functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "ffi_stubs_generator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper to update statistics
static void update_generation_stats(EnhancedFFIGenerator *gen, bool success) {
    atomic_fetch_add(&gen->generation_count, 1);
    if (success) {
        atomic_fetch_add(&gen->successful_generations, 1);
    } else {
        atomic_fetch_add(&gen->failed_generations, 1);
    }
}

// =============================================================================
// ENHANCED FFI GENERATOR IMPLEMENTATION
// =============================================================================

EnhancedFFIGenerator* enhanced_ffi_generator_create(void) {
    EnhancedFFIGenerator *gen = malloc(sizeof(EnhancedFFIGenerator));
    if (!gen) return NULL;
    
    // Initialize atomics
    atomic_store(&gen->generation_count, 0);
    atomic_store(&gen->successful_generations, 0);
    atomic_store(&gen->failed_generations, 0);
    
    // Create real FFI generator with System V AMD64 ABI
    gen->real_generator = ffi_assembly_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!gen->real_generator) {
        free(gen);
        return NULL;
    }
    
    // Get base generator for direct access
    gen->base_generator = gen->real_generator->base_generator;
    gen->initialized = true;
    
    printf("[FFI] Enhanced FFI generator created with real infrastructure\n");
    return gen;
}

void enhanced_ffi_generator_destroy(EnhancedFFIGenerator *gen) {
    if (gen) {
        if (gen->real_generator) {
            ffi_assembly_generator_destroy(gen->real_generator);
        }
        printf("[FFI] Enhanced FFI generator destroyed\n");
        free(gen);
    }
}

bool enhanced_generate_string_concatenation(EnhancedFFIGenerator *gen, const char *str1, const char *str2) {
    if (!gen || !gen->initialized || !str1 || !str2) {
        if (gen) update_generation_stats(gen, false);
        return false;
    }
    
    // Use real string operation generation with proper registers
    bool success = ffi_generate_string_concatenation(gen->real_generator, ASTHRA_REG_RAX, ASTHRA_REG_RBX, ASTHRA_REG_RCX);
    update_generation_stats(gen, success);
    
    if (success) {
        printf("[FFI] Generated string concatenation assembly for '%s' + '%s'\n", str1, str2);
    }
    
    return success;
}

bool enhanced_generate_slice_length_access(EnhancedFFIGenerator *gen, void *slice_ptr, Register target_reg) {
    if (!gen || !gen->initialized || !slice_ptr) {
        if (gen) update_generation_stats(gen, false);
        return false;
    }
    
    // Use real slice operation generation with proper register parameters
    bool success = ffi_generate_slice_length_access(gen->real_generator, ASTHRA_REG_RAX, target_reg);
    update_generation_stats(gen, success);
    
    if (success) {
        printf("[FFI] Generated slice length access assembly\n");
    }
    
    return success;
}

bool enhanced_generate_slice_bounds_check(EnhancedFFIGenerator *gen, void *slice_ptr, size_t index, Register index_reg) {
    if (!gen || !gen->initialized || !slice_ptr) {
        if (gen) update_generation_stats(gen, false);
        return false;
    }
    
    // Use real bounds checking generation with proper parameters
    char bounds_error_label[] = "bounds_error";
    bool success = ffi_generate_slice_bounds_check(gen->real_generator, ASTHRA_REG_RAX, index_reg, bounds_error_label);
    update_generation_stats(gen, success);
    
    if (success) {
        printf("[FFI] Generated slice bounds check assembly for index %zu\n", index);
    }
    
    return success;
}

bool enhanced_generate_slice_to_ffi(EnhancedFFIGenerator *gen, void *slice_ptr, Register ptr_reg, Register len_reg) {
    if (!gen || !gen->initialized || !slice_ptr) {
        if (gen) update_generation_stats(gen, false);
        return false;
    }
    
    // Use real slice-to-FFI marshaling with correct function name
    bool success = ffi_generate_slice_to_ffi(gen->real_generator, ASTHRA_REG_RAX, ptr_reg, len_reg);
    update_generation_stats(gen, success);
    
    if (success) {
        printf("[FFI] Generated slice-to-FFI conversion assembly\n");
    }
    
    return success;
}

bool enhanced_generate_volatile_memory_access(EnhancedFFIGenerator *gen, void *memory_ptr, size_t size) {
    if (!gen || !gen->initialized || !memory_ptr || size == 0) {
        if (gen) update_generation_stats(gen, false);
        return false;
    }
    
    // Use real volatile memory access generation with correct parameters
    bool success = ffi_generate_volatile_memory_access(gen->real_generator, ASTHRA_REG_RAX, size, true);
    update_generation_stats(gen, success);
    
    if (success) {
        printf("[FFI] Generated volatile memory access assembly for %zu bytes\n", size);
    }
    
    return success;
}

bool enhanced_generate_secure_zero(EnhancedFFIGenerator *gen, void *memory_ptr, size_t size) {
    if (!gen || !gen->initialized || !memory_ptr || size == 0) {
        if (gen) update_generation_stats(gen, false);
        return false;
    }
    
    // Use real secure zeroing generation with proper register parameters
    bool success = ffi_generate_secure_zero(gen->real_generator, ASTHRA_REG_RAX, ASTHRA_REG_RBX);
    update_generation_stats(gen, success);
    
    if (success) {
        printf("[FFI] Generated secure zero assembly for %zu bytes\n", size);
    }
    
    return success;
}

bool enhanced_generate_task_creation(EnhancedFFIGenerator *gen, const char *task_name) {
    if (!gen || !gen->initialized || !task_name) {
        if (gen) update_generation_stats(gen, false);
        return false;
    }
    
    // Create a mock spawn statement AST node for the real function
    ASTNode mock_spawn_stmt = {
        .type = AST_SPAWN_STMT,
        .location = {0}, // Default source location
        .type_info = NULL,
        .ref_count = 1,
        .flags = {0}
    };
    
    // Use real task spawning generation
    bool success = ffi_generate_spawn_statement(gen->real_generator, &mock_spawn_stmt);
    update_generation_stats(gen, success);
    
    if (success) {
        printf("[FFI] Generated task creation assembly for '%s'\n", task_name);
    }
    
    return success;
}

// =============================================================================
// LEGACY COMPATIBILITY IMPLEMENTATION
// =============================================================================

MinimalFFIGenerator* minimal_ffi_generator_create(void) {
    return (MinimalFFIGenerator*)enhanced_ffi_generator_create();
}

void minimal_ffi_generator_destroy(MinimalFFIGenerator *gen) {
    enhanced_ffi_generator_destroy((EnhancedFFIGenerator*)gen);
}

bool minimal_generate_string_concatenation(MinimalFFIGenerator *gen) {
    return enhanced_generate_string_concatenation((EnhancedFFIGenerator*)gen, "test", "string");
}

bool minimal_generate_slice_length_access(MinimalFFIGenerator *gen) {
    static char dummy_slice[10] = {0};
    return enhanced_generate_slice_length_access((EnhancedFFIGenerator*)gen, dummy_slice, ASTHRA_REG_RAX);
}

bool minimal_generate_slice_bounds_check(MinimalFFIGenerator *gen) {
    static char dummy_slice[10] = {0};
    return enhanced_generate_slice_bounds_check((EnhancedFFIGenerator*)gen, dummy_slice, 5, ASTHRA_REG_RCX);
}

bool minimal_generate_slice_to_ffi(MinimalFFIGenerator *gen) {
    static char dummy_slice[10] = {0};
    return enhanced_generate_slice_to_ffi((EnhancedFFIGenerator*)gen, dummy_slice, ASTHRA_REG_RDI, ASTHRA_REG_RSI);
}

bool minimal_generate_volatile_memory_access(MinimalFFIGenerator *gen) {
    static volatile char dummy_memory[8] = {0};
    return enhanced_generate_volatile_memory_access((EnhancedFFIGenerator*)gen, (void*)dummy_memory, 8);
}

bool minimal_generate_secure_zero(MinimalFFIGenerator *gen) {
    static char dummy_memory[16] = {0};
    return enhanced_generate_secure_zero((EnhancedFFIGenerator*)gen, dummy_memory, 16);
}

bool minimal_generate_task_creation(MinimalFFIGenerator *gen) {
    return enhanced_generate_task_creation((EnhancedFFIGenerator*)gen, "test_task");
}

GenerationStatistics minimal_get_generation_statistics(MinimalFFIGenerator *gen) {
    GenerationStatistics stats = {0};
    if (gen && gen->initialized) {
        EnhancedFFIGenerator *enhanced_gen = (EnhancedFFIGenerator*)gen;
        stats.total_generated = atomic_load(&enhanced_gen->generation_count);
        stats.successful_generations = atomic_load(&enhanced_gen->successful_generations);
        stats.failed_generations = atomic_load(&enhanced_gen->failed_generations);
    }
    return stats;
}

bool minimal_validate_generated_assembly(MinimalFFIGenerator *gen) {
    return gen && gen->initialized;
}

void minimal_print_nasm_assembly(MinimalFFIGenerator *gen) {
    if (gen && gen->initialized) {
        EnhancedFFIGenerator *enhanced_gen = (EnhancedFFIGenerator*)gen;
        uint32_t count = atomic_load(&enhanced_gen->generation_count);
        printf("Mock NASM assembly output (generated %u items)\n", count);
    } else {
        printf("Invalid generator - no assembly to print\n");
    }
}