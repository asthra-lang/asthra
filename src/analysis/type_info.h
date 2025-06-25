#ifndef ASTHRA_TYPE_INFO_H
#define ASTHRA_TYPE_INFO_H

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
        TYPE_INFO_PRIMITIVE, // Built-in primitive types
        TYPE_INFO_STRUCT,    // User-defined struct types
        TYPE_INFO_SLICE,     // Slice types []T
        TYPE_INFO_POINTER,   // Pointer types *T, *mut T
        TYPE_INFO_RESULT,    // Result<T, E> types
        TYPE_INFO_OPTION,    // Option<T> types
        TYPE_INFO_FUNCTION,  // Function types
        TYPE_INFO_ENUM,      // Enum types (future)
        TYPE_INFO_TUPLE,     // Tuple types (T1, T2, ...)
        TYPE_INFO_MODULE,    // Module types (for module access)
        TYPE_INFO_UNKNOWN,   // Unknown/unresolved types
        TYPE_INFO_ERROR      // Error types (for error reporting)
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
// TYPE INFO LIFECYCLE MANAGEMENT
// =============================================================================

/**
 * Create a new TypeInfo structure
 */
TypeInfo *type_info_create(const char *name, uint32_t type_id);

/**
 * Create TypeInfo from TypeDescriptor (semantic analysis integration)
 */
TypeInfo *type_info_from_descriptor(TypeDescriptor *descriptor);

/**
 * Create TypeInfo from AST type node
 */
TypeInfo *type_info_from_ast_node(ASTNode *type_node);

/**
 * Retain a TypeInfo (increment reference count)
 */
void type_info_retain(TypeInfo *type_info);

/**
 * Release a TypeInfo (decrement reference count, free if zero)
 */
void type_info_release(TypeInfo *type_info);

/**
 * Clone a TypeInfo (deep copy)
 */
TypeInfo *type_info_clone(const TypeInfo *type_info);

// =============================================================================
// TYPE INFO CREATION HELPERS
// =============================================================================

/**
 * Create primitive type info
 */
TypeInfo *type_info_create_primitive(const char *name, int primitive_kind, size_t size);

/**
 * Create struct type info
 */
TypeInfo *type_info_create_struct(const char *name, SymbolEntry **fields, size_t field_count);

/**
 * Create slice type info
 */
TypeInfo *type_info_create_slice(TypeInfo *element_type, bool is_mutable);

/**
 * Create pointer type info
 */
TypeInfo *type_info_create_pointer(TypeInfo *pointee_type, bool is_mutable);

/**
 * Create Result type info
 */
TypeInfo *type_info_create_result(TypeInfo *ok_type, TypeInfo *err_type);

/**
 * Create Option type info
 */
TypeInfo *type_info_create_option(TypeInfo *value_type);

/**
 * Create function type info
 */
TypeInfo *type_info_create_function(TypeInfo **param_types, size_t param_count,
                                    TypeInfo *return_type);

/**
 * Create module type info
 */
TypeInfo *type_info_create_module(const char *module_name, SymbolEntry **exports,
                                  size_t export_count);

/**
 * Create tuple type info
 */
TypeInfo *type_info_create_tuple(TypeInfo **element_types, size_t element_count);

// =============================================================================
// TYPE INFO QUERIES AND OPERATIONS
// =============================================================================

/**
 * Check if two TypeInfo structures represent the same type
 */
bool type_info_equals(const TypeInfo *a, const TypeInfo *b);

/**
 * Check if type A is compatible with type B (for assignment)
 */
bool type_info_is_compatible(const TypeInfo *a, const TypeInfo *b);

/**
 * Check if type A can be cast to type B
 */
bool type_info_can_cast(const TypeInfo *from, const TypeInfo *to);

/**
 * Get the size of a type in bytes
 */
size_t type_info_get_size(const TypeInfo *type_info);

/**
 * Get the alignment of a type in bytes
 */
size_t type_info_get_alignment(const TypeInfo *type_info);

/**
 * Check if type is a numeric type
 */
bool type_info_is_numeric(const TypeInfo *type_info);

/**
 * Check if type is an integer type
 */
bool type_info_is_integer(const TypeInfo *type_info);

/**
 * Check if type is a floating-point type
 */
bool type_info_is_float(const TypeInfo *type_info);

/**
 * Check if type is a signed type
 */
bool type_info_is_signed(const TypeInfo *type_info);

/**
 * Check if type supports comparison operations
 */
bool type_info_is_comparable(const TypeInfo *type_info);

/**
 * Check if type supports logical operations
 */
bool type_info_is_logical(const TypeInfo *type_info);

/**
 * Check if type is FFI-compatible
 */
bool type_info_is_ffi_compatible(const TypeInfo *type_info);

// =============================================================================
// STRUCT TYPE OPERATIONS
// =============================================================================

/**
 * Add a field to a struct type
 */
bool type_info_struct_add_field(TypeInfo *struct_type, const char *field_name, TypeInfo *field_type,
                                size_t offset);

/**
 * Look up a field in a struct type
 */
SymbolEntry *type_info_struct_get_field(const TypeInfo *struct_type, const char *field_name);

/**
 * Get field offset in struct
 */
size_t type_info_struct_get_field_offset(const TypeInfo *struct_type, const char *field_name);

/**
 * Calculate struct layout and field offsets
 */
bool type_info_struct_calculate_layout(TypeInfo *struct_type);

// =============================================================================
// TYPE REGISTRY INTEGRATION
// =============================================================================

/**
 * Register a TypeInfo with the runtime type system
 */
bool type_info_register(TypeInfo *type_info);

/**
 * Look up TypeInfo by type ID
 */
TypeInfo *type_info_lookup_by_id(uint32_t type_id);

/**
 * Look up TypeInfo by name
 */
TypeInfo *type_info_lookup_by_name(const char *name);

/**
 * Get all registered types
 */
TypeInfo **type_info_get_all_types(size_t *count);

// =============================================================================
// AST INTEGRATION
// =============================================================================

/**
 * Attach TypeInfo to an AST node
 */
void ast_node_set_type_info(ASTNode *node, TypeInfo *type_info);

/**
 * Get TypeInfo from an AST node
 */
TypeInfo *ast_node_get_type_info(const ASTNode *node);

/**
 * Propagate type information through AST
 */
bool type_info_propagate_ast(ASTNode *root);

// =============================================================================
// SEMANTIC ANALYSIS INTEGRATION
// =============================================================================

/**
 * Convert TypeDescriptor to TypeInfo
 */
TypeInfo *type_info_from_type_descriptor(const TypeDescriptor *descriptor);

/**
 * Convert TypeInfo to TypeDescriptor
 */
TypeDescriptor *type_descriptor_from_type_info(const TypeInfo *type_info);

/**
 * Synchronize TypeInfo with TypeDescriptor
 */
bool type_info_sync_with_descriptor(TypeInfo *type_info, TypeDescriptor *descriptor);

// =============================================================================
// RUNTIME INTEGRATION
// =============================================================================

/**
 * Get runtime type ID for TypeInfo
 */
uint32_t type_info_get_runtime_id(const TypeInfo *type_info);

/**
 * Register TypeInfo with runtime type system
 */
bool type_info_register_runtime(TypeInfo *type_info);

/**
 * Create runtime type descriptor from TypeInfo
 */
void *type_info_create_runtime_descriptor(const TypeInfo *type_info);

// =============================================================================
// DEBUG AND INTROSPECTION
// =============================================================================

/**
 * Print TypeInfo for debugging
 */
void type_info_print(const TypeInfo *type_info, int indent);

/**
 * Get string representation of TypeInfo
 */
char *type_info_to_string(const TypeInfo *type_info);

/**
 * Validate TypeInfo structure
 */
bool type_info_validate(const TypeInfo *type_info);

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

TypeInfoStats type_info_get_stats(void);

/**
 * Update statistics when a type is created
 */
void type_info_stats_increment(int category);

/**
 * Update statistics when a type is destroyed
 */
void type_info_stats_decrement(int category);

/**
 * Get statistics for a type category
 */
int type_info_stats_get(int category);

/**
 * Reset all statistics counters
 */
void type_info_stats_reset(void);

/**
 * Update statistics after category is set
 */
void type_info_update_stats(TypeInfo *type_info);

// =============================================================================
// BUILT-IN TYPE CONSTANTS
// =============================================================================

// Built-in primitive types (initialized at startup)
extern TypeInfo *TYPE_INFO_I8;
extern TypeInfo *TYPE_INFO_I16;
extern TypeInfo *TYPE_INFO_I32;
extern TypeInfo *TYPE_INFO_I64;
extern TypeInfo *TYPE_INFO_I128;
extern TypeInfo *TYPE_INFO_U8;
extern TypeInfo *TYPE_INFO_U16;
extern TypeInfo *TYPE_INFO_U32;
extern TypeInfo *TYPE_INFO_U64;
extern TypeInfo *TYPE_INFO_U128;
extern TypeInfo *TYPE_INFO_F32;
extern TypeInfo *TYPE_INFO_F64;
extern TypeInfo *TYPE_INFO_BOOL;
extern TypeInfo *TYPE_INFO_CHAR;
extern TypeInfo *TYPE_INFO_STRING;
extern TypeInfo *TYPE_INFO_VOID;
extern TypeInfo *TYPE_INFO_USIZE;
extern TypeInfo *TYPE_INFO_ISIZE;

/**
 * Initialize built-in types
 */
bool type_info_init_builtins(void);

/**
 * Cleanup built-in types
 */
void type_info_cleanup_builtins(void);

// =============================================================================
// TYPE ID ALLOCATION
// =============================================================================

/**
 * Allocate a new unique type ID
 */
uint32_t type_info_allocate_id(void);

// =============================================================================
// TYPE REGISTRY OPERATIONS
// =============================================================================

/**
 * Clean up the type registry
 */
void type_info_registry_cleanup(void);

// External registry variables (for internal use by type_info modules)
extern TypeInfo **g_type_registry;
extern size_t g_type_registry_size;
extern pthread_mutex_t g_type_registry_mutex;

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TYPE_INFO_H
