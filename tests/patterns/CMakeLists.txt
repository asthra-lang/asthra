# Patterns Tests Custom Configuration
# Special handling for pattern tests that need stub implementations

# List of pattern test stub sources
set(PATTERN_STUB_SOURCES
    guard_test_stubs.c
    test_pattern_common.c
)

# Auto-discover test files
file(GLOB ALL_PATTERN_TEST_FILES "*.c")
set(PATTERN_TEST_FILES ${ALL_PATTERN_TEST_FILES})

# Remove stub files from test list
foreach(stub_file ${PATTERN_STUB_SOURCES})
    list(REMOVE_ITEM PATTERN_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${stub_file}")
endforeach()

# List of files to exclude due to API mismatches
set(PATTERNS_EXCLUDE
    test_compilation_optimization.c
    test_complex_guards.c
    test_enum_exhaustiveness.c
    test_enum_performance.c
    test_enum_struct_guards.c
    test_guard_common.c
    test_guard_optimization.c
    test_jump_table_optimization.c
    test_memory_cache_optimization.c
    test_optimization_common.c
    test_pattern_common.c
    test_runtime_performance.c
)

foreach(exclude_file ${PATTERNS_EXCLUDE})
    list(REMOVE_ITEM PATTERN_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${exclude_file}")
endforeach()

# Create test executables with stubs linked
foreach(test_file ${PATTERN_TEST_FILES})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "patterns_${test_name}")
    
    # Link test file with stubs
    add_executable(${test_target} ${test_file} ${PATTERN_STUB_SOURCES})
    
    if(TARGET asthra_test_framework)
        target_link_libraries(${test_target} asthra_test_framework)
    else()
        target_link_libraries(${test_target} asthra_compiler asthra_runtime)
    endif()
    
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES
        TIMEOUT 30
        LABELS patterns
    )
    
    # Add test executable as dependency to build-tests target
    if(TARGET build-tests)
        add_dependencies(build-tests ${test_target})
    endif()
endforeach()