# Core Tests Custom Configuration
# Special handling for comprehensive tests that need to be linked together

# Create the core comprehensive test runner as a special multi-file executable
set(COMPREHENSIVE_TEST_SOURCES
    test_core_main.c
    test_comprehensive_core.c
    test_comprehensive_runners.c
    test_comprehensive_reporting.c
    comprehensive_security_main.c
    test_comprehensive_benchmark.c
    test_grammar_disambiguation.c
    test_grammar_helpers.c
    test_pattern_matching_performance.c
    test_pattern_matching_exhaustive.c
    test_pattern_matching_helpers.c
)

# Create the comprehensive test runner executable
add_executable(core_test_core_main ${COMPREHENSIVE_TEST_SOURCES})

if(TARGET asthra_test_framework)
    target_link_libraries(core_test_core_main asthra_test_framework)
else()
    target_link_libraries(core_test_core_main asthra_compiler asthra_runtime)
endif()

# Add as a test
add_test(NAME core_test_core_main COMMAND core_test_core_main)
set_tests_properties(core_test_core_main PROPERTIES
    TIMEOUT 60
    LABELS core
)

# Add test executable as dependency to build-tests target
if(TARGET build-tests)
    add_dependencies(build-tests core_test_core_main)
endif()

# List of comprehensive test files that should not be built as individual executables
set(COMPREHENSIVE_TEST_FILES
    test_core_main.c
    test_comprehensive_core.c
    test_comprehensive_runners.c
    test_comprehensive_reporting.c
    test_comprehensive_security.c
    comprehensive_security_main.c
    comprehensive_security_timing.c
    comprehensive_security_memory.c
    comprehensive_security_entropy.c
    comprehensive_security_tests.c
    comprehensive_security_utils.c
    test_comprehensive_benchmark.c
    test_grammar_disambiguation.c
    test_grammar_helpers.c
    test_pattern_matching_performance.c
    test_pattern_matching_exhaustive.c
    test_pattern_matching_helpers.c
)

# Grammar tests that need test_grammar_helpers.c
set(GRAMMAR_HELPER_TESTS
    test_grammar_pattern_matching.c
    test_grammar_slice_syntax.c
    test_grammar_string_operations.c
)

# String tests that need test_string_helpers.c
set(STRING_HELPER_TESTS
    test_string_deterministic.c
    test_string_ffi.c
    test_string_memory.c
    test_string_performance.c
)

# String operations main runner - comment out for now as it conflicts with individual tests
# set(STRING_OPERATIONS_MAIN_SOURCES
#     test_string_operations_main.c
#     test_string_helpers.c
# )

# Create executables for grammar helper tests
foreach(test_file ${GRAMMAR_HELPER_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "core_${test_name}")
    
    add_executable(${test_target} ${test_file} test_grammar_helpers.c)
    
    if(TARGET asthra_test_framework)
        target_link_libraries(${test_target} asthra_test_framework)
    else()
        target_link_libraries(${test_target} asthra_compiler asthra_runtime)
    endif()
    
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES
        TIMEOUT 30
        LABELS core
    )
    
    # Add test executable as dependency to build-tests target
    if(TARGET build-tests)
        add_dependencies(build-tests ${test_target})
    endif()
endforeach()

# Create executables for string helper tests
foreach(test_file ${STRING_HELPER_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "core_${test_name}")
    
    add_executable(${test_target} ${test_file} test_string_helpers.c)
    
    if(TARGET asthra_test_framework)
        target_link_libraries(${test_target} asthra_test_framework)
    else()
        target_link_libraries(${test_target} asthra_compiler asthra_runtime)
    endif()
    
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES
        TIMEOUT 30
        LABELS core
    )
    
    # Add test executable as dependency to build-tests target
    if(TARGET build-tests)
        add_dependencies(build-tests ${test_target})
    endif()
endforeach()

# Create the string operations main test runner - commented out due to conflicts
# add_executable(core_test_string_operations_main ${STRING_OPERATIONS_MAIN_SOURCES})
# 
# if(TARGET asthra_test_framework)
#     target_link_libraries(core_test_string_operations_main asthra_test_framework)
# else()
#     target_link_libraries(core_test_string_operations_main asthra_compiler asthra_runtime)
# endif()
# 
# add_test(NAME core_test_string_operations_main COMMAND core_test_string_operations_main)
# set_tests_properties(core_test_string_operations_main PROPERTIES
#     TIMEOUT 60
#     LABELS core
# )

# Create the grammar main test runner as a special multi-file executable
add_executable(core_test_grammar_main 
    test_grammar_main.c
    test_grammar_disambiguation.c
    test_grammar_pattern_matching.c
    test_grammar_string_operations.c
    test_grammar_slice_syntax.c
    test_grammar_helpers.c
)

if(TARGET asthra_test_framework)
    target_link_libraries(core_test_grammar_main asthra_test_framework)
else()
    target_link_libraries(core_test_grammar_main asthra_compiler asthra_runtime)
endif()

add_test(NAME core_test_grammar_main COMMAND core_test_grammar_main)
set_tests_properties(core_test_grammar_main PROPERTIES
    TIMEOUT 60
    LABELS core
)

if(TARGET build-tests)
    add_dependencies(build-tests core_test_grammar_main)
endif()

# Auto-discover other test files (excluding comprehensive tests)
file(GLOB ALL_TEST_FILES "*.c")
set(INDIVIDUAL_TEST_FILES ${ALL_TEST_FILES})

# Remove comprehensive test files from individual test list
foreach(comp_file ${COMPREHENSIVE_TEST_FILES})
    list(REMOVE_ITEM INDIVIDUAL_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${comp_file}")
endforeach()

# Remove grammar helper tests from individual test list
foreach(grammar_test ${GRAMMAR_HELPER_TESTS})
    list(REMOVE_ITEM INDIVIDUAL_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${grammar_test}")
endforeach()

# Remove string helper tests from individual test list
foreach(string_test ${STRING_HELPER_TESTS})
    list(REMOVE_ITEM INDIVIDUAL_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${string_test}")
endforeach()

# Also remove the helper files themselves
list(REMOVE_ITEM INDIVIDUAL_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test_string_helpers.c")

# Remove string operations main runner
list(REMOVE_ITEM INDIVIDUAL_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test_string_operations_main.c")

# Remove grammar main runner and its required files
list(REMOVE_ITEM INDIVIDUAL_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test_grammar_main.c")
list(REMOVE_ITEM INDIVIDUAL_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test_grammar_pattern_matching.c")
list(REMOVE_ITEM INDIVIDUAL_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test_grammar_string_operations.c")
list(REMOVE_ITEM INDIVIDUAL_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test_grammar_slice_syntax.c")

# Remove pattern matching propagation test (needs helpers)
list(REMOVE_ITEM INDIVIDUAL_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test_pattern_matching_propagation.c")

# Create pattern matching propagation test with helpers
add_executable(core_test_pattern_matching_propagation
    test_pattern_matching_propagation.c
    test_pattern_matching_helpers.c
)

if(TARGET asthra_test_framework)
    target_link_libraries(core_test_pattern_matching_propagation asthra_test_framework)
else()
    target_link_libraries(core_test_pattern_matching_propagation asthra_compiler asthra_runtime)
endif()

add_test(NAME core_test_pattern_matching_propagation COMMAND core_test_pattern_matching_propagation)
set_tests_properties(core_test_pattern_matching_propagation PROPERTIES
    TIMEOUT 30
    LABELS core
)

if(TARGET build-tests)
    add_dependencies(build-tests core_test_pattern_matching_propagation)
endif()

# Create individual test executables for non-comprehensive tests
foreach(test_file ${INDIVIDUAL_TEST_FILES})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "core_${test_name}")
    
    add_executable(${test_target} ${test_file})
    
    if(TARGET asthra_test_framework)
        target_link_libraries(${test_target} asthra_test_framework)
    else()
        target_link_libraries(${test_target} asthra_compiler asthra_runtime)
    endif()
    
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES
        TIMEOUT 30
        LABELS core
    )
    
    # Add test executable as dependency to build-tests target
    if(TARGET build-tests)
        add_dependencies(build-tests ${test_target})
    endif()
endforeach()