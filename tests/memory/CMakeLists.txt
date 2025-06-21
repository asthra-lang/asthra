# Memory Tests Custom Configuration
# Special handling for memory tests that need to be linked together

# List of slice tests that need the common implementation
set(SLICE_TEST_SOURCES
    slice_test_common.c
    ../concurrency/concurrency_benchmark_stubs.c
)

# Define slice test executables that need the common implementation
set(SLICE_TEST_EXECUTABLES
    test_slice_bounds
    test_slice_header
    test_slice_large_data
    test_slice_memory_layout
)

# Create each slice test executable with the common implementation
foreach(test_name ${SLICE_TEST_EXECUTABLES})
    add_executable(memory_${test_name} ${test_name}.c ${SLICE_TEST_SOURCES})
    
    if(TARGET asthra_test_framework)
        target_link_libraries(memory_${test_name} asthra_test_framework)
    else()
        target_link_libraries(memory_${test_name} asthra_compiler asthra_runtime)
    endif()
    
    add_test(NAME memory_${test_name} COMMAND memory_${test_name})
    set_tests_properties(memory_${test_name} PROPERTIES
        TIMEOUT 30
        LABELS memory
    )
    
    # Add test executable as dependency to build-tests target
    if(TARGET build-tests)
        add_dependencies(build-tests memory_${test_name})
    endif()
endforeach()

# List of memory test files that should not be built as individual executables
set(MEMORY_SLICE_TEST_FILES
    slice_test_common.c
    test_slice_bounds.c
    test_slice_header.c
    test_slice_large_data.c
    test_slice_memory_layout.c
)

# Auto-discover other test files (excluding slice tests)
file(GLOB ALL_MEMORY_TEST_FILES "*.c")
set(INDIVIDUAL_MEMORY_TEST_FILES ${ALL_MEMORY_TEST_FILES})

# Remove slice test files from individual test list
foreach(slice_file ${MEMORY_SLICE_TEST_FILES})
    list(REMOVE_ITEM INDIVIDUAL_MEMORY_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${slice_file}")
endforeach()

# Create individual test executables for non-slice tests
foreach(test_file ${INDIVIDUAL_MEMORY_TEST_FILES})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "memory_${test_name}")
    
    add_executable(${test_target} ${test_file})
    
    if(TARGET asthra_test_framework)
        target_link_libraries(${test_target} asthra_test_framework)
    else()
        target_link_libraries(${test_target} asthra_compiler asthra_runtime)
    endif()
    
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES
        TIMEOUT 30
        LABELS memory
    )
    
    # Add test executable as dependency to build-tests target
    if(TARGET build-tests)
        add_dependencies(build-tests ${test_target})
    endif()
endforeach()