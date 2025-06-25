# Integration Tests
# Miscellaneous integration tests, prototypes, and complex scenarios

# Tests to exclude due to dependency on deleted code generator
set(EXCLUDED_TESTS
    test_code_generator.c               # depends on code_generator.h
    test_const_generation_simple.c      # depends on ffi_assembly_const.h
    test_generic_struct_phase4.c        # depends on code_generator_core.h
    test_immutable_by_default_phase3.c  # depends on optimization_analysis.h
    test_minimal_codegen.c              # depends on code_generator.h
    test_multiline_string_codegen.c     # depends on code_generator.h
    test_phase5_simple.c                # depends on code_generator.h
    test_simple_arithmetic.c            # depends on code_generator.h
    test_void_semantic_fix_phase4_codegen.c # depends on code_generator.h
)

# Get all test files in this directory
file(GLOB ALL_INTEGRATION_TEST_FILES "test_*.c")

# Filter out excluded tests
set(INTEGRATION_TEST_FILES)
foreach(test_file ${ALL_INTEGRATION_TEST_FILES})
    get_filename_component(test_name ${test_file} NAME)
    if(NOT test_name IN_LIST EXCLUDED_TESTS)
        list(APPEND INTEGRATION_TEST_FILES ${test_file})
    endif()
endforeach()

# Create executables for all integration tests
foreach(test_file ${INTEGRATION_TEST_FILES})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "codegen_${test_name}")
    
    add_executable(${test_target} ${test_file})
    
    if(TARGET asthra_test_framework)
        target_link_libraries(${test_target} asthra_test_framework)
    else()
        target_link_libraries(${test_target} asthra_compiler asthra_runtime)
    endif()
    
    target_include_directories(${test_target} PRIVATE 
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/parser
        ${CMAKE_SOURCE_DIR}/src/analysis
        ${CMAKE_SOURCE_DIR}/src/codegen
        ${CMAKE_SOURCE_DIR}/tests/framework
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES TIMEOUT 30 LABELS "codegen")
    
    if(TARGET build-tests)
        add_dependencies(build-tests ${test_target})
    endif()
endforeach()