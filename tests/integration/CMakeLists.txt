# Integration Tests Custom Configuration
# Special handling for integration tests that need to be linked together

# Loop control flow integration test - needs explicit configuration
add_executable(integration_test_loop_control_flow_integration test_loop_control_flow_integration.c)
if(TARGET asthra_test_framework)
    target_link_libraries(integration_test_loop_control_flow_integration asthra_test_framework)
else()
    target_link_libraries(integration_test_loop_control_flow_integration asthra_compiler asthra_runtime)
endif()
add_test(NAME integration_test_loop_control_flow_integration COMMAND integration_test_loop_control_flow_integration)
set_tests_properties(integration_test_loop_control_flow_integration PROPERTIES TIMEOUT 30 LABELS integration)
if(TARGET build-tests)
    add_dependencies(build-tests integration_test_loop_control_flow_integration)
endif()

# REMOVED: Generic structs tests depend on deleted generic_instantiation.h
# # Create the generic structs modular test runner as a special multi-file executable
# set(GENERIC_STRUCTS_MODULAR_TEST_SOURCES
#     generic_structs_test_modular.c
#     generic_structs_test_common.c
#     generic_structs_test_declarations.c
#     generic_structs_test_usage.c
#     generic_structs_test_system.c
# )
# 
# # Create the modular generic structs test runner executable
# add_executable(integration_generic_structs_test_modular ${GENERIC_STRUCTS_MODULAR_TEST_SOURCES})
# 
# if(TARGET asthra_test_framework)
#     target_link_libraries(integration_generic_structs_test_modular asthra_test_framework)
# else()
#     target_link_libraries(integration_generic_structs_test_modular asthra_compiler asthra_runtime)
# endif()
# 
# # Add as a test
# add_test(NAME integration_generic_structs_test_modular COMMAND integration_generic_structs_test_modular)
# set_tests_properties(integration_generic_structs_test_modular PROPERTIES
#     TIMEOUT 60
#     LABELS integration
# )
# 
# # Add test executable as dependency to build-tests target
# if(TARGET build-tests)
#     add_dependencies(build-tests integration_generic_structs_test_modular)
# endif()

# REMOVED: Real program tests depend on deleted code_generator.h
# # Create the real program test validation executable with all helper files
# set(REAL_PROGRAM_TEST_SOURCES
#     real_program_validation_main.c
#     real_program_test_framework.c
#     real_program_test_data.c
#     real_program_test_utils.c
#     real_program_test_suite.c
#     real_program_test_integration.c
#     real_program_test_quality.c
#     real_program_test_performance.c
#     real_program_test_false_positive.c
#     real_program_test_generators.c
# )
# 
# # Create the real program test validation executable
# add_executable(integration_test_real_program_validation ${REAL_PROGRAM_TEST_SOURCES})
# 
# if(TARGET asthra_test_framework)
#     target_link_libraries(integration_test_real_program_validation asthra_test_framework)
#     target_link_libraries(integration_test_real_program_validation asthra_compiler asthra_runtime)
# else()
#     target_link_libraries(integration_test_real_program_validation asthra_compiler asthra_runtime)
# endif()
# 
# # Add as a test
# # TEMPORARILY DISABLED: Test still has infinite loop issue with invalid programs after modularization
# # The modular structure is complete but the underlying parsing issue needs to be resolved first
# # add_test(NAME integration_test_real_program_validation COMMAND integration_test_real_program_validation)
# # set_tests_properties(integration_test_real_program_validation PROPERTIES
# #     TIMEOUT 120
# #     LABELS integration
# # )
# 
# # Add test executable as dependency to build-tests target
# if(TARGET build-tests)
#     add_dependencies(build-tests integration_test_real_program_validation)
# endif()

# List of integration test files that should not be built as individual executables
set(INTEGRATION_MODULAR_TEST_FILES
    generic_structs_test_modular.c
    generic_structs_test_common.c
    generic_structs_test_declarations.c
    generic_structs_test_usage.c
    generic_structs_test_system.c
)

# Auto-discover other test files (excluding modular tests)
file(GLOB ALL_INTEGRATION_TEST_FILES "*.c")
set(INDIVIDUAL_INTEGRATION_TEST_FILES ${ALL_INTEGRATION_TEST_FILES})

# Remove modular test files from individual test list
foreach(modular_file ${INTEGRATION_MODULAR_TEST_FILES})
    list(REMOVE_ITEM INDIVIDUAL_INTEGRATION_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${modular_file}")
endforeach()

# List of files to exclude due to API mismatches
set(INTEGRATION_EXCLUDE
    real_program_test_framework.c
    real_program_test_data.c  # Helper file for test data
    real_program_test_utils.c  # Helper file for utilities
    real_program_test_suite.c  # Helper file for test suite management
    real_program_test_integration.c  # Helper file for integration testing
    real_program_test_quality.c  # Helper file for quality gates
    real_program_test_performance.c  # Helper file for performance testing
    real_program_test_false_positive.c  # Helper file for false positive detection
    real_program_test_generators.c  # Helper file for program generators
    real_program_validation_main.c  # Main test runner for modular real program validation
    real_program_validation_valid_tests.c  # Modular valid program tests
    real_program_validation_invalid_tests.c  # Modular invalid program tests
    real_program_validation_performance_tests.c  # Modular performance tests
    real_program_validation_feature_tests.c  # Modular feature validation tests
    test_compilation_errors.c
    test_compilation_performance.c  # depends on code_generator.h
    test_concurrency_performance_patterns.c
    test_concurrency_tiers_common.c
    test_concurrency_tiers_main.c
    test_concurrency_migration_patterns.c  # depends on test_concurrency_tiers_common.h
    test_concurrency_real_world_scenarios.c  # depends on test_concurrency_tiers_common.h
    test_concurrency_error_detection.c  # depends on test_concurrency_tiers_common.h
    test_concurrency_tier_progression.c  # depends on test_concurrency_tiers_common.h
    test_generic_structs_phase5_core.c
    test_generic_structs_phase5_main.c
    test_generic_structs_basic_integration.c  # depends on generic_instantiation.h
    test_generic_structs_advanced_integration.c  # depends on generic_instantiation.h
    test_generic_structs_performance_consistency.c  # depends on generic_instantiation.h
    test_integration_main.c
    test_performance_comprehensive.c
    test_real_program_validation.c
    test_static_analysis_main.c
    generic_structs_phase5_core_helper.c  # Helper file, not a test
    test_enum_integration_common.h  # Common header file for enum tests
    test_enum_end_to_end.c  # uses CodeGenerator directly
    test_enum_error_handling.c  # uses CodeGenerator directly
    test_enum_nested_variants.c  # uses CodeGenerator directly
    test_enum_option_type.c  # uses CodeGenerator directly
    test_loop_control_flow_integration.c  # Has its own explicit configuration above
    test_args_integration.c  # Disabled: requires runtime function implementations
    test_option_integration.c  # uses asthra_backend_emit_assembly which doesn't exist
)

foreach(exclude_file ${INTEGRATION_EXCLUDE})
    list(REMOVE_ITEM INDIVIDUAL_INTEGRATION_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${exclude_file}")
endforeach()

# Create individual test executables for non-modular tests
foreach(test_file ${INDIVIDUAL_INTEGRATION_TEST_FILES})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "integration_${test_name}")
    
    # Special handling for tests that need additional sources
    if(test_name MATCHES "^test_concurrency_(error_detection|migration_patterns|real_world_scenarios|tier_progression)$")
        # These tests need the common concurrency implementation
        add_executable(${test_target} ${test_file} test_concurrency_tiers_common.c)
    elseif(test_name MATCHES "^test_generic_structs_(basic_integration|advanced_integration|performance_consistency)$")
        # These tests need the generic structs common implementation and backend wrapper
        add_executable(${test_target} ${test_file} generic_structs_phase5_core_helper.c backend_test_wrapper.c)
    elseif(test_name MATCHES "^test_enum_(compilation_pipeline|end_to_end|error_handling|memory_management|nested_variants|option_type|pattern_matching)$")
        # These tests need the backend wrapper for migration from old CodeGenerator API
        add_executable(${test_target} ${test_file} backend_test_wrapper.c)
    else()
        add_executable(${test_target} ${test_file})
    endif()
    
    if(TARGET asthra_test_framework)
        target_link_libraries(${test_target} asthra_test_framework)
        # Also link asthra_compiler and asthra_runtime for integration tests that need compiler internals
        target_link_libraries(${test_target} asthra_compiler asthra_runtime)
    else()
        target_link_libraries(${test_target} asthra_compiler asthra_runtime)
    endif()
    
    add_test(NAME ${test_target} COMMAND ${test_target})
    set_tests_properties(${test_target} PROPERTIES
        TIMEOUT 30
        LABELS integration
    )
    
    # Add test executable as dependency to build-tests target
    if(TARGET build-tests)
        add_dependencies(build-tests ${test_target})
    endif()
endforeach()