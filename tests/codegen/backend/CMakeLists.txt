# Backend Infrastructure Tests

# Simple backend test with minimal dependencies
add_executable(test_backend_simple
    test_backend_simple.c
    backend_test_stubs.c
    # Backend implementation files
    ${CMAKE_SOURCE_DIR}/src/codegen/backend_factory.c
    ${CMAKE_SOURCE_DIR}/src/codegen/c_backend.c
    ${CMAKE_SOURCE_DIR}/src/codegen/asm_backend.c
    ${CMAKE_SOURCE_DIR}/src/codegen/llvm_backend.c
    # Minimal compiler core files
    ${CMAKE_SOURCE_DIR}/src/compiler/options_validation.c
    ${CMAKE_SOURCE_DIR}/src/compiler/compiler_core.c
)

# Include directories for simple test
target_include_directories(test_backend_simple PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/parser
    ${CMAKE_SOURCE_DIR}/src/analysis
    ${CMAKE_SOURCE_DIR}/src/codegen
)

# Define minimal stubs to avoid linking issues
target_compile_definitions(test_backend_simple PRIVATE
    MINIMAL_TEST_BUILD=1
)

# Link libraries for simple test
target_link_libraries(test_backend_simple PRIVATE
    ${COMMON_LIBRARIES}
)

# Set output directory for simple test
set_target_properties(test_backend_simple PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Register simple test
add_test(NAME test_backend_simple 
         COMMAND test_backend_simple)
set_tests_properties(test_backend_simple PROPERTIES
    LABELS "codegen;backend"
    TIMEOUT 30
)

# Full test executables (commented out for now due to dependency issues)
# Keeping the full infrastructure test for future reference when all dependencies are resolved
# add_executable(test_backend_infrastructure
#     test_backend_infrastructure.c
#     ${CMAKE_SOURCE_DIR}/src/codegen/backend_factory.c
#     ${CMAKE_SOURCE_DIR}/src/codegen/c_backend.c
#     ${CMAKE_SOURCE_DIR}/src/codegen/asm_backend.c
#     ${CMAKE_SOURCE_DIR}/src/codegen/llvm_backend.c
#     ${CMAKE_SOURCE_DIR}/src/compiler/code_generation.c
#     ${CMAKE_SOURCE_DIR}/src/compiler/options_validation.c
#     ${CMAKE_SOURCE_DIR}/src/compiler/argument_list.c
#     ${CMAKE_SOURCE_DIR}/src/compiler/compiler_core.c
#     ${CMAKE_SOURCE_DIR}/src/compiler/error_handling.c
#     ${CMAKE_SOURCE_DIR}/src/codegen/code_generator.c
#     ${CMAKE_SOURCE_DIR}/src/codegen/code_generator_core.c
#     ${CMAKE_SOURCE_DIR}/src/codegen/code_generator_lifecycle.c
#     ${CMAKE_SOURCE_DIR}/src/codegen/codegen_assembly_output.c
#     ${CMAKE_SOURCE_DIR}/tests/codegen/codegen_test_stubs.c
#     ${CMAKE_SOURCE_DIR}/tests/framework/test_framework.c
#     ${CMAKE_SOURCE_DIR}/tests/framework/test_assertions.c
#     ${CMAKE_SOURCE_DIR}/tests/framework/test_context.c
#     ${CMAKE_SOURCE_DIR}/tests/framework/test_suite.c
#     ${CMAKE_SOURCE_DIR}/tests/framework/test_statistics.c
#     ${CMAKE_SOURCE_DIR}/tests/framework/test_formatters.c
# )
# 
# target_include_directories(test_backend_infrastructure PRIVATE
#     ${CMAKE_SOURCE_DIR}/src
#     ${CMAKE_SOURCE_DIR}/src/parser
#     ${CMAKE_SOURCE_DIR}/src/analysis
#     ${CMAKE_SOURCE_DIR}/src/codegen
#     ${CMAKE_SOURCE_DIR}/tests
# )
# 
# target_link_libraries(test_backend_infrastructure PRIVATE
#     ${COMMON_LIBRARIES}
# )
# 
# set_target_properties(test_backend_infrastructure PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
# )
# 
# add_test(NAME test_backend_infrastructure 
#          COMMAND test_backend_infrastructure)
# set_tests_properties(test_backend_infrastructure PROPERTIES
#     LABELS "codegen;backend"
#     TIMEOUT 30
# )