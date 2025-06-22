# Pattern Matching Tests
# Tests that depend on test_pattern_matching_common.c

set(PATTERN_MATCHING_TESTS
    test_match_expression_exhaustiveness.c
    test_nested_pattern_matching.c
    test_pattern_creation_and_matching.c
    test_pattern_guards.c
)

# Standalone pattern matching test for Option<T>
set(test_target "codegen_test_option_pattern_matching")
add_executable(${test_target} test_option_pattern_matching.c)

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

# Create executables for pattern matching tests
foreach(test_file ${PATTERN_MATCHING_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "codegen_${test_name}")
    
    add_executable(${test_target} ${test_file} test_pattern_matching_common.c)
    
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