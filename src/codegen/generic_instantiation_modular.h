/**
 * Asthra Programming Language Compiler
 * Generic Instantiation Modular System
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Umbrella header providing access to all generic instantiation modules
 * This header provides 100% backward compatibility with the original generic_instantiation.h
 */

#ifndef ASTHRA_GENERIC_INSTANTIATION_MODULAR_H
#define ASTHRA_GENERIC_INSTANTIATION_MODULAR_H

// Core type definitions
#include "generic_instantiation_types.h"

// Registry management
#include "generic_instantiation_registry.h"

// Core instantiation operations
#include "generic_instantiation_core.h"

// Code generation
#include "generic_instantiation_codegen.h"

// Validation and error handling
#include "generic_instantiation_validation.h"

// Memory management
#include "generic_instantiation_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// COMPATIBILITY LAYER
// =============================================================================

// All functions from the original generic_instantiation.h are now available
// through the individual module headers included above.
// This provides 100% backward compatibility for existing code.

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_GENERIC_INSTANTIATION_MODULAR_H 
