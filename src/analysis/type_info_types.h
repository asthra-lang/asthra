#ifndef ASTHRA_TYPE_INFO_TYPES_H
#define ASTHRA_TYPE_INFO_TYPES_H

#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct TypeDescriptor TypeDescriptor;
typedef struct ASTNode ASTNode;
typedef struct SymbolEntry SymbolEntry;

// =============================================================================
// TYPE INFO SYSTEM - BRIDGE BETWEEN AST, SEMANTIC ANALYSIS, AND RUNTIME
// =============================================================================

/**
 * TypeInfo provides a unified interface for type information across the
 * Asthra compiler pipeline. It bridges AST nodes, semantic analysis,
 * and runtime type systems.
 */
typedef struct TypeInfo {
    // Core type identification
    uint32_t type_id; // Unique runtime type identifier
    char *name;       // Type name (e.g., "i32", "Point", "Result<string, Error>")

    // Type categorization
    enum {
        TYPE_INFO_PRIMITIVE,   // Built-in primitive types
        TYPE_INFO_STRUCT,      // User-defined struct types
        TYPE_INFO_SLICE,       // Slice types []T
        TYPE_INFO_POINTER,     // Pointer types *T, *mut T
        TYPE_INFO_RESULT,      // Result<T, E> types
        TYPE_INFO_OPTION,      // Option<T> types
        TYPE_INFO_FUNCTION,    // Function types
        TYPE_INFO_ENUM,        // Enum types (future)
        TYPE_INFO_TUPLE,       // Tuple types (T1, T2, ...)
        TYPE_INFO_TASK_HANDLE, // Task handle types TaskHandle<T>
        TYPE_INFO_MODULE,      // Module types (for module access)
        TYPE_INFO_UNKNOWN,     // Unknown/unresolved types
        TYPE_INFO_ERROR        // Error types (for error reporting)
    } category;

    // Memory layout information
    size_t size;      // Size in bytes
    size_t alignment; // Alignment requirements

    // Type properties
    struct {
        bool is_mutable : 1;        // Can be modified
        bool is_owned : 1;          // Owned by current scope
        bool is_borrowed : 1;       // Borrowed reference
        bool is_constant : 1;       // Compile-time constant
        bool is_volatile : 1;       // Volatile memory access
        bool is_atomic : 1;         // Atomic operations
        bool is_ffi_compatible : 1; // Safe for FFI
        bool is_copyable : 1;       // Can be copied
        bool is_movable : 1;        // Can be moved
        bool is_droppable : 1;      // Has destructor
        bool is_zero_sized : 1;     // Zero-sized type
        bool is_unsized : 1;        // Dynamically sized
        uint8_t reserved : 4;       // Reserved for future use
    } flags;

    // Ownership and memory management
    enum {
        OWNERSHIP_INFO_GC,     // Garbage collected
        OWNERSHIP_INFO_C,      // C-managed memory
        OWNERSHIP_INFO_PINNED, // Pinned memory
        OWNERSHIP_INFO_STACK,  // Stack allocated
        OWNERSHIP_INFO_STATIC  // Static/global memory
    } ownership;

    // Type-specific data
    union {
        // Primitive types
        struct {
            enum PrimitiveInfoKind {
                PRIMITIVE_INFO_I8,
                PRIMITIVE_INFO_I16,
                PRIMITIVE_INFO_I32,
                PRIMITIVE_INFO_I64,
                PRIMITIVE_INFO_I128,
                PRIMITIVE_INFO_U8,
                PRIMITIVE_INFO_U16,
                PRIMITIVE_INFO_U32,
                PRIMITIVE_INFO_U64,
                PRIMITIVE_INFO_U128,
                PRIMITIVE_INFO_F32,
                PRIMITIVE_INFO_F64,
                PRIMITIVE_INFO_BOOL,
                PRIMITIVE_INFO_CHAR,
                PRIMITIVE_INFO_STRING,
                PRIMITIVE_INFO_VOID,
                PRIMITIVE_INFO_USIZE,
                PRIMITIVE_INFO_ISIZE,
                PRIMITIVE_INFO_NEVER
            } kind;
            bool is_signed;  // For numeric types
            bool is_integer; // Integer vs floating point
        } primitive;

        // Struct types
        struct {
            SymbolEntry **fields;  // Field symbol entries
            size_t field_count;    // Number of fields
            size_t *field_offsets; // Field memory offsets
            bool is_packed;        // Packed struct layout
            bool has_methods;      // Has impl block methods
        } struct_info;

        // Slice types
        struct {
            struct TypeInfo *element_type; // Element type information
            bool is_mutable;               // Mutable slice
        } slice;

        // Pointer types
        struct {
            struct TypeInfo *pointee_type; // Pointed-to type
            bool is_mutable;               // Mutable pointer
        } pointer;

        // Result types
        struct {
            struct TypeInfo *ok_type;  // Success type
            struct TypeInfo *err_type; // Error type
        } result;

        // Option types
        struct {
            struct TypeInfo *value_type; // Inner value type
        } option;

        // Function types
        struct {
            struct TypeInfo **param_types; // Parameter types
            size_t param_count;            // Number of parameters
            struct TypeInfo *return_type;  // Return type
            bool is_extern;                // External function
            const char *extern_name;       // External library name
        } function;

        // Module types (for module access)
        struct {
            char *module_name;     // Module name
            SymbolEntry **exports; // Exported symbols
            size_t export_count;   // Number of exports
        } module;

        // Task handle types
        struct {
            struct TypeInfo *result_type; // Result type T in TaskHandle<T>
        } task_handle;

        // Tuple types
        struct {
            struct TypeInfo **element_types; // Element types
            size_t element_count;            // Number of elements
            size_t *element_offsets;         // Memory offsets for each element
        } tuple;
    } data;

    // Semantic analysis integration
    const TypeDescriptor *type_descriptor; // Link to semantic analyzer type
    ASTNode *declaration_node;             // AST node where type was declared

    // Runtime integration
    void (*destructor)(void *);                     // Destructor function (if any)
    void (*copy_constructor)(void *, const void *); // Copy constructor
    void (*move_constructor)(void *, void *);       // Move constructor

    // Reference counting for memory management
    atomic_uint_fast32_t ref_count;

    // Debug and introspection
    const char *source_file;     // Source file where defined
    int source_line;             // Source line number
    uint64_t creation_timestamp; // When type info was created
} TypeInfo;

// =============================================================================
// TYPE INFO STATISTICS
// =============================================================================

/**
 * Get TypeInfo statistics
 */
typedef struct {
    size_t total_types;
    size_t primitive_types;
    size_t struct_types;
    size_t slice_types;
    size_t pointer_types;
    size_t result_types;
    size_t function_types;
    size_t module_types;
    size_t memory_usage;
} TypeInfoStats;

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TYPE_INFO_TYPES_H