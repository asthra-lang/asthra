/**
 * Asthra Programming Language Compiler
 * ELF-64 Object File Writer - Core Types and Interface
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_ELF_WRITER_CORE_H
#define ASTHRA_ELF_WRITER_CORE_H

#include "ffi_assembly_generator.h"
#include "../parser/ast.h"
#include "../analysis/semantic_analyzer.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Platform-specific ELF header inclusion
#ifdef __linux__
    #include <elf.h>
#else
    // Use compatibility header for non-Linux platforms (e.g., macOS)
    #include "elf_compat.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

// C17 static assertions for ELF generation
_Static_assert(sizeof(void*) == 8, "ELF-64 requires 64-bit pointers");
_Static_assert(__STDC_VERSION__ >= 201710L, "C17 standard required for ELF writer");

// Metadata format version
#define ASTHRA_METADATA_VERSION 0x00010000  // v1.0

// Forward declarations
typedef struct ELFWriter ELFWriter;
typedef struct ELFSection ELFSection;
typedef struct ELFSymbol ELFSymbol;
typedef struct ELFRelocation ELFRelocation;
typedef struct AsthraSectionMetadata AsthraSectionMetadata;

// =============================================================================
// ASTHRA-SPECIFIC ELF SECTION TYPES
// =============================================================================

// Enhanced special sections for Asthra language features
typedef enum {
    ASTHRA_SECTION_FFI,              // .Asthra.ffi - FFI metadata
    ASTHRA_SECTION_GC,               // .Asthra.gc - GC roots and ownership
    ASTHRA_SECTION_SECURITY_META,    // .Asthra.security_meta - Security annotations
    ASTHRA_SECTION_PATTERN_MATCHING, // .Asthra.pattern_matching - Pattern match metadata
    ASTHRA_SECTION_STRING_OPS,       // .Asthra.string_ops - String operation metadata
    ASTHRA_SECTION_SLICE_META,       // .Asthra.slice_meta - Slice layout and bounds
    ASTHRA_SECTION_CONCURRENCY,      // .Asthra.concurrency - Task spawn metadata
    ASTHRA_SECTION_DEBUG_INFO,       // .Asthra.debug_info - Enhanced debug symbols
    ASTHRA_SECTION_RUNTIME_INIT,     // .Asthra.runtime_init - Runtime initialization
    ASTHRA_SECTION_COUNT
} AsthraSectionType;

// FFI metadata structure for .Asthra.ffi section
typedef struct {
    uint32_t magic;                  // Magic number for validation
    uint32_t version;                // Metadata format version
    uint32_t function_count;         // Number of FFI functions
    uint32_t variadic_count;         // Number of variadic functions
    
    struct {
        char name[64];               // Function name
        uint32_t param_count;        // Parameter count
        uint32_t variadic_start;     // First variadic parameter (0 if not variadic)
        uint32_t calling_conv;       // Calling convention
        uint32_t transfer_semantics; // Ownership transfer information
        uint64_t function_address;   // Function address (filled by linker)
        
        struct {
            uint32_t marshal_type;   // FFIMarshalingType
            uint32_t transfer_type;  // FFITransferType
            uint32_t is_borrowed;    // Lifetime annotation
            uint32_t reserved;       // Padding
        } parameters[16];            // Maximum 16 parameters
    } functions[];                   // Flexible array member
} AsthraSectionFFI;

// GC metadata structure for .Asthra.gc section
typedef struct {
    uint32_t magic;                  // Magic number
    uint32_t version;                // Version
    uint32_t gc_root_count;          // Number of GC roots
    uint32_t ownership_region_count; // Number of ownership regions
    
    struct {
        uint64_t address;            // Address of GC root
        uint32_t size;               // Size in bytes
        uint32_t type_id;            // Type identifier
        uint32_t ownership_type;     // gc/c/pinned
        uint32_t is_mutable;         // Mutability flag
    } gc_roots[];
} AsthraSectionGC;

// Security metadata structure for .Asthra.security_meta section
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t constant_time_count;    // Number of constant-time operations
    uint32_t volatile_memory_count;  // Number of volatile memory regions
    
    struct {
        uint64_t function_address;   // Function containing operation
        uint64_t operation_offset;   // Offset within function
        uint32_t operation_type;     // SecurityOperationType
        uint32_t memory_size;        // Size of memory region
        uint32_t flags;              // Additional flags
        uint32_t reserved;
    } security_operations[];
} AsthraSectionSecurity;

// Pattern matching metadata structure for .Asthra.pattern_matching section
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t match_count;            // Number of match statements
    uint32_t result_type_count;      // Number of Result<T,E> types
    
    struct {
        uint64_t match_address;      // Address of match statement
        uint32_t strategy;           // PatternMatchStrategy
        uint32_t arm_count;          // Number of match arms
        uint32_t is_exhaustive;      // Exhaustiveness flag
        uint32_t has_result_patterns; // Contains Result patterns
        
        struct {
            uint32_t pattern_type;   // Pattern type (Ok/Err/literal/default)
            uint64_t target_address; // Jump target address
            uint32_t binding_count;  // Number of bindings
            uint32_t reserved;
        } arms[32];                  // Maximum 32 arms per match
    } matches[];
} AsthraSectionPatternMatching;

// String operations metadata structure for .Asthra.string_ops section
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t string_op_count;        // Number of string operations
    uint32_t interpolation_count;    // Number of interpolations
    
    struct {
        uint64_t operation_address;  // Address of string operation
        uint32_t operation_type;     // StringOperationType
        uint32_t operand_count;      // Number of operands
        uint32_t is_deterministic;   // Deterministic behavior flag
        uint32_t needs_allocation;   // Memory allocation required
        uint64_t template_address;   // For interpolations
        uint32_t expression_count;   // For interpolations
        uint32_t reserved;
    } string_operations[];
} AsthraSectionStringOps;

// Slice metadata structure for .Asthra.slice_meta section
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t slice_count;            // Number of slice operations
    uint32_t bounds_check_count;     // Number of bounds checks
    
    struct {
        uint64_t slice_address;      // Address of slice operation
        uint32_t operation_type;     // SliceOperationType
        uint32_t element_size;       // Size of slice elements
        uint32_t is_mutable;         // Mutability flag
        uint32_t bounds_checking;    // Bounds checking enabled
        uint64_t bounds_check_addr;  // Address of bounds check
        uint32_t ffi_conversion;     // FFI conversion required
        uint32_t reserved;
    } slice_operations[];
} AsthraSectionSliceMeta;

// Concurrency metadata structure for .Asthra.concurrency section
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t spawn_count;            // Number of spawn statements
    uint32_t task_count;             // Number of task functions
    
    struct {
        uint64_t spawn_address;      // Address of spawn statement
        char function_name[64];      // Spawned function name
        uint32_t argument_count;     // Number of arguments
        uint32_t needs_handle;       // Task handle required
        uint64_t scheduler_call;     // Scheduler function address
        uint32_t memory_barrier;     // Memory barrier required
        uint32_t atomic_operations;  // Atomic operations used
        uint32_t reserved;
    } spawn_points[];
} AsthraSectionConcurrency;

// Asthra metadata container
struct AsthraSectionMetadata {
    // Pointers to section data
    AsthraSectionFFI *ffi_metadata;
    size_t ffi_size;
    
    AsthraSectionGC *gc_metadata;
    size_t gc_size;
    
    AsthraSectionSecurity *security_metadata;
    size_t security_size;
    
    AsthraSectionPatternMatching *pattern_metadata;
    size_t pattern_size;
    
    AsthraSectionStringOps *string_metadata;
    size_t string_size;
    
    AsthraSectionSliceMeta *slice_metadata;
    size_t slice_size;
    
    AsthraSectionConcurrency *concurrency_metadata;
    size_t concurrency_size;
    
    // Version information
    uint32_t metadata_version;
    uint32_t asthra_version;
};

// =============================================================================
// CORE ELF STRUCTURES
// =============================================================================

// ELF section representation
struct ELFSection {
    char *name;                      // Section name
    uint32_t name_offset;            // Offset in string table
    uint32_t type;                   // Section type (SHT_*)
    uint64_t flags;                  // Section flags (SHF_*)
    uint64_t address;                // Virtual address
    uint64_t offset;                 // File offset
    uint64_t size;                   // Section size
    uint32_t link;                   // Link to other section
    uint32_t info;                   // Additional info
    uint64_t alignment;              // Section alignment
    uint64_t entry_size;             // Entry size for table sections
    
    // Section data
    void *data;                      // Section content
    size_t data_size;                // Actual data size
    size_t data_capacity;            // Allocated capacity
    
    // Asthra-specific metadata
    AsthraSectionType asthra_type;   // Asthra section type
    bool is_asthra_section;          // Is this an Asthra special section
};

// ELF symbol representation
struct ELFSymbol {
    char *name;                      // Symbol name
    uint32_t name_offset;            // Offset in string table
    uint64_t value;                  // Symbol value
    uint64_t size;                   // Symbol size
    uint8_t info;                    // Symbol type and binding
    uint8_t other;                   // Symbol visibility
    uint16_t section_index;          // Section index
    
    // Asthra-specific symbol metadata
    bool is_ffi_function;            // FFI function symbol
    bool is_pattern_match;           // Pattern matching symbol
    bool is_string_operation;        // String operation symbol
    bool is_slice_operation;         // Slice operation symbol
    bool is_spawn_point;             // Concurrency spawn point
    bool is_gc_root;                 // GC root symbol
};

// ELF relocation representation
struct ELFRelocation {
    uint64_t offset;                 // Relocation offset
    uint64_t info;                   // Relocation type and symbol
    int64_t addend;                  // Relocation addend
    
    // Convenience fields for easier access (compatible with test expectations)
    uint32_t symbol_index;           // Symbol table index
    uint32_t type;                   // Relocation type
    void* section;                   // Target section
    
    // Enhanced relocation types for Asthra
    bool is_pattern_match_target;    // Pattern match jump target
    bool is_string_op_call;          // String operation runtime call
    bool is_slice_bounds_check;      // Slice bounds check call
    bool is_ffi_call;                // FFI function call
    bool is_spawn_call;              // Task spawn call
};

// ELF Writer main structure
struct ELFWriter {
    // ELF header
    Elf64_Ehdr header;
    
    // Sections
    ELFSection **sections;
    size_t section_count;
    size_t section_capacity;
    
    // Symbols
    ELFSymbol **symbols;
    size_t symbol_count;
    size_t symbol_capacity;
    
    // Relocations
    ELFRelocation **relocations;
    size_t relocation_count;
    size_t relocation_capacity;
    
    // String tables
    char *string_table;              // .strtab
    size_t string_table_size;
    size_t string_table_capacity;
    
    char *section_string_table;      // .shstrtab
    size_t section_string_table_size;
    size_t section_string_table_capacity;
    
    // Asthra-specific metadata
    AsthraSectionMetadata *asthra_metadata;
    
    // Integration with FFI assembly generator
    FFIAssemblyGenerator *ffi_generator;
    
    // Configuration
    struct {
        bool generate_debug_info;
        bool optimize_metadata;
        bool validate_elf_structure;
        bool enable_all_asthra_sections;
        size_t max_symbols;
        size_t max_relocations;
    } config;
    
    // Statistics (using regular variables instead of atomics to avoid macOS issues)
    uint32_t sections_created_count;
    uint32_t symbols_added_count;
    uint32_t relocations_added_count;
    uint32_t asthra_metadata_entries_count;
};

// =============================================================================
// CORE ELF WRITER FUNCTIONS
// =============================================================================

/**
 * Create a new ELF writer with comprehensive metadata support
 */
ELFWriter *elf_writer_create(FFIAssemblyGenerator *ffi_generator);

/**
 * Destroy ELF writer and free all resources
 */
void elf_writer_destroy(ELFWriter *writer);

/**
 * Generate complete ELF object file from FFI assembly generator
 */
bool elf_generate_object_file(ELFWriter *writer, ASTNode *program, 
                             const char *output_filename);

/**
 * Write ELF object file to disk with all metadata sections
 */
bool elf_write_object_file(ELFWriter *writer, const char *filename);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_ELF_WRITER_CORE_H 
