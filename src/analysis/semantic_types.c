/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Type Descriptor Module Implementation - Include Aggregator
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file now serves as an include aggregator for type descriptor operations.
 * The actual implementation is split across specialized modules.
 */

#include "semantic_types.h"
#include "semantic_type_creation.h"
#include "semantic_type_descriptors.h"
#include "semantic_type_resolution.h"

// This file now serves as an include aggregator for type descriptor operations.
// The actual functions are implemented in the specialized modules:
//
// - semantic_type_descriptors.h: type_descriptor_create_primitive, type_descriptor_retain,
//                                type_descriptor_release, type_descriptor_equals,
//                                type_descriptor_hash
// - semantic_type_creation.h: type_descriptor_create_struct, type_descriptor_create_pointer,
//                             type_descriptor_create_slice, type_descriptor_create_result,
//                             type_descriptor_create_function,
//                             type_descriptor_create_function_with_params,
//                             type_descriptor_create_generic_instance,
//                             type_descriptor_add_struct_field, type_descriptor_lookup_struct_field
// - semantic_type_resolution.h: analyze_type_node
//
// All functionality is now available through their respective headers.