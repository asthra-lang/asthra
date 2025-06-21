# Optimization Tests
# Tests that depend on test_optimization_passes_stubs.c

set(OPTIMIZATION_TESTS
    test_common_subexpression_elimination.c
    test_constant_folding.c
    test_dead_code_elimination.c
    test_peephole_optimization.c
    test_strength_reduction.c
    test_optimization_passes.c
)

# Never type optimization test (doesn't need stubs)
set(NEVER_TYPE_TEST test_never_type_optimization.c)

# Create executables for optimization tests with common implementation
foreach(test_file ${OPTIMIZATION_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "codegen_${test_name}")
    
    add_executable(${test_target} ${test_file} test_optimization_passes_stubs.c)
    
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

# Build Never type optimization test separately
add_executable(codegen_test_never_type_optimization ${NEVER_TYPE_TEST})
target_link_libraries(codegen_test_never_type_optimization asthra_compiler asthra_runtime)
add_test(NAME codegen_test_never_type_optimization COMMAND codegen_test_never_type_optimization)
set_tests_properties(codegen_test_never_type_optimization PROPERTIES TIMEOUT 30 LABELS codegen)

if(TARGET build-tests)
    add_dependencies(build-tests codegen_test_never_type_optimization)
endif()