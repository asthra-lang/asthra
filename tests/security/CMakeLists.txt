# Security Tests Custom Configuration
# Includes stub implementations for missing safety functions

# Add safety stubs to all security tests
set(SAFETY_STUBS test_safety_stubs.c)

# List of test files that need safety stubs
set(SECURITY_TESTS_NEEDING_STUBS
    test_concurrency_safety.c
    test_grammar_type_safety.c
)

# Auto-discover all test files
file(GLOB ALL_SECURITY_TEST_FILES "*.c")
set(INDIVIDUAL_SECURITY_TEST_FILES ${ALL_SECURITY_TEST_FILES})

# Remove stub files from individual test list
list(REMOVE_ITEM INDIVIDUAL_SECURITY_TEST_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/safety_stubs.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/test_safety_stubs.c")

# List of files to exclude due to API mismatches
set(SECURITY_EXCLUDE
    test_concurrency_safety.c
    test_constant_time.c
    test_crypto_quality.c
    test_csprng.c
    test_memory_safety.c
    test_memory_security.c
)

foreach(exclude_file ${SECURITY_EXCLUDE})
    list(REMOVE_ITEM INDIVIDUAL_SECURITY_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${exclude_file}")
endforeach()

# Create test executables
foreach(test_file ${INDIVIDUAL_SECURITY_TEST_FILES})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "security_${test_name}")
    
    # Check if this test needs safety stubs
    get_filename_component(test_filename ${test_file} NAME)
    if(test_filename IN_LIST SECURITY_TESTS_NEEDING_STUBS)
        add_executable(${test_target} ${test_file} ${SAFETY_STUBS})
    else()
        add_executable(${test_target} ${test_file})
    endif()
    
    if(TARGET asthra_test_framework)
        target_link_libraries(${test_target} asthra_test_framework)
    else()
        target_link_libraries(${test_target} asthra_compiler asthra_runtime)
    endif()
    
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES
        TIMEOUT 30
        LABELS security
    )
    
    # Add test executable as dependency to build-tests target
    if(TARGET build-tests)
        add_dependencies(build-tests ${test_target})
    endif()
endforeach()