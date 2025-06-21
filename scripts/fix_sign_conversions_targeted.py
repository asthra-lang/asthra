#!/usr/bin/env python3
"""
Targeted Sign Conversion Warning Fix Script
Fixes specific sign conversion patterns found in the Asthra codebase.
"""

import os
import re
import subprocess
import sys
from pathlib import Path

class TargetedSignConversionFixer:
    def __init__(self):
        self.fixes_applied = 0
        self.files_modified = set()
        
    def fix_all_files(self):
        """Fix sign conversion warnings in all relevant files."""
        print("🔧 Fixing sign conversion warnings...")
        
        # Get list of files with sign conversion warnings
        files_to_fix = self.get_files_with_warnings()
        
        for file_path in files_to_fix:
            if self.fix_file(file_path):
                self.files_modified.add(file_path)
        
        print(f"✅ Fixed {self.fixes_applied} warnings in {len(self.files_modified)} files")
    
    def get_files_with_warnings(self):
        """Get list of files that have sign conversion warnings."""
        try:
            result = subprocess.run(['make', 'clean'], capture_output=True, check=True)
            result = subprocess.run(['make'], capture_output=True, text=True)
            build_output = result.stderr + result.stdout
            
            # Extract file paths from warnings
            files = set()
            for line in build_output.split('\n'):
                if 'sign-conversion' in line and '.c:' in line:
                    match = re.search(r'([^:]+\.c):', line)
                    if match:
                        files.add(match.group(1))
            
            return list(files)
        except:
            # Fallback to known problematic files
            return [
                'src/ai_annotations/ai_annotation_processor.c',
                'src/ai_api/ast_navigation.c',
                'src/ai_linter/guidance_output.c',
                'src/analysis/type_info_core.c',
                'src/analysis/type_info_creators.c',
                'src/analysis/type_info_debug.c',
                'src/analysis/type_info_queries.c'
            ]
    
    def fix_file(self, file_path):
        """Fix sign conversion warnings in a specific file."""
        if not os.path.exists(file_path):
            return False
        
        print(f"🔧 Processing {file_path}")
        
        try:
            with open(file_path, 'r') as f:
                content = f.read()
            
            original_content = content
            
            # Apply specific fixes based on file patterns
            content = self.fix_snprintf_offset_pattern(content)
            content = self.fix_size_t_conversions(content)
            content = self.fix_enum_assignments(content)
            content = self.fix_array_indexing(content)
            content = self.fix_function_calls(content)
            
            if content != original_content:
                with open(file_path, 'w') as f:
                    f.write(content)
                print(f"  ✅ Fixed warnings in {file_path}")
                return True
            else:
                print(f"  ℹ️  No changes needed in {file_path}")
                return False
                
        except Exception as e:
            print(f"  ❌ Error processing {file_path}: {e}")
            return False
    
    def fix_snprintf_offset_pattern(self, content):
        """Fix snprintf calls where int offset is used with size_t buffer_size."""
        # Pattern: snprintf(buffer + offset, buffer_size - offset, ...)
        # Fix: Cast offset to size_t
        pattern = r'snprintf\(([^,]+)\s*\+\s*(\w+),\s*([^,]+)\s*-\s*(\w+),'
        
        def replace_snprintf(match):
            buffer = match.group(1)
            offset1 = match.group(2)
            buffer_size = match.group(3)
            offset2 = match.group(4)
            
            if offset1 == offset2:  # Same offset variable
                self.fixes_applied += 1
                return f'snprintf({buffer} + (size_t){offset1}, {buffer_size} - (size_t){offset2},'
            return match.group(0)
        
        return re.sub(pattern, replace_snprintf, content)
    
    def fix_size_t_conversions(self, content):
        """Fix size_t to int and int to size_t conversions."""
        fixes = [
            # size_t variables assigned to int
            (r'(\w+)\s*=\s*([^;]+->[\w_]+);', self.fix_size_t_assignment),
            
            # Function calls with size_t arguments expecting int
            (r'(\w+)\(([^,)]*),\s*([^,)]+)\)', self.fix_function_call_args),
        ]
        
        for pattern, replacer in fixes:
            content = re.sub(pattern, replacer, content)
        
        return content
    
    def fix_size_t_assignment(self, match):
        """Fix size_t assignments that cause sign conversion warnings."""
        var_name = match.group(1)
        assignment = match.group(2)
        
        # If assignment looks like it's from a size_t source to int destination
        if any(keyword in assignment for keyword in ['_count', '_size', 'length', 'strlen']):
            self.fixes_applied += 1
            return f'{var_name} = (int){assignment};'
        
        return match.group(0)
    
    def fix_function_call_args(self, match):
        """Fix function call arguments that cause sign conversion."""
        func_name = match.group(1)
        first_arg = match.group(2)
        second_arg = match.group(3)
        
        # Common patterns that need casting
        if any(func in func_name for func in ['snprintf', 'printf', 'fprintf']):
            # Check if second_arg looks like it needs casting
            if re.match(r'\w+', second_arg.strip()):
                self.fixes_applied += 1
                return f'{func_name}({first_arg}, (size_t){second_arg})'
        
        return match.group(0)
    
    def fix_enum_assignments(self, content):
        """Fix enum assignments that cause sign conversion warnings."""
        # Pattern: variable = enum_value;
        pattern = r'(\w+)\s*=\s*([A-Z_]+);'
        
        def replace_enum(match):
            var_name = match.group(1)
            enum_value = match.group(2)
            
            # Common enum patterns that need casting
            if any(prefix in enum_value for prefix in ['PRIMITIVE_', 'TYPE_', 'SYMBOL_']):
                self.fixes_applied += 1
                return f'{var_name} = (int){enum_value};'
            
            return match.group(0)
        
        return re.sub(pattern, replace_enum, content)
    
    def fix_array_indexing(self, content):
        """Fix array indexing with sign conversion issues."""
        # Pattern: array[variable] where variable might need casting
        pattern = r'\[(\w+)\]'
        
        def replace_index(match):
            index_var = match.group(1)
            
            # If index variable looks like it might be signed when array expects unsigned
            if index_var in ['i', 'j', 'k', 'index', 'idx']:
                self.fixes_applied += 1
                return f'[(size_t){index_var}]'
            
            return match.group(0)
        
        # Only apply to specific contexts to avoid over-fixing
        lines = content.split('\n')
        for i, line in enumerate(lines):
            if 'for (' in line and '[' in line:
                lines[i] = re.sub(pattern, replace_index, line)
        
        return '\n'.join(lines)
    
    def fix_function_calls(self, content):
        """Fix specific function calls that cause sign conversion warnings."""
        fixes = [
            # type_info_stats_increment with enum argument
            (r'type_info_stats_increment\(([^)]+)\)', r'type_info_stats_increment((int)\1)'),
            
            # String conversion functions
            (r'(\w+_to_string)\(([^)]+)\)', r'\1((int)\2)'),
        ]
        
        for pattern, replacement in fixes:
            new_content = re.sub(pattern, replacement, content)
            if new_content != content:
                self.fixes_applied += 1
                content = new_content
        
        return content
    
    def verify_fixes(self):
        """Verify that fixes were successful."""
        print("🔍 Verifying fixes...")
        
        try:
            subprocess.run(['make', 'clean'], capture_output=True, check=True)
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
                # Show some examples of remaining warnings
                warning_lines = [line for line in build_output.split('\n') if 'sign-conversion' in line]
                for line in warning_lines[:5]:  # Show first 5
                    print(f"  {line}")
                return False
                
        except subprocess.CalledProcessError as e:
            print(f"❌ Error verifying fixes: {e}")
            return False

def main():
    """Main function."""
    print("🚀 Targeted Sign Conversion Warning Fix")
    print("=" * 50)
    
    fixer = TargetedSignConversionFixer()
    
    # Fix all files
    fixer.fix_all_files()
    
    # Verify fixes
    success = fixer.verify_fixes()
    
    # Report results
    print(f"\n📄 Summary:")
    print(f"  Fixes applied: {fixer.fixes_applied}")
    print(f"  Files modified: {len(fixer.files_modified)}")
    
    if fixer.files_modified:
        print("  Modified files:")
        for file_path in sorted(fixer.files_modified):
            print(f"    - {file_path}")
    
    if success:
        print("\n🎉 SUCCESS: All sign conversion warnings have been fixed!")
        return 0
    else:
        print("\n⚠️  Some warnings may require manual review")
        return 1

if __name__ == "__main__":
    sys.exit(main()) 
