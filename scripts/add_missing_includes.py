#!/usr/bin/env python3
"""
Add missing include statements to source files for their corresponding header files.
"""

import os
import re
from pathlib import Path

def add_include_to_file(source_file, header_file):
    """Add include statement to source file if not already present."""
    try:
        with open(source_file, 'r') as f:
            content = f.read()
        
        # Check if include already exists
        include_statement = f'#include "{header_file}"'
        if include_statement in content:
            print(f"  Include already exists: {header_file}")
            return False
        
        # Find the best place to insert the include
        lines = content.split('\n')
        insert_index = 0
        
        # Look for existing includes
        for i, line in enumerate(lines):
            if line.strip().startswith('#include'):
                insert_index = i + 1
            elif line.strip().startswith('#include') and 'local' in line:
                # Insert before local includes
                insert_index = i
                break
        
        # If no includes found, insert after initial comments/guards
        if insert_index == 0:
            for i, line in enumerate(lines):
                if (line.strip().startswith('#ifndef') or 
                    line.strip().startswith('#define') or
                    line.strip().startswith('/*') or
                    line.strip().startswith('//')):
                    continue
                else:
                    insert_index = i
                    break
        
        # Insert the include
        lines.insert(insert_index, include_statement)
        
        # Write back to file
        with open(source_file, 'w') as f:
            f.write('\n'.join(lines))
        
        print(f"  ✓ Added include: {header_file}")
        return True
        
    except Exception as e:
        print(f"  ✗ Error adding include to {source_file}: {e}")
        return False

def main():
    # Map of source files to their corresponding header files
    source_to_header = {
        # Semantic analysis files
        'src/analysis/semantic_symbols_aliases.c': 'semantic_symbols_aliases.h',
        'src/analysis/semantic_symbols_core.c': 'semantic_symbols_core.h',
        'src/analysis/semantic_symbols_entries.c': 'semantic_symbols_entries.h',
        'src/analysis/semantic_symbols_stats.c': 'semantic_symbols_stats.h',
        'src/analysis/semantic_symbols_utils.c': 'semantic_symbols_utils.h',
        
        # Codegen files
        'src/codegen/elf_writer_sections.c': 'elf_writer_sections.h',
        'src/codegen/ffi_assembly_concurrency.c': 'ffi_assembly_concurrency.h',
        
        # Compiler files
        'src/compiler/code_generation.c': 'code_generation.h',
        
        # Parser AST destruction files
        'src/parser/ast_destruction_concurrency.c': 'ast_destruction_concurrency.h',
        'src/parser/ast_destruction_declarations.c': 'ast_destruction_declarations.h',
        'src/parser/ast_destruction_expressions.c': 'ast_destruction_expressions.h',
        'src/parser/ast_destruction_literals.c': 'ast_destruction_literals.h',
        'src/parser/ast_destruction_patterns.c': 'ast_destruction_patterns.h',
        'src/parser/ast_destruction_statements.c': 'ast_destruction_statements.h',
        'src/parser/ast_destruction_types.c': 'ast_destruction_types.h',
        'src/parser/ast_node_creation.c': 'ast_node_creation.h',
        
        # Runtime files
        'runtime/asthra_concurrency_channels.c': 'asthra_concurrency_channels.h',
        'runtime/asthra_concurrency_core.c': 'asthra_concurrency_core.h',
        'runtime/asthra_concurrency_patterns.c': 'asthra_concurrency_patterns.h',
        'runtime/asthra_crypto.c': 'asthra_crypto.h',
        'runtime/asthra_integration_stubs.c': 'asthra_integration_stubs.h',
        'runtime/asthra_memory.c': 'asthra_memory.h',
        'runtime/asthra_safety_concurrency_errors.c': 'asthra_safety_concurrency_errors.h',
        'runtime/asthra_safety_core.c': 'asthra_safety_core.h',
        'runtime/asthra_safety_memory_ffi.c': 'asthra_safety_memory_ffi.h',
        'runtime/asthra_safety_security.c': 'asthra_safety_security.h',
    }
    
    total_added = 0
    
    print("Adding missing include statements to source files...")
    
    for source_file, header_file in source_to_header.items():
        print(f"\nProcessing {source_file}")
        
        if not os.path.exists(source_file):
            print(f"  ✗ Source file not found: {source_file}")
            continue
        
        # Check if corresponding header exists
        header_path = Path(source_file).parent / header_file
        if not header_path.exists():
            print(f"  ✗ Header file not found: {header_path}")
            continue
        
        if add_include_to_file(source_file, header_file):
            total_added += 1
    
    print(f"\n=== Include Addition Results ===")
    print(f"Total includes added: {total_added}")
    print(f"Files processed: {len(source_to_header)}")
    
    return 0

if __name__ == "__main__":
    exit(main()) 
