# Integration Tests
# Miscellaneous integration tests, prototypes, and complex scenarios

# Tests to exclude due to dependency on deleted code generator
set(EXCLUDED_TESTS
    test_const_generation_simple.c      # depends on ffi_assembly_const.h
    test_generic_struct_phase4.c        # depends on code_generator_core.h
    test_immutable_by_default_phase3.c  # depends on optimization_analysis.h
    # Deleted files that tested non-existent infrastructure:
    # test_code_generator.c
    # test_phase5_simple.c
    # test_multiline_string_codegen.c
    # test_simple_arithmetic.c
)

# Tests that need the backend wrapper
set(BACKEND_WRAPPER_TESTS
    test_minimal_codegen.c
    test_void_semantic_fix_phase4_codegen.c
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
    get_filename_component(test_filename ${test_file} NAME)
    set(test_target "codegen_${test_name}")
    
    # Check if this test needs the backend wrapper
    if(test_filename IN_LIST BACKEND_WRAPPER_TESTS)
        add_executable(${test_target} ${test_file} ${CMAKE_CURRENT_SOURCE_DIR}/../codegen_backend_wrapper.c)
    else()
        add_executable(${test_target} ${test_file})
    endif()
    
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
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    )
    
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES TIMEOUT 30 LABELS "codegen")
    
    if(TARGET build-tests)
        add_dependencies(build-tests ${test_target})
    endif()
endforeach()