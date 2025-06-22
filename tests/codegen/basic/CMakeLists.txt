# Basic Codegen Tests
# Simple tests that work without problematic headers

set(SIMPLE_CODEGEN_TESTS
    test_codegen_basic.c
    test_statement_generation_simple.c
    test_tuple_codegen.c
    test_option_type_codegen.c
    ../test_never_type_codegen.c
    test_simple_option_usage.c
)

# Create executables for simple tests
foreach(test_file ${SIMPLE_CODEGEN_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "codegen_${test_name}")
    
    add_executable(${test_target} ${test_file})
    target_link_libraries(${test_target} asthra_compiler asthra_runtime asthra_test_framework)
    
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES TIMEOUT 30 LABELS codegen)
    
    # Add test executable as dependency to build-tests target
    if(TARGET build-tests)
        add_dependencies(build-tests ${test_target})
    endif()
endforeach()