/**
 * FFI Test Stubs - Enhanced FFI Generator
 *
 * Header file for enhanced FFI generator functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_FFI_STUBS_GENERATOR_H
#define ASTHRA_FFI_STUBS_GENERATOR_H

#include "code_generator_registers.h"
#include "ffi_stubs_types.h"

// =============================================================================
// ENHANCED FFI GENERATOR FUNCTIONS
// =============================================================================

// Creation and destruction
EnhancedFFIGenerator *enhanced_ffi_generator_create(void);
void enhanced_ffi_generator_destroy(EnhancedFFIGenerator *gen);

// Code generation functions
bool enhanced_generate_string_concatenation(EnhancedFFIGenerator *gen, const char *str1,
                                            const char *str2);
bool enhanced_generate_slice_length_access(EnhancedFFIGenerator *gen, void *slice_ptr,
                                           Register target_reg);
bool enhanced_generate_slice_bounds_check(EnhancedFFIGenerator *gen, void *slice_ptr, size_t index,
                                          Register index_reg);
bool enhanced_generate_slice_to_ffi(EnhancedFFIGenerator *gen, void *slice_ptr, Register ptr_reg,
                                    Register len_reg);
bool enhanced_generate_volatile_memory_access(EnhancedFFIGenerator *gen, void *memory_ptr,
                                              size_t size);
bool enhanced_generate_secure_zero(EnhancedFFIGenerator *gen, void *memory_ptr, size_t size);
bool enhanced_generate_task_creation(EnhancedFFIGenerator *gen, const char *task_name);

// =============================================================================
// LEGACY COMPATIBILITY FUNCTIONS
// =============================================================================

// Legacy wrapper functions for backward compatibility
MinimalFFIGenerator *minimal_ffi_generator_create(void);
void minimal_ffi_generator_destroy(MinimalFFIGenerator *gen);
bool minimal_generate_string_concatenation(MinimalFFIGenerator *gen);
bool minimal_generate_slice_length_access(MinimalFFIGenerator *gen);
bool minimal_generate_slice_bounds_check(MinimalFFIGenerator *gen);
bool minimal_generate_slice_to_ffi(MinimalFFIGenerator *gen);
bool minimal_generate_volatile_memory_access(MinimalFFIGenerator *gen);
bool minimal_generate_secure_zero(MinimalFFIGenerator *gen);
bool minimal_generate_task_creation(MinimalFFIGenerator *gen);

// Statistics and validation
GenerationStatistics minimal_get_generation_statistics(MinimalFFIGenerator *gen);
bool minimal_validate_generated_assembly(MinimalFFIGenerator *gen);
void minimal_print_nasm_assembly(MinimalFFIGenerator *gen);

#endif // ASTHRA_FFI_STUBS_GENERATOR_H