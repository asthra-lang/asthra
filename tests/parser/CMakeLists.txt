# Parser Tests Custom Configuration
# Special handling for parser tests that need to be linked together

# If-let modular test needs special handling - link all if_let test files
set(IF_LET_TEST_SOURCES
    if_let_test_modular.c
    if_let_test_common.c
    if_let_test_parser.c
    if_let_test_semantic.c
    if_let_test_codegen.c
    if_let_test_integration.c
)

add_executable(parser_if_let_test_modular ${IF_LET_TEST_SOURCES})

if(TARGET asthra_test_framework)
    target_link_libraries(parser_if_let_test_modular asthra_test_framework)
else()
    target_link_libraries(parser_if_let_test_modular asthra_compiler asthra_runtime)
endif()

add_test(NAME parser_if_let_test_modular COMMAND parser_if_let_test_modular)
set_tests_properties(parser_if_let_test_modular PROPERTIES
    TIMEOUT 30
    LABELS parser
)

# List of parser test files that should not be built as individual executables
set(PARSER_MULTI_FILE_TESTS
    if_let_test_modular.c
    if_let_test_common.c
    if_let_test_parser.c
    if_let_test_semantic.c
    if_let_test_codegen.c
    if_let_test_integration.c
)

# Auto-discover other test files (excluding multi-file tests)
file(GLOB ALL_PARSER_TEST_FILES "*.c")
set(INDIVIDUAL_PARSER_TEST_FILES ${ALL_PARSER_TEST_FILES})

# Remove multi-file test files from individual test list
foreach(multi_file ${PARSER_MULTI_FILE_TESTS})
    list(REMOVE_ITEM INDIVIDUAL_PARSER_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${multi_file}")
endforeach()

# List of files to exclude due to API mismatches
set(PARSER_EXCLUDE
    test_advanced_pub_impl_self.c
    test_annotations_basic.c
    test_annotations_common.c
    test_annotations_complex.c
    test_annotations_errors.c
    test_annotations_main.c
    test_annotations_performance.c
    test_annotations_validation.c
    test_at_annotation_removal.c
    test_basic_pub_impl_self.c
    test_enum_generic_constructor.c
    test_generic_structs_phase5.c
    test_grammar_main.c
    test_grammar_statements.c
    test_postfix_expression_ambiguity.c
    test_pub_impl_self_common.c
    test_pub_impl_self_main.c
    test_return_statements_v14.c
    test_return_v14_simple.c
    test_type_annotation_requirement.c
    test_v1_18_comprehensive_explicit_syntax.c
)

foreach(exclude_file ${PARSER_EXCLUDE})
    list(REMOVE_ITEM INDIVIDUAL_PARSER_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${exclude_file}")
endforeach()

# Create individual test executables for other tests
foreach(test_file ${INDIVIDUAL_PARSER_TEST_FILES})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "parser_${test_name}")
    
    add_executable(${test_target} ${test_file})
    
    if(TARGET asthra_test_framework)
        target_link_libraries(${test_target} asthra_test_framework)
    else()
        target_link_libraries(${test_target} asthra_compiler asthra_runtime)
    endif()
    
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES
        TIMEOUT 30
        LABELS parser
    )
    
    # Add test executable as dependency to build-tests target
    if(TARGET build-tests)
        add_dependencies(build-tests ${test_target})
    endif()
endforeach()