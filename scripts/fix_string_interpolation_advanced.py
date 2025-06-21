#!/usr/bin/env python3
"""
Advanced String Interpolation Fixer
Converts string interpolation patterns to concatenation for v1.22 grammar conformance.
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

def fix_string_interpolation_advanced(code):
    """Convert string interpolation to concatenation with sophisticated pattern matching."""
    
    def convert_interpolated_string(match):
        full_string = match.group(0)
        
        # Extract the content between quotes
        content = full_string[1:-1]  # Remove surrounding quotes
        
        # Find all interpolation expressions
        interpolation_pattern = r'\{([^}]+)\}'
        parts = []
        last_end = 0
        
        for interp_match in re.finditer(interpolation_pattern, content):
            # Add text before interpolation
            before_text = content[last_end:interp_match.start()]
            if before_text:
                parts.append(f'"{before_text}"')
            
            # Add the interpolated variable
            var_name = interp_match.group(1).strip()
            parts.append(var_name)
            
            last_end = interp_match.end()
        
        # Add remaining text after last interpolation
        after_text = content[last_end:]
        if after_text:
            parts.append(f'"{after_text}"')
        
        # Join with concatenation
        if len(parts) == 0:
            return '""'
        elif len(parts) == 1:
            return parts[0]
        else:
            return ' + '.join(parts)
    
    # Pattern to match strings with interpolation
    pattern = r'"[^"]*\{[^}]+\}[^"]*"'
    return re.sub(pattern, convert_interpolated_string, code)

def fix_remaining_empty_params(code):
    """Fix any remaining empty parameter patterns."""
    patterns = [
        # fn name() -> type
        (r'(fn\s+\w+)\s*\(\s*\)\s*(->\s*[\w<>]+)', r'\1(none) \2'),
        # fn name() {
        (r'(fn\s+\w+)\s*\(\s*\)\s*(\{)', r'\1(none) \2'),
    ]
    
    for pattern, replacement in patterns:
        code = re.sub(pattern, replacement, code)
    
    return code

def process_markdown_file(file_path, backup_dir=None, dry_run=False):
    """Process a markdown file and fix string interpolation in Asthra code examples."""
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
        
        original_code = code
        
        # Apply advanced string interpolation fixes
        code = fix_string_interpolation_advanced(code)
        code = fix_remaining_empty_params(code)
        
        if code != original_code:
            fixes_applied += 1
        
        return prefix + code + suffix
    
    new_content = re.sub(pattern, replace_code_block, content, flags=re.DOTALL)
    
    if dry_run:
        print(f"DRY RUN: Would apply {fixes_applied} fixes to {file_path}")
        return fixes_applied > 0
    
    # Write the fixed content
    if fixes_applied > 0:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(new_content)
        print(f"Applied {fixes_applied} string interpolation fixes to {file_path}")
    else:
        print(f"No string interpolation fixes needed for {file_path}")
    
    return fixes_applied > 0

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 fix_string_interpolation_advanced.py <file_path> [--backup-dir DIR] [--dry-run]")
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
        backup_dir = f"../backups/string_interpolation_fixes_{timestamp}"
    
    print(f"Processing {file_path} for string interpolation fixes")
    if dry_run:
        print("DRY RUN MODE - No changes will be made")
    elif backup_dir:
        print(f"Backup directory: {backup_dir}")
    
    success = process_markdown_file(file_path, backup_dir, dry_run)
    
    if success:
        print("✅ String interpolation fixes completed successfully")
    else:
        print("ℹ️  No string interpolation fixes were needed")

if __name__ == '__main__':
    main() 
