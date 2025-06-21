#!/usr/bin/env python3
"""
Validates that all return statements follow v1.14 syntax
"""

import re
import sys
from pathlib import Path

def validate_file(file_path):
    """Check file for old-style return statements"""
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Find bare return statements
    bare_returns = re.findall(r'\breturn\s*;', content)
    
    if bare_returns:
        print(f"❌ {file_path}: Found {len(bare_returns)} bare return statements")
        return False
    
    return True

def main():
    """Main validation function"""
    asthra_root = Path(__file__).parent.parent
    
    target_dirs = [
        asthra_root / "stdlib",
        asthra_root / "examples",
        asthra_root / "docs" / "examples",
        asthra_root / "tests"
    ]
    
    all_valid = True
    total_files = 0
    
    print("🔍 Validating Return Statement Migration")
    print("=" * 50)
    
    for target_dir in target_dirs:
        if not target_dir.exists():
            continue
            
        asthra_files = list(target_dir.rglob("*.asthra"))
        
        for file_path in asthra_files:
            if not validate_file(file_path):
                all_valid = False
            total_files += 1
    
    print(f"\n📊 Validation Results:")
    print(f"Files checked: {total_files}")
    
    if all_valid:
        print("✅ All return statements follow v1.14 syntax!")
        return 0
    else:
        print("❌ Migration incomplete - bare return statements found")
        return 1

if __name__ == "__main__":
    sys.exit(main()) 
