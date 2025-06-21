# Symbol Export Tests
# Tests for public/private symbol visibility and export functionality

set(SYMBOL_EXPORT_TESTS
    test_mixed_visibility_export.c
    test_private_symbol_export.c
    test_public_symbol_export.c
    test_symbol_export.c
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