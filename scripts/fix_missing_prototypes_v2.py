#!/usr/bin/env python3
"""
Fix missing function prototypes by adding them to appropriate headers.
Improved version that properly extracts function signatures.
"""

import os
import re
import subprocess
import sys
from typing import Dict, List, Tuple, Optional

class PrototypeFixerV2:
    def __init__(self):
        self.missing_functions = {}
        
    def extract_missing_functions(self) -> Dict[str, List[Tuple[str, int]]]:
        """Extract function names and their source files from compilation warnings."""
        print("🔍 Extracting missing prototype warnings...")
        
        # Run build and capture warnings
        result = subprocess.run(['make', 'clean'], capture_output=True, text=True)
        result = subprocess.run(['make'], capture_output=True, text=True, cwd='.')
        
        missing_functions = {}
        
        # Parse warnings from stderr
        warning_pattern = r'([^:]+):(\d+):(\d+): warning: no previous prototype for function \'([^\']+)\''
        
        for line in result.stderr.split('\n'):
            match = re.search(warning_pattern, line)
            if match:
                source_file = match.group(1)
                line_num = int(match.group(2))
                function_name = match.group(4)
                
                if source_file not in missing_functions:
                    missing_functions[source_file] = []
                missing_functions[source_file].append((function_name, line_num))
        
        self.missing_functions = missing_functions
        print(f"✅ Found {sum(len(funcs) for funcs in missing_functions.values())} missing prototypes in {len(missing_functions)} files")
        return missing_functions
    
    def extract_function_signature(self, function_name: str, source_file: str, line_num: int) -> Optional[str]:
        """Extract full function signature from source file."""
        try:
            with open(source_file, 'r') as f:
                lines = f.readlines()
            
            # Look for function definition starting from the warning line
            for i in range(max(0, line_num - 3), min(len(lines), line_num + 3)):
                line = lines[i].strip()
                if function_name in line and '(' in line and not line.startswith('//'):
                    # Found the function definition line
                    signature_lines = []
                    j = i
                    
                    # Collect lines until we find the opening brace
                    brace_count = 0
                    paren_count = 0
                    in_function = False
                    
                    while j < len(lines):
                        current_line = lines[j].rstrip()
                        signature_lines.append(current_line)
                        
                        # Count parentheses to find complete function signature
                        for char in current_line:
                            if char == '(':
                                paren_count += 1
                                in_function = True
                            elif char == ')':
                                paren_count -= 1
                            elif char == '{' and in_function and paren_count == 0:
                                brace_count += 1
                                break
                        
                        if brace_count > 0 or (in_function and paren_count == 0 and '{' in current_line):
                            break
                            
                        j += 1
                        if j - i > 10:  # Safety limit
                            break
                    
                    # Clean up the signature
                    signature = ' '.join(signature_lines).strip()
                    
                    # Remove the opening brace and everything after
                    if '{' in signature:
                        signature = signature[:signature.index('{')].strip()
                    
                    # Normalize whitespace
                    signature = re.sub(r'\s+', ' ', signature)
                    
                    # Remove any leading keywords that shouldn't be in prototype
                    signature = re.sub(r'^(static\s+|inline\s+)', '', signature)
                    
                    # Ensure it ends with semicolon for prototype
                    if not signature.endswith(';'):
                        signature += ';'
                    
                    return signature
            
            return None
            
        except Exception as e:
            print(f"⚠️  Error extracting signature for {function_name} in {source_file}: {e}")
            return None
    
    def determine_header_file(self, function_name: str, source_file: str) -> str:
        """Determine appropriate header file for function."""
        
        # Platform functions
        if source_file == 'src/platform.c':
            return 'src/platform.h'
        
        # For most cases, just use the corresponding .h file
        if source_file.endswith('.c'):
            header_file = source_file[:-2] + '.h'
            return header_file
        
        # Default fallback
        return source_file.replace('.c', '.h')
    
    def add_prototype_to_header(self, header_file: str, prototype: str, function_name: str) -> bool:
        """Add function prototype to header file."""
        try:
            # Read existing content
            if not os.path.exists(header_file):
                print(f"⚠️  Header file {header_file} does not exist, skipping {function_name}")
                return False
                
            with open(header_file, 'r') as f:
                content = f.read()
            
            # Check if prototype already exists
            if function_name in content:
                print(f"⏭️  Prototype for {function_name} already exists in {header_file}")
                return True
            
            # Find a good insertion point
            # Look for the end of existing function declarations
            lines = content.split('\n')
            insertion_line = -1
            
            # Find the last function declaration or before #endif
            for i in range(len(lines) - 1, -1, -1):
                line = lines[i].strip()
                if line.startswith('#endif'):
                    insertion_line = i
                    break
                elif (line.endswith(';') and 
                      ('(' in line and ')' in line) and 
                      not line.startswith('#') and 
                      not line.startswith('//')):
                    insertion_line = i + 1
                    break
            
            if insertion_line == -1:
                # Fallback: add before the last #endif or at the end
                for i in range(len(lines) - 1, -1, -1):
                    if lines[i].strip().startswith('#endif'):
                        insertion_line = i
                        break
                if insertion_line == -1:
                    insertion_line = len(lines)
            
            # Insert the prototype
            lines.insert(insertion_line, f"\n// {function_name}")
            lines.insert(insertion_line + 1, prototype)
            
            # Write back
            new_content = '\n'.join(lines)
            with open(header_file, 'w') as f:
                f.write(new_content)
            
            print(f"✅ Added {function_name} prototype to {header_file}")
            return True
            
        except Exception as e:
            print(f"❌ Error adding prototype to {header_file}: {e}")
            return False
    
    def fix_all_prototypes(self) -> bool:
        """Fix all missing prototypes."""
        print("🔧 Starting prototype fixes...")
        
        success_count = 0
        total_count = 0
        
        for source_file, functions in self.missing_functions.items():
            print(f"\n📁 Processing {source_file} ({len(functions)} functions)")
            
            for function_name, line_num in functions:
                total_count += 1
                print(f"  🔍 Processing {function_name}...")
                
                # Determine header file
                header_file = self.determine_header_file(function_name, source_file)
                
                # Extract function signature
                signature = self.extract_function_signature(function_name, source_file, line_num)
                
                if not signature:
                    print(f"    ⚠️  Could not extract signature for {function_name}")
                    continue
                
                print(f"    📝 Signature: {signature}")
                
                # Add prototype
                if self.add_prototype_to_header(header_file, signature, function_name):
                    success_count += 1
                else:
                    print(f"    ❌ Failed to add prototype for {function_name}")
        
        print(f"\n🎯 Results: {success_count}/{total_count} prototypes added successfully")
        return success_count == total_count
    
    def verify_fixes(self) -> bool:
        """Verify that fixes worked by rebuilding."""
        print("\n🔍 Verifying fixes...")
        
        # Clean and rebuild
        subprocess.run(['make', 'clean'], capture_output=True)
        result = subprocess.run(['make'], capture_output=True, text=True)
        
        # Count remaining missing prototype warnings
        remaining_warnings = result.stderr.count('no previous prototype')
        
        print(f"📊 Remaining missing prototype warnings: {remaining_warnings}")
        
        if remaining_warnings == 0:
            print("🎉 All missing prototype warnings fixed!")
            return True
        else:
            print(f"⚠️  {remaining_warnings} warnings still remain")
            # Show first few remaining warnings for debugging
            remaining_lines = [line for line in result.stderr.split('\n') if 'no previous prototype' in line]
            for line in remaining_lines[:5]:
                print(f"    {line}")
            return False

def main():
    """Main execution function."""
    print("🚀 Starting Missing Prototypes Fix V2")
    print("=" * 50)
    
    fixer = PrototypeFixerV2()
    
    # Step 1: Extract missing functions
    missing_functions = fixer.extract_missing_functions()
    
    if not missing_functions:
        print("✅ No missing prototype warnings found!")
        return 0
    
    # Step 2: Fix all prototypes
    success = fixer.fix_all_prototypes()
    
    if not success:
        print("❌ Some prototypes could not be fixed")
        return 1
    
    # Step 3: Verify fixes
    if fixer.verify_fixes():
        print("\n🎉 Phase 2 Complete: All missing prototype warnings fixed!")
        return 0
    else:
        print("\n⚠️  Some warnings may still remain - manual review needed")
        return 1

if __name__ == "__main__":
    sys.exit(main()) 
