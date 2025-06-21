#!/usr/bin/env python3
"""
Asthra Type Annotation Migration Script
Automatically adds explicit type annotations to variable declarations.

This script helps migrate Asthra code from v1.14 (optional type annotations)
to v1.15+ (mandatory type annotations) as part of the Type Annotation Requirement Plan.

Usage:
    python3 scripts/add_type_annotations.py file1.asthra file2.asthra ...
    python3 scripts/add_type_annotations.py stdlib/**/*.asthra
"""

import re
import sys
import os
from pathlib import Path

def infer_type_from_expression(expr):
    """Infer likely type from common expression patterns"""
    expr = expr.strip()
    
    # Integer literals
    if re.match(r'^\d+$', expr):
        return 'i32'
    
    # Float literals  
    if re.match(r'^\d+\.\d+$', expr):
        return 'f64'
    
    # Boolean literals
    if expr == 'true' or expr == 'false':
        return 'bool'
    
    # String literals
    if expr.startswith('"') and expr.endswith('"'):
        return 'string'
    
    # Character literals
    if expr.startswith("'") and expr.endswith("'"):
        return 'u8'
    
    # Vec creation patterns
    if re.match(r'vec!\[.*\]', expr):
        return 'Vec<auto>'
    
    # HashMap creation
    if 'HashMap::new()' in expr:
        return 'HashMap<auto, auto>'
    
    # Option patterns
    if expr == 'None':
        return 'Option<auto>'
    if expr.startswith('Some('):
        return 'Option<auto>'
    
    # Result patterns
    if expr.startswith('Ok('):
        return 'Result<auto, auto>'
    if expr.startswith('Err('):
        return 'Result<auto, auto>'
    
    # Function call patterns - try to infer from function name
    if '(' in expr and expr.endswith(')'):
        func_name = expr.split('(')[0].strip()
        
        # Common function return types
        if 'create' in func_name or 'new' in func_name:
            return 'auto'
        if 'get' in func_name or 'fetch' in func_name:
            return 'auto'
        if 'parse' in func_name:
            return 'Result<auto, auto>'
        if 'to_string' in func_name or 'format' in func_name:
            return 'string'
        if 'len' in func_name or 'size' in func_name or 'count' in func_name:
            return 'usize'
        if 'is_' in func_name or 'has_' in func_name:
            return 'bool'
    
    # Default fallback
    return 'auto'

def add_type_annotations(content, filename):
    """Add explicit type annotations to variable declarations"""
    
    lines = content.split('\n')
    modified_lines = []
    changes_made = 0
    
    for line_num, line in enumerate(lines, 1):
        original_line = line
        
        # Match let statements without type annotations
        # Pattern: let variable_name = expression;
        match = re.match(r'^(\s*)let\s+(\w+)\s*=\s*([^;]+);(.*)$', line)
        
        if match:
            indent = match.group(1)
            var_name = match.group(2)
            expression = match.group(3)
            comment = match.group(4)
            
            # Infer type from expression
            inferred_type = infer_type_from_expression(expression)
            
            # Create new line with explicit type
            new_line = f"{indent}let {var_name}: {inferred_type} = {expression};{comment}"
            
            if inferred_type == 'auto':
                # Mark for manual review
                new_line = f"{indent}let {var_name}: /* TODO: specify type */ auto = {expression};{comment}"
                print(f"  {filename}:{line_num}: Needs manual review - {var_name}")
            else:
                print(f"  {filename}:{line_num}: {var_name} -> {inferred_type}")
                
            modified_lines.append(new_line)
            changes_made += 1
        else:
            modified_lines.append(line)
    
    return '\n'.join(modified_lines), changes_made

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 add_type_annotations.py <file1> [file2] ...")
        print("Example: python3 add_type_annotations.py stdlib/**/*.asthra")
        sys.exit(1)
    
    total_files = 0
    total_changes = 0
    
    for pattern in sys.argv[1:]:
        # Handle glob patterns
        if '*' in pattern:
            files = Path().glob(pattern)
        else:
            files = [Path(pattern)]
        
        for file_path in files:
            if not file_path.exists():
                print(f"Warning: {file_path} does not exist")
                continue
                
            if not file_path.suffix in ['.asthra', '.c']:
                continue
                
            print(f"\nProcessing {file_path}...")
            
            try:
                with open(file_path, 'r', encoding='utf-8') as f:
                    content = f.read()
                
                # Create backup
                backup_path = file_path.with_suffix(file_path.suffix + '.backup')
                with open(backup_path, 'w', encoding='utf-8') as f:
                    f.write(content)
                print(f"  Created backup: {backup_path}")
                
                # Apply type annotations
                new_content, changes = add_type_annotations(content, str(file_path))
                
                if changes > 0:
                    with open(file_path, 'w', encoding='utf-8') as f:
                        f.write(new_content)
                    print(f"  ✅ Applied {changes} type annotations")
                    total_changes += changes
                else:
                    # Remove backup if no changes
                    backup_path.unlink()
                    print(f"  ⚪ No changes needed")
                
                total_files += 1
                
            except Exception as e:
                print(f"  ❌ Error processing {file_path}: {e}")
    
    print(f"\n📊 Migration Summary:")
    print(f"  Files processed: {total_files}")
    print(f"  Total changes: {total_changes}")
    print(f"  Backup files created: {total_changes}")
    
    if total_changes > 0:
        print(f"\n🔍 Next Steps:")
        print(f"  1. Review files with 'TODO: specify type' comments")
        print(f"  2. Replace 'auto' with specific types where possible")
        print(f"  3. Test compilation with: make test")
        print(f"  4. Remove .backup files when satisfied: find . -name '*.backup' -delete")

if __name__ == "__main__":
    main() 
