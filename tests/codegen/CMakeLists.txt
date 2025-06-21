# Codegen Tests CMakeLists.txt - Hierarchical Organization
# This file organizes codegen tests into logical categories
# Each subdirectory contains related tests with proper labeling

# Phase 1: Core test categories
add_subdirectory(basic)
add_subdirectory(function_calls)
add_subdirectory(variants)
add_subdirectory(statements)

# Phase 2: Analysis and optimization categories
add_subdirectory(enum_variants)
add_subdirectory(control_flow)
add_subdirectory(optimization)
add_subdirectory(memory_safety)
add_subdirectory(data_flow)

# Phase 3: Pattern matching and code generation
add_subdirectory(patterns)
add_subdirectory(pattern_engine)
add_subdirectory(expressions)
add_subdirectory(functions)
add_subdirectory(elf_writer)

# Phase 4: Advanced categories
add_subdirectory(advanced_optimization)
add_subdirectory(complex_structs)
add_subdirectory(register_allocation)
add_subdirectory(symbol_export)
add_subdirectory(cross_platform)
add_subdirectory(async_await)

# Phase 5: Additional categories
add_subdirectory(struct_operations)
add_subdirectory(integration)