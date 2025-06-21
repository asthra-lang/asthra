#!/usr/bin/env python3
"""
Asthra Return Statement Migration Script
Migrates bare 'return;' statements to 'return ();' for v1.14 compliance

Usage: python3 scripts/migrate_return_statements.py
"""

import os
import re
import sys
from pathlib import Path

def migrate_file(file_path):
    """Migrate return statements in a single file"""
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    original_content = content
    
    # Pattern to match bare return statements
    # Matches: return; (with possible whitespace)
    # Avoids: return expr; (already correct)
    pattern = r'\breturn\s*;'
    replacement = 'return ();'
    
    content = re.sub(pattern, replacement, content)
    
    if content != original_content:
        # Create backup
        backup_path = str(file_path) + '.backup'
        with open(backup_path, 'w', encoding='utf-8') as f:
            f.write(original_content)
        
        # Write migrated content
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(content)
        
        changes = len(re.findall(pattern, original_content))
        print(f"✅ Migrated {file_path}: {changes} return statements fixed")
        return changes
    
    return 0

def main():
    """Main migration function"""
    asthra_root = Path(__file__).parent.parent
    
    # Target directories for migration
    target_dirs = [
        asthra_root / "stdlib",
        asthra_root / "examples", 
        asthra_root / "docs" / "examples",
        asthra_root / "tests"
    ]
    
    total_files = 0
    total_changes = 0
    
    print("🚀 Starting Return Statement Migration (v1.14)")
    print("=" * 60)
    
    for target_dir in target_dirs:
        if not target_dir.exists():
            continue
            
        print(f"\n📁 Processing: {target_dir}")
        
        # Find all .asthra files
        asthra_files = list(target_dir.rglob("*.asthra"))
        
        for file_path in asthra_files:
            changes = migrate_file(file_path)
            total_changes += changes
            total_files += 1
    
    print("\n" + "=" * 60)
    print(f"✅ Migration Complete!")
    print(f"📊 Files processed: {total_files}")
    print(f"🔧 Return statements migrated: {total_changes}")
    
    if total_changes > 0:
        print(f"\n💾 Backup files created (.backup extensions)")
        print(f"📝 Please review changes before committing")

if __name__ == "__main__":
    main() 
