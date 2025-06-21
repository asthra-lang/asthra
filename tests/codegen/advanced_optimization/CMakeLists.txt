# Advanced Optimization Tests
# Tests for multi-pass optimization, pipeline effectiveness, and validation

set(ADVANCED_OPTIMIZATION_TESTS
    test_multi_pass_optimization.c
    test_optimization_effectiveness.c
    test_optimization_pipeline_creation.c
    test_optimization_validation.c
)

# Create executables for advanced optimization tests
foreach(test_file ${ADVANCED_OPTIMIZATION_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "codegen_${test_name}")
    
    add_executable(${test_target} ${test_file} test_advanced_optimization_pipeline_common.c)
    
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
    set_tests_properties(${test_target} PROPERTIES LABELS "codegen")
    
    if(TARGET build-tests)
        add_dependencies(build-tests ${test_target})
    endif()
endforeach()