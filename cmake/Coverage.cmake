# Comprehensive Code Coverage Infrastructure for Asthra
# Provides module-specific, category-specific, and detailed coverage analysis

# Include guard to prevent duplicate inclusion
if(ASTHRA_COVERAGE_INCLUDED)
    return()
endif()
set(ASTHRA_COVERAGE_INCLUDED TRUE)

option(ENABLE_COVERAGE "Enable code coverage analysis" OFF)
set(COVERAGE_THRESHOLD "80" CACHE STRING "Minimum coverage threshold percentage")
set(COVERAGE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/coverage" CACHE PATH "Coverage output directory")

# Create coverage output directory
file(MAKE_DIRECTORY ${COVERAGE_OUTPUT_DIR})

if(ENABLE_COVERAGE)
    message(STATUS "Enabling code coverage analysis")
    message(STATUS "Coverage threshold: ${COVERAGE_THRESHOLD}%")
    
    # Detect coverage tools
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
        # Common coverage flags
        set(COVERAGE_COMPILE_FLAGS "")
        set(COVERAGE_LINK_FLAGS "")
        
        if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
            set(COVERAGE_COMPILE_FLAGS --coverage -fprofile-arcs -ftest-coverage)
            set(COVERAGE_LINK_FLAGS --coverage)
            
            find_program(GCOV_EXECUTABLE gcov)
            find_program(LCOV_EXECUTABLE lcov)
            find_program(GENHTML_EXECUTABLE genhtml)
            
            if(NOT GCOV_EXECUTABLE)
                message(WARNING "gcov not found, coverage reports will not be available")
            endif()
        elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang")
            set(COVERAGE_COMPILE_FLAGS -fprofile-instr-generate -fcoverage-mapping)
            set(COVERAGE_LINK_FLAGS -fprofile-instr-generate -fcoverage-mapping)
            
            # Try to find LLVM tools with version suffixes first
            find_program(LLVM_PROFDATA_EXECUTABLE 
                NAMES llvm-profdata-20 llvm-profdata-19 llvm-profdata-18 llvm-profdata
                HINTS ${LLVM_TOOLS_BINARY_DIR} /usr/bin /usr/local/bin
            )
            find_program(LLVM_COV_EXECUTABLE 
                NAMES llvm-cov-20 llvm-cov-19 llvm-cov-18 llvm-cov
                HINTS ${LLVM_TOOLS_BINARY_DIR} /usr/bin /usr/local/bin
            )
            
            if(NOT LLVM_COV_EXECUTABLE)
                message(WARNING "llvm-cov not found, coverage reports will not be available")
            else()
                message(STATUS "Found llvm-cov: ${LLVM_COV_EXECUTABLE}")
                message(STATUS "Found llvm-profdata: ${LLVM_PROFDATA_EXECUTABLE}")
            endif()
        endif()
        
        # Apply coverage flags globally when enabled
        add_compile_options(${COVERAGE_COMPILE_FLAGS})
        add_link_options(${COVERAGE_LINK_FLAGS})
        
        # Function to create coverage target for a specific module
        function(add_coverage_target_for_module module)
            set(module_coverage_dir "${COVERAGE_OUTPUT_DIR}/${module}")
            file(MAKE_DIRECTORY ${module_coverage_dir})
            
            if(CMAKE_C_COMPILER_ID STREQUAL "GNU" AND LCOV_EXECUTABLE AND GENHTML_EXECUTABLE)
                add_custom_target(coverage-${module}
                    # Clean previous coverage data
                    COMMAND ${LCOV_EXECUTABLE} --directory . --zerocounters
                    
                    # Run tests for this module
                    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -L ${module}
                    
                    # Capture coverage data
                    COMMAND ${LCOV_EXECUTABLE} --directory . --capture --output-file ${module}_coverage.info
                    
                    # Filter to only include module files
                    COMMAND ${LCOV_EXECUTABLE} --extract ${module}_coverage.info 
                            "${CMAKE_SOURCE_DIR}/src/${module}/*" 
                            --output-file ${module}_coverage_filtered.info
                    
                    # Generate HTML report
                    COMMAND ${GENHTML_EXECUTABLE} ${module}_coverage_filtered.info 
                            --output-directory ${module_coverage_dir}/html
                            --title "Asthra ${module} Coverage Report"
                            --legend
                            --show-details
                    
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    COMMENT "Generating coverage report for ${module}"
                )
            elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang" AND LLVM_PROFDATA_EXECUTABLE AND LLVM_COV_EXECUTABLE)
                add_custom_target(coverage-${module}
                    # Clean previous profile data
                    COMMAND ${CMAKE_COMMAND} -E remove -f ${module}.profraw ${module}.profdata
                    
                    # Run tests for this module
                    COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=${module}.profraw
                            ${CMAKE_CTEST_COMMAND} --output-on-failure -L ${module}
                    
                    # Merge profile data
                    COMMAND ${LLVM_PROFDATA_EXECUTABLE} merge -sparse ${module}.profraw -o ${module}.profdata
                    
                    # Generate HTML coverage report
                    COMMAND ${LLVM_COV_EXECUTABLE} show $<TARGET_FILE:asthra>
                            -instr-profile=${module}.profdata
                            -format=html
                            -output-dir=${module_coverage_dir}/html
                            -show-line-counts-or-regions
                            -show-expansions
                            -show-instantiations
                            -Xdemangler=c++filt
                            -path-equivalence="${CMAKE_SOURCE_DIR},."
                            ${CMAKE_SOURCE_DIR}/src/${module}
                    
                    # Generate text summary
                    COMMAND ${LLVM_COV_EXECUTABLE} report $<TARGET_FILE:asthra>
                            -instr-profile=${module}.profdata
                            ${CMAKE_SOURCE_DIR}/src/${module}
                            > ${module_coverage_dir}/summary.txt
                    
                    # Generate JSON export for further processing
                    COMMAND ${LLVM_COV_EXECUTABLE} export $<TARGET_FILE:asthra>
                            -instr-profile=${module}.profdata
                            -format=text
                            ${CMAKE_SOURCE_DIR}/src/${module}
                            > ${module_coverage_dir}/coverage.json
                    
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    COMMENT "Generating coverage report for ${module}"
                )
            endif()
        endfunction()
        
        # Create module-specific coverage targets
        set(ASTHRA_MODULES parser codegen analysis runtime fast_check ai_api ai_linter)
        foreach(module ${ASTHRA_MODULES})
            add_coverage_target_for_module(${module})
        endforeach()
        
        # Category-specific coverage targets
        function(add_coverage_target_for_category category)
            set(category_coverage_dir "${COVERAGE_OUTPUT_DIR}/test-${category}")
            file(MAKE_DIRECTORY ${category_coverage_dir})
            
            if(CMAKE_C_COMPILER_ID STREQUAL "GNU" AND LCOV_EXECUTABLE AND GENHTML_EXECUTABLE)
                add_custom_target(coverage-test-${category}
                    COMMAND ${LCOV_EXECUTABLE} --directory . --zerocounters
                    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -L ${category}
                    COMMAND ${LCOV_EXECUTABLE} --directory . --capture --output-file ${category}_test_coverage.info
                    COMMAND ${LCOV_EXECUTABLE} --remove ${category}_test_coverage.info '/usr/*' '*/tests/*' 
                            --output-file ${category}_test_coverage_filtered.info
                    COMMAND ${GENHTML_EXECUTABLE} ${category}_test_coverage_filtered.info 
                            --output-directory ${category_coverage_dir}/html
                            --title "Asthra ${category} Test Coverage"
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    COMMENT "Generating coverage for ${category} tests"
                )
            endif()
        endfunction()
        
        # Create category coverage targets
        set(COVERAGE_TEST_CATEGORIES ffi concurrency parser codegen semantic)
        foreach(category ${COVERAGE_TEST_CATEGORIES})
            add_coverage_target_for_category(${category})
        endforeach()
        
        # Main coverage targets
        if(CMAKE_C_COMPILER_ID STREQUAL "GNU" AND LCOV_EXECUTABLE AND GENHTML_EXECUTABLE)
            # Full coverage report
            add_custom_target(coverage
                COMMAND ${LCOV_EXECUTABLE} --directory . --zerocounters
                COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --parallel 4
                COMMAND ${LCOV_EXECUTABLE} --directory . --capture --output-file coverage.info
                COMMAND ${LCOV_EXECUTABLE} --remove coverage.info '/usr/*' '*/tests/*' '*/third-party/*' 
                        --output-file coverage_filtered.info
                COMMAND ${GENHTML_EXECUTABLE} coverage_filtered.info 
                        --output-directory ${COVERAGE_OUTPUT_DIR}/html
                        --title "Asthra Complete Coverage Report"
                        --legend
                        --show-details
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Generating complete code coverage report"
            )
            
            # Coverage summary
            add_custom_target(coverage-summary
                COMMAND ${LCOV_EXECUTABLE} --summary coverage_filtered.info
                DEPENDS coverage
                COMMENT "Display coverage summary"
            )
            
            # Coverage with HTML open
            add_custom_target(coverage-open
                COMMAND ${CMAKE_COMMAND} -E echo "Opening coverage report..."
                COMMAND ${CMAKE_COMMAND} -E echo "file://${COVERAGE_OUTPUT_DIR}/html/index.html"
                DEPENDS coverage
                COMMENT "Open coverage report in browser"
            )
            
        elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang" AND LLVM_PROFDATA_EXECUTABLE AND LLVM_COV_EXECUTABLE)
            # Full coverage report for Clang/LLVM
            add_custom_target(coverage
                COMMAND ${CMAKE_COMMAND} -E remove -f asthra_full.profraw asthra_full.profdata
                COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=asthra_full.profraw
                        ${CMAKE_CTEST_COMMAND} --output-on-failure --parallel 4
                COMMAND ${LLVM_PROFDATA_EXECUTABLE} merge -sparse asthra_full.profraw -o asthra_full.profdata
                
                # Generate HTML report with full details
                COMMAND ${LLVM_COV_EXECUTABLE} show $<TARGET_FILE:asthra>
                        -instr-profile=asthra_full.profdata
                        -format=html
                        -output-dir=${COVERAGE_OUTPUT_DIR}/html
                        -show-line-counts-or-regions
                        -show-expansions
                        -show-instantiations
                        -Xdemangler=c++filt
                        -path-equivalence="${CMAKE_SOURCE_DIR},."
                        "-ignore-filename-regex=(tests/|third-party/|build/)"
                
                # Generate text report summary
                COMMAND ${LLVM_COV_EXECUTABLE} report $<TARGET_FILE:asthra>
                        -instr-profile=asthra_full.profdata
                        "-ignore-filename-regex=(tests/|third-party/|build/)"
                        -show-region-summary=false
                        > ${COVERAGE_OUTPUT_DIR}/summary.txt
                
                # Generate JSON export for CI/CD integration
                COMMAND ${LLVM_COV_EXECUTABLE} export $<TARGET_FILE:asthra>
                        -instr-profile=asthra_full.profdata
                        -format=lcov
                        "-ignore-filename-regex=(tests/|third-party/|build/)"
                        > ${COVERAGE_OUTPUT_DIR}/coverage.lcov
                
                # Display summary
                COMMAND ${CMAKE_COMMAND} -E cat ${COVERAGE_OUTPUT_DIR}/summary.txt
                
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Generating complete code coverage report with llvm-cov"
            )
            
            # Coverage summary
            add_custom_target(coverage-summary
                COMMAND ${LLVM_COV_EXECUTABLE} report $<TARGET_FILE:asthra>
                        -instr-profile=asthra_full.profdata
                        "-ignore-filename-regex=(tests/|third-party/|build/)"
                        -show-functions=true
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Display detailed coverage summary"
            )
            
            # Coverage threshold check with llvm-cov
            add_custom_target(coverage-check-llvm
                COMMAND ${CMAKE_COMMAND} -E echo "Checking coverage against threshold: ${COVERAGE_THRESHOLD}%"
                COMMAND ${CMAKE_COMMAND} -D LLVM_COV_EXECUTABLE=${LLVM_COV_EXECUTABLE}
                        -D PROFDATA_FILE=asthra_full.profdata
                        -D THRESHOLD=${COVERAGE_THRESHOLD}
                        -D TARGET_FILE=$<TARGET_FILE:asthra>
                        -P ${CMAKE_SOURCE_DIR}/cmake/CheckCoverageThreshold.cmake
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                DEPENDS coverage
                COMMENT "Check coverage meets threshold of ${COVERAGE_THRESHOLD}%"
            )
            
            # Export coverage for different formats
            add_custom_target(coverage-export-lcov
                COMMAND ${LLVM_COV_EXECUTABLE} export $<TARGET_FILE:asthra>
                        -instr-profile=asthra_full.profdata
                        -format=lcov
                        "-ignore-filename-regex=(tests/|third-party/|build/)"
                        > ${COVERAGE_OUTPUT_DIR}/coverage.lcov
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                DEPENDS coverage
                COMMENT "Export coverage in LCOV format"
            )
            
            add_custom_target(coverage-export-json
                COMMAND ${LLVM_COV_EXECUTABLE} export $<TARGET_FILE:asthra>
                        -instr-profile=asthra_full.profdata
                        -format=text
                        "-ignore-filename-regex=(tests/|third-party/|build/)"
                        > ${COVERAGE_OUTPUT_DIR}/coverage.json
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                DEPENDS coverage
                COMMENT "Export coverage in JSON format"
            )
        endif()
        
        # Coverage cleaning
        add_custom_target(coverage-clean
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${COVERAGE_OUTPUT_DIR}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_OUTPUT_DIR}
            COMMAND ${CMAKE_COMMAND} -E remove -f 
                    ${CMAKE_BINARY_DIR}/*.gcda 
                    ${CMAKE_BINARY_DIR}/*.gcno
                    ${CMAKE_BINARY_DIR}/*.profraw
                    ${CMAKE_BINARY_DIR}/*.profdata
                    ${CMAKE_BINARY_DIR}/*.info
            COMMENT "Clean coverage data"
        )
        
        # Coverage threshold check
        add_custom_target(coverage-check
            COMMAND ${CMAKE_COMMAND} -E echo "Checking coverage against threshold: ${COVERAGE_THRESHOLD}%"
            DEPENDS coverage-summary
            COMMENT "Check coverage threshold"
        )
        
        # Quick coverage targets
        add_custom_target(coverage-parser-basic
            DEPENDS coverage-parser coverage-test-parser
            COMMENT "Parser coverage (basic)"
        )
        
        add_custom_target(coverage-codegen-basic
            DEPENDS coverage-codegen coverage-test-codegen
            COMMENT "Code generation coverage (basic)"
        )
        
        add_custom_target(coverage-analysis-basic
            DEPENDS coverage-analysis
            COMMENT "Semantic analysis coverage (basic)"
        )
        
        # HTML coverage variants
        add_custom_target(coverage-html-gcov
            DEPENDS coverage
            COMMENT "HTML coverage report (gcov)"
        )
        
        add_custom_target(coverage-html-llvm
            DEPENDS coverage
            COMMENT "HTML coverage report (llvm)"
        )
        
    else()
        message(WARNING "Coverage is only supported with Clang compiler")
    endif()
else()
    # Create dummy targets when coverage is disabled
    add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND} -E echo "Coverage is not enabled. Use -DENABLE_COVERAGE=ON"
        COMMENT "Coverage not enabled"
    )
    
    add_custom_target(coverage-clean
        COMMAND ${CMAKE_COMMAND} -E echo "Coverage is not enabled"
        COMMENT "Coverage not enabled"
    )
endif()

# Coverage help
add_custom_target(coverage-info
    COMMAND ${CMAKE_COMMAND} -E echo "Coverage Targets:"
    COMMAND ${CMAKE_COMMAND} -E echo "  coverage              - Full coverage report"
    COMMAND ${CMAKE_COMMAND} -E echo "  coverage-summary      - Coverage summary"
    COMMAND ${CMAKE_COMMAND} -E echo "  coverage-check        - Check against threshold"
    COMMAND ${CMAKE_COMMAND} -E echo "  coverage-clean        - Clean coverage data"
    COMMAND ${CMAKE_COMMAND} -E echo "  coverage-open         - Open HTML report"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Module Coverage:"
    COMMAND ${CMAKE_COMMAND} -E echo "  coverage-parser       - Parser module coverage"
    COMMAND ${CMAKE_COMMAND} -E echo "  coverage-codegen      - Code generation coverage"
    COMMAND ${CMAKE_COMMAND} -E echo "  coverage-analysis     - Analysis coverage"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Test Category Coverage:"
    COMMAND ${CMAKE_COMMAND} -E echo "  coverage-test-ffi     - FFI test coverage"
    COMMAND ${CMAKE_COMMAND} -E echo "  coverage-test-parser  - Parser test coverage"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Options:"
    COMMAND ${CMAKE_COMMAND} -E echo "  -DENABLE_COVERAGE=ON  - Enable coverage"
    COMMAND ${CMAKE_COMMAND} -E echo "  -DCOVERAGE_THRESHOLD=80 - Set threshold"
    COMMENT "Coverage information"
)