# FFI Tests Custom Configuration
# Special handling for enhanced FFI tests that need to be linked together

# Create the FFI comprehensive test runner as a special multi-file executable
set(FFI_ENHANCED_TEST_SOURCES
    test_ffi.c
    test_ffi_enhanced_simple.c
)

# Create the comprehensive FFI test runner executable
add_executable(ffi_test_ffi ${FFI_ENHANCED_TEST_SOURCES})

if(TARGET asthra_test_framework)
    target_link_libraries(ffi_test_ffi asthra_test_framework)
else()
    target_link_libraries(ffi_test_ffi asthra_compiler asthra_runtime)
endif()

# Add as a test
add_test(NAME ffi_test_ffi COMMAND ffi_test_ffi)
set_tests_properties(ffi_test_ffi PROPERTIES
    TIMEOUT 60
    LABELS ffi
)

# Add test executable as dependency to build-tests target
if(TARGET build-tests)
    add_dependencies(build-tests ffi_test_ffi)
endif()

# List of FFI test files that should not be built as individual executables
set(FFI_ENHANCED_TEST_FILES
    test_ffi.c
    test_ffi_enhanced.c
    test_ffi_enhanced_implementations.c
    test_ffi_enhanced_simple.c
)

# Create FFI assembly core test with common helpers
add_executable(ffi_test_ffi_assembly_core
    test_ffi_assembly_core.c
    test_ffi_assembly_common.c
)

if(TARGET asthra_test_framework)
    target_link_libraries(ffi_test_ffi_assembly_core asthra_test_framework)
else()
    target_link_libraries(ffi_test_ffi_assembly_core asthra_compiler asthra_runtime)
endif()

add_test(NAME ffi_test_ffi_assembly_core COMMAND ffi_test_ffi_assembly_core)
set_tests_properties(ffi_test_ffi_assembly_core PROPERTIES
    TIMEOUT 30
    LABELS ffi
)

if(TARGET build-tests)
    add_dependencies(build-tests ffi_test_ffi_assembly_core)
endif()

# Auto-discover other test files (excluding enhanced tests)
file(GLOB ALL_FFI_TEST_FILES "*.c")
set(INDIVIDUAL_FFI_TEST_FILES ${ALL_FFI_TEST_FILES})

# Remove enhanced test files from individual test list
foreach(enhanced_file ${FFI_ENHANCED_TEST_FILES})
    list(REMOVE_ITEM INDIVIDUAL_FFI_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${enhanced_file}")
endforeach()

# Remove FFI assembly tests that need common helpers
list(REMOVE_ITEM INDIVIDUAL_FFI_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test_ffi_assembly_core.c")
list(REMOVE_ITEM INDIVIDUAL_FFI_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test_ffi_assembly_common.c")

# List of tests that should link directly to compiler without test framework
set(FFI_ASSEMBLY_TESTS
    test_ffi_assembly_calls.c
    # test_ffi_assembly_patterns.c - commented out due to outdated AST API
)

# Create executables for FFI assembly tests with common file
foreach(assembly_test ${FFI_ASSEMBLY_TESTS})
    get_filename_component(test_name ${assembly_test} NAME_WE)
    set(test_target "ffi_${test_name}")
    
    add_executable(${test_target} 
        ${CMAKE_CURRENT_SOURCE_DIR}/${assembly_test} 
        test_ffi_assembly_common.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../framework/semantic_test_utils.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../framework/performance_test_utils.c)
    target_link_libraries(${test_target} asthra_compiler asthra_runtime)
    
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES
        TIMEOUT 30
        LABELS ffi
    )
    
    # Add test executable as dependency to build-tests target
    if(TARGET build-tests)
        add_dependencies(build-tests ${test_target})
    endif()
endforeach()

# Remove assembly tests from individual test list
foreach(assembly_test ${FFI_ASSEMBLY_TESTS})
    list(REMOVE_ITEM INDIVIDUAL_FFI_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${assembly_test}")
endforeach()

# Also remove common files and problematic tests
list(REMOVE_ITEM INDIVIDUAL_FFI_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test_ffi_assembly_common.c")

# Exclude minimal FFI tests that have symbol conflicts
set(MINIMAL_FFI_TESTS_EXCLUDE
    test_ffi_minimal_concurrency.c
    test_ffi_minimal_security.c
    test_ffi_minimal_strings.c
    test_ffi_minimal_slices.c
)

# Remove minimal FFI tests from individual test list to avoid symbol conflicts
foreach(minimal_test ${MINIMAL_FFI_TESTS_EXCLUDE})
    list(REMOVE_ITEM INDIVIDUAL_FFI_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${minimal_test}")
endforeach()

# Remove all FFI assembly tests except those handled specially
set(FFI_ASSEMBLY_EXCLUDE
    test_ffi_assembly_patterns.c
    test_ffi_assembly_master_v2.c
    test_ffi_assembly_optimization.c
    test_ffi_assembly_pattern_matching.c
    test_ffi_assembly_patterns_simple.c
    test_ffi_assembly_security_concurrency_simple.c
    test_ffi_assembly_security_concurrency.c
    test_ffi_assembly_string_slice.c
    test_ffi_assembly_strings_slices_simple.c
    test_ffi_assembly_strings_slices.c
    test_ffi_complete.c
    test_ffi_grammar_compliance.c
    test_ffi_memory_diagnostics.c
    test_ffi_memory_ownership.c
    test_ffi_memory_safety.c
    test_ffi_memory_security.c
    test_ffi_memory_slices.c
    test_ffi_memory_strings.c
    test_ffi_minimal_runner.c
    test_ffi_minimal_tests.c
    test_ffi_ownership.c
    test_ffi_simple.c
    test_ffi_stubs.c
    # Also exclude memory tests that need special handling
    test_ffi_memory_all.c
    test_ffi_memory_allocation.c
    test_ffi_memory_integration.c
    test_ffi_memory_patterns.c
    test_ffi_memory_common.c
)

foreach(exclude_file ${FFI_ASSEMBLY_EXCLUDE})
    list(REMOVE_ITEM INDIVIDUAL_FFI_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${exclude_file}")
endforeach()

# Create the enhanced FFI stubs test with all its modular components
set(FFI_STUBS_TEST_SOURCES
    test_ffi_stubs.c
    ffi_stubs_generator.c
    ffi_stubs_memory.c
    ffi_stubs_marshaling.c
    ffi_stubs_ast_parser.c
    ffi_stubs_codegen.c
    ffi_stubs_test_suite.c
)

add_executable(ffi_test_ffi_stubs ${FFI_STUBS_TEST_SOURCES})

if(TARGET asthra_test_framework)
    target_link_libraries(ffi_test_ffi_stubs asthra_test_framework)
else()
    target_link_libraries(ffi_test_ffi_stubs asthra_compiler asthra_runtime)
endif()

add_test(NAME ffi_test_ffi_stubs COMMAND ffi_test_ffi_stubs)
set_tests_properties(ffi_test_ffi_stubs PROPERTIES
    TIMEOUT 60
    LABELS ffi
)

if(TARGET build-tests)
    add_dependencies(build-tests ffi_test_ffi_stubs)
endif()

# Also exclude the modular FFI stubs files from individual test compilation
set(FFI_STUBS_MODULAR_FILES
    ffi_stubs_generator.c
    ffi_stubs_memory.c
    ffi_stubs_marshaling.c
    ffi_stubs_ast_parser.c
    ffi_stubs_codegen.c
    ffi_stubs_test_suite.c
)

foreach(modular_file ${FFI_STUBS_MODULAR_FILES})
    list(REMOVE_ITEM INDIVIDUAL_FFI_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${modular_file}")
endforeach()

# Create individual test executables for non-enhanced tests
foreach(test_file ${INDIVIDUAL_FFI_TEST_FILES})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "ffi_${test_name}")
    
    add_executable(${test_target} ${test_file})
    
    # Use test framework for other tests
    if(TARGET asthra_test_framework)
        target_link_libraries(${test_target} asthra_test_framework)
    else()
        target_link_libraries(${test_target} asthra_compiler asthra_runtime)
    endif()
    
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES
        TIMEOUT 30
        LABELS ffi
    )
    
    # Add test executable as dependency to build-tests target
    if(TARGET build-tests)
        add_dependencies(build-tests ${test_target})
    endif()
endforeach()