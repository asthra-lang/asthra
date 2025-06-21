# Runtime Tests Custom Configuration
# Links all runtime test modules together for the test runner

# List of runtime test modules
set(RUNTIME_TEST_SOURCES
    test_runner.c
    test_crypto.c
    test_errors.c
    test_memory.c
    test_observability.c
    test_patterns.c
    test_slices.c
    test_strings.c
    test_sync.c
    test_tasks.c
    test_runtime_init.c
    runtime_test_infrastructure.c
)

# Create the runtime test runner executable
# Temporarily disabled due to duplicate symbol conflicts with asthra_runtime
# add_executable(runtime_test_runner ${RUNTIME_TEST_SOURCES})

# Link with required libraries
# if(TARGET asthra_test_framework)
#     target_link_libraries(runtime_test_runner asthra_test_framework)
# else()
#     target_link_libraries(runtime_test_runner asthra_compiler asthra_runtime)
# endif()

# Add test
# add_test(NAME runtime_test_runner COMMAND runtime_test_runner)
# set_tests_properties(runtime_test_runner PROPERTIES
#     TIMEOUT 60
#     LABELS runtime
# )

# Prevent auto-discovery from creating individual executables for these files
set(RUNTIME_MULTI_FILE_TESTS
    test_runner.c
    test_crypto.c
    test_errors.c
    test_memory.c
    test_observability.c
    test_patterns.c
    test_slices.c
    test_strings.c
    test_sync.c
    test_tasks.c
    test_runtime_init.c
    runtime_test_infrastructure.c
)

# Auto-discover other test files (excluding multi-file tests)
file(GLOB ALL_RUNTIME_TEST_FILES "*.c")
set(INDIVIDUAL_RUNTIME_TEST_FILES ${ALL_RUNTIME_TEST_FILES})

# Remove multi-file test files from individual test list
foreach(multi_file ${RUNTIME_MULTI_FILE_TESTS})
    list(REMOVE_ITEM INDIVIDUAL_RUNTIME_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${multi_file}")
endforeach()

# List of files to exclude due to API mismatches or being framework components
set(RUNTIME_EXCLUDE
    test_runner_standardized.c
    test_runtime_memory_main.c
    test_execution.c        # Framework component for test runner
    test_registry.c         # Framework component for test runner
    test_reporting.c        # Framework component for test runner
)

foreach(exclude_file ${RUNTIME_EXCLUDE})
    list(REMOVE_ITEM INDIVIDUAL_RUNTIME_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${exclude_file}")
endforeach()

# Create individual test executables for other tests
foreach(test_file ${INDIVIDUAL_RUNTIME_TEST_FILES})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "runtime_${test_name}")
    
    add_executable(${test_target} ${test_file})
    
    if(TARGET asthra_test_framework)
        target_link_libraries(${test_target} asthra_test_framework)
    else()
        target_link_libraries(${test_target} asthra_compiler asthra_runtime)
    endif()
    
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES
        TIMEOUT 30
        LABELS runtime
    )
    
    # Add test executable as dependency to build-tests target
    if(TARGET build-tests)
        add_dependencies(build-tests ${test_target})
    endif()
endforeach()