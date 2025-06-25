# Asthra Compiler Library - Auto-discover sources
file(GLOB_RECURSE ASTHRA_COMPILER_SOURCES
    "*.c"
)

# Exclude main.c from library
list(FILTER ASTHRA_COMPILER_SOURCES EXCLUDE REGEX "main\\.c$")

# Exclude ai_server files that depend on json-c
list(FILTER ASTHRA_COMPILER_SOURCES EXCLUDE REGEX "ai_server/.*\\.c$")

# Exclude json_utils.c that also depends on json-c
list(FILTER ASTHRA_COMPILER_SOURCES EXCLUDE REGEX "utils/json_utils\\.c$")

# Platform-specific exclusions
if(NOT APPLE)
    # Exclude macOS-specific Mach-O files on non-Apple platforms
    list(FILTER ASTHRA_COMPILER_SOURCES EXCLUDE REGEX "codegen/macho_.*\\.c$")
endif()

if(NOT UNIX OR APPLE)
    # Exclude ELF files on non-Unix platforms or macOS (which uses Mach-O)
    list(FILTER ASTHRA_COMPILER_SOURCES EXCLUDE REGEX "codegen/elf_.*\\.c$")
endif()

# AI Server components (separate library that depends on json-c)
file(GLOB_RECURSE AI_SERVER_SOURCES
    "ai_server/*.c"
)
# Exclude server_main.c from the library (it's for the executable)
list(FILTER AI_SERVER_SOURCES EXCLUDE REGEX "ai_server/server_main\\.c$")
# Exclude development_server_stubs.c (contains duplicate implementations)
list(FILTER AI_SERVER_SOURCES EXCLUDE REGEX "ai_server/development_server_stubs\\.c$")
# Exclude simple_dev_server.c (standalone server with its own main())
list(FILTER AI_SERVER_SOURCES EXCLUDE REGEX "ai_server/simple_dev_server\\.c$")

# Create AI server library
add_library(asthra_ai_server STATIC ${AI_SERVER_SOURCES})

target_include_directories(asthra_ai_server
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ai_server>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/fast_check>
        $<INSTALL_INTERFACE:include/asthra>
)

target_link_libraries(asthra_ai_server
    PUBLIC
        asthra_compiler
        json-c::json-c
        pthread
)

# AI Server development executable
add_executable(asthra_dev_server ai_server/server_main.c)
target_link_libraries(asthra_dev_server asthra_ai_server)

# Create compiler library
add_library(asthra_compiler STATIC ${ASTHRA_COMPILER_SOURCES})

target_include_directories(asthra_compiler
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/analysis>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/parser>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/codegen>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ai_api>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ai_linter>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ai_annotations>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/platform>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/linker>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/utils>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/diagnostics>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ast>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/compiler>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/fast_check>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ai_server>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/performance>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tools>
        $<INSTALL_INTERFACE:include/asthra>
)

target_link_libraries(asthra_compiler
    PUBLIC
        asthra_runtime
        llvm::llvm  # LLVM is now required
)

target_compile_definitions(asthra_compiler
    PRIVATE
        ASTHRA_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        ASTHRA_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        ASTHRA_VERSION_PATCH=${PROJECT_VERSION_PATCH}
        ASTHRA_COMPILER_BUILD
)

# Compiler executable
add_executable(asthra main.c)
target_link_libraries(asthra asthra_compiler)

# Export targets
install(TARGETS asthra asthra_compiler
    EXPORT AsthraTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Include sanitizer and coverage support
include(${CMAKE_SOURCE_DIR}/cmake/Sanitizers.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/Coverage.cmake)
