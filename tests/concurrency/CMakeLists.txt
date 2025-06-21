# Concurrency Tests Custom Configuration
# Special handling for bridge tests that need to be linked together

# Create the bridge test runner as a special multi-file executable
set(BRIDGE_TEST_SOURCES
    test_bridge_runner.c
    test_bridge_init.c
    test_bridge_tasks.c
    test_bridge_callbacks.c
    test_bridge_threads.c
    test_bridge_sync.c
    test_bridge_integration.c
    test_bridge_monitoring.c
    test_bridge_missing_stubs.c
)

# Create the bridge test runner executable
add_executable(concurrency_test_bridge_runner ${BRIDGE_TEST_SOURCES})

if(TARGET asthra_test_framework)
    target_link_libraries(concurrency_test_bridge_runner asthra_test_framework)
else()
    target_link_libraries(concurrency_test_bridge_runner asthra_compiler asthra_runtime)
endif()

# Add as a test
add_test(NAME concurrency_test_bridge_runner COMMAND concurrency_test_bridge_runner)
set_tests_properties(concurrency_test_bridge_runner PROPERTIES
    TIMEOUT 60
    LABELS concurrency
)

# Add test executable as dependency to build-tests target
if(TARGET build-tests)
    add_dependencies(build-tests concurrency_test_bridge_runner)
endif()

# List of bridge test files that should not be built as individual executables
set(BRIDGE_TEST_FILES
    test_bridge_runner.c
    test_bridge_init.c
    test_bridge_tasks.c
    test_bridge_callbacks.c
    test_bridge_threads.c
    test_bridge_sync.c
    test_bridge_integration.c
    test_bridge_monitoring.c
    test_bridge_missing_stubs.c
)

# Tests that need concurrency_test_utils.c
set(UTILS_DEPENDENT_TESTS
    test_error_handling.c
    test_future_extensions.c
    test_simple_none_parsing.c
    test_task_lifecycle.c
)

# Create executables for utils-dependent tests
foreach(test_file ${UTILS_DEPENDENT_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "concurrency_${test_name}")
    
    # Add test_spawn_common.c for tests that need simple_task_function
    if(test_file STREQUAL "test_future_extensions.c")
        add_executable(${test_target} ${test_file} concurrency_test_utils.c test_spawn_common.c)
    else()
        add_executable(${test_target} ${test_file} concurrency_test_utils.c)
    endif()
    
    if(TARGET asthra_test_framework)
        target_link_libraries(${test_target} asthra_test_framework)
    else()
        target_link_libraries(${test_target} asthra_compiler asthra_runtime)
    endif()
    
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES
        TIMEOUT 30
        LABELS concurrency
    )
    
    # Add test executable as dependency to build-tests target
    if(TARGET build-tests)
        add_dependencies(build-tests ${test_target})
    endif()
endforeach()

# Auto-discover other test files (excluding bridge tests)
file(GLOB ALL_TEST_FILES "*.c")
set(INDIVIDUAL_TEST_FILES ${ALL_TEST_FILES})

# Remove bridge test files from individual test list
foreach(bridge_file ${BRIDGE_TEST_FILES})
    list(REMOVE_ITEM INDIVIDUAL_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${bridge_file}")
endforeach()

# Remove utils-dependent test files from individual test list
foreach(utils_test ${UTILS_DEPENDENT_TESTS})
    list(REMOVE_ITEM INDIVIDUAL_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${utils_test}")
endforeach()

# Also remove common/utility files from individual test list
set(COMMON_FILES
    concurrency_test_utils.c
    test_advanced_concurrency_common.c
    test_spawn_common.c
    concurrency_benchmark_stubs.c
    concurrency_lockfree_queue.c
    concurrency_thread_management.c
    concurrency_sync_primitives.c
    concurrency_test_context.c
    concurrency_stress_tests.c
    concurrency_runtime_stubs.c
    bridge_test_utils.c
    bridge_ffi_init.c
    bridge_ffi_tasks.c
    bridge_ffi_callbacks.c
    bridge_ffi_threads.c
    bridge_ffi_sync.c
    bridge_ffi_stats.c
)
foreach(common_file ${COMMON_FILES})
    list(REMOVE_ITEM INDIVIDUAL_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${common_file}")
endforeach()

# Also remove test runners that expect external functions
set(TEST_RUNNERS
    test_spawn_main.c
    test_spawn_main_standardized.c
)
foreach(runner ${TEST_RUNNERS})
    list(REMOVE_ITEM INDIVIDUAL_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${runner}")
endforeach()

# Create a library for concurrency integration components
add_library(concurrency_integration_lib STATIC
    concurrency_lockfree_queue.c
    concurrency_thread_management.c
    concurrency_sync_primitives.c
    concurrency_test_context.c
    concurrency_stress_tests.c
    concurrency_runtime_stubs.c
)

if(TARGET asthra_test_framework)
    target_link_libraries(concurrency_integration_lib asthra_test_framework)
else()
    target_link_libraries(concurrency_integration_lib asthra_compiler asthra_runtime)
endif()

# Note: bridge_ffi components are not compiled as they duplicate runtime functions
# The bridge tests use the actual runtime implementations

# Tests that need test_advanced_concurrency_common.c
set(ADVANCED_CONCURRENCY_TESTS
    test_concurrency_tier1.c
    test_concurrency_tier2.c
    test_concurrency_tier3_removal.c
    test_concurrency_semantic_analysis.c
)

# Tests that need test_spawn_common.c
set(SPAWN_COMMON_TESTS
    test_spawn_basic.c
    test_spawn_error_handling.c
    test_spawn_c_integration.c
    test_spawn_performance.c
    test_spawn_handles_await.c
)

# Create individual test executables for non-bridge tests
foreach(test_file ${INDIVIDUAL_TEST_FILES})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "concurrency_${test_name}")
    
    # Check if this test needs the advanced concurrency common implementation
    get_filename_component(test_filename ${test_file} NAME)
    if(test_filename IN_LIST ADVANCED_CONCURRENCY_TESTS)
        add_executable(${test_target} ${test_file} test_advanced_concurrency_common.c)
    elseif(test_filename IN_LIST SPAWN_COMMON_TESTS)
        add_executable(${test_target} ${test_file} test_spawn_common.c concurrency_test_utils.c)
    elseif(test_filename STREQUAL "test_concurrency_integration.c")
        add_executable(${test_target} ${test_file} test_advanced_concurrency_common.c)
    else()
        add_executable(${test_target} ${test_file})
    endif()
    
    if(TARGET asthra_test_framework)
        target_link_libraries(${test_target} asthra_test_framework)
    else()
        target_link_libraries(${test_target} asthra_compiler asthra_runtime)
    endif()
    
    # Additional linking for concurrency integration test
    if(test_filename STREQUAL "test_concurrency_integration.c")
        target_link_libraries(${test_target} concurrency_integration_lib)
    endif()
    
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES
        TIMEOUT 30
        LABELS concurrency
    )
    
    # Add test executable as dependency to build-tests target
    if(TARGET build-tests)
        add_dependencies(build-tests ${test_target})
    endif()
endforeach()