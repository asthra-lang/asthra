#!/usr/bin/env python3
"""
Fix Remaining Examples Script
Part of the User Manual Maintenance Plan Phase 1 Completion

This script addresses the final 23 invalid examples identified by the validation script:
- String interpolation removal (4 examples)
- Missing type annotations (18 examples) 
- Question mark operator removal (1 example)

Usage: python3 scripts/fix_remaining_examples.py
"""

import os
import re
import sys
import shutil
from pathlib import Path

# Configuration
USER_MANUAL_DIR = "docs/user-manual"
BACKUP_DIR = "temp/manual_fixes_backup"

# Statistics
total_fixes = 0
files_modified = 0

def create_backup(file_path):
    """Create backup of file before modification"""
    backup_path = Path(BACKUP_DIR) / Path(file_path).name
    backup_path.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(file_path, backup_path)
    print(f"  📁 Backup created: {backup_path}")

def fix_string_interpolation(content):
    """Fix string interpolation patterns by converting to concatenation"""
    global total_fixes
    
    # Pattern 1: Simple interpolation like "Result: {}" -> "Result: " + value
    pattern1 = r'"([^"]*)\{([^}]*)\}([^"]*)"'
    def replace1(match):
        global total_fixes
        total_fixes += 1
        before = match.group(1)
        var = match.group(2)
        after = match.group(3)
        
        if before and after:
            return f'"{before}" + {var} + "{after}"'
        elif before:
            return f'"{before}" + {var}'
        elif after:
            return f'{var} + "{after}"'
        else:
            return var
    
    content = re.sub(pattern1, replace1, content)
    
    # Pattern 2: Complex interpolation with format specifiers
    pattern2 = r'"([^"]*)\{([^}]*):([^}]*)\}([^"]*)"'
    def replace2(match):
        global total_fixes
        total_fixes += 1
        before = match.group(1)
        var = match.group(2)
        format_spec = match.group(3)
        after = match.group(4)
        
        # Convert format specifiers to method calls
        if format_spec == "?":
            var_expr = f"{var}.to_string()"
        else:
            var_expr = f"{var}.to_string()"
        
        if before and after:
            return f'"{before}" + {var_expr} + "{after}"'
        elif before:
            return f'"{before}" + {var_expr}'
        elif after:
            return f'{var_expr} + "{after}"'
        else:
            return var_expr
    
    content = re.sub(pattern2, replace2, content)
    
    return content

def fix_question_mark_operator(content):
    """Remove question mark operators and replace with explicit match expressions"""
    global total_fixes
    
    # Pattern: expression? -> match expression { Result.Ok(val) => val, Result.Err(e) => return Result.Err(e) }
    pattern = r'(\w+(?:\.\w+)*)\?'
    def replace_qmark(match):
        global total_fixes
        total_fixes += 1
        expr = match.group(1)
        return f'match {expr} {{ Result.Ok(val) => val, Result.Err(e) => return Result.Err(e) }}'
    
    content = re.sub(pattern, replace_qmark, content)
    return content

def fix_missing_type_annotations(content):
    """Add missing type annotations to variable declarations"""
    global total_fixes
    
    # Pattern 1: let variable = value; -> let variable: type = value;
    patterns = [
        # Integer literals
        (r'let (\w+) = (\d+);', r'let \1: i32 = \2;'),
        # String literals  
        (r'let (\w+) = "([^"]*)";', r'let \1: string = "\2";'),
        # Boolean literals
        (r'let (\w+) = (true|false);', r'let \1: bool = \2;'),
        # Float literals
        (r'let (\w+) = (\d+\.\d+);', r'let \1: f64 = \2;'),
        # Function calls that return Result
        (r'let (\w+) = (\w+\([^)]*\));', r'let \1: Result<string, string> = \2;'),
        # Array/slice literals
        (r'let (\w+) = \[\];', r'let \1: []string = [];'),
        # Complex expressions - use generic type
        (r'let (\w+) = ([^;]+);', r'let \1: auto = \2;'),
    ]
    
    for pattern, replacement in patterns:
        old_content = content
        content = re.sub(pattern, replacement, content)
        if content != old_content:
            total_fixes += 1
    
    return content

def fix_function_parameters(content):
    """Add missing parameter lists to functions"""
    global total_fixes
    
    # Pattern: pub fn name() -> type { -> pub fn name(none) -> type {
    pattern = r'pub fn (\w+)\(\) -> ([^{]+)\{'
    def replace_params(match):
        global total_fixes
        total_fixes += 1
        name = match.group(1)
        return_type = match.group(2).strip()
        return f'pub fn {name}(none) -> {return_type} {{'
    
    content = re.sub(pattern, replace_params, content)
    return content

def process_file(file_path):
    """Process a single markdown file"""
    global files_modified
    
    print(f"\n🔧 Processing: {file_path}")
    
    # Read file content
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
    except Exception as e:
        print(f"  ❌ Error reading file: {e}")
        return
    
    original_content = content
    
    # Create backup
    create_backup(file_path)
    
    # Apply fixes
    content = fix_string_interpolation(content)
    content = fix_question_mark_operator(content)
    content = fix_missing_type_annotations(content)
    content = fix_function_parameters(content)
    
    # Write back if changed
    if content != original_content:
        try:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(content)
            files_modified += 1
            print(f"  ✅ File updated successfully")
        except Exception as e:
            print(f"  ❌ Error writing file: {e}")
            # Restore from backup
            shutil.copy2(Path(BACKUP_DIR) / Path(file_path).name, file_path)
    else:
        print(f"  ℹ️  No changes needed")

def main():
    """Main execution function"""
    print("🚀 Fix Remaining Examples Script")
    print("=" * 50)
    print("Targeting the final 23 invalid examples:")
    print("- String interpolation removal (4 examples)")
    print("- Missing type annotations (18 examples)")
    print("- Question mark operator removal (1 example)")
    print()
    
    # Create backup directory
    Path(BACKUP_DIR).mkdir(parents=True, exist_ok=True)
    
    # Target files with known issues
    target_files = [
        "ai_generation_guidelines.md",
        "concurrency_balanced.md", 
        "concurrency.md",
        "generic_structs_migration.md",
        "introduction.md"
    ]
    
    # Process each target file
    for filename in target_files:
        file_path = Path(USER_MANUAL_DIR) / filename
        if file_path.exists():
            process_file(file_path)
        else:
            print(f"⚠️  File not found: {file_path}")
    
    # Summary
    print("\n" + "=" * 50)
    print("🎯 Fix Summary")
    print("=" * 50)
    print(f"Files processed: {len(target_files)}")
    print(f"Files modified: {files_modified}")
    print(f"Total fixes applied: {total_fixes}")
    print(f"Backup location: {BACKUP_DIR}")
    print()
    print("Next steps:")
    print("1. Run validation script: bash scripts/validate_manual_examples.sh")
    print("2. Review changes in modified files")
    print("3. Commit changes if validation passes")
    print()
    
    if total_fixes > 0:
        print("✅ Fixes applied successfully!")
        return 0
    else:
        print("ℹ️  No fixes were needed")
        return 0

if __name__ == "__main__":
    sys.exit(main()) 
