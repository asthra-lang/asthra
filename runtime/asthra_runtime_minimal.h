/**
 * Asthra Programming Language Runtime v1.2 - Minimal Runtime Header
 * Essential Runtime Components for Basic Applications
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * MINIMAL RUNTIME HEADER
 * 
 * This header provides only the essential runtime components needed for
 * basic applications. It includes core types, memory management, and
 * error handling, but excludes advanced features like concurrency,
 * cryptography, and complex string operations.
 * 
 * Use this header for applications that need minimal runtime overhead
 * and faster compilation times.
 */

#ifndef ASTHRA_RUNTIME_MINIMAL_H
#define ASTHRA_RUNTIME_MINIMAL_H

// =============================================================================
// MINIMAL RUNTIME INCLUDES
// =============================================================================

// Core foundation - essential types and feature detection
#include "core/asthra_runtime_core.h"

// Memory management and GC - essential for any runtime usage
#include "memory/asthra_runtime_memory.h"

// Error handling - essential for proper error reporting
#include "errors/asthra_runtime_errors.h"

// Basic utility functions - timing and I/O support
#include "utils/asthra_runtime_utils.h"

// =============================================================================
// MINIMAL COMPATIBILITY VERIFICATION
// =============================================================================

#ifdef __cplusplus
extern "C" {
#endif

// Verify that essential minimal types are available
ASTHRA_STATIC_ASSERT(sizeof(AsthraGCConfig) > 0, "AsthraGCConfig must be defined");
ASTHRA_STATIC_ASSERT(ASTHRA_OWNERSHIP_GC >= 0, "AsthraOwnershipHint must be defined");
ASTHRA_STATIC_ASSERT(ASTHRA_ZONE_GC >= 0, "AsthraMemoryZone must be defined");
ASTHRA_STATIC_ASSERT(ASTHRA_ERROR_NONE == 0, "AsthraErrorCode must be defined");

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_RUNTIME_MINIMAL_H 
