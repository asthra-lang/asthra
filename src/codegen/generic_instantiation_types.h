/**
 * Asthra Programming Language Compiler
 * Generic Instantiation Type Definitions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Core type definitions for generic struct instantiation and monomorphization
 */

#ifndef ASTHRA_GENERIC_INSTANTIATION_TYPES_H
#define ASTHRA_GENERIC_INSTANTIATION_TYPES_H

#include "code_generator_types.h"
#include "../analysis/semantic_analyzer.h"
#include "../parser/ast.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// GENERIC INSTANTIATION DATA STRUCTURES
// =============================================================================

// Represents a specific instantiation of a generic struct
typedef struct GenericInstantiation {
    char *concrete_name;           // Generated name: Vec_i32, Pair_string_bool
    TypeDescriptor **type_args;    // Array of type arguments used in this instantiation
    size_t type_arg_count;         // Number of type arguments
    
    // Generated concrete struct information
    char *c_struct_definition;     // Generated C struct definition
    SymbolTable *concrete_fields;  // Field symbols with concrete types
    
    // Metadata
    size_t struct_size;            // Size of the concrete struct in bytes
    size_t struct_alignment;       // Alignment requirements
    bool is_ffi_compatible;        // Whether this instantiation is FFI-safe
    
    // Reference tracking
    atomic_uint_fast32_t ref_count;
    struct GenericInstantiation *next;
} GenericInstantiation;

// Tracks all instantiations of a specific generic struct
typedef struct GenericStructInfo {
    char *generic_name;            // Original generic struct name: Vec, Pair
    ASTNode *original_decl;        // Original AST declaration
    TypeDescriptor *generic_type;  // Original generic type descriptor
    
    // Instantiation tracking
    GenericInstantiation **instantiations;
    size_t instantiation_count;
    size_t instantiation_capacity;
    
    // Type parameter information
    char **type_param_names;       // Array of type parameter names [T, U, V]
    size_t type_param_count;
    
    // Generation state
    atomic_bool is_generating;     // Prevents recursive generation
    atomic_uint_fast32_t generation_id;
    
    struct GenericStructInfo *next;
} GenericStructInfo;

// Registry for all generic structs in the compilation unit
typedef struct GenericRegistry {
    GenericStructInfo **structs;
    size_t struct_count;
    size_t struct_capacity;
    
    // Thread safety for concurrent monomorphization
    pthread_rwlock_t rwlock;
    
    // Statistics
    atomic_uint_fast32_t total_instantiations;
    atomic_uint_fast32_t concrete_structs_generated;
    atomic_uint_fast64_t bytes_generated;
} GenericRegistry;

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_GENERIC_INSTANTIATION_TYPES_H 
