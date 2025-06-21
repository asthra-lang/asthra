#!/usr/bin/env python3
"""
Complete Sign Conversion Warning Fix Script
Systematically fixes all remaining sign conversion warnings in the Asthra project.
"""

import os
import re
import subprocess
import sys
from pathlib import Path
from collections import defaultdict, namedtuple
from typing import List, Dict, Tuple, Optional

# Data structures
SignConversionWarning = namedtuple('SignConversionWarning', [
    'file_path', 'line_number', 'column', 'warning_text', 'context'
])

class SignConversionFixer:
    def __init__(self):
        self.warnings = []
        self.fixes_applied = 0
        self.files_modified = set()
        
    def capture_warnings(self) -> List[SignConversionWarning]:
        """Capture all sign conversion warnings from build output."""
        print("🔍 Capturing sign conversion warnings...")
        
        try:
            # Clean build to get fresh warnings
            subprocess.run(['make', 'clean'], capture_output=True, check=True)
            
            # Build and capture warnings
            result = subprocess.run(['make'], capture_output=True, text=True)
            build_output = result.stderr + result.stdout
            
            # Parse warnings with improved regex
            warning_pattern = r'([^:]+):(\d+):(\d+):\s*warning:\s*([^[]*)\[-Wsign-conversion\]'
            
            for match in re.finditer(warning_pattern, build_output, re.MULTILINE):
                file_path = match.group(1).strip()
                line_number = int(match.group(2))
                column = int(match.group(3))
                warning_text = match.group(4).strip()
                
                warning = SignConversionWarning(
                    file_path=file_path,
                    line_number=line_number,
                    column=column,
                    warning_text=warning_text,
                    context=""
                )
                self.warnings.append(warning)
                
            print(f"📊 Found {len(self.warnings)} sign conversion warnings")
            return self.warnings
            
        except subprocess.CalledProcessError as e:
            print(f"❌ Error capturing warnings: {e}")
            return []
    
    def fix_warnings(self):
        """Fix all captured warnings systematically."""
        print("🔧 Fixing sign conversion warnings...")
        
        # Group warnings by file for efficient processing
        warnings_by_file = defaultdict(list)
        for warning in self.warnings:
            warnings_by_file[warning.file_path].append(warning)
        
        for file_path, file_warnings in warnings_by_file.items():
            self.fix_file_warnings(file_path, file_warnings)
    
    def fix_file_warnings(self, file_path: str, warnings: List[SignConversionWarning]):
        """Fix all warnings in a specific file."""
        if not os.path.exists(file_path):
            print(f"⚠️  File not found: {file_path}")
            return
        
        print(f"🔧 Fixing {len(warnings)} warnings in {file_path}")
        
        try:
            with open(file_path, 'r') as f:
                lines = f.readlines()
            
            # Sort warnings by line number in reverse order to avoid line number shifts
            warnings.sort(key=lambda w: w.line_number, reverse=True)
            
            modified = False
            for warning in warnings:
                if self.fix_single_warning(lines, warning):
                    modified = True
                    self.fixes_applied += 1
            
            if modified:
                with open(file_path, 'w') as f:
                    f.writelines(lines)
                self.files_modified.add(file_path)
                print(f"  ✅ Fixed warnings in {file_path}")
            
        except Exception as e:
            print(f"  ❌ Error fixing {file_path}: {e}")
    
    def fix_single_warning(self, lines: List[str], warning: SignConversionWarning) -> bool:
        """Fix a single sign conversion warning."""
        line_idx = warning.line_number - 1
        if line_idx >= len(lines):
            return False
        
        line = lines[line_idx]
        original_line = line
        
        # Pattern 1: int to size_t conversion (most common)
        if "implicit conversion changes signedness: 'int' to 'size_t'" in warning.warning_text:
            line = self.fix_int_to_size_t(line, warning)
        
        # Pattern 2: size_t to int conversion
        elif "implicit conversion changes signedness: 'size_t' to 'int'" in warning.warning_text:
            line = self.fix_size_t_to_int(line, warning)
        
        # Pattern 3: unsigned to signed conversion
        elif "implicit conversion changes signedness:" in warning.warning_text and "unsigned" in warning.warning_text:
            line = self.fix_unsigned_to_signed(line, warning)
        
        # Pattern 4: signed to unsigned conversion
        elif "implicit conversion changes signedness:" in warning.warning_text and "signed" in warning.warning_text:
            line = self.fix_signed_to_unsigned(line, warning)
        
        # Update line if changed
        if line != original_line:
            lines[line_idx] = line
            return True
        
        return False
    
    def fix_int_to_size_t(self, line: str, warning: SignConversionWarning) -> str:
        """Fix int to size_t conversions."""
        # Common patterns for int to size_t conversion
        patterns = [
            # Function calls with int arguments that expect size_t
            (r'(\w+)\(([^,)]*),\s*(\w+)\s*\)', r'\1(\2, (size_t)\3)'),
            (r'(\w+)\((\w+)\s*\)', r'\1((size_t)\2)'),
            
            # Array indexing with int
            (r'\[(\w+)\]', r'[(size_t)\1]'),
            
            # Assignment to size_t variables
            (r'(\w+)\s*=\s*(\w+);', r'\1 = (size_t)\2;'),
            
            # Return statements
            (r'return\s+(\w+);', r'return (size_t)\1;'),
        ]
        
        for pattern, replacement in patterns:
            new_line = re.sub(pattern, replacement, line)
            if new_line != line:
                return new_line
        
        return line
    
    def fix_size_t_to_int(self, line: str, warning: SignConversionWarning) -> str:
        """Fix size_t to int conversions."""
        patterns = [
            # Function calls with size_t arguments that expect int
            (r'(\w+)\(([^,)]*),\s*(\w+)\s*\)', r'\1(\2, (int)\3)'),
            (r'(\w+)\((\w+)\s*\)', r'\1((int)\2)'),
            
            # Assignment to int variables
            (r'(\w+)\s*=\s*(\w+);', r'\1 = (int)\2;'),
            
            # Return statements
            (r'return\s+(\w+);', r'return (int)\1;'),
        ]
        
        for pattern, replacement in patterns:
            new_line = re.sub(pattern, replacement, line)
            if new_line != line:
                return new_line
        
        return line
    
    def fix_unsigned_to_signed(self, line: str, warning: SignConversionWarning) -> str:
        """Fix unsigned to signed conversions."""
        # Extract the specific types from warning text
        type_match = re.search(r"'([^']+)' to '([^']+)'", warning.warning_text)
        if type_match:
            from_type = type_match.group(1)
            to_type = type_match.group(2)
            
            # Add explicit cast
            patterns = [
                (r'(\w+)\s*=\s*(\w+);', rf'\1 = ({to_type})\2;'),
                (r'return\s+(\w+);', rf'return ({to_type})\1;'),
                (r'(\w+)\((\w+)\)', rf'\1(({to_type})\2)'),
            ]
            
            for pattern, replacement in patterns:
                new_line = re.sub(pattern, replacement, line)
                if new_line != line:
                    return new_line
        
        return line
    
    def fix_signed_to_unsigned(self, line: str, warning: SignConversionWarning) -> str:
        """Fix signed to unsigned conversions."""
        # Extract the specific types from warning text
        type_match = re.search(r"'([^']+)' to '([^']+)'", warning.warning_text)
        if type_match:
            from_type = type_match.group(1)
            to_type = type_match.group(2)
            
            # Add explicit cast
            patterns = [
                (r'(\w+)\s*=\s*(\w+);', rf'\1 = ({to_type})\2;'),
                (r'return\s+(\w+);', rf'return ({to_type})\1;'),
                (r'(\w+)\((\w+)\)', rf'\1(({to_type})\2)'),
            ]
            
            for pattern, replacement in patterns:
                new_line = re.sub(pattern, replacement, line)
                if new_line != line:
                    return new_line
        
        return line
    
    def verify_fixes(self) -> bool:
        """Verify that fixes were successful by rebuilding."""
        print("🔍 Verifying fixes...")
        
        try:
            # Clean build
            subprocess.run(['make', 'clean'], capture_output=True, check=True)
            
            # Build and capture warnings
            result = subprocess.run(['make'], capture_output=True, text=True)
            build_output = result.stderr + result.stdout
            
            # Count remaining sign conversion warnings
            remaining_warnings = len(re.findall(r'\[-Wsign-conversion\]', build_output))
            
            print(f"📊 Remaining sign conversion warnings: {remaining_warnings}")
            
            if remaining_warnings == 0:
                print("🎉 All sign conversion warnings fixed!")
                return True
            else:
                print(f"⚠️  {remaining_warnings} warnings still remain")
                return False
                
        except subprocess.CalledProcessError as e:
            print(f"❌ Error verifying fixes: {e}")
            return False
    
    def generate_report(self):
        """Generate a summary report of fixes applied."""
        print("\n📄 Sign Conversion Fix Report")
        print("=" * 50)
        print(f"Total warnings found: {len(self.warnings)}")
        print(f"Fixes applied: {self.fixes_applied}")
        print(f"Files modified: {len(self.files_modified)}")
        print("\nModified files:")
        for file_path in sorted(self.files_modified):
            print(f"  - {file_path}")

def main():
    """Main function to fix all sign conversion warnings."""
    print("🚀 Complete Sign Conversion Warning Fix")
    print("=" * 50)
    
    fixer = SignConversionFixer()
    
    # Step 1: Capture warnings
    warnings = fixer.capture_warnings()
    if not warnings:
        print("✅ No sign conversion warnings found!")
        return 0
    
    # Step 2: Fix warnings
    fixer.fix_warnings()
    
    # Step 3: Verify fixes
    success = fixer.verify_fixes()
    
    # Step 4: Generate report
    fixer.generate_report()
    
    if success:
        print("\n🎉 SUCCESS: All sign conversion warnings have been fixed!")
        return 0
    else:
        print("\n⚠️  Some warnings may require manual review")
        return 1

if __name__ == "__main__":
    sys.exit(main()) 
