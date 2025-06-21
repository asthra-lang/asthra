# Memory Safety Tests
# Tests that depend on test_memory_safety_common.c

set(MEMORY_SAFETY_TESTS
    test_bounds_checking.c
    test_lifetime_management.c
    test_memory_leak_detection.c
    test_ownership_tracking.c
)

# Create executables for memory safety tests
foreach(test_file ${MEMORY_SAFETY_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "codegen_${test_name}")
    
    add_executable(${test_target} ${test_file} test_memory_safety_common.c)
    
    if(TARGET asthra_test_framework)
        target_link_libraries(${test_target} asthra_test_framework)
    else()
        target_link_libraries(${test_target} asthra_compiler asthra_runtime)
    endif()
    
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES TIMEOUT 30 LABELS codegen)
    
    # Add test executable as dependency to build-tests target
    if(TARGET build-tests)
        add_dependencies(build-tests ${test_target})
    endif()
endforeach()