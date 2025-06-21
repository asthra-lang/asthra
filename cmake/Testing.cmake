# Advanced Testing Infrastructure for Asthra
# Provides category-based testing, reporting, parallel execution, and filtering

# Include guard to prevent duplicate inclusion
if(ASTHRA_TESTING_INCLUDED)
    return()
endif()
set(ASTHRA_TESTING_INCLUDED TRUE)

include(CTest)
enable_testing()

# Test configuration options
set(ASTHRA_TEST_PARALLEL "4" CACHE STRING "Number of parallel test jobs")
set(ASTHRA_TEST_TIMEOUT "30" CACHE STRING "Default test timeout in seconds")
set(ASTHRA_TEST_RETRY "3" CACHE STRING "Number of test retries on failure")
option(ASTHRA_TEST_FAIL_FAST "Stop on first test failure" OFF)
option(ASTHRA_TEST_VERBOSE "Enable verbose test output" OFF)
set(ASTHRA_TEST_FILTER "" CACHE STRING "Test filter pattern (regex)")
set(ASTHRA_TEST_EXCLUDE "" CACHE STRING "Test exclusion pattern (regex)")

# Test output formats
set(ASTHRA_TEST_OUTPUT_FORMAT "console" CACHE STRING "Test output format")
set_property(CACHE ASTHRA_TEST_OUTPUT_FORMAT PROPERTY STRINGS console json csv junit html markdown)

# Test categories from the Makefile
set(ASTHRA_TEST_CATEGORIES
    ai_annotations
    ai_api
    ai_linter
    basic
    character
    codegen
    concurrency
    core
    diagnostics
    enum
    fast_check
    ffi
    immutable_by_default
    integration
    io
    lexer
    linker
    memory
    optimization
    parser
    patterns
    performance
    pipeline
    platform
    runtime
    security
    semantic
    stdlib
    struct
    types
    utils
)

# Create test output directory
set(ASTHRA_TEST_OUTPUT_DIR "${CMAKE_BINARY_DIR}/test-results")
file(MAKE_DIRECTORY ${ASTHRA_TEST_OUTPUT_DIR})

# Create a target to build all test executables
add_custom_target(build-tests
    COMMENT "Building all test executables"
)

# Function to add category-specific test target
function(add_category_test_target category)
    # Find all tests in this category
    get_property(category_tests GLOBAL PROPERTY ASTHRA_TEST_TARGETS_${category})
    
    if(category_tests)
        # Main category test target
        add_custom_target(test-${category}
            COMMAND ${CMAKE_CTEST_COMMAND} 
                    --output-on-failure
                    --label-regex "^${category}$"
                    --parallel ${ASTHRA_TEST_PARALLEL}
                    --timeout ${ASTHRA_TEST_TIMEOUT}
                    $<$<BOOL:${ASTHRA_TEST_VERBOSE}>:--verbose>
                    $<$<BOOL:${ASTHRA_TEST_FAIL_FAST}>:--stop-on-failure>
            COMMENT "Running ${category} tests"
            USES_TERMINAL
        )
        
        # Category test with retry
        add_custom_target(test-${category}-retry
            COMMAND ${CMAKE_CTEST_COMMAND}
                    --output-on-failure
                    --label-regex "^${category}$"
                    --repeat until-pass:${ASTHRA_TEST_RETRY}
            COMMENT "Running ${category} tests with retry"
            USES_TERMINAL
        )
    endif()
endfunction()

# Create category-specific test targets
foreach(category ${ASTHRA_TEST_CATEGORIES})
    add_category_test_target(${category})
endforeach()

# Main test targets matching Makefile functionality
add_custom_target(run-tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    COMMENT "Running basic tests"
    USES_TERMINAL
)

add_custom_target(test-all
    COMMAND ${CMAKE_CTEST_COMMAND} 
            --output-on-failure
            --parallel ${ASTHRA_TEST_PARALLEL}
            $<$<BOOL:${ASTHRA_TEST_VERBOSE}>:--verbose>
    COMMENT "Running all tests"
    USES_TERMINAL
)

add_custom_target(test-quick
    COMMAND ${CMAKE_CTEST_COMMAND}
            --output-on-failure
            --label-regex "basic|lexer|parser"
            --parallel ${ASTHRA_TEST_PARALLEL}
    COMMENT "Running quick tests"
    USES_TERMINAL
)

add_custom_target(test-comprehensive
    COMMAND ${CMAKE_CTEST_COMMAND}
            --output-on-failure
            --parallel ${ASTHRA_TEST_PARALLEL}
            --repeat until-pass:${ASTHRA_TEST_RETRY}
    COMMENT "Running comprehensive test suite"
    USES_TERMINAL
)

add_custom_target(test-verbose
    COMMAND ${CMAKE_CTEST_COMMAND}
            --output-on-failure
            --verbose
            --parallel ${ASTHRA_TEST_PARALLEL}
    COMMENT "Running tests with verbose output"
    USES_TERMINAL
)

# Test without performance tests
add_custom_target(test-all-sans-performance
    COMMAND ${CMAKE_CTEST_COMMAND}
            --output-on-failure
            --label-exclude "performance"
            --parallel ${ASTHRA_TEST_PARALLEL}
    COMMENT "Running all tests except performance"
    USES_TERMINAL
)

# Alias for compatibility
add_custom_target(test-sans-performance DEPENDS test-all-sans-performance)

# CI optimized test target
add_custom_target(test-ci
    COMMAND ${CMAKE_CTEST_COMMAND}
            --output-on-failure
            --parallel ${ASTHRA_TEST_PARALLEL}
            --label-exclude "performance|integration"
            --timeout 120
    COMMENT "Running CI-optimized test suite"
    USES_TERMINAL
)

# Test reporting infrastructure
add_custom_target(test-report-csv
    COMMAND ${CMAKE_COMMAND} -E echo "Generating CSV test report..."
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --no-compress-output -T Test
    COMMAND ${CMAKE_COMMAND} -E echo "CSV report saved to: ${ASTHRA_TEST_OUTPUT_DIR}/results.csv"
    COMMENT "Generate CSV test report"
)

add_custom_target(test-report-json
    COMMAND ${CMAKE_COMMAND} -E echo "Generating JSON test report..."
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --output-junit ${ASTHRA_TEST_OUTPUT_DIR}/results.xml
    COMMAND ${CMAKE_COMMAND} -E echo "Converting to JSON..."
    COMMAND ${CMAKE_COMMAND} -E echo "JSON report saved to: ${ASTHRA_TEST_OUTPUT_DIR}/results.json"
    COMMENT "Generate JSON test report"
)

add_custom_target(test-report-html
    COMMAND ${CMAKE_COMMAND} -E echo "Generating HTML test report..."
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -T Test
    COMMAND ${CMAKE_COMMAND} -E echo "HTML report saved to: ${ASTHRA_TEST_OUTPUT_DIR}/results.html"
    COMMENT "Generate HTML test report"
)

# Test filtering function
function(add_filtered_test_target name filter)
    add_custom_target(test-${name}
        COMMAND ${CMAKE_CTEST_COMMAND}
                --output-on-failure
                --tests-regex "${filter}"
                --parallel ${ASTHRA_TEST_PARALLEL}
        COMMENT "Running tests matching: ${filter}"
        USES_TERMINAL
    )
endfunction()

# Curated test suite (high-value tests)
add_custom_target(test-curated
    COMMAND ${CMAKE_CTEST_COMMAND}
            --output-on-failure
            --label-regex "basic|core|parser|semantic|codegen"
            --parallel ${ASTHRA_TEST_PARALLEL}
    COMMENT "Running curated test suite"
    USES_TERMINAL
)

# Real validation tests
add_custom_target(test-real-validation
    COMMAND ${CMAKE_CTEST_COMMAND}
            --output-on-failure
            --label-regex "integration|validation"
            --parallel ${ASTHRA_TEST_PARALLEL}
    COMMENT "Running real program validation tests"
    USES_TERMINAL
)

# Function to run tests with specific configuration
function(run_tests_with_config config_name)
    add_custom_target(test-${config_name}
        COMMAND ${CMAKE_COMMAND} --build . --config ${config_name}
        COMMAND ${CMAKE_CTEST_COMMAND} -C ${config_name} --output-on-failure
        COMMENT "Running tests in ${config_name} configuration"
    )
endfunction()

# Add test configurations
run_tests_with_config(Debug)
run_tests_with_config(Release)
run_tests_with_config(Profile)

# Test execution with custom environment
add_custom_target(test-with-env
    COMMAND ${CMAKE_COMMAND} -E env
            ASTHRA_TEST_PARALLEL=${ASTHRA_TEST_PARALLEL}
            ASTHRA_TEST_TIMEOUT=${ASTHRA_TEST_TIMEOUT}
            ${CMAKE_CTEST_COMMAND} --output-on-failure
    COMMENT "Running tests with custom environment"
)

# Memory leak detection (if valgrind is available)
find_program(VALGRIND_EXECUTABLE valgrind)
if(VALGRIND_EXECUTABLE)
    add_custom_target(test-memcheck
        COMMAND ${CMAKE_CTEST_COMMAND}
                --output-on-failure
                -T memcheck
                --parallel ${ASTHRA_TEST_PARALLEL}
        COMMENT "Running tests with memory leak detection"
    )
endif()

# Test discovery and listing
add_custom_target(test-list
    COMMAND ${CMAKE_CTEST_COMMAND} -N
    COMMENT "List all available tests"
)

add_custom_target(test-list-categories
    COMMAND ${CMAKE_COMMAND} -E echo "Available test categories:"
    COMMAND ${CMAKE_COMMAND} -E echo "${ASTHRA_TEST_CATEGORIES}"
    COMMENT "List test categories"
)

# Granular test execution
function(add_granular_test_target category test_name)
    add_custom_target(test-${category}-${test_name}
        COMMAND ${CMAKE_CTEST_COMMAND}
                --output-on-failure
                -R "${category}.*${test_name}"
        COMMENT "Running ${category}/${test_name} test"
        USES_TERMINAL
    )
endfunction()

# Test statistics
add_custom_target(test-stats
    COMMAND ${CMAKE_COMMAND} -E echo "Collecting test statistics..."
    COMMAND ${CMAKE_CTEST_COMMAND} -N | grep "Total Tests:" || echo "Total Tests: 0"
    COMMAND ${CMAKE_COMMAND} -E echo "Test categories: ${ASTHRA_TEST_CATEGORIES}"
    COMMENT "Display test statistics"
)

# Clean test results
add_custom_target(test-clean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${ASTHRA_TEST_OUTPUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${ASTHRA_TEST_OUTPUT_DIR}
    COMMENT "Clean test results"
)

# Function to add test with proper configuration
function(asthra_add_test target category)
    add_test(NAME ${target} COMMAND ${target})
    
    set_tests_properties(${target} PROPERTIES
        TIMEOUT ${ASTHRA_TEST_TIMEOUT}
        LABELS ${category}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        ENVIRONMENT "ASTHRA_TEST_CATEGORY=${category}"
    )
    
    # Add to category tracking
    set_property(GLOBAL APPEND PROPERTY ASTHRA_TEST_TARGETS_${category} ${target})
    
    # Add test executable as dependency to build-tests target
    if(TARGET ${target})
        add_dependencies(build-tests ${target})
    endif()
endfunction()

# Memory check for fast check
if(VALGRIND_EXECUTABLE)
    add_custom_target(memcheck-fast-check
        COMMAND ${VALGRIND_EXECUTABLE}
                --leak-check=full
                --show-leak-kinds=all
                --track-origins=yes
                --verbose
                $<TARGET_FILE:asthra>
                --fast-check
        DEPENDS asthra
        COMMENT "Memory check for fast check system"
        USES_TERMINAL
    )
endif()

# Thread safety testing
add_custom_target(test-thread-safety
    COMMAND ${CMAKE_CTEST_COMMAND}
            --output-on-failure
            --label-regex "concurrency|thread"
            --parallel 1
    COMMENT "Running thread safety tests"
    USES_TERMINAL
)

# Cache performance testing
add_custom_target(test-cache-performance
    COMMAND ${CMAKE_COMMAND} -E echo "Testing cache performance..."
    COMMAND ${CMAKE_CTEST_COMMAND}
            --output-on-failure
            --label-regex "performance|cache"
    COMMENT "Running cache performance tests"
    USES_TERMINAL
)

# Test result formats
add_custom_target(test-report-junit
    COMMAND ${CMAKE_CTEST_COMMAND}
            --output-on-failure
            --output-junit ${ASTHRA_TEST_OUTPUT_DIR}/junit-results.xml
    COMMENT "Generate JUnit test report"
)

add_custom_target(test-report-all
    DEPENDS test-report-csv test-report-json test-report-html test-report-junit
    COMMENT "Generate all test report formats"
)

# Granular test targets for specific categories
foreach(category ${ASTHRA_TEST_CATEGORIES})
    # Skip CMake-generated entries
    if(NOT category MATCHES "^(CMakeFiles|CMakeLists|CTestTestfile|Makefile|cmake_install)"
       AND NOT category MATCHES "\\.(txt|cmake)$")
        # Create granular targets for common subtests
        if(category STREQUAL "codegen")
            add_custom_target(test-codegen-function-calls
                COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R "codegen.*function.*call"
                COMMENT "Running codegen function call tests"
                USES_TERMINAL
            )
            add_custom_target(test-codegen-control-flow
                COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R "codegen.*control.*flow"
                COMMENT "Running codegen control flow tests"
                USES_TERMINAL
            )
        elseif(category STREQUAL "parser")
            add_custom_target(test-parser-expressions
                COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R "parser.*expression"
                COMMENT "Running parser expression tests"
                USES_TERMINAL
            )
            add_custom_target(test-parser-statements
                COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R "parser.*statement"
                COMMENT "Running parser statement tests"
                USES_TERMINAL
            )
        endif()
    endif()
endforeach()