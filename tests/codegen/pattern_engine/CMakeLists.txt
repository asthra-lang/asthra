# Pattern Engine Tests
# Tests that depend on test_pattern_engine_common.c

set(PATTERN_ENGINE_TESTS
    test_engine_match_expression_exhaustiveness.c
    test_engine_nested_pattern_matching.c
    test_engine_pattern_creation_and_matching.c
    test_engine_pattern_guards.c
    test_pattern_engine_main.c
)

# Create executables for pattern engine tests
foreach(test_file ${PATTERN_ENGINE_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "codegen_${test_name}")
    
    add_executable(${test_target} ${test_file} test_pattern_engine_common.c)
    
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