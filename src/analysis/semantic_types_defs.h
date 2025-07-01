/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Type System Definitions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Type descriptors, primitive kinds, and type system structures
 */

#ifndef ASTHRA_SEMANTIC_TYPES_DEFS_H
#define ASTHRA_SEMANTIC_TYPES_DEFS_H

#include "../parser/ast.h"
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct SymbolTable SymbolTable;

// =============================================================================
// TYPE SYSTEM DEFINITIONS WITH C17 FEATURES
// =============================================================================

// Primitive type kinds
typedef enum {
    PRIMITIVE_VOID,
    PRIMITIVE_BOOL,
    PRIMITIVE_I8,
    PRIMITIVE_I16,
    PRIMITIVE_I32,
    PRIMITIVE_I64,
    PRIMITIVE_I128,
    PRIMITIVE_ISIZE,
    PRIMITIVE_U8,
    PRIMITIVE_U16,
    PRIMITIVE_U32,
    PRIMITIVE_U64,
    PRIMITIVE_U128,
    PRIMITIVE_USIZE,
    PRIMITIVE_F32,
    PRIMITIVE_F64,
    PRIMITIVE_CHAR,
    PRIMITIVE_STRING,
    PRIMITIVE_NEVER, // Never type for functions that don't return
    PRIMITIVE_COUNT
} PrimitiveKind;

// C17 static assertion for primitive kinds
_Static_assert(PRIMITIVE_COUNT <= 32, "Primitive kinds must fit in 5 bits");

// Type categories for semantic analysis
typedef enum {
    TYPE_PRIMITIVE,
    TYPE_STRUCT,
    TYPE_ENUM,
    TYPE_POINTER,
    TYPE_SLICE,
    TYPE_ARRAY, // Fixed-size array type [N]T
    TYPE_RESULT,
    TYPE_OPTION, // Option<T> type
    TYPE_FUNCTION,
    TYPE_BUILTIN,          // For builtin types like string
    TYPE_INTEGER,          // Integer types (i8, i16, i32, i64, u8, u16, u32, u64)
    TYPE_FLOAT,            // Floating point types (f32, f64)
    TYPE_BOOL,             // Boolean type
    TYPE_GENERIC_INSTANCE, // Instantiated generic types (e.g., Vec<i32>)
    TYPE_TUPLE,            // Tuple types (T1, T2, ...)
    TYPE_TASK_HANDLE,      // Task handle types (TaskHandle<T>)
    TYPE_UNKNOWN,
    TYPE_ERROR,
    TYPE_COUNT // For validation
} TypeCategory;

// C17 static assertion for type category enum
_Static_assert(TYPE_COUNT <= 32, "Type categories must fit in 5 bits for compact storage");

// Type flags using C17 bitfields
typedef struct {
    bool is_mutable : 1;
    bool is_owned : 1;
    bool is_borrowed : 1;
    bool is_constant : 1;
    bool is_volatile : 1;
    bool is_atomic : 1;
    bool is_ffi_compatible : 1;
    uint8_t reserved : 1;
} TypeFlags;

// Forward declaration for TypeDescriptor
typedef struct TypeDescriptor TypeDescriptor;

// Type descriptor with C17 designated initializers support
struct TypeDescriptor {
    TypeCategory category;
    TypeFlags flags;
    size_t size;
    size_t alignment;
    const char *name;

    // Type-specific data
    union {
        struct {
            int primitive_kind; // int, float, bool, etc.
        } primitive;

        struct {
            SymbolTable *fields;
            size_t field_count;
            SymbolTable *methods; // Methods defined in impl blocks
        } struct_type;

        struct {
            size_t variant_count;
            SymbolTable *variants; // Symbol table for enum variants
        } enum_type;

        struct {
            TypeDescriptor *pointee_type;
        } pointer;

        struct {
            TypeDescriptor *element_type;
        } slice;

        struct {
            TypeDescriptor *ok_type;
            TypeDescriptor *err_type;
        } result;

        struct {
            TypeDescriptor *value_type;
        } option;

        struct {
            TypeDescriptor **param_types;
            size_t param_count;
            TypeDescriptor *return_type;

            // FFI-specific metadata
            bool is_extern;               // True if this is an extern function
            const char *extern_name;      // External library name (e.g., "libc")
            ASTNodeList *ffi_annotations; // FFI annotations (transfer_full, borrowed, etc.)
            size_t ffi_annotation_count;  // Number of FFI annotations
            bool requires_ffi_marshaling; // True if parameters/return need marshaling
        } function;

        struct {
            TypeDescriptor *base_type;  // The generic type template (e.g., Vec<T>)
            TypeDescriptor **type_args; // The concrete type arguments (e.g., [i32])
            size_t type_arg_count;      // Number of type arguments
            char *canonical_name;       // Cached name like "Vec<i32>"
        } generic_instance;

        struct {
            TypeDescriptor **element_types; // Array of element types
            size_t element_count;           // Number of elements
            size_t *element_offsets;        // Memory offsets for each element
        } tuple;

        struct {
            TypeDescriptor *element_type; // Type of array elements
            size_t size;                  // Number of elements (compile-time constant)
        } array;
        
        struct {
            TypeDescriptor *result_type;  // Type T in TaskHandle<T>
        } task_handle;
    } data;

    // C17 atomic reference counting for thread-safe type management
    atomic_uint_fast32_t ref_count;
};

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_TYPES_DEFS_H
