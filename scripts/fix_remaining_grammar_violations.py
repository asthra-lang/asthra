#!/usr/bin/env python3
"""
Targeted fix script for remaining grammar violations in documentation files.
Addresses specific patterns that the general fix script missed.
"""

import re
import sys
import os
from pathlib import Path

def fix_empty_parameters_in_examples(content):
    """Fix empty parameter lists in code examples within markdown."""
    fixes_applied = 0
    
    # Pattern for function declarations with empty parameters
    patterns = [
        # Standard function declarations
        (r'(fn\s+\w+)\s*\(\s*\)\s*(->\s*[^{]+)', r'\1(none) \2'),
        # Extern function declarations
        (r'(extern\s+"C"\s+fn\s+\w+)\s*\(\s*\)\s*(->\s*[^;]+)', r'\1(none) \2'),
        # Public function declarations
        (r'(pub\s+fn\s+\w+)\s*\(\s*\)\s*(->\s*[^{]+)', r'\1(none) \2'),
        # Private function declarations
        (r'(priv\s+fn\s+\w+)\s*\(\s*\)\s*(->\s*[^{]+)', r'\1(none) \2'),
    ]
    
    for pattern, replacement in patterns:
        new_content, count = re.subn(pattern, replacement, content)
        if count > 0:
            content = new_content
            fixes_applied += count
            print(f"  Fixed {count} empty parameter lists with pattern: {pattern}")
    
    return content, fixes_applied

def fix_string_interpolation_in_examples(content):
    """Fix string interpolation patterns in code examples."""
    fixes_applied = 0
    
    # More specific patterns for string interpolation
    patterns = [
        # Simple variable interpolation
        (r'"([^"]*)\{(\w+)\}([^"]*)"', r'"\1" + \2 + "\3"'),
        # Complex interpolation with expressions
        (r'"([^"]*)\{([^}]+)\}([^"]*)"', r'"\1" + (\2) + "\3"'),
    ]
    
    for pattern, replacement in patterns:
        # Apply the pattern multiple times to handle multiple interpolations in one string
        prev_content = ""
        while prev_content != content:
            prev_content = content
            new_content, count = re.subn(pattern, replacement, content)
            if count > 0:
                content = new_content
                fixes_applied += count
                print(f"  Fixed {count} string interpolation patterns")
    
    return content, fixes_applied

def fix_markdown_file(file_path):
    """Fix grammar violations in a markdown file."""
    print(f"Processing {file_path}")
    
    # Read the file
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    original_content = content
    total_fixes = 0
    
    # Apply fixes
    content, param_fixes = fix_empty_parameters_in_examples(content)
    total_fixes += param_fixes
    
    content, interp_fixes = fix_string_interpolation_in_examples(content)
    total_fixes += interp_fixes
    
    # Write back if changes were made
    if content != original_content:
        # Create backup
        backup_dir = Path("../backups/targeted_grammar_fixes")
        backup_dir.mkdir(parents=True, exist_ok=True)
        backup_path = backup_dir / Path(file_path).name
        
        with open(backup_path, 'w', encoding='utf-8') as f:
            f.write(original_content)
        print(f"  Created backup: {backup_path}")
        
        # Write fixed content
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(content)
        
        print(f"  Applied {total_fixes} total fixes")
        return total_fixes
    else:
        print("  No fixes needed")
        return 0

def main():
    """Main function to process files."""
    if len(sys.argv) < 2:
        print("Usage: python3 fix_remaining_grammar_violations.py <file1> [file2] ...")
        sys.exit(1)
    
    total_fixes_all = 0
    
    for file_path in sys.argv[1:]:
        if not os.path.exists(file_path):
            print(f"Error: File {file_path} does not exist")
            continue
        
        fixes = fix_markdown_file(file_path)
        total_fixes_all += fixes
    
    print(f"\n✅ Processing completed. Total fixes applied: {total_fixes_all}")

if __name__ == "__main__":
    main() 
