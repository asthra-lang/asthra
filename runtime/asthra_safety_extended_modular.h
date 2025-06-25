/**
 * Asthra Programming Language Runtime Safety - Extended Modular Interface
 * Umbrella header providing backward compatibility for asthra_safety_extended.c
 *
 * This header includes all modular safety components and maintains full
 * backward compatibility with the original asthra_safety_extended.c interface.
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_SAFETY_EXTENDED_MODULAR_H
#define ASTHRA_SAFETY_EXTENDED_MODULAR_H

// Include all modular safety components
#include "safety/asthra_safety_boundary_validation.h"
#include "safety/asthra_safety_ffi_tracking.h"
#include "safety/asthra_safety_runtime_validation.h"
#include "safety/asthra_safety_security_performance.h"
#include "safety/asthra_safety_type_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// MODULAR SAFETY SYSTEM OVERVIEW
// =============================================================================

/*
 * The Asthra Safety Extended system has been modularized into 5 focused components:
 *
 * 1. Core Type Safety (asthra_safety_type_core.{h,c})
 *    - Slice type validation and element type compatibility checking
 *    - Type safety verification for runtime operations
 *
 * 2. Boundary & Memory Validation (asthra_safety_boundary_validation.{h,c})
 *    - Enhanced boundary checks with comprehensive validation
 *    - Memory layout validation and corruption detection
 *    - String operation validation for interpolation safety
 *
 * 3. FFI Safety & Tracking (asthra_safety_ffi_tracking.{h,c})
 *    - FFI pointer tracking and reference counting
 *    - Variadic function call validation
 *    - FFI call logging and monitoring
 *
 * 4. Runtime Validation Systems (asthra_safety_runtime_validation.{h,c})
 *    - Result tracking for unhandled error detection
 *    - Pattern matching exhaustiveness verification
 *    - Scheduler event logging and C-task interaction tracking
 *
 * 5. Security & Performance Monitoring (asthra_safety_security_performance.{h,c})
 *    - Constant-time operation verification
 *    - Secure memory zeroing validation
 *    - Stack canary management and fault injection testing
 *    - Performance monitoring and configuration management
 *
 * Benefits of Modularization:
 * - Enhanced maintainability with focused, single-responsibility modules
 * - Improved debugging through isolated component testing
 * - Better scalability for adding new safety features
 * - Cleaner dependencies and reduced compilation times
 * - Full backward compatibility through this umbrella header
 */

// =============================================================================
// BACKWARD COMPATIBILITY NOTES
// =============================================================================

/*
 * All functions from the original asthra_safety_extended.c are available
 * through their respective modular headers included above. No code changes
 * are required for existing users of the safety system.
 *
 * To use specific modules individually:
 * #include "safety/asthra_safety_type_core.h"           // Type safety only
 * #include "safety/asthra_safety_boundary_validation.h" // Boundary checks only
 * #include "safety/asthra_safety_ffi_tracking.h"        // FFI safety only
 * #include "safety/asthra_safety_runtime_validation.h"  // Runtime validation only
 * #include "safety/asthra_safety_security_performance.h" // Security/performance only
 *
 * Or include this umbrella header for complete functionality:
 * #include "asthra_safety_extended_modular.h"
 */

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SAFETY_EXTENDED_MODULAR_H
