# Backend Infrastructure Tests

# Simple backend test commented out - backend abstraction removed
# add_executable(test_backend_simple
#     test_backend_simple.c
#     backend_test_stubs.c
# )
# 
# # Include directories for simple test
# target_include_directories(test_backend_simple PRIVATE
#     ${CMAKE_SOURCE_DIR}/src
#     ${CMAKE_SOURCE_DIR}/src/parser
#     ${CMAKE_SOURCE_DIR}/src/analysis
#     ${CMAKE_SOURCE_DIR}/src/codegen
# )
# 
# # Define minimal stubs to avoid linking issues
# target_compile_definitions(test_backend_simple PRIVATE
#     MINIMAL_TEST_BUILD=1
# )
# 
# # Link libraries for simple test
# target_link_libraries(test_backend_simple PRIVATE
#     asthra_compiler
#     ${COMMON_LIBRARIES}
# )
# 
# # Set output directory for simple test
# set_target_properties(test_backend_simple PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
# )
# 
# # Register simple test
# add_test(NAME test_backend_simple 
#          COMMAND test_backend_simple)
# set_tests_properties(test_backend_simple PROPERTIES
#     LABELS "codegen;backend"
#     TIMEOUT 30
# )

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

# LLVM backend test commented out - backend abstraction removed
# add_executable(test_llvm_backend
#     test_llvm_backend.c
#     backend_test_stubs.c
# )
# 
# # Include directories for LLVM test
# target_include_directories(test_llvm_backend PRIVATE
#     ${CMAKE_SOURCE_DIR}/src
#     ${CMAKE_SOURCE_DIR}/src/parser
#     ${CMAKE_SOURCE_DIR}/src/analysis
#     ${CMAKE_SOURCE_DIR}/src/codegen
# )
# 
# # Define minimal stubs to avoid linking issues
# target_compile_definitions(test_llvm_backend PRIVATE
#     MINIMAL_TEST_BUILD=1
# )
# 
# # Link libraries for LLVM test
# target_link_libraries(test_llvm_backend PRIVATE
#     asthra_compiler
#     ${COMMON_LIBRARIES}
# )
# 
# # Conditionally link LLVM if enabled
# if(ENABLE_LLVM_BACKEND)
#     target_link_libraries(test_llvm_backend PRIVATE llvm::llvm)
# endif()
# 
# # Set output directory for LLVM test
# set_target_properties(test_llvm_backend PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
# )
# 
# # Register LLVM test
# add_test(NAME test_llvm_backend 
#          COMMAND test_llvm_backend)
# set_tests_properties(test_llvm_backend PROPERTIES
#     LABELS "codegen;backend;llvm"
#     TIMEOUT 30
# )

# LLVM types module test (simplified)
add_executable(test_llvm_types
    test_llvm_types_simple.c
    backend_test_stubs.c
)

# Include directories for LLVM types test
target_include_directories(test_llvm_types PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/parser
    ${CMAKE_SOURCE_DIR}/src/analysis
    ${CMAKE_SOURCE_DIR}/src/codegen
)

# Define minimal stubs to avoid linking issues
target_compile_definitions(test_llvm_types PRIVATE
    MINIMAL_TEST_BUILD=1
)

# Link libraries for LLVM types test
target_link_libraries(test_llvm_types PRIVATE
    asthra_compiler
    ${COMMON_LIBRARIES}
)

# Conditionally link LLVM if enabled
if(ENABLE_LLVM_BACKEND)
    target_link_libraries(test_llvm_types PRIVATE llvm::llvm)
endif()

# Set output directory for LLVM types test
set_target_properties(test_llvm_types PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Register LLVM types test
add_test(NAME test_llvm_types 
         COMMAND test_llvm_types)
set_tests_properties(test_llvm_types PROPERTIES
    LABELS "codegen;backend;llvm;types"
    TIMEOUT 30
)

# LLVM expression generation module test (debug version)
add_executable(test_llvm_expr_gen
    test_llvm_expr_gen_simple.c
    backend_test_stubs.c
)

# Include directories for LLVM expression gen test
target_include_directories(test_llvm_expr_gen PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/parser
    ${CMAKE_SOURCE_DIR}/src/analysis
    ${CMAKE_SOURCE_DIR}/src/codegen
)

# Define minimal stubs to avoid linking issues
target_compile_definitions(test_llvm_expr_gen PRIVATE
    MINIMAL_TEST_BUILD=1
)

# Link libraries for LLVM expression gen test
target_link_libraries(test_llvm_expr_gen PRIVATE
    asthra_compiler
    ${COMMON_LIBRARIES}
)

# Conditionally link LLVM if enabled
if(ENABLE_LLVM_BACKEND)
    target_link_libraries(test_llvm_expr_gen PRIVATE llvm::llvm)
endif()

# Set output directory for LLVM expression gen test
set_target_properties(test_llvm_expr_gen PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Register LLVM expression gen test
add_test(NAME test_llvm_expr_gen 
         COMMAND test_llvm_expr_gen)
set_tests_properties(test_llvm_expr_gen PROPERTIES
    LABELS "codegen;backend;llvm;expressions"
    TIMEOUT 30
)

# LLVM statement generation module test
add_executable(test_llvm_stmt_gen
    test_llvm_stmt_gen.c
    backend_test_stubs.c
)

# Include directories for LLVM statement gen test
target_include_directories(test_llvm_stmt_gen PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/parser
    ${CMAKE_SOURCE_DIR}/src/analysis
    ${CMAKE_SOURCE_DIR}/src/codegen
)

# Define minimal stubs to avoid linking issues
target_compile_definitions(test_llvm_stmt_gen PRIVATE
    MINIMAL_TEST_BUILD=1
)

# Link libraries for LLVM statement gen test
target_link_libraries(test_llvm_stmt_gen PRIVATE
    asthra_compiler
    ${COMMON_LIBRARIES}
)

# Conditionally link LLVM if enabled
if(ENABLE_LLVM_BACKEND)
    target_link_libraries(test_llvm_stmt_gen PRIVATE llvm::llvm)
endif()

# Set output directory for LLVM statement gen test
set_target_properties(test_llvm_stmt_gen PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Register LLVM statement gen test
add_test(NAME test_llvm_stmt_gen 
         COMMAND test_llvm_stmt_gen)
set_tests_properties(test_llvm_stmt_gen PROPERTIES
    LABELS "codegen;backend;llvm;statements"
    TIMEOUT 30
)

# LLVM cast expression module test
add_executable(test_llvm_cast_expr
    test_llvm_cast_expr.c
    backend_test_stubs.c
)

# Include directories for LLVM cast expr test
target_include_directories(test_llvm_cast_expr PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/parser
    ${CMAKE_SOURCE_DIR}/src/analysis
    ${CMAKE_SOURCE_DIR}/src/codegen
)

# Define minimal stubs to avoid linking issues
target_compile_definitions(test_llvm_cast_expr PRIVATE
    MINIMAL_TEST_BUILD=1
)

# Link libraries for LLVM cast expr test
target_link_libraries(test_llvm_cast_expr PRIVATE
    asthra_compiler
    ${COMMON_LIBRARIES}
)

# Conditionally link LLVM if enabled
if(ENABLE_LLVM_BACKEND)
    target_link_libraries(test_llvm_cast_expr PRIVATE llvm::llvm)
endif()

# Set output directory for LLVM cast expr test
set_target_properties(test_llvm_cast_expr PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Register LLVM cast expr test
add_test(NAME test_llvm_cast_expr 
         COMMAND test_llvm_cast_expr)
set_tests_properties(test_llvm_cast_expr PROPERTIES
    LABELS "codegen;backend;llvm;expressions;cast"
    TIMEOUT 30
)

# LLVM break/continue statements module test
add_executable(test_llvm_break_continue
    test_llvm_break_continue.c
    backend_test_stubs.c
)

# Include directories for LLVM break/continue test
target_include_directories(test_llvm_break_continue PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/parser
    ${CMAKE_SOURCE_DIR}/src/analysis
    ${CMAKE_SOURCE_DIR}/src/codegen
)

# Define minimal stubs to avoid linking issues
target_compile_definitions(test_llvm_break_continue PRIVATE
    MINIMAL_TEST_BUILD=1
)

# Link libraries for LLVM break/continue test
target_link_libraries(test_llvm_break_continue PRIVATE
    asthra_compiler
    ${COMMON_LIBRARIES}
)

# Conditionally link LLVM if enabled
if(ENABLE_LLVM_BACKEND)
    target_link_libraries(test_llvm_break_continue PRIVATE llvm::llvm)
endif()

# Set output directory for LLVM break/continue test
set_target_properties(test_llvm_break_continue PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Register LLVM break/continue test
add_test(NAME test_llvm_break_continue 
         COMMAND test_llvm_break_continue)
set_tests_properties(test_llvm_break_continue PROPERTIES
    LABELS "codegen;backend;llvm;statements;control_flow"
    TIMEOUT 30
)

# LLVM pattern matching (match statements) module test
add_executable(test_llvm_pattern_matching
    test_llvm_pattern_matching.c
    backend_test_stubs.c
)

# Include directories for LLVM pattern matching test
target_include_directories(test_llvm_pattern_matching PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/parser
    ${CMAKE_SOURCE_DIR}/src/analysis
    ${CMAKE_SOURCE_DIR}/src/codegen
)

# Define minimal stubs to avoid linking issues
target_compile_definitions(test_llvm_pattern_matching PRIVATE
    MINIMAL_TEST_BUILD=1
)

# Link libraries for LLVM pattern matching test
target_link_libraries(test_llvm_pattern_matching PRIVATE
    asthra_compiler
    ${COMMON_LIBRARIES}
)

# Conditionally link LLVM if enabled
if(ENABLE_LLVM_BACKEND)
    target_link_libraries(test_llvm_pattern_matching PRIVATE llvm::llvm)
endif()

# Set output directory for LLVM pattern matching test
set_target_properties(test_llvm_pattern_matching PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Register LLVM pattern matching test
add_test(NAME test_llvm_pattern_matching 
         COMMAND test_llvm_pattern_matching)
set_tests_properties(test_llvm_pattern_matching PROPERTIES
    LABELS "codegen;backend;llvm;statements;pattern_matching"
    TIMEOUT 30
)

# LLVM for loop iteration module test
add_executable(test_llvm_for_loops
    test_llvm_for_loops.c
    backend_test_stubs.c
)

# Include directories for LLVM for loops test
target_include_directories(test_llvm_for_loops PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/parser
    ${CMAKE_SOURCE_DIR}/src/analysis
    ${CMAKE_SOURCE_DIR}/src/codegen
)

# Define minimal stubs to avoid linking issues
target_compile_definitions(test_llvm_for_loops PRIVATE
    MINIMAL_TEST_BUILD=1
)

# Link libraries for LLVM for loops test
target_link_libraries(test_llvm_for_loops PRIVATE
    asthra_compiler
    ${COMMON_LIBRARIES}
)

# Conditionally link LLVM if enabled
if(ENABLE_LLVM_BACKEND)
    target_link_libraries(test_llvm_for_loops PRIVATE llvm::llvm)
endif()

# Set output directory for LLVM for loops test
set_target_properties(test_llvm_for_loops PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Register LLVM for loops test
add_test(NAME test_llvm_for_loops 
         COMMAND test_llvm_for_loops)
set_tests_properties(test_llvm_for_loops PROPERTIES
    LABELS "codegen;backend;llvm;statements;for_loops"
    TIMEOUT 30
)