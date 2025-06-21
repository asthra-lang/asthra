/**
 * Asthra Programming Language Runtime Safety - Core Type Safety
 * Header file for type validation and slice safety functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_SAFETY_TYPE_CORE_H
#define ASTHRA_SAFETY_TYPE_CORE_H

#include "asthra_safety.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CORE TYPE SAFETY FUNCTIONS
// =============================================================================

/**
 * Validates slice type safety by checking element type compatibility
 * @param slice The slice header to validate
 * @param expected_element_type_id Expected type ID for slice elements
 * @return Type safety check result with validation details
 */
AsthraTypeSafetyCheck asthra_safety_validate_slice_type_safety(AsthraSliceHeader slice, uint32_t expected_element_type_id);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SAFETY_TYPE_CORE_H 
