/**
 * Asthra Programming Language Runtime v1.2 - Modular Umbrella Header
 * Complete Runtime API with Modular Architecture
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * BACKWARD COMPATIBILITY HEADER
 * 
 * This header provides complete backward compatibility with the original
 * asthra_runtime.h by including all modular components. Applications can
 * use this header as a drop-in replacement for the original monolithic header.
 * 
 * For new applications, consider including only the specific modules you need
 * to reduce compilation overhead and improve build times.
 * 
 * DESIGN GOALS:
 * - Zero dependencies except libc
 * - POSIX-compliant implementation
 * - Conservative GC with distinct memory zones for FFI safety
 * - Pattern matching engine for Result<T,E> types
 * - Deterministic string operations (concatenation + interpolation)
 * - Formalized slice management with SliceHeader<T>
 * - Lightweight task concurrency with spawn support
 * - Cryptographic readiness with CSPRNG and secure operations
 */

#ifndef ASTHRA_RUNTIME_MODULAR_H
#define ASTHRA_RUNTIME_MODULAR_H

// =============================================================================
// MODULAR RUNTIME INCLUDES
// =============================================================================

// Core foundation - must be included first
#include "core/asthra_runtime_core.h"

// Memory management and GC
#include "memory/asthra_runtime_memory.h"

// Error handling - included early for error reporting
#include "errors/asthra_runtime_errors.h"

// Collections and data structures
#include "collections/asthra_runtime_slices.h"

// Type system and pattern matching
#include "types/asthra_runtime_result.h"

// String operations and UTF-8 support
#include "strings/asthra_runtime_strings.h"

// Concurrency and task system
#include "concurrency/asthra_runtime_tasks.h"
#include "concurrency/asthra_runtime_sync.h"

// Cryptographic support and security
#include "crypto/asthra_runtime_crypto.h"

// FFI interface
#include "ffi/asthra_runtime_ffi.h"

// Logging and diagnostics
#include "diagnostics/asthra_runtime_logging.h"

// Utility functions
#include "utils/asthra_runtime_utils.h"

// 128-bit integer support
#include "asthra_int128.h"

// =============================================================================
// COMPATIBILITY VERIFICATION
// =============================================================================

#ifdef __cplusplus
extern "C" {
#endif

// Verify that all essential types are available
ASTHRA_STATIC_ASSERT(sizeof(AsthraSliceHeader) > 0, "AsthraSliceHeader must be defined");
ASTHRA_STATIC_ASSERT(sizeof(AsthraString) > 0, "AsthraString must be defined");
ASTHRA_STATIC_ASSERT(sizeof(AsthraResult) > 0, "AsthraResult must be defined");
ASTHRA_STATIC_ASSERT(sizeof(AsthraGCConfig) > 0, "AsthraGCConfig must be defined");

// Verify that essential enums are defined
ASTHRA_STATIC_ASSERT(ASTHRA_OWNERSHIP_GC >= 0, "AsthraOwnershipHint must be defined");
ASTHRA_STATIC_ASSERT(ASTHRA_ZONE_GC >= 0, "AsthraMemoryZone must be defined");
ASTHRA_STATIC_ASSERT(ASTHRA_TRANSFER_FULL >= 0, "AsthraTransferType must be defined");
ASTHRA_STATIC_ASSERT(ASTHRA_RESULT_OK >= 0, "AsthraResultTag must be defined");

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_RUNTIME_MODULAR_H 
