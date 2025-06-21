# Variant Tests
# Tests that depend on test_variant_types_common.c

set(VARIANT_TESTS
    test_basic_variant_creation.c
    test_option_type_variants.c
    test_result_type_variants.c
    test_nested_variant_structures.c
)

# Create executables for variant tests with common file
foreach(test_file ${VARIANT_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "codegen_${test_name}")
    
    add_executable(${test_target} ${test_file} test_variant_types_common.c)
    
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