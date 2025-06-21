/**
 * Asthra Safe C Memory Interface v1.0
 * Enhanced FFI Memory Management with Slice Support and Pattern Matching
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * DESIGN GOALS:
 * - Safe boundary between Asthra GC and C manual memory
 * - Formalized slice management with SliceHeader<T> structure
 * - Pattern matching integration for systematic error handling
 * - Support for varied data sizes (PQC-ready)
 * - Thread-safe operations with zero-cost abstractions
 * - Pure C99 compatibility
 */

#ifndef ASTHRA_FFI_MEMORY_H
#define ASTHRA_FFI_MEMORY_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CORE FFI MEMORY TYPES
// =============================================================================

/**
 * Memory zone hints for allocation strategy
 */
typedef enum {
    ASTHRA_ZONE_HINT_GC,           // Prefer GC-managed heap
    ASTHRA_ZONE_HINT_MANUAL,       // Prefer manual memory zone
    ASTHRA_ZONE_HINT_PINNED,       // Prefer pinned memory zone
    ASTHRA_ZONE_HINT_STACK,        // Stack-allocated (temporary)
    ASTHRA_ZONE_HINT_SECURE        // Secure memory (locked, zeroed on free)
} AsthraMemoryZoneHint;

/**
 * Ownership transfer semantics for FFI operations
 */
typedef enum {
    ASTHRA_OWNERSHIP_TRANSFER_FULL,    // Full ownership transfer
    ASTHRA_OWNERSHIP_TRANSFER_NONE,    // No ownership transfer (borrowed)
    ASTHRA_OWNERSHIP_TRANSFER_SHARED   // Shared ownership (reference counted)
} AsthraOwnershipTransfer;

/**
 * Enhanced FFI slice header with comprehensive metadata
 */
typedef struct {
    void *ptr;                         // Pointer to data
    size_t len;                        // Number of elements
    size_t cap;                        // Capacity (for mutable slices)
    size_t element_size;               // Size of each element in bytes
    uint32_t type_id;                  // Runtime type identifier
    AsthraMemoryZoneHint zone_hint;    // Memory zone hint
    AsthraOwnershipTransfer ownership; // Ownership semantics
    bool is_mutable;                   // Mutability flag
    bool bounds_checking;              // Enable runtime bounds checking
    uint32_t magic;                    // Magic number for validation
} AsthraFFISliceHeader;

#define ASTHRA_SLICE_MAGIC 0x534C4943  // "SLIC" in hex

/**
 * Result type for FFI operations with enhanced error information
 */
typedef enum {
    ASTHRA_FFI_RESULT_OK,
    ASTHRA_FFI_RESULT_ERR
} AsthraFFIResultTag;

typedef struct {
    AsthraFFIResultTag tag;
    union {
        struct {
            void *value;
            size_t value_size;
            uint32_t value_type_id;
            AsthraOwnershipTransfer ownership;
        } ok;
        struct {
            int error_code;
            char error_message[256];
            const char *error_source;
            void *error_context;
        } err;
    } data;
} AsthraFFIResult;

/**
 * String type for FFI operations
 */
typedef struct {
    char *data;                        // UTF-8 encoded string data
    size_t len;                        // Length in bytes
    size_t cap;                        // Capacity in bytes
    AsthraOwnershipTransfer ownership; // Ownership semantics
    bool is_mutable;                   // Mutability flag
} AsthraFFIString;

/**
 * Variant type for variadic function arguments
 */
typedef enum {
    ASTHRA_VARIANT_NULL,
    ASTHRA_VARIANT_BOOL,
    ASTHRA_VARIANT_I8, ASTHRA_VARIANT_U8,
    ASTHRA_VARIANT_I16, ASTHRA_VARIANT_U16,
    ASTHRA_VARIANT_I32, ASTHRA_VARIANT_U32,
    ASTHRA_VARIANT_I64, ASTHRA_VARIANT_U64,
    ASTHRA_VARIANT_F32, ASTHRA_VARIANT_F64,
    ASTHRA_VARIANT_PTR,
    ASTHRA_VARIANT_STRING,
    ASTHRA_VARIANT_SLICE
} AsthraVariantType;

typedef struct {
    AsthraVariantType type;
    union {
        bool bool_val;
        int8_t i8_val; uint8_t u8_val;
        int16_t i16_val; uint16_t u16_val;
        int32_t i32_val; uint32_t u32_val;
        int64_t i64_val; uint64_t u64_val;
        float f32_val; double f64_val;
        void *ptr_val;
        AsthraFFIString string_val;
        AsthraFFISliceHeader slice_val;
    } value;
} AsthraVariant;

typedef struct {
    AsthraVariant *args;
    size_t count;
    size_t capacity;
} AsthraVariantArray;

// =============================================================================
// CORE FFI MEMORY ALLOCATION FUNCTIONS
// =============================================================================

/**
 * Allocate memory with zone hint and ownership tracking
 * size Size in bytes to allocate
 * zone_hint Preferred memory zone
 * Pointer to allocated memory or NULL on failure
 */
void* Asthra_ffi_alloc(size_t size, AsthraMemoryZoneHint zone_hint);

/**
 * Free memory with zone hint and ownership validation
 * ptr Pointer to memory to free
 * current_zone_hint Current zone hint for validation
 */
void Asthra_ffi_free(void* ptr, AsthraMemoryZoneHint current_zone_hint);

/**
 * Reallocate memory with zone migration support
 * ptr Existing pointer (can be NULL)
 * new_size New size in bytes
 * zone_hint Target zone hint
 * Pointer to reallocated memory or NULL on failure
 */
void* Asthra_ffi_realloc(void* ptr, size_t new_size, AsthraMemoryZoneHint zone_hint);

/**
 * Allocate zeroed memory
 * size Size in bytes to allocate
 * zone_hint Preferred memory zone
 * Pointer to zeroed memory or NULL on failure
 */
void* Asthra_ffi_alloc_zeroed(size_t size, AsthraMemoryZoneHint zone_hint);

// =============================================================================
// FORMALIZED SLICE MANAGEMENT
// =============================================================================

/**
 * Create slice from raw C array pointer and length
 * c_array_ptr Pointer to C array
 * len Number of elements
 * element_size Size of each element in bytes
 * is_mutable Whether slice is mutable
 * ownership Ownership transfer semantics
 * Initialized slice header
 */
AsthraFFISliceHeader Asthra_slice_from_raw_parts(void* c_array_ptr, size_t len, 
                                              size_t element_size, bool is_mutable, 
                                              AsthraOwnershipTransfer ownership);

/**
 * Create new slice with specified capacity
 * element_size Size of each element in bytes
 * len Initial length
 * cap Initial capacity
 * zone_hint Memory zone hint
 * Initialized slice header
 */
AsthraFFISliceHeader Asthra_slice_new(size_t element_size, size_t len, size_t cap, 
                                   AsthraMemoryZoneHint zone_hint);

/**
 * Get pointer from slice with bounds validation
 * slice Slice header
 * Pointer to slice data or NULL if invalid
 */
void* Asthra_slice_get_ptr(AsthraFFISliceHeader slice);

/**
 * Get length from slice
 * slice Slice header
 * Number of elements in slice
 */
size_t Asthra_slice_get_len(AsthraFFISliceHeader slice);

/**
 * Get capacity from slice
 * slice Slice header
 * Capacity of slice
 */
size_t Asthra_slice_get_cap(AsthraFFISliceHeader slice);

/**
 * Get element size from slice
 * slice Slice header
 * Size of each element in bytes
 */
size_t Asthra_slice_get_element_size(AsthraFFISliceHeader slice);

/**
 * Perform bounds check on slice access
 * slice Slice header
 * index Index to check
 * Result indicating success or bounds error
 */
AsthraFFIResult Asthra_slice_bounds_check(AsthraFFISliceHeader slice, size_t index);

/**
 * Get element from slice with bounds checking
 * slice Slice header
 * index Element index
 * out_element Output buffer for element (must be element_size bytes)
 * Result indicating success or error
 */
AsthraFFIResult Asthra_slice_get_element(AsthraFFISliceHeader slice, size_t index, void* out_element);

/**
 * Set element in slice with bounds checking
 * slice Slice header
 * index Element index
 * element Element data to set (must be element_size bytes)
 * Result indicating success or error
 */
AsthraFFIResult Asthra_slice_set_element(AsthraFFISliceHeader slice, size_t index, const void* element);

/**
 * Create subslice from existing slice
 * slice Source slice
 * start Start index (inclusive)
 * end End index (exclusive)
 * Result containing subslice or error
 */
AsthraFFIResult Asthra_slice_subslice(AsthraFFISliceHeader slice, size_t start, size_t end);

/**
 * Free slice and its underlying memory if owned
 * slice Slice header to free
 */
void Asthra_slice_free(AsthraFFISliceHeader slice);

/**
 * Validate slice header integrity
 * slice Slice header to validate
 * true if slice is valid, false otherwise
 */
bool Asthra_slice_is_valid(AsthraFFISliceHeader slice);

// =============================================================================
// STRING OPERATIONS WITH FFI INTEGRATION
// =============================================================================

/**
 * Create FFI string from C string
 * cstr C string (null-terminated)
 * ownership Ownership transfer semantics
 * FFI string structure
 */
AsthraFFIString Asthra_string_from_cstr(const char* cstr, AsthraOwnershipTransfer ownership);

/**
 * Concatenate two FFI strings
 * a First string
 * b Second string
 * Result containing concatenated string or error
 */
AsthraFFIResult Asthra_string_concat(AsthraFFIString a, AsthraFFIString b);

/**
 * String interpolation with variant arguments
 * template Template string with {} placeholders
 * args Array of variant arguments
 * Result containing interpolated string or error
 */
AsthraFFIResult Asthra_string_interpolate(const char* template, AsthraVariantArray args);

/**
 * Convert FFI string to C string
 * s FFI string
 * transfer_ownership Whether to transfer ownership to caller
 * C string (caller must free if transfer_ownership is true)
 */
char* Asthra_string_to_cstr(AsthraFFIString s, bool transfer_ownership);

/**
 * Convert FFI string to slice
 * s FFI string
 * Slice header representing string bytes
 */
AsthraFFISliceHeader Asthra_string_to_slice(AsthraFFIString s);

/**
 * Free FFI string
 * s FFI string to free
 */
void Asthra_string_free(AsthraFFIString s);

// =============================================================================
// PATTERN MATCHING ERROR HANDLING
// =============================================================================

/**
 * Create successful result
 * value Pointer to value data
 * value_size Size of value in bytes
 * type_id Type identifier
 * ownership Ownership transfer semantics
 * Result structure with OK status
 */
AsthraFFIResult Asthra_result_ok(void* value, size_t value_size, uint32_t type_id, 
                                AsthraOwnershipTransfer ownership);

/**
 * Create error result
 * error_code Error code
 * error_message Error message
 * error_source Source of error (function name, etc.)
 * error_context Additional error context
 * Result structure with ERR status
 */
AsthraFFIResult Asthra_result_err(int error_code, const char* error_message, 
                                 const char* error_source, void* error_context);

/**
 * Check if result is OK
 * result Result to check
 * true if result is OK, false otherwise
 */
bool Asthra_result_is_ok(AsthraFFIResult result);

/**
 * Check if result is error
 * result Result to check
 * true if result is error, false otherwise
 */
bool Asthra_result_is_err(AsthraFFIResult result);

/**
 * Unwrap OK value from result (unsafe - check is_ok first)
 * result Result to unwrap
 * Pointer to value data
 */
void* Asthra_result_unwrap_ok(AsthraFFIResult result);

/**
 * Get error code from result (unsafe - check is_err first)
 * result Result to get error from
 * Error code
 */
int Asthra_result_get_error_code(AsthraFFIResult result);

/**
 * Get error message from result (unsafe - check is_err first)
 * result Result to get error from
 * Error message string
 */
const char* Asthra_result_get_error_message(AsthraFFIResult result);

/**
 * Free result and its contained data
 * result Result to free
 */
void Asthra_result_free(AsthraFFIResult result);

// =============================================================================
// VARIANT ARRAY OPERATIONS
// =============================================================================

/**
 * Create new variant array
 * initial_capacity Initial capacity
 * Initialized variant array
 */
AsthraVariantArray Asthra_variant_array_new(size_t initial_capacity);

/**
 * Add variant to array
 * array Variant array
 * variant Variant to add
 * Result indicating success or error
 */
AsthraFFIResult Asthra_variant_array_push(AsthraVariantArray* array, AsthraVariant variant);

/**
 * Get variant from array
 * array Variant array
 * index Index of variant
 * Result containing variant or error
 */
AsthraFFIResult Asthra_variant_array_get(AsthraVariantArray array, size_t index);

/**
 * Free variant array
 * array Variant array to free
 */
void Asthra_variant_array_free(AsthraVariantArray array);

// =============================================================================
// SECURE MEMORY OPERATIONS
// =============================================================================

/**
 * Securely zero memory using slice reference
 * slice_ref Slice containing memory to zero
 */
void Asthra_secure_zero_slice(AsthraFFISliceHeader slice_ref);

/**
 * Securely zero memory region
 * ptr Pointer to memory
 * size Size in bytes
 */
void Asthra_secure_zero(void* ptr, size_t size);

/**
 * Allocate secure memory (locked, zeroed on free)
 * size Size in bytes
 * Pointer to secure memory or NULL on failure
 */
void* Asthra_secure_alloc(size_t size);

/**
 * Free secure memory with automatic zeroing
 * ptr Pointer to secure memory
 * size Size in bytes (for verification)
 */
void Asthra_secure_free(void* ptr, size_t size);

// =============================================================================
// OWNERSHIP TRACKING AND VALIDATION
// =============================================================================

/**
 * Register external pointer for ownership tracking
 * ptr Pointer to register
 * size Size of memory region
 * ownership Ownership semantics
 * cleanup Cleanup function (can be NULL)
 * Result indicating success or error
 */
AsthraFFIResult Asthra_ownership_register(void* ptr, size_t size, 
                                         AsthraOwnershipTransfer ownership,
                                         void (*cleanup)(void*));

/**
 * Unregister external pointer
 * ptr Pointer to unregister
 * Result indicating success or error
 */
AsthraFFIResult Asthra_ownership_unregister(void* ptr);

/**
 * Transfer ownership of pointer
 * ptr Pointer to transfer
 * new_ownership New ownership semantics
 * Result indicating success or error
 */
AsthraFFIResult Asthra_ownership_transfer(void* ptr, AsthraOwnershipTransfer new_ownership);

/**
 * Query ownership of pointer
 * ptr Pointer to query
 * Result containing ownership information or error
 */
AsthraFFIResult Asthra_ownership_query(void* ptr);

// =============================================================================
// DEBUGGING AND DIAGNOSTICS
// =============================================================================

/**
 * Get FFI memory statistics
 * Statistics structure
 */
typedef struct {
    size_t total_allocations;
    size_t total_frees;
    size_t current_allocations;
    size_t peak_allocations;
    size_t bytes_allocated;
    size_t bytes_freed;
    size_t current_bytes;
    size_t peak_bytes;
    size_t slice_count;
    size_t string_count;
    size_t result_count;
} AsthraFFIMemoryStats;

AsthraFFIMemoryStats Asthra_ffi_get_memory_stats(void);

/**
 * Reset FFI memory statistics
 */
void Asthra_ffi_reset_memory_stats(void);

/**
 * Validate all tracked pointers
 * Result indicating validation status
 */
AsthraFFIResult Asthra_ffi_validate_all_pointers(void);

/**
 * Dump memory state for debugging
 * output_file File to write debug output (NULL for stderr)
 */
void Asthra_ffi_dump_memory_state(FILE* output_file);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_FFI_MEMORY_H 
