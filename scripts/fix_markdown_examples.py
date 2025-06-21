#!/usr/bin/env python3
"""
Markdown Example Grammar Fixer
Fixes Asthra code examples in markdown files for v1.22 grammar conformance.
"""

import re
import sys
import os
import shutil
from datetime import datetime

def backup_file(file_path, backup_dir):
    """Create a backup of the file."""
    if not os.path.exists(backup_dir):
        os.makedirs(backup_dir)
    
    backup_path = os.path.join(backup_dir, os.path.basename(file_path))
    shutil.copy2(file_path, backup_path)
    return backup_path

def fix_string_interpolation(code):
    """Convert string interpolation to concatenation."""
    # Pattern: "text {variable} more text" -> "text " + variable + " more text"
    def replace_interpolation(match):
        full_string = match.group(0)
        # Simple replacement for basic cases
        # This is a simplified version - the full fixer has more sophisticated logic
        if '{' in full_string and '}' in full_string:
            # For now, just add a comment indicating manual fix needed
            return full_string + ' // TODO: Convert string interpolation to concatenation'
        return full_string
    
    pattern = r'"[^"]*\{[^}]+\}[^"]*"'
    return re.sub(pattern, replace_interpolation, code)

def fix_empty_parameters(code):
    """Fix empty parameter lists to use (none)."""
    # Pattern: fn name() -> type { -> fn name(none) -> type {
    pattern = r'(fn\s+\w+)\s*\(\s*\)\s*(->\s*\w+\s*\{)'
    replacement = r'\1(none) \2'
    return re.sub(pattern, replacement, code)

def fix_visibility_modifiers(code):
    """Add missing visibility modifiers."""
    # Pattern: fn name -> pub fn name (for examples, use pub as default)
    patterns = [
        (r'^(fn\s+\w+)', r'pub \1'),
        (r'^(struct\s+\w+)', r'pub \1'),
        (r'^(enum\s+\w+)', r'pub \1'),
    ]
    
    for pattern, replacement in patterns:
        code = re.sub(pattern, replacement, code, flags=re.MULTILINE)
    
    return code

def fix_return_void(code):
    """Fix return void; to return;."""
    pattern = r'return\s+void\s*;'
    replacement = 'return;'
    return re.sub(pattern, replacement, code)

def fix_auto_types(code):
    """Fix auto type annotations (basic cases)."""
    # This is simplified - the full fixer has sophisticated type inference
    patterns = [
        (r':\s*auto\s*=\s*(\d+)', r': i32 = \1'),  # auto = number -> i32
        (r':\s*auto\s*=\s*"[^"]*"', r': string = '),  # auto = string -> string
        (r':\s*auto\s*=\s*true|false', r': bool = '),  # auto = bool -> bool
    ]
    
    for pattern, replacement in patterns:
        code = re.sub(pattern, replacement, code)
    
    return code

def fix_asthra_code_block(code_block):
    """Apply all fixes to a single Asthra code block."""
    original = code_block
    
    # Apply fixes in order
    code_block = fix_string_interpolation(code_block)
    code_block = fix_empty_parameters(code_block)
    code_block = fix_visibility_modifiers(code_block)
    code_block = fix_return_void(code_block)
    code_block = fix_auto_types(code_block)
    
    return code_block, code_block != original

def process_markdown_file(file_path, backup_dir=None, dry_run=False):
    """Process a markdown file and fix all Asthra code examples."""
    if not os.path.exists(file_path):
        print(f"Error: File '{file_path}' not found")
        return False
    
    # Read the file
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Create backup if requested
    if backup_dir and not dry_run:
        backup_path = backup_file(file_path, backup_dir)
        print(f"Created backup: {backup_path}")
    
    # Find and fix all Asthra code blocks
    pattern = r'(```asthra\n)(.*?)(\n```)'
    fixes_applied = 0
    
    def replace_code_block(match):
        nonlocal fixes_applied
        prefix = match.group(1)
        code = match.group(2)
        suffix = match.group(3)
        
        fixed_code, was_changed = fix_asthra_code_block(code)
        if was_changed:
            fixes_applied += 1
        
        return prefix + fixed_code + suffix
    
    new_content = re.sub(pattern, replace_code_block, content, flags=re.DOTALL)
    
    if dry_run:
        print(f"DRY RUN: Would apply {fixes_applied} fixes to {file_path}")
        return fixes_applied > 0
    
    # Write the fixed content
    if fixes_applied > 0:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(new_content)
        print(f"Applied {fixes_applied} fixes to {file_path}")
    else:
        print(f"No fixes needed for {file_path}")
    
    return fixes_applied > 0

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 fix_markdown_examples.py <file_path> [--backup-dir DIR] [--dry-run]")
        sys.exit(1)
    
    file_path = sys.argv[1]
    backup_dir = None
    dry_run = False
    
    # Parse arguments
    i = 2
    while i < len(sys.argv):
        if sys.argv[i] == '--backup-dir' and i + 1 < len(sys.argv):
            backup_dir = sys.argv[i + 1]
            i += 2
        elif sys.argv[i] == '--dry-run':
            dry_run = True
            i += 1
        else:
            print(f"Unknown argument: {sys.argv[i]}")
            sys.exit(1)
    
    # Set default backup directory
    if backup_dir is None and not dry_run:
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        backup_dir = f"../backups/markdown_fixes_{timestamp}"
    
    print(f"Processing {file_path}")
    if dry_run:
        print("DRY RUN MODE - No changes will be made")
    elif backup_dir:
        print(f"Backup directory: {backup_dir}")
    
    success = process_markdown_file(file_path, backup_dir, dry_run)
    
    if success:
        print("✅ Processing completed successfully")
    else:
        print("ℹ️  No changes were needed")

if __name__ == '__main__':
    main() 
