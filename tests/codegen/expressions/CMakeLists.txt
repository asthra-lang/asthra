# Expression Generation Tests
# Tests that use expression_parsing_helper.c

set(EXPRESSION_GENERATION_TESTS
    test_arithmetic_expressions.c
    test_logical_expressions.c
    test_comparison_expressions.c
    test_call_expressions.c
    test_unary_expressions.c
    test_bitwise_expressions.c
)

# Create executables for expression generation tests
foreach(test_file ${EXPRESSION_GENERATION_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "codegen_${test_name}")
    
    # Check if this is an expression test that needs the helper
    if(test_name MATCHES ".*expression.*")
        add_executable(${test_target} ${test_file} expression_parsing_helper.c)
    else()
        add_executable(${test_target} ${test_file})
    endif()
    
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