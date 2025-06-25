/**
 * Asthra Memory Manager
 * Integrated Memory Management with Ownership System
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * DESIGN GOALS:
 * - Integration with ownership annotations (#[ownership(gc|c|pinned)])
 * - Semantic analysis validation of memory operations
 * - Compile-time ownership tracking and validation
 * - Runtime memory zone management
 * - FFI boundary safety enforcement
 * - AI-friendly error reporting and diagnostics
 */

#ifndef ASTHRA_MEMORY_MANAGER_H
#define ASTHRA_MEMORY_MANAGER_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../../runtime/asthra_runtime.h"
#include "../parser/ast.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// OWNERSHIP TRACKING TYPES
// =============================================================================

/**
 * Ownership context for semantic analysis
 */
typedef struct {
    OwnershipType ownership_type; // From AST annotations
    AsthraMemoryZone memory_zone; // Runtime memory zone
    bool is_mutable;              // Mutability flag
    bool is_borrowed;             // Borrowed reference flag
    bool requires_cleanup;        // Manual cleanup required
    const char *source_location;  // Source location for debugging
} OwnershipContext;

/**
 * Memory allocation metadata
 */
typedef struct MemoryAllocation {
    void *ptr;                     // Allocated pointer
    size_t size;                   // Allocation size
    OwnershipContext ownership;    // Ownership information
    uint64_t allocation_id;        // Unique allocation ID
    const char *type_name;         // Type name for debugging
    struct MemoryAllocation *next; // Linked list for tracking
} MemoryAllocation;

/**
 * Memory zone statistics
 */
typedef struct {
    atomic_uint_fast64_t total_allocations;
    atomic_uint_fast64_t total_deallocations;
    atomic_size_t current_bytes;
    atomic_size_t peak_bytes;
    atomic_uint_fast32_t active_allocations;
    atomic_uint_fast32_t ownership_violations;
} MemoryZoneStats;

/**
 * Memory manager context
 */
typedef struct {
    MemoryAllocation *allocations[4]; // One list per memory zone
    MemoryZoneStats zone_stats[4];    // Statistics per zone
    pthread_mutex_t allocation_mutex; // Thread safety
    atomic_uint_fast64_t next_allocation_id;
    bool initialized;
    bool strict_mode; // Strict ownership checking
    bool debug_mode;  // Enhanced debugging
} MemoryManager;

// =============================================================================
// OWNERSHIP VALIDATION TYPES
// =============================================================================

/**
 * Ownership validation result
 */
typedef enum {
    OWNERSHIP_VALID,
    OWNERSHIP_INVALID_ANNOTATION,
    OWNERSHIP_ZONE_MISMATCH,
    OWNERSHIP_TRANSFER_VIOLATION,
    OWNERSHIP_LIFETIME_VIOLATION,
    OWNERSHIP_FFI_BOUNDARY_ERROR,
    OWNERSHIP_CIRCULAR_REFERENCE,
    OWNERSHIP_DOUBLE_FREE,
    OWNERSHIP_USE_AFTER_FREE
} OwnershipValidationResult;

/**
 * Ownership validation context
 */
typedef struct {
    const ASTNode *node;       // AST node being validated
    OwnershipContext *context; // Ownership context
    const char *operation;     // Operation being performed
    SourceLocation location;   // Source location
    char error_message[256];   // Detailed error message
} OwnershipValidation;

// =============================================================================
// MEMORY MANAGER INTERFACE
// =============================================================================

/**
 * Initialize the memory manager
 */
bool memory_manager_init(MemoryManager *manager, bool strict_mode, bool debug_mode);

/**
 * Cleanup the memory manager
 */
void memory_manager_cleanup(MemoryManager *manager);

/**
 * Allocate memory with ownership tracking
 */
void *memory_manager_alloc(MemoryManager *manager, size_t size, const OwnershipContext *ownership,
                           const char *type_name);

/**
 * Free memory with ownership validation
 */
bool memory_manager_free(MemoryManager *manager, void *ptr, const OwnershipContext *ownership);

/**
 * Register external allocation (for FFI)
 */
bool memory_manager_register_external(MemoryManager *manager, void *ptr, size_t size,
                                      const OwnershipContext *ownership, const char *type_name);

/**
 * Unregister external allocation
 */
bool memory_manager_unregister_external(MemoryManager *manager, void *ptr);

// =============================================================================
// OWNERSHIP VALIDATION INTERFACE
// =============================================================================

/**
 * Validate ownership annotation on AST node
 */
OwnershipValidationResult validate_ownership_annotation(const ASTNode *node,
                                                        OwnershipValidation *validation);

/**
 * Validate ownership transfer in function call
 */
OwnershipValidationResult validate_ownership_transfer(const ASTNode *call_node,
                                                      const ASTNode *function_decl,
                                                      OwnershipValidation *validation);

/**
 * Validate ownership at FFI boundary
 */
OwnershipValidationResult validate_ffi_ownership(const ASTNode *extern_node,
                                                 const ASTNode *call_node,
                                                 OwnershipValidation *validation);

/**
 * Validate lifetime annotations
 */
OwnershipValidationResult validate_lifetime_annotations(const ASTNode *node,
                                                        OwnershipValidation *validation);

/**
 * Check for ownership violations in expression
 */
OwnershipValidationResult check_ownership_violations(const ASTNode *expr,
                                                     OwnershipValidation *validation);

// =============================================================================
// SEMANTIC ANALYSIS INTEGRATION
// =============================================================================

/**
 * Extract ownership context from AST annotations
 */
bool extract_ownership_context(const ASTNode *node, OwnershipContext *context);

/**
 * Convert ownership type to memory zone
 */
AsthraMemoryZone ownership_to_memory_zone(OwnershipType ownership);

/**
 * Convert memory zone to ownership type
 */
OwnershipType memory_zone_to_ownership(AsthraMemoryZone zone);

/**
 * Validate ownership consistency in struct declaration
 */
bool validate_struct_ownership(const ASTNode *struct_decl, OwnershipValidation *validation);

/**
 * Validate ownership in variable declaration
 */
bool validate_variable_ownership(const ASTNode *var_decl, OwnershipValidation *validation);

/**
 * Validate ownership in function parameters
 */
bool validate_parameter_ownership(const ASTNode *param_list, OwnershipValidation *validation);

// =============================================================================
// RUNTIME INTEGRATION
// =============================================================================

/**
 * Initialize runtime memory zones
 */
bool runtime_init_memory_zones(void);

/**
 * Allocate in specific memory zone
 */
void *runtime_zone_alloc(AsthraMemoryZone zone, size_t size, const char *type_name);

/**
 * Free from specific memory zone
 */
void runtime_zone_free(AsthraMemoryZone zone, void *ptr);

/**
 * Pin memory for FFI operations
 */
bool runtime_pin_memory(void *ptr, size_t size);

/**
 * Unpin memory after FFI operations
 */
void runtime_unpin_memory(void *ptr);

/**
 * Secure zero memory
 */
void runtime_secure_zero(void *ptr, size_t size);

// =============================================================================
// DIAGNOSTICS AND DEBUGGING
// =============================================================================

/**
 * Get memory zone statistics
 */
MemoryZoneStats memory_manager_get_zone_stats(const MemoryManager *manager, AsthraMemoryZone zone);

/**
 * Print memory allocation summary
 */
void memory_manager_print_summary(const MemoryManager *manager);

/**
 * Dump active allocations for debugging
 */
void memory_manager_dump_allocations(const MemoryManager *manager, AsthraMemoryZone zone);

/**
 * Validate all active allocations
 */
bool memory_manager_validate_all(const MemoryManager *manager);

/**
 * Get ownership validation error message
 */
const char *ownership_validation_error_message(OwnershipValidationResult result);

/**
 * Format ownership context for debugging
 */
void format_ownership_context(const OwnershipContext *context, char *buffer, size_t buffer_size);

// =============================================================================
// COMPILE-TIME VALIDATION MACROS
// =============================================================================

#define MEMORY_VALIDATE_OWNERSHIP(node, context)                                                   \
    do {                                                                                           \
        OwnershipValidation validation = {0};                                                      \
        OwnershipValidationResult result = validate_ownership_annotation(node, &validation);       \
        if (result != OWNERSHIP_VALID) {                                                           \
            fprintf(stderr, "Ownership validation failed: %s\n",                                   \
                    ownership_validation_error_message(result));                                   \
        }                                                                                          \
    } while (0)

#define MEMORY_TRACK_ALLOCATION(manager, ptr, size, ownership, type)                               \
    do {                                                                                           \
        if (!(manager)->debug_mode)                                                                \
            break;                                                                                 \
        printf("[MEMORY] Allocated %zu bytes at %p (type: %s, ownership: %d)\n", (size), (ptr),    \
               (type), (ownership)->ownership_type);                                               \
    } while (0)

#define MEMORY_TRACK_DEALLOCATION(manager, ptr)                                                    \
    do {                                                                                           \
        if (!(manager)->debug_mode)                                                                \
            break;                                                                                 \
        printf("[MEMORY] Deallocated pointer %p\n", (ptr));                                        \
    } while (0)

// =============================================================================
// GLOBAL MEMORY MANAGER INSTANCE
// =============================================================================

extern MemoryManager g_memory_manager;

/**
 * Initialize global memory manager
 */
bool init_global_memory_manager(bool strict_mode, bool debug_mode);

/**
 * Cleanup global memory manager
 */
void cleanup_global_memory_manager(void);

/**
 * Get global memory manager instance
 */
MemoryManager *get_global_memory_manager(void);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_MEMORY_MANAGER_H
