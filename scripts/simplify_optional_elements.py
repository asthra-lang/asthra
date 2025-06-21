#!/usr/bin/env python3
"""
Optional Elements Simplification Migration Script

This script automatically migrates Asthra source files to comply with the
Optional Elements Simplification Plan (Phase 1-2, v1.18).

The script transforms code to use explicit syntax patterns:
- Adds explicit visibility modifiers (pub/priv)
- Adds void to empty parameter lists
- Adds void to empty struct/enum bodies  
- Removes import aliases for simplicity
- Adds void to empty function calls
- Adds void to empty pattern arguments
- Adds void to empty annotation parameters
- Adds void to empty array literals

Usage:
    python3 scripts/simplify_optional_elements.py file1.asthra file2.asthra ...
    python3 scripts/simplify_optional_elements.py src/**/*.asthra
"""

import re
import sys
import os
from pathlib import Path


def simplify_visibility(content):
    """Add explicit visibility modifiers where missing"""
    
    # Add explicit private visibility to declarations without pub
    # Match struct, enum, fn, impl at line start without existing visibility
    content = re.sub(
        r'^(struct|enum|fn|impl)(\s+\w+)',
        r'priv \1\2',
        content,
        flags=re.MULTILINE
    )
    
    # Fix functions that already have pub (avoid "priv pub")
    content = re.sub(r'priv pub ', 'pub ', content)
    
    # Fix extern functions
    content = re.sub(r'priv extern ', 'extern ', content)
    
    return content


def simplify_parameters(content):
    """Add void to empty parameter lists"""
    
    # Add void to empty function parameter lists
    content = re.sub(r'fn (\w+)\(\)', r'fn \1(void)', content)
    
    # Handle extern functions
    content = re.sub(r'extern fn (\w+)\(\)', r'extern fn \1(void)', content)
    
    # Handle method declarations in impl blocks
    content = re.sub(r'(\s+)fn (\w+)\(\)', r'\1fn \2(void)', content)
    
    return content


def simplify_struct_enum_bodies(content):
    """Add void to empty struct/enum bodies"""
    
    # Add void to empty struct bodies
    content = re.sub(r'struct (\w+)\s*\{\s*\}', r'struct \1 { void }', content)
    
    # Add void to empty enum bodies  
    content = re.sub(r'enum (\w+)\s*\{\s*\}', r'enum \1 { void }', content)
    
    # Handle generic structs/enums
    content = re.sub(r'struct (\w+)<([^>]+)>\s*\{\s*\}', r'struct \1<\2> { void }', content)
    content = re.sub(r'enum (\w+)<([^>]+)>\s*\{\s*\}', r'enum \1<\2> { void }', content)
    
    return content


def simplify_imports(content):
    """Remove import aliases for simplicity"""
    
    # Remove import aliases (import "path" as alias; -> import "path";)
    content = re.sub(r'import "([^"]+)" as \w+;', r'import "\1";', content)
    
    return content


def simplify_function_calls(content):
    """Add void to empty function calls"""
    
    # Add void to empty function calls: func() -> func(void)
    # Be careful not to match function declarations
    content = re.sub(
        r'(\w+)\(\)(?=\s*[;}])',  # Function call followed by ; or }
        r'\1(void)',
        content
    )
    
    # Handle function calls in expressions and statements
    content = re.sub(
        r'(\w+)\(\)(?=\s*[+\-*/%&|^<>=!.,;\s])',  # Function call followed by operators
        r'\1(void)',
        content
    )
    
    return content


def simplify_pattern_arguments(content):
    """Add void to empty pattern arguments"""
    
    # Handle enum patterns without arguments: Option.None -> Option.None(void)
    # Match pattern like EnumName.VariantName not followed by (
    content = re.sub(
        r'(\w+)\.(\w+)(?!\s*\()',  # EnumName.VariantName not followed by (
        r'\1.\2(void)',
        content
    )
    
    return content


def simplify_annotation_parameters(content):
    """Add void to empty annotation parameters"""
    
    # Handle annotations without parameters: #[name] -> #[name(void)]
    content = re.sub(
        r'#\[(\w+)\]',  # #[annotation_name]
        r'#[\1(void)]',
        content
    )
    
    # Handle the special case of #[non_deterministic] which should stay as is
    content = re.sub(
        r'#\[non_deterministic\(void\)\]',
        r'#[non_deterministic]',
        content
    )
    
    return content


def simplify_array_elements(content):
    """Add void to empty array literals"""
    
    # Handle empty arrays: [] -> [void]
    # Be careful with type annotations like []i32
    content = re.sub(
        r'=\s*\[\s*\]',  # Assignment to empty array
        r'= [void]',
        content
    )
    
    # Handle array literals in other contexts
    content = re.sub(
        r'(?<!:)\s*\[\s*\](?!\w)',  # [] not preceded by : (type annotation)
        r' [void]',
        content
    )
    
    return content


def simplify_return_statements(content):
    """Fix return statements in void functions"""
    
    # Fix return statements in void functions: return; -> return void;
    # This is more complex as we need to identify void functions
    
    # Look for function definitions with void return type
    void_functions = re.findall(r'fn (\w+)\([^)]*\) -> void', content)
    
    for func_name in void_functions:
        # Find the function body and replace bare returns
        pattern = fr'(fn {func_name}\([^)]*\) -> void\s*\{{[^}}]*?)return\s*;'
        content = re.sub(
            pattern,
            r'\1return void;',
            content,
            flags=re.DOTALL
        )
    
    return content


def simplify_optional_elements(content):
    """Apply all optional element simplifications"""
    
    print("  - Adding explicit visibility modifiers...")
    content = simplify_visibility(content)
    
    print("  - Adding void to empty parameter lists...")
    content = simplify_parameters(content)
    
    print("  - Adding void to empty struct/enum bodies...")
    content = simplify_struct_enum_bodies(content)
    
    print("  - Removing import aliases...")
    content = simplify_imports(content)
    
    print("  - Adding void to empty function calls...")
    content = simplify_function_calls(content)
    
    print("  - Adding void to empty pattern arguments...")
    content = simplify_pattern_arguments(content)
    
    print("  - Adding void to empty annotation parameters...")
    content = simplify_annotation_parameters(content)
    
    print("  - Adding void to empty array literals...")
    content = simplify_array_elements(content)
    
    print("  - Fixing return statements in void functions...")
    content = simplify_return_statements(content)
    
    return content


def create_backup(filename):
    """Create a backup of the original file"""
    backup_name = f"{filename}.backup"
    if os.path.exists(backup_name):
        # If backup already exists, create numbered backup
        counter = 1
        while os.path.exists(f"{backup_name}.{counter}"):
            counter += 1
        backup_name = f"{backup_name}.{counter}"
    
    import shutil
    shutil.copy2(filename, backup_name)
    return backup_name


def main():
    """Main migration script"""
    
    if len(sys.argv) < 2:
        print("Usage: python3 scripts/simplify_optional_elements.py <file1.asthra> <file2.asthra> ...")
        print("       python3 scripts/simplify_optional_elements.py src/**/*.asthra")
        sys.exit(1)
    
    print("🔧 Optional Elements Simplification Migration (v1.18)")
    print("=" * 60)
    
    total_files = 0
    processed_files = 0
    
    for filename in sys.argv[1:]:
        if not filename.endswith('.asthra'):
            print(f"⚠️  Skipping non-Asthra file: {filename}")
            continue
            
        if not os.path.exists(filename):
            print(f"❌ File not found: {filename}")
            continue
        
        total_files += 1
        
        print(f"\n📄 Processing: {filename}")
        
        try:
            # Read original content
            with open(filename, 'r', encoding='utf-8') as f:
                original_content = f.read()
            
            # Create backup
            backup_name = create_backup(filename)
            print(f"  ✅ Created backup: {backup_name}")
            
            # Apply transformations
            simplified_content = simplify_optional_elements(original_content)
            
            # Write transformed content
            with open(filename, 'w', encoding='utf-8') as f:
                f.write(simplified_content)
            
            print(f"  ✅ Simplified optional elements in {filename}")
            processed_files += 1
            
        except Exception as e:
            print(f"  ❌ Error processing {filename}: {e}")
    
    print(f"\n📊 Migration Summary:")
    print(f"   Total files: {total_files}")
    print(f"   Processed: {processed_files}")
    print(f"   Errors: {total_files - processed_files}")
    
    if processed_files > 0:
        print(f"\n✅ Migration completed successfully!")
        print(f"💡 Tip: Review changes and run tests to validate the migration.")
        print(f"🔙 Backup files created for easy rollback if needed.")
    else:
        print(f"\n❌ No files were successfully processed.")


if __name__ == "__main__":
    main() 
