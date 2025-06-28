# Asthra Runtime Library
file(GLOB ASTHRA_RUNTIME_SOURCES "*.c")
file(GLOB ASTHRA_RUNTIME_MEMORY "memory/*.c")
# TODO: Fix safety header dependencies before including
# file(GLOB ASTHRA_RUNTIME_SAFETY "safety/*.c")

# Conditionally include concurrency files based on platform
if(WIN32)
    # On Windows, exclude pthread-dependent concurrency files
    set(ASTHRA_RUNTIME_CONCURRENCY "")
    # Add Windows platform stub
    list(APPEND ASTHRA_RUNTIME_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/platform/asthra_platform_stub.c")
else()
    file(GLOB ASTHRA_RUNTIME_CONCURRENCY "concurrency/*.c")
endif()

add_library(asthra_runtime STATIC 
    ${ASTHRA_RUNTIME_SOURCES} 
    ${ASTHRA_RUNTIME_CONCURRENCY}
    ${ASTHRA_RUNTIME_MEMORY}
)

target_include_directories(asthra_runtime
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/asthra/runtime>
)

target_compile_definitions(asthra_runtime
    PRIVATE
        ASTHRA_RUNTIME_BUILD
        ASTHRA_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        ASTHRA_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        ASTHRA_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

# Platform-specific runtime features
if(NOT WIN32)
    target_link_libraries(asthra_runtime PRIVATE pthread)
endif()

if(APPLE)
    target_link_libraries(asthra_runtime PRIVATE "-framework CoreFoundation")
endif()

if(UNIX AND NOT APPLE)
    # Linux-specific libraries
    target_link_libraries(asthra_runtime PRIVATE dl)
endif()

# Math library on Unix systems
if(UNIX)
    target_link_libraries(asthra_runtime PRIVATE m)
endif()

# Export runtime library
install(TARGETS asthra_runtime
    EXPORT AsthraTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
) 