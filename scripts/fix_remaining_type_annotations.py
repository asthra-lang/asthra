#!/usr/bin/env python3
"""
Fix remaining missing type annotations in Asthra files.
Part of Type Annotation Requirement Plan - Phase 3 completion.
"""

import re
import sys
import os
from pathlib import Path

def infer_type_from_expression(expr):
    """Infer type from common expression patterns."""
    expr = expr.strip()
    
    # String literals
    if expr.startswith('"') and expr.endswith('"'):
        return "string"
    
    # Character literals
    if expr.startswith("'") and expr.endswith("'"):
        return "char"
    
    # Boolean literals
    if expr in ['true', 'false']:
        return "bool"
    
    # Integer literals
    if re.match(r'^\d+$', expr):
        return "i32"
    
    # Float literals
    if re.match(r'^\d+\.\d*$', expr) or re.match(r'^\.\d+$', expr):
        return "f64"
    
    # Function calls with known return types
    if 'open(' in expr and 'OpenMode' in expr:
        return "Result<File, FSError>"
    
    if 'read_' in expr and '_string' in expr:
        return "Result<string, FSError>"
    
    if 'read_' in expr and ('_bytes' in expr or '_to_bytes' in expr):
        return "Result<[]u8, FSError>"
    
    if 'write_' in expr:
        return "Result<void, FSError>"
    
    if 'set_file_permissions' in expr:
        return "Result<void, FSError>"
    
    if 'metadata(' in expr:
        return "Result<FileMetadata, FSError>"
    
    # OpenMode expressions
    if 'OpenMode.' in expr:
        return "OpenMode"
    
    # Conditional expressions
    if expr.startswith('if ') and ' {' in expr:
        # Look for the return type in the branches
        if 'OpenMode.' in expr:
            return "OpenMode"
        return "auto"  # Fallback for complex conditionals
    
    # Pattern match results
    if 'match ' in expr:
        return "auto"  # Complex match expressions need manual review
    
    # Default fallback
    return "auto"

def fix_type_annotations_in_file(file_path):
    """Fix missing type annotations in a single file."""
    print(f"Processing {file_path}...")
    
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    lines = content.split('\n')
    modified = False
    
    for i, line in enumerate(lines):
        # Skip comments and empty lines
        if re.match(r'^\s*//|^\s*$', line):
            continue
        
        # Look for let statements without type annotations
        match = re.match(r'^(\s*)let\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*(.+);?\s*$', line)
        if match:
            indent, var_name, expr = match.groups()
            
            # Remove trailing semicolon from expression if present
            if expr.endswith(';'):
                expr = expr[:-1].strip()
            
            # Infer the type
            inferred_type = infer_type_from_expression(expr)
            
            # Create new line with type annotation
            new_line = f"{indent}let {var_name}: {inferred_type} = {expr};"
            
            print(f"  Line {i+1}: {var_name} -> {inferred_type}")
            lines[i] = new_line
            modified = True
    
    if modified:
        # Write back to file
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write('\n'.join(lines))
        print(f"  ✅ Updated {file_path}")
        return True
    else:
        print(f"  ➖ No changes needed in {file_path}")
        return False

def main():
    """Main function to process all files."""
    print("Type Annotation Fixer for Asthra v1.15+")
    print("=" * 50)
    
    # Directories to process
    target_dirs = ['stdlib', 'examples', 'docs/examples']
    total_files = 0
    modified_files = 0
    
    for dir_name in target_dirs:
        if not os.path.exists(dir_name):
            print(f"⚠️  Directory {dir_name} not found, skipping...")
            continue
        
        print(f"\n📁 Processing directory: {dir_name}")
        
        # Find all .asthra files
        for file_path in Path(dir_name).rglob('*.asthra'):
            total_files += 1
            if fix_type_annotations_in_file(file_path):
                modified_files += 1
    
    print(f"\n📊 Summary:")
    print(f"Total files processed: {total_files}")
    print(f"Files modified: {modified_files}")
    
    if modified_files > 0:
        print("\n🎯 Next steps:")
        print("1. Run the linting script to verify fixes:")
        print("   bash scripts/check_type_annotations.sh")
        print("2. Review any remaining 'auto' types manually")
        print("3. Test compilation to ensure syntax is correct")
    else:
        print("\n✅ All files already have proper type annotations!")

if __name__ == "__main__":
    main() 
