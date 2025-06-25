# Statement Generation Tests
# Common source files for all statement generation tests
set(STATEMENT_GENERATION_COMMON_SOURCES
    test_fixture_management.c
    test_helper_functions.c
    test_fragment_parser.c
    test_suite_configuration.c
)

set(STATEMENT_GENERATION_TESTS
    test_control_flow_statements.c
    test_function_call_statements.c
    test_return_statements.c
    test_variable_declarations.c
    test_simple_program_generation.c
    test_assignment_statements.c
    test_bare_minimum.c
    test_isolate_y_error.c
)

# Create executables for statement generation tests
foreach(test_file ${STATEMENT_GENERATION_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "codegen_${test_name}")
    
    add_executable(${test_target} ${test_file} ${STATEMENT_GENERATION_COMMON_SOURCES})
    
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