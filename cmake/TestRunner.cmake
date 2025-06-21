# Interim test runner for CMake migration
# This provides a simple way to run tests while we fix CTest integration

add_custom_target(run-working-tests
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run-built-tests.sh
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running successfully built tests"
)

add_custom_target(test-summary
    COMMAND echo "=== Test Build Summary ==="
    COMMAND echo "Total test categories: 26"
    COMMAND echo "Total tests defined: 544"
    COMMAND echo "Tests built: $(ls ${CMAKE_BINARY_DIR}/bin/*test* 2>/dev/null | wc -l)"
    COMMAND echo "Run 'make run-working-tests' to execute built tests"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Test status summary"
)