# Function Call Tests
# Tests that depend on test_function_calls_common.c

set(FUNCTION_CALL_TESTS
    test_associated_function_calls.c
    test_instance_method_calls.c
    test_self_parameter_handling.c
    test_chained_method_calls.c
)

# Additional function call tests
set(FUNCTION_CALL_ADDITIONAL_TESTS
    test_function_calls_framework_minimal.c
    test_function_calls_main.c
    test_function_calls_minimal.c
    test_function_calls_simple.c
)

# Method mangling tests
set(METHOD_MANGLING_TESTS
    test_method_mangling_framework_minimal.c
    test_method_mangling_minimal.c
    test_method_mangling.c
)

# Create executables for function call tests with common file
foreach(test_file ${FUNCTION_CALL_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "codegen_${test_name}")
    
    add_executable(${test_target} ${test_file} test_function_calls_common.c)
    
    # Note: test_function_calls_main uses a simplified approach
    # No special defines needed
    
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

# Create executables for additional function call tests
foreach(test_file ${FUNCTION_CALL_ADDITIONAL_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "codegen_${test_name}")
    
    add_executable(${test_target} ${test_file} test_function_calls_common.c)
    
    # Note: test_function_calls_main uses a simplified approach
    # No special defines needed
    
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

# Create executables for method mangling tests
foreach(test_file ${METHOD_MANGLING_TESTS})
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