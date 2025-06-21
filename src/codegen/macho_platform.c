/**
 * Asthra Programming Language Compiler
 * Mach-O Platform Detection
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Platform-specific detection for Mach-O generation
 */

#include "macho_platform.h"
#include <mach/machine.h>

// =============================================================================
// PLATFORM DETECTION
// =============================================================================

uint32_t macho_get_cpu_type(void) {
#ifdef __arm64__
    return CPU_TYPE_ARM64;
#elif defined(__x86_64__)
    return CPU_TYPE_X86_64;
#else
    return CPU_TYPE_ANY;
#endif
}

uint32_t macho_get_cpu_subtype(void) {
#ifdef __arm64__
    return CPU_SUBTYPE_ARM64_ALL;
#elif defined(__x86_64__)
    return CPU_SUBTYPE_X86_64_ALL;
#else
    return CPU_SUBTYPE_MULTIPLE;
#endif
}