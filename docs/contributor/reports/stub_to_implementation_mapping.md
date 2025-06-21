# Stub Functions to Real Implementation Mapping

This document maps stub functions from `tests/basic/comprehensive_stubs.c` to their real implementations in the `src/` directory.

## Component Organization

### 1. AST Functions

| Stub Function | Real Implementation | File Location |
|---------------|-------------------|---------------|
| `ast_free_node()` | `ast_free_node()` | `src/parser/ast_node_destruction.c` |
| `create_identifier()` | `create_identifier()` | `src/parser/parser_ast_helpers.c` |
| `current_location()` | - | (Inline in lexer/parser) |

### 2. Parser Functions (parse_*)

| Stub Function | Real Implementation | File Location |
|---------------|-------------------|---------------|
| `parse_annotation_list()` | `parse_annotation_list()` | `src/parser/grammar_annotations.c` |
| `parse_const_decl()` | `parse_const_decl()` | `src/parser/grammar_toplevel_const.c` |
| `parse_enum_decl()` | `parse_enum_decl()` | `src/parser/grammar_toplevel_enum.c` |
| `parse_extern_decl()` | `parse_extern_decl()` | `src/parser/grammar_toplevel_extern.c` |
| `parse_for_stmt()` | `parse_for_stmt()` | `src/parser/grammar_statements_control.c` |
| `parse_function_decl()` | `parse_function_decl()` | `src/parser/grammar_toplevel_function.c` |
| `parse_if_stmt()` | `parse_if_stmt()` | `src/parser/grammar_statements_control.c` |
| `parse_impl_block()` | `parse_impl_block()` | `src/parser/grammar_toplevel_struct.c` |
| `parse_match_stmt()` | `parse_match_stmt()` | `src/parser/grammar_statements_control.c` |
| `parse_method_decl()` | `parse_method_decl()` | `src/parser/grammar_toplevel_method.c` |
| `parse_safe_ffi_annotation()` | - | (Part of annotation parsing) |
| `parse_spawn_stmt()` | `parse_spawn_stmt()` | `src/parser/grammar_statements_concurrency.c` |
| `parse_struct_decl()` | `parse_struct_decl()` | `src/parser/grammar_toplevel_struct.c` |
| `parse_type()` | `parse_type()` | `src/parser/parser_types.c` |
| `parse_unary()` | `parse_unary()` | `src/parser/grammar_expressions_unary.c` |

### 3. Semantic Analysis Functions (analyze_*)

| Stub Function | Real Implementation | File Location |
|---------------|-------------------|---------------|
| `analyze_annotation_inheritance()` | `analyze_annotation_inheritance()` | `src/analysis/semantic_annotations_inheritance.c` |
| `analyze_const_declaration()` | `analyze_const_declaration()` | `src/analysis/semantic_const_declarations.c` |
| `analyze_declaration_annotations()` | - | (Part of semantic annotations) |
| `analyze_enum_declaration()` | `analyze_enum_declaration()` | `src/analysis/semantic_enums.c` |
| `analyze_expression_annotations()` | - | (Part of semantic annotations) |
| `analyze_impl_block()` | `analyze_impl_block()` | `src/analysis/semantic_methods.c` |
| `analyze_import_declaration()` | `analyze_import_declaration()` | `src/analysis/semantic_imports.c` |
| `analyze_statement_annotations()` | - | (Part of semantic annotations) |
| `analyze_struct_declaration()` | `analyze_struct_declaration()` | `src/analysis/semantic_structs_declaration.c` |
| `analyze_struct_literal_expression()` | `analyze_struct_literal_expression()` | `src/analysis/semantic_structs_literals.c` |
| `analyze_tier1_concurrency_feature()` | - | `src/analysis/semantic_annotations_concurrency.c` |
| `analyze_visibility_modifier()` | `analyze_visibility_modifier()` | `src/analysis/semantic_structs_visibility.c` |

### 4. Semantic Core Functions (semantic_*)

| Stub Function | Real Implementation | File Location |
|---------------|-------------------|---------------|
| `semantic_check_type_compatibility()` | `semantic_check_type_compatibility()` | `src/analysis/semantic_type_helpers.c` |
| `semantic_clear_errors()` | `semantic_clear_errors()` | `src/analysis/semantic_errors.c` |
| `semantic_declare_symbol()` | `semantic_declare_symbol()` | `src/analysis/semantic_symbols.c` |
| `semantic_enter_scope()` | `semantic_enter_scope()` | `src/analysis/semantic_scopes.c` |
| `semantic_exit_scope()` | `semantic_exit_scope()` | `src/analysis/semantic_scopes.c` |
| `semantic_get_builtin_type()` | `semantic_get_builtin_type()` | `src/analysis/semantic_builtins.c` |
| `semantic_get_current_function()` | `semantic_get_current_function()` | `src/analysis/semantic_functions.c` |
| `semantic_get_expression_type()` | `semantic_get_expression_type()` | `src/analysis/semantic_expressions.c` |
| `semantic_init_builtin_types()` | `semantic_init_builtin_types()` | `src/analysis/semantic_builtins.c` |
| `semantic_init_predeclared_identifiers()` | `semantic_init_predeclared_identifiers()` | `src/analysis/semantic_builtins.c` |
| `semantic_report_error()` | `semantic_report_error()` | `src/analysis/semantic_errors.c` |
| `semantic_report_warning()` | `semantic_report_warning()` | `src/analysis/semantic_errors.c` |
| `semantic_reset_statistics()` | `semantic_reset_statistics()` | `src/analysis/semantic_statistics.c` |
| `semantic_resolve_identifier()` | `semantic_resolve_identifier()` | `src/analysis/semantic_symbols.c` |
| `semantic_set_expression_type()` | `semantic_set_expression_type()` | `src/analysis/semantic_expressions.c` |

### 5. Type Information Functions

| Stub Function | Real Implementation | File Location |
|---------------|-------------------|---------------|
| `create_type_info_from_descriptor()` | `create_type_info_from_descriptor()` | `src/analysis/semantic_type_helpers.c` |
| `get_builtin_type_descriptor()` | `get_builtin_type_descriptor()` | `src/analysis/type_info_builtins.c` |
| `get_promoted_integer_type()` | `get_promoted_integer_type()` | `src/analysis/semantic_type_helpers.c` |
| `get_promoted_type()` | `get_promoted_type()` | `src/analysis/semantic_type_helpers.c` |
| `type_info_from_descriptor()` | `type_info_from_descriptor()` | `src/analysis/type_info_creators.c` |

### 6. Code Generation Functions

| Stub Function | Real Implementation | File Location |
|---------------|-------------------|---------------|
| `generate_c_code()` | `generate_c_code()` | `src/compiler/code_generation.c` |
| `generate_concrete_struct_definition()` | `generate_concrete_struct_definition()` | `src/codegen/generic_instantiation_codegen.c` |
| `emit_instruction()` | `emit_instruction()` | `src/codegen/instruction_buffer.c` |
| `create_load_local()` | `create_load_local()` | `src/codegen/instruction_factory.c` |
| `create_store_local()` | `create_store_local()` | `src/codegen/instruction_factory.c` |
| `get_register_info()` | `get_register_info()` | `src/codegen/register_allocator.c` |

### 7. Generic Instantiation Functions

| Stub Function | Real Implementation | File Location |
|---------------|-------------------|---------------|
| `generic_instantiation_create()` | `generic_instantiation_create()` | `src/codegen/generic_instantiation_core.c` |
| `generic_instantiation_release()` | `generic_instantiation_release()` | `src/codegen/generic_instantiation_memory.c` |
| `generic_instantiation_retain()` | `generic_instantiation_retain()` | `src/codegen/generic_instantiation_memory.c` |
| `generic_registry_create()` | `generic_registry_create()` | `src/codegen/generic_instantiation_registry.c` |
| `generic_registry_destroy()` | `generic_registry_destroy()` | `src/codegen/generic_instantiation_registry.c` |
| `generic_struct_info_create()` | `generic_struct_info_create()` | `src/codegen/generic_instantiation_core.c` |
| `generic_struct_info_destroy()` | `generic_struct_info_destroy()` | `src/codegen/generic_instantiation_memory.c` |

### 8. Code Generation Infrastructure

| Stub Function | Real Implementation | File Location |
|---------------|-------------------|---------------|
| `instruction_buffer_create()` | `instruction_buffer_create()` | `src/codegen/instruction_buffer.c` |
| `instruction_buffer_destroy()` | `instruction_buffer_destroy()` | `src/codegen/instruction_buffer.c` |
| `label_manager_create()` | `label_manager_create()` | `src/codegen/label_manager.c` |
| `label_manager_destroy()` | `label_manager_destroy()` | `src/codegen/label_manager.c` |
| `local_symbol_table_create()` | `local_symbol_table_create()` | `src/codegen/symbol_table.c` |
| `local_symbol_table_destroy()` | `local_symbol_table_destroy()` | `src/codegen/symbol_table.c` |
| `register_allocator_create()` | `register_allocator_create()` | `src/codegen/register_allocator.c` |
| `register_allocator_destroy()` | `register_allocator_destroy()` | `src/codegen/register_allocator.c` |

### 9. Symbol Table Functions

| Stub Function | Real Implementation | File Location |
|---------------|-------------------|---------------|
| `symbol_entry_create()` | `symbol_entry_create()` | `src/analysis/semantic_symbols_entries.c` |
| `symbol_entry_destroy()` | `symbol_entry_destroy()` | `src/analysis/semantic_symbols_entries.c` |

### 10. Validation and Error Functions

| Stub Function | Real Implementation | File Location |
|---------------|-------------------|---------------|
| `check_annotation_conflicts()` | `check_annotation_conflicts()` | `src/analysis/semantic_annotations_conflicts.c` |
| `check_duplicate_annotations()` | `check_duplicate_annotations()` | `src/analysis/semantic_annotations_validation.c` |
| `check_const_dependency_cycle()` | `check_const_dependency_cycle()` | `src/analysis/semantic_const_declarations.c` |
| `check_instantiation_cycles()` | `check_instantiation_cycles()` | `src/codegen/generic_instantiation_validation.c` |
| `validate_annotation_dependencies()` | `validate_annotation_dependencies()` | `src/analysis/semantic_annotations_validation.c` |
| `validate_instantiation_args()` | `validate_instantiation_args()` | `src/codegen/generic_instantiation_validation.c` |
| `validate_ffi_safety()` | `validate_ffi_safety()` | `src/analysis/semantic_ffi.c` |

### 11. Const Value Functions

| Stub Function | Real Implementation | File Location |
|---------------|-------------------|---------------|
| `const_value_destroy()` | `const_value_destroy()` | `src/analysis/const_evaluator.c` |
| `create_const_value()` | `create_const_value()` | `src/analysis/const_evaluator.c` |
| `evaluate_const_expression()` | `evaluate_const_expression()` | `src/analysis/const_evaluator.c` |
| `lookup_const_value()` | `lookup_const_value()` | `src/analysis/semantic_const_declarations.c` |
| `register_const_value()` | `register_const_value()` | `src/analysis/semantic_const_declarations.c` |

### 12. Import/Module Functions

| Stub Function | Real Implementation | File Location |
|---------------|-------------------|---------------|
| `resolve_import_path()` | `resolve_import_path()` | `src/analysis/semantic_imports.c` |
| `load_module()` | `load_module()` | `src/analysis/semantic_imports.c` |

### 13. FFI Functions

| Stub Function | Real Implementation | File Location |
|---------------|-------------------|---------------|
| `create_ffi_binding()` | `create_ffi_binding()` | `src/analysis/semantic_ffi.c` |

### 14. Optimization Functions

| Stub Function | Real Implementation | File Location |
|---------------|-------------------|---------------|
| `optimize_ast()` | `optimize_ast()` | `src/codegen/optimizer_core.c` |
| `optimize_ir()` | `optimize_ir()` | `src/codegen/optimizer_core.c` |

### 15. Error Recovery Functions

| Stub Function | Real Implementation | File Location |
|---------------|-------------------|---------------|
| `error_recovery_strategy()` | `error_recovery_strategy()` | `src/parser/parser_errors.c` |
| `recover_from_error()` | `recover_from_error()` | `src/parser/parser_errors.c` |

### 16. Memory Arena Functions

| Stub Function | Real Implementation | File Location |
|---------------|-------------------|---------------|
| `arena_create_child()` | - | (Part of memory manager) |
| `arena_destroy_child()` | - | (Part of memory manager) |

### 17. Pattern Matching Functions

| Stub Function | Real Implementation | File Location |
|---------------|-------------------|---------------|
| `match_pattern()` | `match_pattern()` | `src/parser/grammar_patterns.c` |
| `compile_pattern()` | `compile_pattern()` | `src/parser/grammar_patterns.c` |

### 18. Reporting Functions

| Stub Function | Real Implementation | File Location |
|---------------|-------------------|---------------|
| `issue_annotation_warning()` | `issue_annotation_warning()` | `src/analysis/semantic_annotations_errors.c` |
| `report_annotation_conflict_error()` | `report_annotation_conflict_error()` | `src/analysis/semantic_annotations_errors.c` |
| `report_context_validation_error()` | `report_context_validation_error()` | `src/analysis/semantic_annotations_errors.c` |
| `report_parameter_validation_error()` | `report_parameter_validation_error()` | `src/analysis/semantic_annotations_errors.c` |
| `report_unknown_annotation_error()` | `report_unknown_annotation_error()` | `src/analysis/semantic_annotations_errors.c` |

### 19. Type Query Functions

| Stub Function | Real Implementation | File Location |
|---------------|-------------------|---------------|
| `is_numeric_type()` | `is_numeric_type()` | `src/analysis/semantic_type_helpers.c` |
| `is_integral_type()` | `is_integral_type()` | `src/analysis/semantic_type_helpers.c` |
| `types_compatible()` | `types_compatible()` | `src/analysis/semantic_type_helpers.c` |
| `is_annotation_valid()` | `is_annotation_valid()` | `src/analysis/semantic_annotations_validation.c` |

### 20. Scanner/Lexer Functions

| Stub Function | Real Implementation | File Location |
|---------------|-------------------|---------------|
| `scan_token()` | `scan_token()` | `src/parser/lexer_scan.c` |

### 21. Performance Functions

| Stub Function | Real Implementation | File Location |
|---------------|-------------------|---------------|
| `performance_validation_run_complete()` | - | (Part of performance benchmarking) |

### 22. Test Framework Functions

| Stub Function | Real Implementation | File Location |
|---------------|-------------------|---------------|
| `asthra_test_assert_bool()` | `asthra_test_assert_bool()` | `tests/framework/test_framework.h` |

## Component Summary

### Parser Component
- **Location**: `src/parser/`
- **Key Files**: 
  - Grammar files: `grammar_*.c` (toplevel, expressions, statements, patterns, annotations)
  - Core parser: `parser_core.c`, `parser_ast_helpers.c`
  - Lexer: `lexer.c`, `lexer_scan.c` and related modules
  - AST: `ast.c`, `ast_node_creation.c`, `ast_node_destruction.c`

### Semantic Analysis Component
- **Location**: `src/analysis/`
- **Key Files**:
  - Core: `semantic_core.c`, `semantic_analyzer.h`
  - Specialized analyzers: `semantic_structs.c`, `semantic_enums.c`, `semantic_functions.c`, etc.
  - Type system: `type_info_*.c`, `semantic_types.c`, `semantic_type_helpers.c`
  - Symbols: `semantic_symbols_*.c` modules
  - Annotations: `semantic_annotations_*.c` modules
  - Constants: `const_evaluator.c`, `semantic_const_declarations.c`

### Code Generation Component
- **Location**: `src/codegen/`
- **Key Files**:
  - Core: `code_generator.c`, `code_generator_core.c`
  - Infrastructure: `instruction_buffer.c`, `label_manager.c`, `register_allocator.c`
  - Generic instantiation: `generic_instantiation_*.c` modules
  - Optimization: `optimizer_*.c` modules
  - ELF generation: `elf_writer_*.c` modules
  - FFI: `ffi_assembly_*.c` modules

### Compiler Integration
- **Location**: `src/compiler/`
- **Key Files**:
  - Pipeline: `compilation_pipeline.c`
  - Code generation: `code_generation.c`
  - Error handling: `enhanced_error_reporting.c`

## Notes

1. Some stub functions have no direct real implementation and are either:
   - Part of a larger module (marked with "Part of...")
   - Inline implementations within other functions
   - Test-specific helpers that don't exist in production code

2. The real implementations often have more complex signatures and additional parameters compared to the stubs. For example:
   - Stub: `void* instruction_buffer_create(void)`
   - Real: `InstructionBuffer *instruction_buffer_create(size_t initial_capacity)`

3. Many functions are split across multiple files following a modular architecture pattern.

4. File locations are relative to the project root `/Users/bmuthuka/asthra-lang/asthra/`.

5. The stubs provide a minimal interface for testing, while the real implementations include:
   - Thread safety (mutexes, atomic operations)
   - Memory management (reference counting, proper cleanup)
   - Error handling and reporting
   - Performance optimizations
   - Integration with other components
