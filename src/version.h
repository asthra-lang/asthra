/**
 * Asthra Programming Language Version Information
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_VERSION_H
#define ASTHRA_VERSION_H

// Version information
#ifndef ASTHRA_VERSION_MAJOR
#define ASTHRA_VERSION_MAJOR 1
#endif
#ifndef ASTHRA_VERSION_MINOR
#define ASTHRA_VERSION_MINOR 0
#endif
#ifndef ASTHRA_VERSION_PATCH
#define ASTHRA_VERSION_PATCH 0
#endif
#ifndef ASTHRA_VERSION_SUFFIX
#define ASTHRA_VERSION_SUFFIX ""
#endif

// Build version string
#define ASTHRA_VERSION_STRING "1.0.0"

// Feature flags for this version
#define ASTHRA_FEATURE_PATTERN_MATCHING 1
#define ASTHRA_FEATURE_FFI_SAFETY 1
#define ASTHRA_FEATURE_GARBAGE_COLLECTION 1
#define ASTHRA_FEATURE_CONCURRENCY 1
#define ASTHRA_FEATURE_CRYPTOGRAPHY 1
#define ASTHRA_FEATURE_SLICE_MANAGEMENT 1
#define ASTHRA_FEATURE_STRING_INTERPOLATION 0
#define ASTHRA_FEATURE_RESULT_TYPES 1

// Compatibility information
#define ASTHRA_MIN_RUNTIME_VERSION "1.0.0"
#define ASTHRA_TARGET_PLATFORM "POSIX"

// Build information (set by build system)
#ifndef ASTHRA_BUILD_DATE
#define ASTHRA_BUILD_DATE __DATE__
#endif

#ifndef ASTHRA_BUILD_TIME
#define ASTHRA_BUILD_TIME __TIME__
#endif

#ifndef ASTHRA_BUILD_COMMIT
#define ASTHRA_BUILD_COMMIT "unknown"
#endif

// Version comparison macros
#define ASTHRA_VERSION_ENCODE(major, minor, patch) (((major) << 16) | ((minor) << 8) | (patch))

#define ASTHRA_VERSION_CURRENT                                                                     \
    ASTHRA_VERSION_ENCODE(ASTHRA_VERSION_MAJOR, ASTHRA_VERSION_MINOR, ASTHRA_VERSION_PATCH)

#endif // ASTHRA_VERSION_H
