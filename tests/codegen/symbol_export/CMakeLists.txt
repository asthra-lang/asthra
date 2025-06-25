# Symbol Export Tests
# Tests for public/private symbol visibility and export functionality
# REMOVED: Symbol export tests depend on deleted assembly backend code
# LLVM backend handles symbol exports internally

set(SYMBOL_EXPORT_TESTS
    # REMOVED: test_mixed_visibility_export.c (depends on deleted assembly backend)
    # REMOVED: test_private_symbol_export.c (depends on deleted assembly backend)
    # REMOVED: test_public_symbol_export.c (depends on deleted assembly backend)
    # REMOVED: test_symbol_export.c (depends on deleted assembly backend)
)

# Create executables for symbol export tests
foreach(test_file ${SYMBOL_EXPORT_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "codegen_${test_name}")
    
    add_executable(${test_target} ${test_file} symbol_export_test_common.c)
    
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