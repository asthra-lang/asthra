# Control Flow Analysis Tests
# Tests that depend on test_control_flow_common.c
# DISABLED: These tests depend on the removed optimizer infrastructure

# The optimizer was removed along with the C/Assembly backends
# These tests would need to be rewritten to use LLVM's optimization passes
# if control flow analysis is needed in the future

return()  # Skip this directory

set(CONTROL_FLOW_TESTS
    test_cfg_construction.c
    test_dominator_analysis.c
    test_loop_detection.c
)

# Create executables for control flow tests with common file
foreach(test_file ${CONTROL_FLOW_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "codegen_${test_name}")
    
    add_executable(${test_target} ${test_file} test_control_flow_common.c test_optimization_passes_stubs.c)
    
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