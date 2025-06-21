/**
 * Asthra Programming Language Runtime Safety - FFI Safety & Tracking
 * Header file for FFI pointer tracking, variadic validation, and FFI call logging
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_SAFETY_FFI_TRACKING_H
#define ASTHRA_SAFETY_FFI_TRACKING_H

#include "asthra_safety.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// FFI SAFETY & TRACKING FUNCTIONS
// =============================================================================

/**
 * Unregisters an FFI pointer from tracking system
 * @param ptr Pointer to unregister
 * @return 0 on success, -1 on error
 */
int asthra_safety_unregister_ffi_pointer(void *ptr);

/**
 * Gets tracking information for an FFI pointer
 * @param ptr Pointer to look up
 * @return Pointer tracker information or NULL if not found
 */
AsthraFFIPointerTracker *asthra_safety_get_ffi_pointer_info(void *ptr);

/**
 * Validates variadic function call arguments
 * @param func_ptr Function pointer being called
 * @param args Array of variadic arguments
 * @param arg_count Number of arguments
 * @param expected_types Expected argument types
 * @param expected_count Expected number of arguments
 * @return Variadic validation result
 */
AsthraVariadicValidation asthra_safety_validate_variadic_call(void *func_ptr, AsthraVarArg *args, size_t arg_count,
                                                              AsthraVarArgType *expected_types, size_t expected_count);

/**
 * Logs an FFI function call for debugging and monitoring
 * @param function_name Name of the function being called
 * @param function_ptr Pointer to the function
 * @param args Array of argument pointers
 * @param arg_count Number of arguments
 */
void asthra_safety_log_ffi_call(const char *function_name, void *function_ptr, void **args, size_t arg_count);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SAFETY_FFI_TRACKING_H 
