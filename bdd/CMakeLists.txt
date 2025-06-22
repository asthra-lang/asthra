# BDD Testing Infrastructure for Asthra
# Provides Behavior-Driven Development testing framework

message(STATUS "Configuring BDD testing infrastructure")

# BDD test output directory
set(BDD_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/bin)
file(MAKE_DIRECTORY ${BDD_OUTPUT_DIR})

# Set BDD-specific output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BDD_OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)

# BDD test categories
set(BDD_TEST_CATEGORIES
    acceptance
    integration
    scenario
    feature
)

# Include directories for BDD tests
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/steps
    ${CMAKE_CURRENT_SOURCE_DIR}/support
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/runtime
    ${PROJECT_SOURCE_DIR}/tests/framework
)

# Function to add BDD test executable
function(add_bdd_test name category)
    set(test_target bdd_${category}_${name})
    
    # Collect source files
    file(GLOB step_sources 
        ${CMAKE_CURRENT_SOURCE_DIR}/steps/${category}/${name}_steps.c
        ${CMAKE_CURRENT_SOURCE_DIR}/steps/${category}/common_steps.c
    )
    
    file(GLOB support_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/support/*.c
    )
    
    # Create test executable
    add_executable(${test_target} 
        ${step_sources}
        ${support_sources}
    )
    
    # Link with Asthra runtime and test framework
    target_link_libraries(${test_target}
        asthra_runtime
        test_framework
    )
    
    # Set output directory
    set_target_properties(${test_target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${BDD_OUTPUT_DIR}
    )
    
    # Add to CTest
    add_test(NAME ${test_target} 
        COMMAND ${test_target}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    
    # Set test properties
    set_tests_properties(${test_target} PROPERTIES
        LABELS "bdd;${category}"
        TIMEOUT 60
        ENVIRONMENT "BDD_FEATURES_DIR=${CMAKE_CURRENT_SOURCE_DIR}/features"
    )
    
    # Add to build-tests target
    add_dependencies(build-tests ${test_target})
endfunction()

# Create BDD test targets
add_custom_target(bdd-tests
    COMMAND ${CMAKE_CTEST_COMMAND} 
            --output-on-failure
            --label-regex "^bdd$"
    COMMENT "Running all BDD tests"
    USES_TERMINAL
)

# Category-specific BDD test targets
foreach(category ${BDD_TEST_CATEGORIES})
    add_custom_target(bdd-${category}
        COMMAND ${CMAKE_CTEST_COMMAND}
                --output-on-failure
                --label-regex "bdd.*${category}"
        COMMENT "Running BDD ${category} tests"
        USES_TERMINAL
    )
endforeach()

# BDD test runner with Gherkin support
add_custom_target(bdd-run
    COMMAND ${CMAKE_COMMAND} -E echo "Running BDD tests with Gherkin features..."
    COMMAND ${CMAKE_CTEST_COMMAND}
            --output-on-failure
            --label-regex "^bdd$"
            --verbose
    COMMENT "Run BDD tests with verbose output"
    USES_TERMINAL
)

# Generate BDD test report
add_custom_target(bdd-report
    COMMAND ${CMAKE_CTEST_COMMAND}
            --output-on-failure
            --label-regex "^bdd$"
            --output-junit ${CMAKE_CURRENT_BINARY_DIR}/bdd-results.xml
    COMMENT "Generate BDD test report"
)

# Clean BDD test outputs
add_custom_target(bdd-clean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${BDD_OUTPUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${BDD_OUTPUT_DIR}
    COMMENT "Clean BDD test outputs"
)

# Note: Individual BDD tests will be added as feature files and step definitions are created
# Example usage:
# add_bdd_test(user_login acceptance)
# add_bdd_test(compiler_workflow integration)

message(STATUS "BDD testing infrastructure configured")