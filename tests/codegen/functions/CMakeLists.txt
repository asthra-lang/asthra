# Function Generation Tests
# Individual test files for function code generation

set(FUNCTION_GENERATION_TESTS
    test_function_prologue_epilogue.c
    test_function_parameters.c
    test_function_local_variables.c
    test_function_recursive.c
    test_function_control_flow.c
    test_function_calling_conventions.c
)

# Create executables for function generation tests
foreach(test_file ${FUNCTION_GENERATION_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "codegen_${test_name}")
    
    add_executable(${test_target} ${test_file})
    
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