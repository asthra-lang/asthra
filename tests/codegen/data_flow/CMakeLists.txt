# Data Flow Analysis Tests
# Tests that depend on test_data_flow_common.c
# NOTE: These tests use incompatible Instruction type definitions that conflict
# with the framework. They need to be refactored to use consistent types.

set(DATA_FLOW_TESTS
    test_reaching_definitions.c
    test_live_variable_analysis.c
    test_use_def_chains.c
)

# Additional data flow analysis tests
set(DATA_FLOW_ANALYSIS_TESTS
    test_data_flow_analysis_core.c
    test_data_flow_analysis_main.c
    test_data_flow_analysis_tests.c
    test_data_flow_control_flow.c
    test_data_flow_live_variables.c
    test_data_flow_reaching_definitions.c
    test_data_flow_use_def_chains.c
)

# Create executables for data flow tests with common file
foreach(test_file ${DATA_FLOW_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "codegen_${test_name}")
    
    add_executable(${test_target} ${test_file} test_data_flow_common.c)
    
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

# Create executables for additional data flow analysis tests
foreach(test_file ${DATA_FLOW_ANALYSIS_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "codegen_${test_name}")
    
    if(test_file STREQUAL "test_data_flow_analysis_tests.c")
        add_executable(${test_target} ${test_file} data_flow_stubs.c)
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
    )
    
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES TIMEOUT 30 LABELS "codegen")
    
    if(TARGET build-tests)
        add_dependencies(build-tests ${test_target})
    endif()
endforeach()