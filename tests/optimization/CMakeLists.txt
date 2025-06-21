# Optimization Tests Custom Configuration
# Special handling for optimization tests that need to be linked together

# Gperf integration test needs special handling - DISABLED due to API mismatches
# set(GPERF_INTEGRATION_SOURCES
#     test_gperf_integration_main.c
#     gperf_test_stubs.c
# )

# add_executable(optimization_test_gperf_integration_main ${GPERF_INTEGRATION_SOURCES})

# if(TARGET asthra_test_framework)
#     target_link_libraries(optimization_test_gperf_integration_main asthra_test_framework)
# else()
#     target_link_libraries(optimization_test_gperf_integration_main asthra_compiler asthra_runtime)
# endif()

# add_test(NAME optimization_test_gperf_integration_main COMMAND optimization_test_gperf_integration_main)
# set_tests_properties(optimization_test_gperf_integration_main PROPERTIES
#     TIMEOUT 30
#     LABELS optimization
# )

# List of optimization test files that should not be built as individual executables
set(OPTIMIZATION_MULTI_FILE_TESTS
    test_gperf_integration_main.c
    gperf_test_stubs.c
)

# Auto-discover other test files (excluding multi-file tests)
file(GLOB ALL_OPTIMIZATION_TEST_FILES "*.c")
set(INDIVIDUAL_OPTIMIZATION_TEST_FILES ${ALL_OPTIMIZATION_TEST_FILES})

# Remove multi-file test files from individual test list
foreach(multi_file ${OPTIMIZATION_MULTI_FILE_TESTS})
    list(REMOVE_ITEM INDIVIDUAL_OPTIMIZATION_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${multi_file}")
endforeach()

# List of files to exclude due to API mismatches
set(OPTIMIZATION_EXCLUDE
    test_optimization_main.c
    test_optimization_tests.c
    test_pgo_performance_suite.c
    test_sanitizer_integration_main.c
    test_sanitizer_msan.c
    test_sanitizer_tsan.c
)

foreach(exclude_file ${OPTIMIZATION_EXCLUDE})
    list(REMOVE_ITEM INDIVIDUAL_OPTIMIZATION_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${exclude_file}")
endforeach()

# Tests that need test_optimization_common.c
set(COMMON_DEPENDENT_TESTS
    test_automation_suite.c
    test_cicd_suite.c
    test_makefile_suite.c
    test_pgo_suite.c
    test_sanitizer_suite.c
)

# Tests that need test_pgo_performance_common.c
set(PGO_DEPENDENT_TESTS
    test_pgo_benchmarks.c
)

# Tests that need test_gperf_common.c
set(GPERF_DEPENDENT_TESTS
    test_gperf_file_generation.c
    test_gperf_hash_correctness.c
    test_gperf_keyword_extraction.c
    test_gperf_performance.c
    test_gperf_suite.c
)

# Create individual test executables for other tests
foreach(test_file ${INDIVIDUAL_OPTIMIZATION_TEST_FILES})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "optimization_${test_name}")
    
    # Check if this test needs test_optimization_common.c or test_gperf_common.c
    get_filename_component(test_file_name ${test_file} NAME)
    if("${test_file_name}" IN_LIST COMMON_DEPENDENT_TESTS)
        add_executable(${test_target} ${test_file} test_optimization_common.c)
    elseif("${test_file_name}" IN_LIST PGO_DEPENDENT_TESTS)
        add_executable(${test_target} ${test_file} test_optimization_common.c test_pgo_performance_common.c)
    elseif("${test_file_name}" IN_LIST GPERF_DEPENDENT_TESTS)
        # Special case for test_gperf_suite.c which needs both
        if("${test_file_name}" STREQUAL "test_gperf_suite.c")
            add_executable(${test_target} ${test_file} test_gperf_common.c test_optimization_common.c)
        else()
            add_executable(${test_target} ${test_file} test_gperf_common.c)
        endif()
    else()
        # Special case for test_pgo_performance_common.c which needs test_optimization_common.c
        if("${test_file_name}" STREQUAL "test_pgo_performance_common.c")
            add_executable(${test_target} ${test_file} test_optimization_common.c)
        else()
            add_executable(${test_target} ${test_file})
        endif()
    endif()
    
    if(TARGET asthra_test_framework)
        target_link_libraries(${test_target} asthra_test_framework)
    else()
        target_link_libraries(${test_target} asthra_compiler asthra_runtime)
    endif()
    
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES
        TIMEOUT 30
        LABELS optimization
    )
    
    # Add test executable as dependency to build-tests target
    if(TARGET build-tests)
        add_dependencies(build-tests ${test_target})
    endif()
endforeach()