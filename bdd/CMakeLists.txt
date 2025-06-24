# BDD Testing Infrastructure for Asthra
# Provides Behavior-Driven Development testing framework

message(STATUS "Configuring BDD testing infrastructure")

# Check for BDD testing frameworks
find_program(CUCUMBER cucumber)
find_library(CGREEN_LIB cgreen)

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
    # Check if step files are in category subdirectory or root steps directory
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/steps/${category}/${name}_steps.c)
        file(GLOB step_sources 
            ${CMAKE_CURRENT_SOURCE_DIR}/steps/${category}/${name}_steps.c
            ${CMAKE_CURRENT_SOURCE_DIR}/steps/${category}/common_steps.c
        )
    else()
        # Look in root steps directory
        file(GLOB step_sources 
            ${CMAKE_CURRENT_SOURCE_DIR}/steps/${name}_steps.c
        )
    endif()
    
    # Always include the main common_steps.c if it exists
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/steps/common_steps.c)
        list(APPEND step_sources ${CMAKE_CURRENT_SOURCE_DIR}/steps/common_steps.c)
    endif()
    
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
        asthra_test_framework
    )
    
    # Platform-specific configurations
    if(APPLE)
        target_link_libraries(${test_target} "-framework Foundation")
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(${test_target} pthread m)
    endif()
    
    # Set output directory
    set_target_properties(${test_target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${BDD_OUTPUT_DIR}
    )
    
    # Add to CTest
    add_test(NAME ${test_target} 
        COMMAND ${test_target}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    
    # Set test properties with appropriate labels
    if(category STREQUAL "acceptance" OR category STREQUAL "scenario" OR category STREQUAL "feature")
        set(test_labels "bdd;${category};bdd_cucumber")
    elseif(category STREQUAL "integration")
        set(test_labels "bdd;${category};bdd_integration")
    else()
        set(test_labels "bdd;${category};bdd_unit")
    endif()
    
    set_tests_properties(${test_target} PROPERTIES
        LABELS "${test_labels}"
        TIMEOUT 60
        ENVIRONMENT "BDD_FEATURES_DIR=${CMAKE_CURRENT_SOURCE_DIR}/features"
    )
    
    # Add to build targets
    add_dependencies(build-tests ${test_target})
    add_dependencies(bdd_tests ${test_target})
endfunction()

# Create BDD test targets
add_custom_target(bdd-tests
    COMMAND ${CMAKE_CTEST_COMMAND} 
            --output-on-failure
            --label-regex "^bdd$"
    COMMENT "Running all BDD tests"
    USES_TERMINAL
)

# Target: bdd_tests - Build all BDD test executables
add_custom_target(bdd_tests
    COMMENT "Building all BDD test executables"
)

# Target: run_bdd_tests - Execute full BDD test suite
add_custom_target(run_bdd_tests
    COMMAND ${CMAKE_CTEST_COMMAND}
            --test-dir ${CMAKE_BINARY_DIR}
            --output-on-failure
            --label-regex "bdd"
            --parallel ${CMAKE_BUILD_PARALLEL_LEVEL}
    COMMENT "Executing full BDD test suite"
    USES_TERMINAL
    DEPENDS bdd_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Target: bdd_cucumber - Run Cucumber features
if(CUCUMBER)
    add_custom_target(bdd_cucumber
        COMMAND ${CMAKE_COMMAND} -E echo "Running Cucumber BDD features..."
        COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR}
                ${CUCUMBER} features/ --format progress --tags "not @wip"
        COMMENT "Running Cucumber BDD features"
        USES_TERMINAL
    )
else()
    add_custom_target(bdd_cucumber
        COMMAND ${CMAKE_COMMAND} -E echo "Cucumber not found. Running C-based BDD tests with Cucumber labels..."
        COMMAND ${CMAKE_CTEST_COMMAND}
                --output-on-failure
                --label-regex "bdd_cucumber"
                --parallel ${CMAKE_BUILD_PARALLEL_LEVEL}
        COMMENT "Running C-based BDD tests (Cucumber not available)"
        USES_TERMINAL
    )
endif()

# Target: bdd_unit - Run C BDD unit tests
add_custom_target(bdd_unit
    COMMAND ${CMAKE_CTEST_COMMAND}
            --output-on-failure
            --label-regex "bdd.*unit"
            --parallel ${CMAKE_BUILD_PARALLEL_LEVEL}
    COMMENT "Running C BDD unit tests"
    USES_TERMINAL
)

# Target: bdd_integration - Run integration tests
add_custom_target(bdd_integration
    COMMAND ${CMAKE_CTEST_COMMAND}
            --output-on-failure
            --label-regex "bdd.*integration"
            --parallel ${CMAKE_BUILD_PARALLEL_LEVEL}
    COMMENT "Running BDD integration tests"
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

# Register BDD tests
add_bdd_test(compilation unit)
add_bdd_test(parser unit)
add_bdd_test(semantic unit)
add_bdd_test(cli integration)
add_bdd_test(compiler_basic unit)
add_bdd_test(if_conditions unit)
add_bdd_test(function_calls unit)
add_bdd_test(bitwise_operators unit)
add_bdd_test(boolean_operators unit)
add_bdd_test(ffi_integration integration)

# Configuration summary
message(STATUS "BDD testing infrastructure configured")
message(STATUS "  BDD output directory: ${BDD_OUTPUT_DIR}")
message(STATUS "  Cucumber found: ${CUCUMBER}")
message(STATUS "  Cgreen library: ${CGREEN_LIB}")
message(STATUS "  Test categories: ${BDD_TEST_CATEGORIES}")
message(STATUS "")
message(STATUS "BDD test targets available:")
message(STATUS "  bdd_tests         - Build all BDD test executables")
message(STATUS "  run_bdd_tests     - Execute full BDD test suite")
message(STATUS "  bdd_cucumber      - Run Cucumber features (or C-based equivalents)")
message(STATUS "  bdd_unit          - Run C BDD unit tests")
message(STATUS "  bdd_integration   - Run BDD integration tests")
message(STATUS "  bdd-report        - Generate BDD test report")
message(STATUS "  bdd-clean         - Clean BDD test outputs")