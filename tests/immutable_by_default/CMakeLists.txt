# Immutable By Default Tests Custom Configuration
# Special handling for immutable by default tests that need to be linked together

# Supporting source files that need to be included with tests
set(IMMUTABLE_SUPPORT_FILES
    immutable_analysis.c
    immutable_codegen.c
    immutable_creators.c
    immutable_testing.c
    test_immutable_by_default_common.c
)

# Create the immutable by default comprehensive test runner as a special multi-file executable
set(IMMUTABLE_BY_DEFAULT_TEST_SOURCES
    test_immutable_by_default_main.c
    test_basic_immutability.c
    test_ffi_integration.c
    test_function_parameters.c
    test_performance_ai_patterns.c
    test_real_world_scenarios.c
    test_smart_optimization.c
    ${IMMUTABLE_SUPPORT_FILES}
)

# Create the comprehensive immutable by default test runner executable
add_executable(immutable_by_default_test_immutable_by_default_main ${IMMUTABLE_BY_DEFAULT_TEST_SOURCES})

if(TARGET asthra_test_framework)
    target_link_libraries(immutable_by_default_test_immutable_by_default_main asthra_test_framework)
else()
    target_link_libraries(immutable_by_default_test_immutable_by_default_main asthra_compiler asthra_runtime)
endif()

# Add as a test
add_test(NAME immutable_by_default_test_immutable_by_default_main COMMAND immutable_by_default_test_immutable_by_default_main)
set_tests_properties(immutable_by_default_test_immutable_by_default_main PROPERTIES
    TIMEOUT 60
    LABELS immutable_by_default
)

# Add test executable as dependency to build-tests target
if(TARGET build-tests)
    add_dependencies(build-tests immutable_by_default_test_immutable_by_default_main)
endif()

# List of immutable by default test files that should not be built as individual executables
set(IMMUTABLE_BY_DEFAULT_TEST_FILES
    test_immutable_by_default_main.c
    test_basic_immutability.c
    test_ffi_integration.c
    test_function_parameters.c
    test_performance_ai_patterns.c
    test_real_world_scenarios.c
    test_smart_optimization.c
)

# Auto-discover other test files (excluding comprehensive tests)
file(GLOB ALL_IMMUTABLE_TEST_FILES "*.c")
set(INDIVIDUAL_IMMUTABLE_TEST_FILES ${ALL_IMMUTABLE_TEST_FILES})

# Remove comprehensive test files from individual test list
foreach(main_file ${IMMUTABLE_BY_DEFAULT_TEST_FILES})
    list(REMOVE_ITEM INDIVIDUAL_IMMUTABLE_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${main_file}")
endforeach()

# List of files to exclude due to API mismatches
set(IMMUTABLE_BY_DEFAULT_EXCLUDE
    test_stubs.c
    legacy_stubs.c
)

foreach(exclude_file ${IMMUTABLE_BY_DEFAULT_EXCLUDE})
    list(REMOVE_ITEM INDIVIDUAL_IMMUTABLE_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${exclude_file}")
endforeach()

# Supporting source files are already defined above

# Create individual test executables for non-comprehensive tests
foreach(test_file ${INDIVIDUAL_IMMUTABLE_TEST_FILES})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "immutable_by_default_${test_name}")
    
    # Include supporting files with each individual test
    add_executable(${test_target} ${test_file} ${IMMUTABLE_SUPPORT_FILES})
    
    if(TARGET asthra_test_framework)
        target_link_libraries(${test_target} asthra_test_framework)
    else()
        target_link_libraries(${test_target} asthra_compiler asthra_runtime)
    endif()
    
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES
        TIMEOUT 30
        LABELS immutable_by_default
    )
    
    # Add test executable as dependency to build-tests target
    if(TARGET build-tests)
        add_dependencies(build-tests ${test_target})
    endif()
endforeach()