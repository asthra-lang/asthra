# Asthra Test Suite
enable_testing()

# Test categories - auto-discover from directory structure
file(GLOB TEST_CATEGORY_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*")
list(FILTER TEST_CATEGORY_DIRS INCLUDE REGEX "^[^.].*")  # Exclude hidden dirs
list(FILTER TEST_CATEGORY_DIRS EXCLUDE REGEX "framework|README.md")

message(STATUS "Discovered test categories: ${TEST_CATEGORY_DIRS}")

# Test framework library
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/framework")
    file(GLOB FRAMEWORK_SOURCES "framework/*.c")
    # Exclude test runner files that contain duplicate FFI symbols
    # but ensure test_runner_core.c is included for basic test functions
    # Also exclude original test_suite_runner.c as it's now modularized
    list(REMOVE_ITEM FRAMEWORK_SOURCES 
        "${CMAKE_CURRENT_SOURCE_DIR}/framework/test_runner_minimal.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/framework/test_runner_unified.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/framework/test_suite_runner.c")
    if(FRAMEWORK_SOURCES)
        add_library(asthra_test_framework STATIC ${FRAMEWORK_SOURCES})
        
        target_include_directories(asthra_test_framework
            PUBLIC
                ${CMAKE_CURRENT_SOURCE_DIR}/framework
        )
        
        target_link_libraries(asthra_test_framework
            PUBLIC
                asthra_compiler
                asthra_runtime
        )
        
        target_compile_definitions(asthra_test_framework
            PRIVATE
                ASTHRA_TEST_FRAMEWORK_BUILD
        )
        
        # Disable pedantic warnings for test_c17_framework_demo.c due to variadic macro usage
        if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
            set_source_files_properties(
                ${CMAKE_CURRENT_SOURCE_DIR}/framework/test_c17_framework_demo.c
                PROPERTIES COMPILE_FLAGS "-Wno-pedantic"
            )
        endif()
        
        message(STATUS "Test framework library created with ${FRAMEWORK_SOURCES}")
    endif()
endif()

# Function to create test executables
function(add_asthra_test category test_file)
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_target "${category}_${test_name}")
    
    add_executable(${test_target} ${test_file})
    
    if(TARGET asthra_test_framework)
        target_link_libraries(${test_target} asthra_test_framework)
    else()
        target_link_libraries(${test_target} asthra_compiler asthra_runtime)
    endif()
    
    add_test(NAME ${test_target} COMMAND ${test_target})
    
    # Set test properties
    set_tests_properties(${test_target} PROPERTIES
        TIMEOUT 30
        LABELS ${category}
    )
    
    # Set working directory for stdlib tests to project tests directory
    if(${category} STREQUAL "stdlib")
        set_tests_properties(${test_target} PROPERTIES
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests
        )
    endif()
    
    # Add test executable as dependency to build-tests target
    if(TARGET build-tests)
        add_dependencies(build-tests ${test_target})
    endif()
    
    # Add to category list
    set_property(GLOBAL APPEND PROPERTY ASTHRA_TEST_TARGETS_${category} ${test_target})
endfunction()

# Auto-discover and create tests
set(TOTAL_TESTS 0)
foreach(category ${TEST_CATEGORY_DIRS})
    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${category}")
        # Check if category has its own CMakeLists.txt
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${category}/CMakeLists.txt")
            # Use subdirectory's CMakeLists.txt
            add_subdirectory(${category})
            message(STATUS "Category ${category}: Using custom CMakeLists.txt")
        else()
            # Auto-discover tests
            file(GLOB test_files "${category}/*.c")
            
            # Exclude tests that depend on removed pipeline orchestrator
            if(category STREQUAL "pipeline")
                list(REMOVE_ITEM test_files
                    "${CMAKE_CURRENT_SOURCE_DIR}/pipeline/test_pipeline_orchestrator.c"
                    "${CMAKE_CURRENT_SOURCE_DIR}/pipeline/test_pipeline_orchestrator_minimal.c"
                    "${CMAKE_CURRENT_SOURCE_DIR}/pipeline/test_end_to_end_compilation.c"
                )
            endif()
            
            set(category_test_count 0)
            foreach(test_file ${test_files})
                add_asthra_test(${category} ${test_file})
                math(EXPR category_test_count "${category_test_count} + 1")
                math(EXPR TOTAL_TESTS "${TOTAL_TESTS} + 1")
            endforeach()
            if(category_test_count GREATER 0)
                message(STATUS "Category ${category}: ${category_test_count} tests")
            endif()
        endif()
    endif()
endforeach()

message(STATUS "Total tests discovered: ${TOTAL_TESTS}")

# Custom test targets - removed because they're now defined in cmake/Testing.cmake
# The Testing.cmake module provides all these targets with more features

# Category-specific test targets
foreach(category ${TEST_CATEGORY_DIRS})
    add_custom_target(test-${category}
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -L ${category}
        COMMENT "Running ${category} tests"
    )
endforeach()

# Advanced test features
option(ENABLE_TEST_COVERAGE "Enable test coverage analysis" OFF)
option(ENABLE_TEST_VALGRIND "Enable Valgrind memory checking for tests" OFF)

if(ENABLE_TEST_VALGRIND)
    find_program(VALGRIND_EXECUTABLE valgrind)
    if(VALGRIND_EXECUTABLE)
        add_custom_target(test-valgrind
            COMMAND ${CMAKE_CTEST_COMMAND} 
                --output-on-failure 
                --overwrite MemoryCheckCommand=${VALGRIND_EXECUTABLE}
                --overwrite MemoryCheckCommandOptions="--leak-check=full --error-exitcode=1"
                -T memcheck
            COMMENT "Running tests with Valgrind"
        )
        message(STATUS "Valgrind testing enabled")
    else()
        message(WARNING "Valgrind not found, test-valgrind target will not work")
    endif()
endif()

# Test result reporting
add_custom_target(test-report
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -T Test
    COMMENT "Generating test report"
) 