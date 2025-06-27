# Semantic Tests CMakeLists.txt
# This file defines how to build semantic tests with their common dependencies

# Tests that depend on test_type_system_common.c
set(TYPE_SYSTEM_TESTS
    test_primitive_types.c
    test_type_inference.c
    test_composite_types.c
    test_type_compatibility.c
    test_function_types.c
    test_advanced_types.c
    test_tuple_type_checking.c
    test_args_function.c
    test_panic_log_functions.c
    test_infinite_function.c
    test_i128_u128_types.c
)

# Create executables for type system tests with common file
foreach(test_file ${TYPE_SYSTEM_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "semantic_${test_name}")
    
    add_executable(${test_target} ${test_file} test_type_system_common.c)
    
    if(TARGET asthra_test_framework)
        target_link_libraries(${test_target} asthra_test_framework)
    else()
        target_link_libraries(${test_target} asthra_compiler asthra_runtime)
    endif()
    
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES
        TIMEOUT 30
        LABELS semantic
    )
    
    # Add test executable as dependency to build-tests target
    if(TARGET build-tests)
        add_dependencies(build-tests ${test_target})
    endif()
endforeach()

# Mutability tests with their own common file
set(MUTABILITY_TESTS
    test_immutable_variables.c
    test_mutable_variables.c
    test_function_parameters.c
    test_struct_mutability.c
    test_complex_mutability.c
    test_loop_mutability.c
)

# Create executables for mutability tests with common file
foreach(test_file ${MUTABILITY_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "semantic_${test_name}")
    
    add_executable(${test_target} ${test_file} test_mutability_common.c test_type_system_common.c)
    
    if(TARGET asthra_test_framework)
        target_link_libraries(${test_target} asthra_test_framework)
    else()
        target_link_libraries(${test_target} asthra_compiler asthra_runtime)
    endif()
    
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES
        TIMEOUT 30
        LABELS semantic
    )
    
    # Add test executable as dependency to build-tests target
    if(TARGET build-tests)
        add_dependencies(build-tests ${test_target})
    endif()
endforeach()

# Standalone tests that don't need the common file
set(STANDALONE_TESTS
    test_type_alias_constraint.c
    test_array_slice_initialization_semantic.c
    test_extern_simple.c
    test_never_simple.c
    test_option_types.c
)

# Create executables for standalone tests
foreach(test_file ${STANDALONE_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "semantic_${test_name}")
    
    add_executable(${test_target} ${test_file})
    
    if(TARGET asthra_test_framework)
        target_link_libraries(${test_target} asthra_test_framework)
    else()
        target_link_libraries(${test_target} asthra_compiler asthra_runtime)
    endif()
    
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES
        TIMEOUT 30
        LABELS semantic
    )
    
    # Add test executable as dependency to build-tests target
    if(TARGET build-tests)
        add_dependencies(build-tests ${test_target})
    endif()
endforeach()

# Loop control flow test needs test_type_system_common.c
add_executable(semantic_test_loop_control_flow_semantic 
    test_loop_control_flow_semantic.c
    test_type_system_common.c
)

if(TARGET asthra_test_framework)
    target_link_libraries(semantic_test_loop_control_flow_semantic asthra_test_framework)
else()
    target_link_libraries(semantic_test_loop_control_flow_semantic asthra_compiler asthra_runtime)
endif()

add_test(NAME semantic_test_loop_control_flow_semantic COMMAND semantic_test_loop_control_flow_semantic)
set_tests_properties(semantic_test_loop_control_flow_semantic PROPERTIES
    TIMEOUT 30
    LABELS semantic
)

# Add test executable as dependency to build-tests target
if(TARGET build-tests)
    add_dependencies(build-tests semantic_test_loop_control_flow_semantic)
endif()

# Pattern matching test with multiple source files
set(PATTERN_MATCHING_SOURCES
    test_pattern_matching_main.c
    test_pattern_matching_helpers.c
    test_pattern_matching_enum.c
    test_pattern_matching_struct.c
    test_pattern_matching_advanced.c
)

# Create pattern matching test executable
add_executable(semantic_pattern_matching_test ${PATTERN_MATCHING_SOURCES} test_type_system_common.c)

if(TARGET asthra_test_framework)
    target_link_libraries(semantic_pattern_matching_test asthra_test_framework)
else()
    target_link_libraries(semantic_pattern_matching_test asthra_compiler asthra_runtime)
endif()

add_test(NAME semantic_pattern_matching_test COMMAND semantic_pattern_matching_test)
set_tests_properties(semantic_pattern_matching_test PROPERTIES
    TIMEOUT 30
    LABELS semantic
)

# Add test executable as dependency to build-tests target
if(TARGET build-tests)
    add_dependencies(build-tests semantic_pattern_matching_test)
endif()

# Symbol resolution test with single main file that includes others
set(SYMBOL_RESOLUTION_SOURCES
    test_symbol_resolution_main.c
    test_symbol_resolution_helpers.c
)

# Create symbol resolution test executable
add_executable(semantic_symbol_resolution_test ${SYMBOL_RESOLUTION_SOURCES})

if(TARGET asthra_test_framework)
    target_link_libraries(semantic_symbol_resolution_test asthra_test_framework)
else()
    target_link_libraries(semantic_symbol_resolution_test asthra_compiler asthra_runtime)
endif()

add_test(NAME semantic_symbol_resolution_test COMMAND semantic_symbol_resolution_test)
set_tests_properties(semantic_symbol_resolution_test PROPERTIES
    TIMEOUT 30
    LABELS semantic
)

# Add test executable as dependency to build-tests target
if(TARGET build-tests)
    add_dependencies(build-tests semantic_symbol_resolution_test)
endif()

# Expression validation test with multiple source files
set(EXPRESSION_VALIDATION_SOURCES
    test_expression_validation_main.c
    test_expression_validation_helpers.c
)

# Create expression validation test executable
add_executable(semantic_expression_validation_test ${EXPRESSION_VALIDATION_SOURCES} test_type_system_common.c)

if(TARGET asthra_test_framework)
    target_link_libraries(semantic_expression_validation_test asthra_test_framework)
else()
    target_link_libraries(semantic_expression_validation_test asthra_compiler asthra_runtime)
endif()

add_test(NAME semantic_expression_validation_test COMMAND semantic_expression_validation_test)
set_tests_properties(semantic_expression_validation_test PROPERTIES
    TIMEOUT 30
    LABELS semantic
)

# Add test executable as dependency to build-tests target
if(TARGET build-tests)
    add_dependencies(build-tests semantic_expression_validation_test)
endif()

# Files to exclude from auto-discovery
set(EXCLUDE_FROM_AUTO_DISCOVERY
    ${TYPE_SYSTEM_TESTS}
    ${MUTABILITY_TESTS}
    ${STANDALONE_TESTS}
    ${PATTERN_MATCHING_SOURCES}
    ${SYMBOL_RESOLUTION_SOURCES}
    ${EXPRESSION_VALIDATION_SOURCES}
    test_symbol_resolution_variables.c
    test_symbol_resolution_functions.c
    test_symbol_resolution_scopes.c
    test_symbol_resolution_imports.c
    test_symbol_resolution_types.c
    test_symbol_resolution_methods.c
    test_symbol_resolution_errors.c
    test_expression_validation_test.c
    test_expression_validation_binary.c
    test_expression_validation_unary.c
    test_expression_validation_function_calls.c
    test_expression_validation_field_access.c
    test_expression_validation_arrays.c
    test_expression_validation_methods.c
    test_expression_validation_type_casts.c
    test_expression_validation_complex.c
    test_type_system_common.c
    test_mutability_common.c
    test_pattern_matching_common.h
    test_symbol_resolution_common.h
    test_expression_validation_common.h
    test_type_system_original.c.bak  # Backup file
    test_mutability.c  # Old file being replaced
    test_loop_control_flow_semantic.c  # Handled separately above
)

# Get all .c files in the directory
file(GLOB ALL_C_FILES "*.c")

# Create a list of any remaining tests not explicitly handled
set(REMAINING_TESTS)
foreach(file ${ALL_C_FILES})
    get_filename_component(filename ${file} NAME)
    list(FIND EXCLUDE_FROM_AUTO_DISCOVERY ${filename} found_index)
    if(found_index EQUAL -1)
        list(APPEND REMAINING_TESTS ${filename})
    endif()
endforeach()

# Add any remaining tests normally
foreach(test_file ${REMAINING_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "semantic_${test_name}")
    
    add_executable(${test_target} ${test_file})
    
    if(TARGET asthra_test_framework)
        target_link_libraries(${test_target} asthra_test_framework)
    else()
        target_link_libraries(${test_target} asthra_compiler asthra_runtime)
    endif()
    
    add_test(NAME ${test_target} COMMAND ${test_target})
    
    # Special case for struct_field_visibility test - increase timeout
    if(${test_name} STREQUAL "test_struct_field_visibility_semantic")
        set_tests_properties(${test_target} PROPERTIES
            TIMEOUT 120
            LABELS semantic
        )
    else()
        set_tests_properties(${test_target} PROPERTIES
            TIMEOUT 30
            LABELS semantic
        )
    endif()
    
    # Add test executable as dependency to build-tests target
    if(TARGET build-tests)
        add_dependencies(build-tests ${test_target})
    endif()
endforeach()