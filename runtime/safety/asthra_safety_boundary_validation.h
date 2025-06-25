/**
 * Asthra Programming Language Runtime Safety - Boundary & Memory Validation
 * Header file for enhanced boundary checks and memory layout validation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_SAFETY_BOUNDARY_VALIDATION_H
#define ASTHRA_SAFETY_BOUNDARY_VALIDATION_H

#include "asthra_safety.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// BOUNDARY & MEMORY VALIDATION FUNCTIONS
// =============================================================================

/**
 * Enhanced boundary check with comprehensive validation
 * @param slice The slice header to check
 * @param index The index to validate
 * @return Boundary check result with detailed error information
 */
AsthraBoundaryCheck asthra_safety_enhanced_boundary_check(AsthraSliceHeader slice, size_t index);

/**
 * Validates memory layout and structure integrity of slice headers
 * @param slice The slice header to validate
 * @return Memory layout validation result with corruption details
 */
AsthraMemoryLayoutValidation asthra_safety_validate_slice_header(AsthraSliceHeader slice);

// String interpolation safety validation removed - feature deprecated for AI generation efficiency

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SAFETY_BOUNDARY_VALIDATION_H
