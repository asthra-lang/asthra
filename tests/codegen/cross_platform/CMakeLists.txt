# Cross-Platform Tests
# Tests for platform detection, cross-compilation, and platform-specific code generation
# DISABLED: These tests depend on the removed assembly/platform-specific code generation
# LLVM backend handles cross-platform compilation internally

return()  # Skip this directory

set(CROSS_PLATFORM_TESTS
    test_platform_detection.c
    test_platform_specific_codegen.c
    test_calling_convention_handling.c
    test_cross_compilation_validation.c
)

# Create executables for cross-platform tests (no common file needed)
foreach(test_file ${CROSS_PLATFORM_TESTS})
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