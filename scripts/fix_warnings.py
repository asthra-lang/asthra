#!/usr/bin/env python3
"""
Fix compilation warnings in the Asthra project.
This script addresses the most common warning categories.
"""

import os
import re
import sys
from pathlib import Path

def fix_missing_newlines():
    """Fix missing newlines at end of files."""
    print("Fixing missing newlines at end of files...")
    
    # Files that need newlines based on the warnings
    files_needing_newlines = [
        "src/utils/json_utils.h",
        "src/utils/json_utils.c", 
        "runtime/asthra_concurrency_callbacks.c",
        "runtime/asthra_concurrency_channels.c",
        "runtime/asthra_concurrency_core.c",
        "runtime/asthra_concurrency_patterns.c",
        "runtime/asthra_concurrency_tasks.c",
        "runtime/concurrency/asthra_concurrency_threads_common.h",
        "runtime/concurrency/asthra_concurrency_thread_registry.h",
        "runtime/concurrency/asthra_concurrency_gc_roots.h",
        "runtime/concurrency/asthra_concurrency_mutex.h",
        "runtime/concurrency/asthra_concurrency_condvar.h",
        "runtime/concurrency/asthra_concurrency_rwlock.h",
        "runtime/asthra_concurrency_threads_modular.h",
        "runtime/asthra_concurrency_threads.c",
        "runtime/asthra_crypto.c",
        "runtime/asthra_enum_support.h",
        "runtime/asthra_enum_support.c",
        "runtime/asthra_ffi_runtime.c",
        "runtime/asthra_integration_stubs.c",
        "runtime/asthra_memory.c",
        "runtime/asthra_safety_core.c",
        "runtime/asthra_safety_memory_ffi.c",
        "runtime/asthra_safety_security.c",
        "runtime/asthra_safety.h",
        "runtime/asthra_safety_strings_slices.c"
    ]
    
    fixed_count = 0
    for file_path in files_needing_newlines:
        if os.path.exists(file_path):
            try:
                with open(file_path, 'rb') as f:
                    content = f.read()
                
                # Check if file ends with newline
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
    
    print(f"Fixed {fixed_count} files with missing newlines.")

def fix_string_function_prototypes():
    """Add missing function prototypes for string functions."""
    print("Fixing missing string function prototypes...")
    
    # Based on the warnings, these functions need prototypes in asthra_string.h
    missing_prototypes = [
        "char* asthra_string_to_cstr_ffi(AsthraString str);",
        "AsthraString asthra_string_substring(AsthraString str, size_t start, size_t len);",
        "AsthraString asthra_string_replace(AsthraString str, AsthraString old_str, AsthraString new_str);",
        "bool asthra_string_starts_with(AsthraString str, AsthraString prefix);",
        "bool asthra_string_ends_with(AsthraString str, AsthraString suffix);",
        "AsthraString asthra_string_trim(AsthraString str);",
        "AsthraString asthra_string_to_upper(AsthraString str);",
        "AsthraString asthra_string_to_lower(AsthraString str);",
        "AsthraSliceHeader asthra_string_split(AsthraString str, AsthraString delimiter);",
        "AsthraString asthra_string_join(AsthraSliceHeader strings, AsthraString separator);",
        "int asthra_string_find(AsthraString str, AsthraString substring);"
    ]
    
    # Find the string header file
    string_header_paths = [
        "runtime/asthra_string.h",
        "runtime/strings/asthra_runtime_strings.h"
    ]
    
    for header_path in string_header_paths:
        if os.path.exists(header_path):
            try:
                with open(header_path, 'r') as f:
                    content = f.read()
                
                # Check if prototypes are already present
                prototypes_to_add = []
                for prototype in missing_prototypes:
                    func_name = prototype.split('(')[0].split()[-1]
                    if func_name not in content:
                        prototypes_to_add.append(prototype)
                
                if prototypes_to_add:
                    # Find a good place to insert prototypes (before #endif)
                    if '#endif' in content:
                        insertion_point = content.rfind('#endif')
                        new_content = (content[:insertion_point] + 
                                     "\n// Missing function prototypes\n" +
                                     "\n".join(prototypes_to_add) + "\n\n" +
                                     content[insertion_point:])
                    else:
                        new_content = content + "\n// Missing function prototypes\n" + "\n".join(prototypes_to_add) + "\n"
                    
                    with open(header_path, 'w') as f:
                        f.write(new_content)
                    
                    print(f"  Added {len(prototypes_to_add)} prototypes to {header_path}")
                else:
                    print(f"  All prototypes already present in {header_path}")
                break
            except Exception as e:
                print(f"  Error fixing {header_path}: {e}")
    else:
        print("  String header file not found")

def fix_documentation_warnings():
    """Fix documentation warnings by removing orphaned @return/@param commands."""
    print("Fixing documentation warnings...")
    
    # Files with documentation warnings
    doc_files = [
        "runtime/asthra_ffi_memory.h",
        "runtime/asthra_concurrency_bridge_modular.h"
    ]
    
    for file_path in doc_files:
        if os.path.exists(file_path):
            try:
                with open(file_path, 'r') as f:
                    content = f.read()
                
                # Remove orphaned @return and @param commands that don't have corresponding function declarations
                # This is a simple fix - remove lines that start with * @return or * @param but aren't followed by function
                lines = content.split('\n')
                fixed_lines = []
                i = 0
                while i < len(lines):
                    line = lines[i]
                    if ('* @return' in line or '* @param' in line) and i + 1 < len(lines):
                        # Check if next few lines contain a function declaration
                        next_lines = '\n'.join(lines[i+1:i+5])
                        if not re.search(r'\w+\s*\([^)]*\)\s*[;{]', next_lines):
                            # Skip this orphaned documentation line
                            print(f"  Removing orphaned doc line: {line.strip()}")
                            i += 1
                            continue
                    fixed_lines.append(line)
                    i += 1
                
                if len(fixed_lines) != len(lines):
                    with open(file_path, 'w') as f:
                        f.write('\n'.join(fixed_lines))
                    print(f"  Fixed documentation in {file_path}")
                else:
                    print(f"  No documentation issues found in {file_path}")
            except Exception as e:
                print(f"  Error fixing {file_path}: {e}")
        else:
            print(f"  Not found: {file_path}")

def fix_type_qualifier_warnings():
    """Fix type qualifier warnings."""
    print("Fixing type qualifier warnings...")
    
    # Fix const char[] to void* casting in stdlib_concurrency_support.c
    file_path = "runtime/stdlib_concurrency_support.c"
    if os.path.exists(file_path):
        try:
            with open(file_path, 'r') as f:
                content = f.read()
            
            # Fix the specific cast warning
            old_pattern = r'return asthra_result_err\(\(void\*\)"([^"]+)"'
            new_pattern = r'return asthra_result_err((void*)(uintptr_t)"\1"'
            
            new_content = re.sub(old_pattern, new_pattern, content)
            
            if new_content != content:
                with open(file_path, 'w') as f:
                    f.write(new_content)
                print(f"  Fixed type qualifier warning in {file_path}")
            else:
                print(f"  No type qualifier issues found in {file_path}")
        except Exception as e:
            print(f"  Error fixing {file_path}: {e}")
    else:
        print(f"  Not found: {file_path}")

def main():
    """Main function to run all fixes."""
    print("Starting compilation warning fixes...")
    print("=" * 50)
    
    # Change to project root if we're in scripts directory
    if os.path.basename(os.getcwd()) == 'scripts':
        os.chdir('..')
    
    fix_missing_newlines()
    print()
    fix_string_function_prototypes()
    print()
    fix_documentation_warnings()
    print()
    fix_type_qualifier_warnings()
    
    print("=" * 50)
    print("Warning fixes completed!")
    print("Run 'make clean && make' to verify the fixes.")

if __name__ == "__main__":
    main() 
