/**
 * Asthra Programming Language Runtime Safety System
 * Modular Safety System - Umbrella Header for Backward Compatibility
 * 
 * This header provides a unified interface to all safety modules while
 * maintaining backward compatibility with existing code.
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_SAFETY_MODULAR_H
#define ASTHRA_SAFETY_MODULAR_H

/**
 * Asthra Safety System - Modular Umbrella Header
 * Provides full backward compatibility with the original asthra_safety.h
 * 
 * This header includes all modular safety components and provides
 * a unified interface identical to the original monolithic implementation.
 */

// Include all modular components
#include "asthra_safety_common.h"

// Core system management is included in common header
// Individual modules are included automatically through function calls

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// MODULAR SAFETY SYSTEM INFORMATION
// =============================================================================

#define ASTHRA_SAFETY_MODULAR_VERSION_MAJOR 1
#define ASTHRA_SAFETY_MODULAR_VERSION_MINOR 0
#define ASTHRA_SAFETY_MODULAR_VERSION_PATCH 0
#define ASTHRA_SAFETY_MODULAR_VERSION "1.0.0"

typedef struct {
    const char *module_name;
    const char *version;
    size_t functions_count;
    size_t structures_count;
    bool is_available;
} AsthraSafetyModuleInfo;

// =============================================================================
// MODULE INFORMATION API
// =============================================================================

/**
 * Get information about all safety modules
 */
const AsthraSafetyModuleInfo *asthra_safety_get_module_info(size_t *module_count);

/**
 * Get version information for the modular safety system
 */
const char *asthra_safety_get_modular_version(void);

/**
 * Check if all safety modules are available
 */
bool asthra_safety_check_modules_availability(void);

/**
 * Get unified safety statistics across all modules
 */
typedef struct {
    size_t total_type_checks;
    size_t total_boundary_checks;
    size_t total_ffi_operations;
    size_t total_runtime_validations;
    size_t total_security_checks;
    uint64_t total_processing_time_ns;
    size_t active_ffi_pointers;
    size_t active_result_trackers;
    size_t active_task_events;
} AsthraSafetyUnifiedStats;

AsthraSafetyUnifiedStats asthra_safety_get_unified_stats(void);

/**
 * Reset all safety module statistics
 */
void asthra_safety_reset_all_stats(void);

/**
 * Initialize all safety modules with consistent configuration
 */
int asthra_safety_init_all_modules(const AsthraSafetyConfig *config);

// =============================================================================
// CONVENIENCE MACROS FOR MODULAR SAFETY
// =============================================================================

// Enhanced safety check macros that route to appropriate modules
#define ASTHRA_SAFETY_TYPE_CHECK(slice, expected_type, message) \
    do { \
        AsthraTypeSafetyCheck check = asthra_safety_validate_slice_type_safety(slice, expected_type); \
        if (!check.is_valid) { \
            asthra_safety_report_violation(ASTHRA_VIOLATION_TYPE_SAFETY, ASTHRA_SAFETY_LEVEL_STANDARD, \
                                         message ": " check.type_error_message, __FILE__, __LINE__, __func__, NULL, 0); \
        } \
    } while (0)

#define ASTHRA_SAFETY_BOUNDS_CHECK(slice, index, message) \
    do { \
        AsthraBoundaryCheck check = asthra_safety_enhanced_boundary_check(slice, index); \
        if (!check.is_valid) { \
            asthra_safety_report_violation(ASTHRA_VIOLATION_MEMORY_SAFETY, ASTHRA_SAFETY_LEVEL_STANDARD, \
                                         message ": " check.error_details, __FILE__, __LINE__, __func__, NULL, 0); \
        } \
    } while (0)

#define ASTHRA_SAFETY_FFI_REGISTER(ptr, size, transfer, ownership, borrowed) \
    asthra_safety_register_ffi_pointer(ptr, size, transfer, ownership, borrowed, __FILE__, __LINE__)

#define ASTHRA_SAFETY_RESULT_TRACK(result) \
    asthra_safety_register_result_tracker(result, __FILE__ ":" ASTHRA_STRINGIFY(__LINE__))

// Helper macro for stringification
#define ASTHRA_STRINGIFY(x) #x

// =============================================================================
// BACKWARD COMPATIBILITY INTERFACE
// =============================================================================

// All types and functions are already declared in asthra_safety_common.h
// This header serves as a drop-in replacement for asthra_safety.h

// Additional module cleanup functions (internal use)
extern void asthra_safety_memory_ffi_cleanup(void);
extern void asthra_safety_concurrency_errors_cleanup(void);
extern void asthra_safety_security_cleanup(void);

// Enhanced cleanup function that calls all module cleanups
static inline void asthra_safety_cleanup_all_modules(void) {
    asthra_safety_memory_ffi_cleanup();
    asthra_safety_concurrency_errors_cleanup();
    asthra_safety_security_cleanup();
    asthra_safety_cleanup();
}

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SAFETY_MODULAR_H 
