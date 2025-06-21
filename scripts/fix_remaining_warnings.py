#!/usr/bin/env python3
"""
Fix remaining compilation warnings in the Asthra project.
This script addresses additional warning categories found after the initial fixes.
"""

import os
import re
import sys
from pathlib import Path

def fix_additional_missing_newlines():
    """Fix additional missing newlines at end of files."""
    print("Fixing additional missing newlines...")
    
    # Additional files that need newlines based on remaining warnings
    additional_files = [
        "src/ai_annotations/ai_annotation_grammar.h",
        "src/ai_annotations/ai_annotation_semantic.h", 
        "src/ai_annotations/ai_annotation_processor.h",
        "src/cli.c",
        "src/platform.c",
        "src/static_analysis.c"
    ]
    
    fixed_count = 0
    for file_path in additional_files:
        if os.path.exists(file_path):
            try:
                with open(file_path, 'rb') as f:
                    content = f.read()
                
                if content and not content.endswith(b'\n'):
                    with open(file_path, 'ab') as f:
                        f.write(b'\n')
                    print(f"  Fixed: {file_path}")
                    fixed_count += 1
                else:
                    print(f"  Already OK: {file_path}")
            except Exception as e:
                print(f"  Error fixing {file_path}: {e}")
        else:
            print(f"  Not found: {file_path}")
    
    print(f"Fixed {fixed_count} additional files with missing newlines.")

def fix_type_conversion_warnings():
    """Fix type conversion warnings in json_utils.c and other files."""
    print("Fixing type conversion warnings...")
    
    # Fix json_utils.c type conversion warnings
    file_path = "src/utils/json_utils.c"
    if os.path.exists(file_path):
        try:
            with open(file_path, 'r') as f:
                content = f.read()
            
            # Fix size_t to int conversion
            content = re.sub(
                r'return json_object_array_length\(array\);',
                r'return (int)json_object_array_length(array);',
                content
            )
            
            # Fix int to size_t conversion
            content = re.sub(
                r'json_object_array_get_idx\(array, index\)',
                r'json_object_array_get_idx(array, (size_t)index)',
                content
            )
            
            # Fix size_t return type conversion
            content = re.sub(
                r'return json_object_object_length\(obj\);',
                r'return (size_t)json_object_object_length(obj);',
                content
            )
            
            with open(file_path, 'w') as f:
                f.write(content)
            print(f"  Fixed type conversion warnings in {file_path}")
        except Exception as e:
            print(f"  Error fixing {file_path}: {e}")
    else:
        print(f"  Not found: {file_path}")

def fix_const_qualifier_warnings():
    """Fix const qualifier warnings in concurrency code."""
    print("Fixing const qualifier warnings...")
    
    file_path = "runtime/asthra_concurrency_channels.c"
    if os.path.exists(file_path):
        try:
            with open(file_path, 'r') as f:
                content = f.read()
            
            # Fix const void* to void* cast
            content = re.sub(
                r'context->send_values\[index\] = \(void\*\)value;',
                r'context->send_values[index] = (void*)(uintptr_t)value;',
                content
            )
            
            with open(file_path, 'w') as f:
                f.write(content)
            print(f"  Fixed const qualifier warnings in {file_path}")
        except Exception as e:
            print(f"  Error fixing {file_path}: {e}")
    else:
        print(f"  Not found: {file_path}")

def fix_undefined_macro_warnings():
    """Fix undefined macro warnings."""
    print("Fixing undefined macro warnings...")
    
    # Fix ASTHRA_CONCURRENCY_HAS_C17_THREADS and ASTHRA_HAS_C11_THREADS
    files_to_fix = [
        "runtime/asthra_concurrency_core.c",
        "runtime/asthra_runtime_core.c"
    ]
    
    for file_path in files_to_fix:
        if os.path.exists(file_path):
            try:
                with open(file_path, 'r') as f:
                    content = f.read()
                
                # Add macro definitions at the top if not present
                if 'ASTHRA_CONCURRENCY_HAS_C17_THREADS' in content and '#ifndef ASTHRA_CONCURRENCY_HAS_C17_THREADS' not in content:
                    # Add the macro definition after includes
                    include_pattern = r'(#include[^\n]*\n)+'
                    match = re.search(include_pattern, content)
                    if match:
                        insertion_point = match.end()
                        macro_defs = """
#ifndef ASTHRA_CONCURRENCY_HAS_C17_THREADS
#define ASTHRA_CONCURRENCY_HAS_C17_THREADS 0
#endif

#ifndef ASTHRA_HAS_C11_THREADS
#define ASTHRA_HAS_C11_THREADS 0
#endif

"""
                        content = content[:insertion_point] + macro_defs + content[insertion_point:]
                        
                        with open(file_path, 'w') as f:
                            f.write(content)
                        print(f"  Added macro definitions to {file_path}")
                    else:
                        print(f"  Could not find insertion point in {file_path}")
                else:
                    print(f"  Macros already defined or not needed in {file_path}")
            except Exception as e:
                print(f"  Error fixing {file_path}: {e}")
        else:
            print(f"  Not found: {file_path}")

def add_missing_prototypes():
    """Add missing function prototypes to header files."""
    print("Adding missing function prototypes...")
    
    # Functions that need prototypes based on warnings
    platform_functions = [
        "asthra_thread_id_t asthra_get_current_thread_id(void);",
        "asthra_process_id_t asthra_get_current_process_id(void);", 
        "void* asthra_realloc_safe(void* ptr, size_t old_size, size_t new_size);",
        "bool asthra_string_ends_with(const char* str, const char* suffix);"
    ]
    
    # Add to platform header
    platform_header = "src/platform.h"
    if os.path.exists(platform_header):
        try:
            with open(platform_header, 'r') as f:
                content = f.read()
            
            prototypes_to_add = []
            for prototype in platform_functions:
                func_name = prototype.split('(')[0].split()[-1]
                if func_name not in content:
                    prototypes_to_add.append(prototype)
            
            if prototypes_to_add:
                if '#endif' in content:
                    insertion_point = content.rfind('#endif')
                    new_content = (content[:insertion_point] + 
                                 "\n// Additional platform functions\n" +
                                 "\n".join(prototypes_to_add) + "\n\n" +
                                 content[insertion_point:])
                else:
                    new_content = content + "\n// Additional platform functions\n" + "\n".join(prototypes_to_add) + "\n"
                
                with open(platform_header, 'w') as f:
                    f.write(new_content)
                print(f"  Added {len(prototypes_to_add)} prototypes to {platform_header}")
            else:
                print(f"  All prototypes already present in {platform_header}")
        except Exception as e:
            print(f"  Error fixing {platform_header}: {e}")
    else:
        print(f"  Not found: {platform_header}")

def add_static_declarations():
    """Add static declarations for internal functions (commented out to avoid breaking API)."""
    print("Note: Static declarations for internal functions are available but commented out")
    print("      to avoid potentially breaking the public API. Uncomment if desired.")
    
    # This function is intentionally left mostly empty as adding 'static' 
    # to functions might break the intended API. The warnings suggest making
    # functions static, but they might be intended to be used externally.
    
    # Example of what could be done (but we're being conservative):
    """
    files_with_internal_functions = [
        "runtime/asthra_concurrency_channels.c",
        "runtime/asthra_concurrency_core.c", 
        "runtime/asthra_concurrency_patterns.c",
        "runtime/asthra_crypto.c",
        "runtime/asthra_integration_stubs.c",
        "runtime/asthra_memory.c",
        "runtime/asthra_safety_concurrency_errors.c",
        "runtime/asthra_safety_core.c",
        "runtime/asthra_safety_memory_ffi.c",
        "runtime/asthra_safety_security.c"
    ]
    
    # We could add 'static' to functions that the compiler suggests,
    # but this might break intended external usage
    """
    pass

def main():
    """Main function to run additional fixes."""
    print("Starting additional compilation warning fixes...")
    print("=" * 60)
    
    # Change to project root if we're in scripts directory
    if os.path.basename(os.getcwd()) == 'scripts':
        os.chdir('..')
    
    fix_additional_missing_newlines()
    print()
    fix_type_conversion_warnings()
    print()
    fix_const_qualifier_warnings()
    print()
    fix_undefined_macro_warnings()
    print()
    add_missing_prototypes()
    print()
    add_static_declarations()
    
    print("=" * 60)
    print("Additional warning fixes completed!")
    print("Run 'make clean && make' to verify the additional fixes.")
    print()
    print("Note: Some remaining warnings may be intentional (e.g., functions")
    print("      that should be public API but don't have prototypes yet).")

if __name__ == "__main__":
    main() 
