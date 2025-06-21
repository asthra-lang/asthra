#!/usr/bin/env python3
"""
Phase 2 Complete Missing Prototypes Fix Script
Fixes all 80 missing prototype warnings by creating header files and adding prototypes.
"""

import os
import re
import sys
from pathlib import Path

def extract_function_signature(file_path, function_name):
    """Extract the complete function signature from a C file."""
    try:
        with open(file_path, 'r') as f:
            content = f.read()
        
        # Pattern to match function definitions
        # Handles multi-line function signatures
        pattern = rf'^\s*([^/\n]*?)\s+{re.escape(function_name)}\s*\([^{{]*?\)\s*{{'
        match = re.search(pattern, content, re.MULTILINE | re.DOTALL)
        
        if match:
            signature = match.group(1) + ' ' + function_name
            # Extract parameters
            param_start = content.find('(', match.start())
            param_end = content.find(')', param_start)
            if param_start != -1 and param_end != -1:
                params = content[param_start:param_end+1]
                signature += params
                # Clean up the signature
                signature = re.sub(r'\s+', ' ', signature.strip())
                signature = signature.replace('static ', '')  # Remove static if present
                return signature + ';'
        
        return None
    except Exception as e:
        print(f"Error extracting signature for {function_name} from {file_path}: {e}")
        return None

def create_header_file(header_path, prototypes, includes=None):
    """Create a header file with the given prototypes."""
    header_guard = header_path.name.upper().replace('.', '_').replace('/', '_')
    
    content = f"""#ifndef {header_guard}
#define {header_guard}

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
"""
    
    if includes:
        for include in includes:
            content += f"#include \"{include}\"\n"
    
    content += "\n#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n"
    
    for prototype in prototypes:
        content += f"{prototype}\n"
    
    content += "\n#ifdef __cplusplus\n}\n#endif\n\n"
    content += f"#endif // {header_guard}\n"
    
    # Create directory if it doesn't exist
    header_path.parent.mkdir(parents=True, exist_ok=True)
    
    with open(header_path, 'w') as f:
        f.write(content)
    
    print(f"Created header file: {header_path}")

def main():
    # Define all missing prototypes organized by file
    missing_prototypes = {
        # Semantic analysis files
        'src/analysis/semantic_symbols_aliases.h': {
            'source': 'src/analysis/semantic_symbols_aliases.c',
            'functions': [
                'semantic_register_module_alias',
                'semantic_resolve_module_alias', 
                'semantic_has_module_alias',
                'semantic_cleanup_module_aliases'
            ],
            'includes': ['semantic_symbols.h']
        },
        'src/analysis/semantic_symbols_core.h': {
            'source': 'src/analysis/semantic_symbols_core.c',
            'functions': [
                'symbol_table_insert_impl',
                'symbol_table_lookup_impl',
                'symbol_table_contains',
                'symbol_table_capacity',
                'symbol_table_remove'
            ],
            'includes': ['semantic_symbols.h']
        },
        'src/analysis/semantic_symbols_entries.h': {
            'source': 'src/analysis/semantic_symbols_entries.c',
            'functions': [
                'symbol_entry_copy',
                'semantic_resolve_symbol_impl'
            ],
            'includes': ['semantic_symbols.h']
        },
        'src/analysis/semantic_symbols_stats.h': {
            'source': 'src/analysis/semantic_symbols_stats.c',
            'functions': [
                'semantic_print_symbol_statistics'
            ],
            'includes': ['semantic_symbols.h']
        },
        'src/analysis/semantic_symbols_utils.h': {
            'source': 'src/analysis/semantic_symbols_utils.c',
            'functions': [
                'symbol_kind_name',
                'symbol_kind_is_callable',
                'symbol_kind_is_type',
                'symbol_kind_is_value',
                'semantic_validate_symbol_name',
                'semantic_is_reserved_keyword',
                'semantic_check_symbol_shadowing'
            ],
            'includes': ['semantic_symbols.h']
        },
        
        # Codegen files
        'src/codegen/elf_writer_sections.h': {
            'source': 'src/codegen/elf_writer_sections.c',
            'functions': [
                'elf_append_section_data',
                'elf_set_section_data'
            ],
            'includes': ['elf_writer_core.h']
        },
        'src/codegen/ffi_assembly_concurrency.h': {
            'source': 'src/codegen/ffi_assembly_concurrency.c',
            'functions': [
                'ffi_generate_task_creation_with_handle',
                'ffi_validate_concurrency_annotations'
            ],
            'includes': ['ffi_assembly_core.h']
        },
        
        # Compiler files
        'src/compiler/code_generation.h': {
            'source': 'src/compiler/code_generation.c',
            'functions': [
                'generate_c_code'
            ],
            'includes': ['compiler_core.h']
        },
        
        # Parser AST destruction files
        'src/parser/ast_destruction_concurrency.h': {
            'source': 'src/parser/ast_destruction_concurrency.c',
            'functions': [
                'ast_free_concurrency_nodes'
            ],
            'includes': ['ast.h']
        },
        'src/parser/ast_destruction_declarations.h': {
            'source': 'src/parser/ast_destruction_declarations.c',
            'functions': [
                'ast_free_declaration_nodes'
            ],
            'includes': ['ast.h']
        },
        'src/parser/ast_destruction_expressions.h': {
            'source': 'src/parser/ast_destruction_expressions.c',
            'functions': [
                'ast_free_expression_nodes'
            ],
            'includes': ['ast.h']
        },
        'src/parser/ast_destruction_literals.h': {
            'source': 'src/parser/ast_destruction_literals.c',
            'functions': [
                'ast_free_literal_nodes'
            ],
            'includes': ['ast.h']
        },
        'src/parser/ast_destruction_patterns.h': {
            'source': 'src/parser/ast_destruction_patterns.c',
            'functions': [
                'ast_free_pattern_nodes'
            ],
            'includes': ['ast.h']
        },
        'src/parser/ast_destruction_statements.h': {
            'source': 'src/parser/ast_destruction_statements.c',
            'functions': [
                'ast_free_statement_nodes'
            ],
            'includes': ['ast.h']
        },
        'src/parser/ast_destruction_types.h': {
            'source': 'src/parser/ast_destruction_types.c',
            'functions': [
                'ast_free_type_nodes'
            ],
            'includes': ['ast.h']
        },
        'src/parser/ast_node_creation.h': {
            'source': 'src/parser/ast_node_creation.c',
            'functions': [
                'ast_clone_node'
            ],
            'includes': ['ast.h']
        },
        
        # Runtime concurrency files
        'runtime/asthra_concurrency_channels.h': {
            'source': 'runtime/asthra_concurrency_channels.c',
            'functions': [
                'Asthra_channel_len',
                'Asthra_channel_cap',
                'Asthra_channel_is_empty',
                'Asthra_channel_is_full',
                'Asthra_channel_get_info',
                'Asthra_channel_dump_info'
            ],
            'includes': ['asthra_concurrency_core.h']
        },
        'runtime/asthra_concurrency_core.h': {
            'source': 'runtime/asthra_concurrency_core.c',
            'functions': [
                'asthra_concurrency_atomic_increment_counter_explicit',
                'asthra_concurrency_atomic_decrement_counter_explicit',
                'asthra_concurrency_atomic_load_counter_explicit',
                'asthra_concurrency_atomic_store_counter_explicit',
                'asthra_concurrency_atomic_cas_ptr_explicit',
                'asthra_concurrency_atomic_cas_ptr',
                'asthra_concurrency_lock_free_stack_push',
                'asthra_concurrency_lock_free_stack_pop',
                'asthra_concurrency_create_thread_hybrid',
                'asthra_concurrency_get_thread_data',
                'asthra_concurrency_set_thread_data'
            ],
            'includes': ['asthra_runtime_core.h']
        },
        'runtime/asthra_concurrency_patterns.h': {
            'source': 'runtime/asthra_concurrency_patterns.c',
            'functions': [
                'Asthra_worker_pool_dump_stats',
                'Asthra_multiplexer_create',
                'Asthra_multiplexer_start',
                'Asthra_multiplexer_stop',
                'Asthra_multiplexer_destroy',
                'Asthra_load_balancer_create',
                'Asthra_load_balancer_submit',
                'Asthra_load_balancer_destroy',
                'Asthra_dump_pattern_diagnostics'
            ],
            'includes': ['asthra_concurrency_core.h']
        },
        
        # Runtime crypto files
        'runtime/asthra_crypto.h': {
            'source': 'runtime/asthra_crypto.c',
            'functions': [
                'asthra_crypto_random_string',
                'asthra_crypto_random_bytes',
                'asthra_hash_bytes',
                'asthra_hash_string',
                'asthra_crypto_string_equals'
            ],
            'includes': ['asthra_runtime_core.h']
        },
        
        # Runtime integration files
        'runtime/asthra_integration_stubs.h': {
            'source': 'runtime/asthra_integration_stubs.c',
            'functions': [
                'Asthra_register_c_thread',
                'Asthra_unregister_c_thread',
                'asthra_test_context_init',
                'lexer_create',
                'parser_create',
                'parser_destroy',
                'parser_parse_program',
                'semantic_analyze_program',
                'semantic_analyzer_create',
                'semantic_analyzer_destroy'
            ],
            'includes': ['asthra_runtime_core.h']
        },
        
        # Runtime memory files
        'runtime/asthra_memory.h': {
            'source': 'runtime/asthra_memory.c',
            'functions': [
                'asthra_memory_zones_init',
                'asthra_memory_zones_cleanup'
            ],
            'includes': ['asthra_runtime_core.h']
        },
        
        # Runtime safety files
        'runtime/asthra_safety_concurrency_errors.h': {
            'source': 'runtime/asthra_safety_concurrency_errors.c',
            'functions': [
                'asthra_safety_log_task_lifecycle_event_enhanced',
                'asthra_safety_concurrency_errors_cleanup'
            ],
            'includes': ['asthra_safety_core.h']
        },
        'runtime/asthra_safety_core.h': {
            'source': 'runtime/asthra_safety_core.c',
            'functions': [
                'asthra_safety_get_config'
            ],
            'includes': ['asthra_runtime_core.h']
        },
        'runtime/asthra_safety_memory_ffi.h': {
            'source': 'runtime/asthra_safety_memory_ffi.c',
            'functions': [
                'asthra_safety_memory_ffi_cleanup'
            ],
            'includes': ['asthra_safety_core.h']
        },
        'runtime/asthra_safety_security.h': {
            'source': 'runtime/asthra_safety_security.c',
            'functions': [
                'asthra_safety_security_cleanup'
            ],
            'includes': ['asthra_safety_core.h']
        }
    }
    
    total_functions = 0
    created_headers = 0
    
    for header_path, info in missing_prototypes.items():
        source_file = info['source']
        functions = info['functions']
        includes = info.get('includes', [])
        
        print(f"\nProcessing {source_file} -> {header_path}")
        
        if not os.path.exists(source_file):
            print(f"Warning: Source file {source_file} not found")
            continue
        
        prototypes = []
        for func_name in functions:
            signature = extract_function_signature(source_file, func_name)
            if signature:
                prototypes.append(signature)
                print(f"  ✓ {func_name}")
                total_functions += 1
            else:
                print(f"  ✗ Failed to extract signature for {func_name}")
        
        if prototypes:
            create_header_file(Path(header_path), prototypes, includes)
            created_headers += 1
    
    print(f"\n=== Phase 2 Complete Results ===")
    print(f"Created header files: {created_headers}")
    print(f"Added function prototypes: {total_functions}")
    print(f"Expected to fix: 80 missing prototype warnings")
    
    # Verify by building again
    print(f"\nRunning verification build...")
    os.system("make clean > /dev/null 2>&1")
    result = os.system("make 2>&1 | grep -E 'warning.*no previous prototype' | wc -l")
    
    return 0

if __name__ == "__main__":
    sys.exit(main()) 
