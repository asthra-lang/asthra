# CheckCoverageThreshold.cmake
# Script to check if coverage meets the required threshold using llvm-cov
# 
# Required variables:
# - LLVM_COV_EXECUTABLE: Path to llvm-cov
# - PROFDATA_FILE: Path to the profdata file
# - THRESHOLD: Coverage threshold percentage
# - TARGET_FILE: Path to the target executable

if(NOT LLVM_COV_EXECUTABLE)
    message(FATAL_ERROR "LLVM_COV_EXECUTABLE not set")
endif()

if(NOT PROFDATA_FILE)
    message(FATAL_ERROR "PROFDATA_FILE not set")
endif()

if(NOT THRESHOLD)
    message(FATAL_ERROR "THRESHOLD not set")
endif()

if(NOT TARGET_FILE)
    message(FATAL_ERROR "TARGET_FILE not set")
endif()

# Run llvm-cov report and capture output
execute_process(
    COMMAND ${LLVM_COV_EXECUTABLE} report ${TARGET_FILE}
            -instr-profile=${PROFDATA_FILE}
            "-ignore-filename-regex=(tests/|third-party/|build/)"
    OUTPUT_VARIABLE COV_OUTPUT
    ERROR_VARIABLE COV_ERROR
    RESULT_VARIABLE COV_RESULT
)

if(NOT COV_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to run llvm-cov: ${COV_ERROR}")
endif()

# Parse the total coverage from the output
# llvm-cov report output has a line like:
# TOTAL      1234      567    89.12%     ...
string(REGEX MATCH "TOTAL[[:space:]]+[0-9]+[[:space:]]+[0-9]+[[:space:]]+([0-9]+\\.[0-9]+)%" COVERAGE_MATCH "${COV_OUTPUT}")

if(CMAKE_MATCH_1)
    set(COVERAGE_PERCENT ${CMAKE_MATCH_1})
    message(STATUS "Total coverage: ${COVERAGE_PERCENT}%")
    
    # Compare with threshold
    if(COVERAGE_PERCENT LESS THRESHOLD)
        message(FATAL_ERROR "Coverage ${COVERAGE_PERCENT}% is below threshold ${THRESHOLD}%")
    else()
        message(STATUS "Coverage ${COVERAGE_PERCENT}% meets threshold ${THRESHOLD}%")
    endif()
else()
    message(WARNING "Could not parse coverage percentage from llvm-cov output")
    message(STATUS "Output was: ${COV_OUTPUT}")
endif()