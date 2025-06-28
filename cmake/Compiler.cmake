# Compiler detection and configuration

# Detect compiler - Asthra requires Clang/LLVM only
if(NOT (CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "AppleClang"))
    message(FATAL_ERROR "Unsupported compiler: ${CMAKE_C_COMPILER_ID}. Asthra requires Clang/LLVM. No other compilers are supported.")
endif()
set(ASTHRA_COMPILER "Clang")

# C23 feature detection for Clang
include(CheckCCompilerFlag)
# C23 feature detection for Clang
check_c_compiler_flag("-std=c23" COMPILER_SUPPORTS_C23)
if(NOT COMPILER_SUPPORTS_C23)
    message(FATAL_ERROR "Asthra requires C23 support. Your compiler (${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}) does not support C23. Please upgrade to a newer version of Clang/LLVM.")
endif()
message(STATUS "Using C23 standard")

# Common warning flags for Clang
set(COMMON_WARNING_FLAGS
    -Wall -Wextra -Werror
    -Wno-unused-parameter
    -Wno-unused-variable
    -Wno-sign-compare
    -Wno-unused-function
    -Wno-missing-field-initializers
    -Wno-unused-value
    -Wno-format
)

# Clang-specific flags
list(APPEND COMMON_WARNING_FLAGS
    -Wno-newline-eof
    -Wno-unused-but-set-variable
    -Wno-incompatible-pointer-types
    -Wno-gnu-zero-variadic-macro-arguments
    -Wno-deprecated-pragma
    -fno-strict-aliasing
)

# Clang-specific optimizations
set(COMPILER_OPTIMIZATION_FLAGS
    -march=native -mtune=native
    -ffast-math -funroll-loops
    -fvectorize -fslp-vectorize
)

# Apply common warning flags
add_compile_options(${COMMON_WARNING_FLAGS})

# Platform-specific flags
if(APPLE)
    add_compile_options(-mmacosx-version-min=10.15)
    add_link_options(-mmacosx-version-min=10.15)
elseif(UNIX AND NOT APPLE)
    # No additional definitions for Linux
endif()

# Thread support
find_package(Threads REQUIRED)
add_compile_options(-pthread)

# Position Independent Code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Link-time optimization support
include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_OUTPUT)
if(IPO_SUPPORTED)
    message(STATUS "IPO/LTO supported")
else()
    message(STATUS "IPO/LTO not supported: ${IPO_OUTPUT}")
endif()

# Build type specific configurations (now handled by BuildTypes.cmake)
# But we keep backward compatibility here
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# Simple debug/release flags for backward compatibility
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG=1 ASTHRA_DEBUG=1)
    add_compile_options(-g3 -O0 -fno-omit-frame-pointer)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(NDEBUG=1)
    add_compile_options(-O3 ${COMPILER_OPTIMIZATION_FLAGS})
    if(IPO_SUPPORTED)
        add_compile_options(-flto)
        add_link_options(-flto)
    endif()
endif()

# Function to apply Asthra-specific compiler settings to a target
function(asthra_configure_compiler_for_target target)
    # Apply C17 standard
    set_target_properties(${target} PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF
    )
    
    # Apply build configuration specific settings
    if(TARGET asthra_configure_build_target)
        asthra_configure_build_target(${target})
    endif()
endfunction()

# Export compiler information
set(ASTHRA_COMPILER_INFO "${ASTHRA_COMPILER} ${CMAKE_C_COMPILER_VERSION}" CACHE STRING "Compiler information")

# Display configuration
message(STATUS "Asthra Compiler: ${ASTHRA_COMPILER}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
if(IPO_SUPPORTED)
    message(STATUS "Link-time optimization: Enabled")
else()
    message(STATUS "Link-time optimization: Disabled")
endif()