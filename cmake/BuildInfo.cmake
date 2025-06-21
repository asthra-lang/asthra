# Build Information Display for Asthra
# Provides comprehensive build system information and configuration display

# Get Git information
find_package(Git)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE ASTHRA_GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --always
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE ASTHRA_GIT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE ASTHRA_GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
else()
    set(ASTHRA_GIT_HASH "unknown")
    set(ASTHRA_GIT_TAG "unknown")
    set(ASTHRA_GIT_BRANCH "unknown")
endif()

# Main info target
add_custom_target(info
    COMMAND ${CMAKE_COMMAND} -E echo "================================================================"
    COMMAND ${CMAKE_COMMAND} -E echo "                    Asthra Build Information"
    COMMAND ${CMAKE_COMMAND} -E echo "================================================================"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Project Information:"
    COMMAND ${CMAKE_COMMAND} -E echo "  Name: ${PROJECT_NAME}"
    COMMAND ${CMAKE_COMMAND} -E echo "  Version: ${PROJECT_VERSION}"
    COMMAND ${CMAKE_COMMAND} -E echo "  Description: ${PROJECT_DESCRIPTION}"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Source Information:"
    COMMAND ${CMAKE_COMMAND} -E echo "  Source Directory: ${CMAKE_SOURCE_DIR}"
    COMMAND ${CMAKE_COMMAND} -E echo "  Binary Directory: ${CMAKE_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E echo "  Git Branch: ${ASTHRA_GIT_BRANCH}"
    COMMAND ${CMAKE_COMMAND} -E echo "  Git Hash: ${ASTHRA_GIT_HASH}"
    COMMAND ${CMAKE_COMMAND} -E echo "  Git Tag: ${ASTHRA_GIT_TAG}"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Build Configuration:"
    COMMAND ${CMAKE_COMMAND} -E echo "  Build Type: ${CMAKE_BUILD_TYPE}"
    COMMAND ${CMAKE_COMMAND} -E echo "  Generator: ${CMAKE_GENERATOR}"
    COMMAND ${CMAKE_COMMAND} -E echo "  CMake Version: ${CMAKE_VERSION}"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Compiler Information:"
    COMMAND ${CMAKE_COMMAND} -E echo "  C Compiler: ${CMAKE_C_COMPILER}"
    COMMAND ${CMAKE_COMMAND} -E echo "  C Compiler ID: ${CMAKE_C_COMPILER_ID}"
    COMMAND ${CMAKE_COMMAND} -E echo "  C Compiler Version: ${CMAKE_C_COMPILER_VERSION}"
    COMMAND ${CMAKE_COMMAND} -E echo "  C Standard: C${CMAKE_C_STANDARD}"
    COMMAND ${CMAKE_COMMAND} -E echo "  C Flags: ${CMAKE_C_FLAGS}"
    COMMAND ${CMAKE_COMMAND} -E echo "  C Flags (${CMAKE_BUILD_TYPE}): ${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE}}"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Platform Information:"
    COMMAND ${CMAKE_COMMAND} -E echo "  System: ${CMAKE_SYSTEM_NAME}"
    COMMAND ${CMAKE_COMMAND} -E echo "  System Version: ${CMAKE_SYSTEM_VERSION}"
    COMMAND ${CMAKE_COMMAND} -E echo "  Processor: ${CMAKE_SYSTEM_PROCESSOR}"
    COMMAND ${CMAKE_COMMAND} -E echo "  Platform: ${ASTHRA_PLATFORM}"
    COMMAND ${CMAKE_COMMAND} -E echo "  Architecture: ${ASTHRA_ARCH}"
    COMMAND ${CMAKE_COMMAND} -E echo "  Endianness: ${ASTHRA_ENDIANNESS}"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Feature Status:"
    COMMAND ${CMAKE_COMMAND} -E echo "  Coverage: $<IF:$<BOOL:${ENABLE_COVERAGE}>,Enabled,Disabled>"
    COMMAND ${CMAKE_COMMAND} -E echo "  Benchmarks: $<IF:$<BOOL:${ENABLE_BENCHMARKS}>,Enabled,Disabled>"
    COMMAND ${CMAKE_COMMAND} -E echo "  Dev Tools: $<IF:$<BOOL:${ENABLE_DEV_TOOLS}>,Enabled,Disabled>"
    COMMAND ${CMAKE_COMMAND} -E echo "  Fast Check: $<IF:$<BOOL:${ENABLE_FAST_CHECK}>,Enabled,Disabled>"
    COMMAND ${CMAKE_COMMAND} -E echo "  AI Integration: $<IF:$<BOOL:${ENABLE_AI_INTEGRATION}>,Enabled,Disabled>"
    COMMAND ${CMAKE_COMMAND} -E echo "  Documentation: $<IF:$<BOOL:${BUILD_DOCUMENTATION}>,Enabled,Disabled>"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "================================================================"
    COMMENT "Display build information"
)

# Detailed CMake module info
add_custom_target(cmake-info
    COMMAND ${CMAKE_COMMAND} -E echo "=== CMake Module Information ==="
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Core Modules:"
    COMMAND ${CMAKE_COMMAND} -E echo "  Platform.cmake      - Platform detection and configuration"
    COMMAND ${CMAKE_COMMAND} -E echo "  Compiler.cmake      - Compiler detection and flags"
    COMMAND ${CMAKE_COMMAND} -E echo "  ThirdParty.cmake    - External dependency management"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Build Modules:"
    COMMAND ${CMAKE_COMMAND} -E echo "  BuildTypes.cmake    - Build configurations (Debug/Release/Profile/PGO)"
    COMMAND ${CMAKE_COMMAND} -E echo "  Sanitizers.cmake    - Sanitizer support (ASan/TSan/UBSan/MSan)"
    COMMAND ${CMAKE_COMMAND} -E echo "  Coverage.cmake      - Code coverage analysis"
    COMMAND ${CMAKE_COMMAND} -E echo "  Benchmarks.cmake    - Performance benchmarking"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Development Modules:"
    COMMAND ${CMAKE_COMMAND} -E echo "  DevTools.cmake      - Development tools (AST viz, grammar validator)"
    COMMAND ${CMAKE_COMMAND} -E echo "  FastCheck.cmake     - Fast compilation and AI integration"
    COMMAND ${CMAKE_COMMAND} -E echo "  Testing.cmake       - Advanced testing infrastructure"
    COMMAND ${CMAKE_COMMAND} -E echo "  Formatting.cmake    - Code formatting and static analysis"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Utility Modules:"
    COMMAND ${CMAKE_COMMAND} -E echo "  Documentation.cmake - Documentation generation"
    COMMAND ${CMAKE_COMMAND} -E echo "  Installation.cmake  - Install/uninstall/distribution"
    COMMAND ${CMAKE_COMMAND} -E echo "  BuildInfo.cmake     - Build information display"
    COMMAND ${CMAKE_COMMAND} -E echo "  Help.cmake          - Comprehensive help system"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Module Locations:"
    COMMAND ${CMAKE_COMMAND} -E echo "  ${CMAKE_SOURCE_DIR}/cmake/"
    COMMENT "Display CMake module information"
)

# Environment variables info
add_custom_target(env-info
    COMMAND ${CMAKE_COMMAND} -E echo "=== Environment Variables ==="
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Build Variables:"
    COMMAND ${CMAKE_COMMAND} -E echo "  CC = $ENV{CC}"
    COMMAND ${CMAKE_COMMAND} -E echo "  CFLAGS = $ENV{CFLAGS}"
    COMMAND ${CMAKE_COMMAND} -E echo "  LDFLAGS = $ENV{LDFLAGS}"
    COMMAND ${CMAKE_COMMAND} -E echo "  CMAKE_PREFIX_PATH = $ENV{CMAKE_PREFIX_PATH}"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Asthra-specific Variables:"
    COMMAND ${CMAKE_COMMAND} -E echo "  ASTHRA_TEST_PARALLEL = $ENV{ASTHRA_TEST_PARALLEL}"
    COMMAND ${CMAKE_COMMAND} -E echo "  ASTHRA_TEST_TIMEOUT = $ENV{ASTHRA_TEST_TIMEOUT}"
    COMMAND ${CMAKE_COMMAND} -E echo "  ASTHRA_COVERAGE_THRESHOLD = $ENV{ASTHRA_COVERAGE_THRESHOLD}"
    COMMAND ${CMAKE_COMMAND} -E echo "  AI_API_KEY = $<IF:$<STREQUAL:$ENV{AI_API_KEY},>,Not Set,Set>"
    COMMENT "Display environment variables"
)

# Dependencies info
add_custom_target(deps-info
    COMMAND ${CMAKE_COMMAND} -E echo "=== Dependencies Information ==="
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Required Dependencies:"
    COMMAND ${CMAKE_COMMAND} -E echo "  Threads: Found"
    COMMAND ${CMAKE_COMMAND} -E echo "  json-c: $<IF:$<BOOL:${JSON_C_FOUND}>,Found,Not Found>"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Optional Dependencies:"
    COMMAND ${CMAKE_COMMAND} -E echo "  Doxygen: $<IF:$<BOOL:${DOXYGEN_FOUND}>,Found,Not Found>"
    COMMAND ${CMAKE_COMMAND} -E echo "  clang-format: $<IF:$<BOOL:${CLANG_FORMAT_EXECUTABLE}>,Found,Not Found>"
    COMMAND ${CMAKE_COMMAND} -E echo "  clang-tidy: $<IF:$<BOOL:${CLANG_TIDY_EXECUTABLE}>,Found,Not Found>"
    COMMAND ${CMAKE_COMMAND} -E echo "  cppcheck: $<IF:$<BOOL:${CPPCHECK_EXECUTABLE}>,Found,Not Found>"
    COMMAND ${CMAKE_COMMAND} -E echo "  valgrind: $<IF:$<BOOL:${VALGRIND_EXECUTABLE}>,Found,Not Found>"
    COMMAND ${CMAKE_COMMAND} -E echo "  Git: $<IF:$<BOOL:${GIT_FOUND}>,Found,Not Found>"
    COMMENT "Display dependencies information"
)

# Targets info
add_custom_target(targets-info
    COMMAND ${CMAKE_COMMAND} -E echo "=== Key Build Targets ==="
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Main Targets:"
    COMMAND ${CMAKE_COMMAND} -E echo "  asthra              - Main compiler executable"
    COMMAND ${CMAKE_COMMAND} -E echo "  asthra_runtime      - Runtime library"
    COMMAND ${CMAKE_COMMAND} -E echo "  test_framework      - Test framework library"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Test Targets:"
    COMMAND ${CMAKE_COMMAND} -E echo "  run-tests           - Run basic tests"
    COMMAND ${CMAKE_COMMAND} -E echo "  test-all            - Run all tests"
    COMMAND ${CMAKE_COMMAND} -E echo "  test-<category>     - Run category tests"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Development Targets:"
    COMMAND ${CMAKE_COMMAND} -E echo "  format              - Format code"
    COMMAND ${CMAKE_COMMAND} -E echo "  analyze             - Static analysis"
    COMMAND ${CMAKE_COMMAND} -E echo "  coverage            - Generate coverage"
    COMMAND ${CMAKE_COMMAND} -E echo "  benchmark           - Run benchmarks"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Help Targets:"
    COMMAND ${CMAKE_COMMAND} -E echo "  asthra-help         - Main help"
    COMMAND ${CMAKE_COMMAND} -E echo "  info                - Build information"
    COMMAND ${CMAKE_COMMAND} -E echo "  cmake-info          - CMake module info"
    COMMENT "Display available targets"
)

# Comprehensive info
add_custom_target(info-all
    DEPENDS info cmake-info env-info deps-info targets-info
    COMMENT "Display all build information"
)

# Configuration summary (called during configuration)
function(print_configuration_summary)
    message(STATUS "")
    message(STATUS "=== Asthra Configuration Summary ===")
    message(STATUS "Version: ${PROJECT_VERSION}")
    message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
    message(STATUS "C Compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
    message(STATUS "Platform: ${ASTHRA_PLATFORM} (${ASTHRA_ARCH})")
    message(STATUS "")
    message(STATUS "Features:")
    message(STATUS "  Coverage: ${ENABLE_COVERAGE}")
    message(STATUS "  Benchmarks: ${ENABLE_BENCHMARKS}")
    message(STATUS "  Dev Tools: ${ENABLE_DEV_TOOLS}")
    message(STATUS "  Fast Check: ${ENABLE_FAST_CHECK}")
    message(STATUS "  AI Integration: ${ENABLE_AI_INTEGRATION}")
    message(STATUS "  Documentation: ${BUILD_DOCUMENTATION}")
    message(STATUS "")
    message(STATUS "Use 'make info' for detailed build information")
    message(STATUS "Use 'make asthra-help' for available targets")
    message(STATUS "===================================")
endfunction()

# Call configuration summary
print_configuration_summary()