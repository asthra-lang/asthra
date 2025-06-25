/**
 * FFI Test Stubs - Parameter Marshaling
 *
 * Header file for FFI parameter marshaling functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_FFI_STUBS_MARSHALING_H
#define ASTHRA_FFI_STUBS_MARSHALING_H

#include "code_generator_registers.h"
#include "ffi_stubs_types.h"

// =============================================================================
// FFI MARSHALING FUNCTIONS
// =============================================================================

// Initialize and cleanup enhanced FFI marshaler
bool enhanced_ffi_marshaler_init(void);
void enhanced_ffi_marshaler_cleanup(void);

// Enhanced parameter marshaling with real FFI generation
bool enhanced_ffi_generate_parameter_marshaling(void *param, size_t size,
                                                FFIOwnershipTransferType transfer_type,
                                                Register target_reg);

// Enhanced extern function call generation
bool enhanced_ffi_generate_extern_call(const char *function_name, void **params, size_t param_count,
                                       void *return_value);

// Enhanced variadic function call generation
bool enhanced_ffi_generate_variadic_call(const char *function_name, void **params,
                                         size_t param_count, const char *format);

// Get FFI marshaling statistics
FFIMarshalingStats ffi_get_marshaling_stats(void);

// =============================================================================
// LEGACY COMPATIBILITY FUNCTIONS
// =============================================================================

// Legacy compatibility wrappers with different names to avoid conflicts
bool stub_ffi_generate_parameter_marshaling(void *param, size_t size,
                                            FFIOwnershipTransferType transfer_type,
                                            Register target_reg);

bool stub_ffi_generate_extern_call(const char *function_name, void **params, size_t param_count,
                                   void *return_value);

bool stub_ffi_generate_variadic_call(const char *function_name, void **params, size_t param_count,
                                     const char *format);

#endif // ASTHRA_FFI_STUBS_MARSHALING_H