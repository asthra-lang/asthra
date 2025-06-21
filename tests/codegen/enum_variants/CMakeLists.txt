# Enum Variant Tests
# Tests that depend on test_enum_variant_common.c

set(ENUM_VARIANT_TESTS
    test_result_variant_codegen.c
    test_option_variant_codegen.c
    test_enum_variant_complex.c
)

# Create executables for enum variant tests with common file
foreach(test_file ${ENUM_VARIANT_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "codegen_${test_name}")
    
    add_executable(${test_target} ${test_file} test_enum_variant_common.c)
    
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