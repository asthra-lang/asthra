cmake_minimum_required(VERSION 3.20)
project(asthra 
    VERSION 2.0.0
    LANGUAGES C
    DESCRIPTION "Asthra Programming Language - AI-Friendly Systems Programming"
)

# Set C standard
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Build configuration
set(CMAKE_BUILD_TYPE_INIT "Release")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# Core CMake modules
include(cmake/Platform.cmake)
include(cmake/Compiler.cmake)
include(cmake/ThirdParty.cmake)

# Build system modules
include(cmake/BuildTypes.cmake)    # Advanced build configurations
include(cmake/Sanitizers.cmake)    # Sanitizer support
include(cmake/Testing.cmake)       # Advanced testing infrastructure
include(cmake/Coverage.cmake)      # Code coverage
include(cmake/Benchmarks.cmake)    # Performance benchmarking

# Development modules
include(cmake/DevTools.cmake)      # Development tools
include(cmake/FastCheck.cmake)     # Fast check and AI integration
include(cmake/Formatting.cmake)    # Code formatting and static analysis
include(cmake/Documentation.cmake) # Documentation generation

# Installation and distribution
include(cmake/Installation.cmake)  # Install/uninstall/dist targets

# Information and help
include(cmake/BuildInfo.cmake)     # Build information display
include(cmake/Help.cmake)          # Comprehensive help

# Global configuration
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Add subdirectories
add_subdirectory(src)
add_subdirectory(runtime)
add_subdirectory(stdlib)
add_subdirectory(tools)
add_subdirectory(tests)

# Include interim test runner
include(cmake/TestRunner.cmake)

# Main targets
include(cmake/Targets.cmake)