# Performance Tests Custom Configuration
# Special handling for performance tests that need shared timer implementation

# Common implementations
set(COMMON_SOURCES 
    test_concurrency_timer_impl.c
    test_concurrency_stubs.c
)

# List of concurrency benchmark tests that need common implementations
set(CONCURRENCY_BENCHMARK_TESTS
    test_channel_performance
    test_concurrency_benchmarks_main
)

# Create executables with common implementations linked
foreach(test_name ${CONCURRENCY_BENCHMARK_TESTS})
    add_executable(performance_${test_name} ${test_name}.c ${COMMON_SOURCES})
    
    if(TARGET asthra_test_framework)
        target_link_libraries(performance_${test_name} asthra_test_framework)
    else()
        target_link_libraries(performance_${test_name} asthra_compiler asthra_runtime)
    endif()
    
    add_test(NAME performance_${test_name} COMMAND performance_${test_name})
    set_tests_properties(performance_${test_name} PROPERTIES
        TIMEOUT 60
        LABELS performance
    )
    
    # Add test executable as dependency to build-tests target
    if(TARGET build-tests)
        add_dependencies(build-tests performance_${test_name})
    endif()
endforeach()

# List of performance test files that should not be built as individual executables
set(PERFORMANCE_MULTI_FILE_TESTS
    test_channel_performance.c
    test_concurrency_benchmarks_main.c
    test_concurrency_timer_impl.c
    test_concurrency_stubs.c
)

# Auto-discover other test files (excluding multi-file tests)
file(GLOB ALL_PERFORMANCE_TEST_FILES "*.c")
set(INDIVIDUAL_PERFORMANCE_TEST_FILES ${ALL_PERFORMANCE_TEST_FILES})

# Remove multi-file test files from individual test list
foreach(multi_file ${PERFORMANCE_MULTI_FILE_TESTS})
    list(REMOVE_ITEM INDIVIDUAL_PERFORMANCE_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${multi_file}")
endforeach()

# List of files to exclude due to API mismatches
set(PERFORMANCE_EXCLUDE
    test_coordination_performance.c
    test_memory_and_tier_comparison.c
    test_performance_comprehensive.c
    test_stress_benchmarks.c
)

foreach(exclude_file ${PERFORMANCE_EXCLUDE})
    list(REMOVE_ITEM INDIVIDUAL_PERFORMANCE_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${exclude_file}")
endforeach()

# Create individual test executables for other tests
foreach(test_file ${INDIVIDUAL_PERFORMANCE_TEST_FILES})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "performance_${test_name}")
    
    add_executable(${test_target} ${test_file})
    
    if(TARGET asthra_test_framework)
        target_link_libraries(${test_target} asthra_test_framework)
    else()
        target_link_libraries(${test_target} asthra_compiler asthra_runtime)
    endif()
    
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES
        TIMEOUT 30
        LABELS performance
    )
    
    # Add test executable as dependency to build-tests target
    if(TARGET build-tests)
        add_dependencies(build-tests ${test_target})
    endif()
endforeach()