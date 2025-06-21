#!/usr/bin/env python3
"""
Field Pattern Migration Script for Asthra v1.13

This script converts shorthand field patterns to explicit field binding syntax
as required by the Field Pattern Simplification Plan Phase 2.

Usage:
    python3 scripts/migrate_field_patterns.py file1.asthra file2.asthra ...
    python3 scripts/migrate_field_patterns.py stdlib/**/*.asthra
    python3 scripts/migrate_field_patterns.py examples/**/*.asthra

Changes:
    Point { x, y } -> Point { x: x, y: y }
    Point { x: px, y } -> Point { x: px, y: y }
    Point { x, y: _ } -> Point { x: x, y: _ }
"""

import re
import sys
import os
from typing import List, Tuple

def migrate_field_patterns(content: str) -> Tuple[str, int]:
    """
    Convert shorthand field patterns to explicit form.
    
    Returns:
        Tuple of (migrated_content, num_changes)
    """
    changes_made = 0
    
    # Pattern to match struct patterns in context: StructName { fields } or just { fields }
    # Look for patterns that are likely field patterns (preceded by identifier or in match context)
    struct_pattern = r'(\w+\s*\{\s*[^}]+\s*\}|\{\s*[^}]+\s*\})'
    
    def replace_struct_fields(match):
        nonlocal changes_made
        full_match = match.group(0)
        
        # Extract just the fields part
        brace_start = full_match.find('{')
        if brace_start == -1:
            return full_match
            
        prefix = full_match[:brace_start + 1]  # "StructName {" or "{"
        fields_str = full_match[brace_start + 1:-1]  # content between braces
        suffix = full_match[-1]  # "}"
        
        # Skip if it contains array literals or function calls
        if '[' in fields_str or ('(' in fields_str and ')' in fields_str):
            return full_match
        
        # Split fields carefully
        fields = []
        current_field = ""
        in_string = False
        escape_next = False
        
        for char in fields_str + ',':  # Add comma to process last field
            if escape_next:
                escape_next = False
                current_field += char
            elif char == '\\' and in_string:
                escape_next = True
                current_field += char
            elif char == '"':
                in_string = not in_string
                current_field += char
            elif char == ',' and not in_string:
                if current_field.strip():
                    fields.append(current_field.strip())
                current_field = ""
            else:
                current_field += char
        
        migrated_fields = []
        field_changed = False
        
        for field in fields:
            field = field.strip()
            if not field:
                continue
                
            # Skip partial patterns (..)
            if field == '..' or field == '...':
                migrated_fields.append(field)
                continue
            
            # Check if field already has explicit binding
            if ':' in field:
                # Already explicit - keep as is
                migrated_fields.append(field)
            else:
                # Shorthand form - make explicit
                # field -> field: field
                if re.match(r'^[a-zA-Z_][a-zA-Z0-9_]*$', field):
                    migrated_fields.append(f"{field}: {field}")
                    field_changed = True
                else:
                    # Complex field - keep as is (might be invalid syntax)
                    migrated_fields.append(field)
        
        if field_changed:
            changes_made += 1
            return prefix + " " + ", ".join(migrated_fields) + " " + suffix
        else:
            return full_match
    
    # Apply the transformation
    migrated_content = re.sub(struct_pattern, replace_struct_fields, content)
    
    return migrated_content, changes_made

def process_file(filepath: str) -> bool:
    """
    Process a single file and migrate field patterns.
    
    Returns:
        True if file was modified, False otherwise
    """
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            original_content = f.read()
    except Exception as e:
        print(f"Error reading {filepath}: {e}")
        return False
    
    migrated_content, changes_made = migrate_field_patterns(original_content)
    
    if changes_made > 0:
        try:
            # Create backup
            backup_path = filepath + '.backup'
            with open(backup_path, 'w', encoding='utf-8') as f:
                f.write(original_content)
            
            # Write migrated content
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(migrated_content)
            
            print(f"✅ {filepath}: {changes_made} patterns migrated (backup: {backup_path})")
            return True
        except Exception as e:
            print(f"Error writing {filepath}: {e}")
            return False
    else:
        print(f"⚪ {filepath}: No changes needed")
        return False

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 migrate_field_patterns.py <file1> [file2] ...")
        print("\nExample:")
        print("  python3 migrate_field_patterns.py stdlib/**/*.asthra")
        print("  python3 migrate_field_patterns.py examples/basic.asthra")
        return 1
    
    files_to_process = []
    
    # Expand glob patterns and collect files
    for pattern in sys.argv[1:]:
        if '*' in pattern:
            # This is a glob pattern - in production, you'd use glob.glob()
            print(f"Warning: Glob pattern '{pattern}' - please expand manually")
            continue
        elif os.path.isfile(pattern):
            files_to_process.append(pattern)
        elif os.path.isdir(pattern):
            # Find .asthra files in directory
            for root, dirs, files in os.walk(pattern):
                for file in files:
                    if file.endswith('.asthra'):
                        files_to_process.append(os.path.join(root, file))
        else:
            print(f"Warning: {pattern} not found")
    
    if not files_to_process:
        print("No .asthra files found to process")
        return 1
    
    print(f"🚀 Migrating field patterns in {len(files_to_process)} files...")
    print("=" * 60)
    
    modified_count = 0
    for filepath in files_to_process:
        if process_file(filepath):
            modified_count += 1
    
    print("=" * 60)
    print(f"✅ Migration complete: {modified_count}/{len(files_to_process)} files modified")
    print("\n📋 Summary:")
    print("- Converted shorthand patterns (x, y) to explicit patterns (x: x, y: y)")
    print("- Preserved explicit patterns (x: px, y: _) unchanged")
    print("- Created .backup files for all modified files")
    print("\n🔧 Next steps:")
    print("1. Test the migrated code with 'make test'")
    print("2. Remove .backup files if everything works: rm **/*.backup")
    print("3. Commit the changes to version control")
    
    return 0

if __name__ == "__main__":
    sys.exit(main()) 
