#!/usr/bin/env python3
"""
Fix missing function prototypes by adding them to appropriate headers.
This script systematically addresses all 86 missing prototype warnings.
"""

import os
import re
import subprocess
import sys
from pathlib import Path
from typing import Dict, List, Tuple, Optional

class PrototypeFixer:
    def __init__(self):
        self.missing_functions = {}
        self.header_mappings = {}
        self.function_signatures = {}
        
    def extract_missing_functions(self) -> Dict[str, List[str]]:
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
    
    def determine_header_file(self, function_name: str, source_file: str) -> str:
        """Determine appropriate header file for function."""
        
        # Platform functions
        if source_file == 'src/platform.c':
            return 'src/platform.h'
        
        # Runtime functions - map to modular headers
        if source_file.startswith('runtime/'):
            if 'concurrency' in source_file:
                if 'channels' in source_file:
                    return 'runtime/concurrency/asthra_concurrency_channels.h'
                elif 'patterns' in source_file:
                    return 'runtime/concurrency/asthra_concurrency_patterns.h'
                elif 'core' in source_file:
                    return 'runtime/concurrency/asthra_concurrency_core.h'
                else:
                    return 'runtime/asthra_concurrency_bridge_modular.h'
            elif 'crypto' in source_file:
                return 'runtime/crypto/asthra_runtime_crypto.h'
            elif 'memory' in source_file:
                return 'runtime/memory/asthra_runtime_memory.h'
            elif 'safety' in source_file:
                if 'concurrency' in source_file:
                    return 'runtime/safety/asthra_runtime_safety_concurrency.h'
                elif 'memory' in source_file:
                    return 'runtime/safety/asthra_runtime_safety_memory.h'
                elif 'security' in source_file:
                    return 'runtime/safety/asthra_runtime_safety_security.h'
                else:
                    return 'runtime/safety/asthra_runtime_safety_core.h'
            elif 'integration_stubs' in source_file:
                return 'runtime/asthra_integration_stubs.h'
            else:
                return 'runtime/asthra_runtime_modular.h'
        
        # Semantic analysis functions
        if source_file.startswith('src/analysis/semantic_symbols'):
            return 'src/analysis/semantic_symbols.h'
        elif source_file.startswith('src/analysis/'):
            return source_file.replace('.c', '.h')
        
        # Parser functions
        if source_file.startswith('src/parser/'):
            return source_file.replace('.c', '.h')
        
        # Codegen functions
        if source_file.startswith('src/codegen/'):
            return source_file.replace('.c', '.h')
        
        # Compiler functions
        if source_file.startswith('src/compiler/'):
            return source_file.replace('.c', '.h')
        
        # Default: same directory, .h extension
        return source_file.replace('.c', '.h')
    
    def extract_function_signature(self, function_name: str, source_file: str, line_num: int) -> Optional[str]:
        """Extract full function signature from source file."""
        try:
            with open(source_file, 'r') as f:
                lines = f.readlines()
            
            # Look for function definition around the line number
            start_line = max(0, line_num - 5)
            end_line = min(len(lines), line_num + 10)
            
            # Find the function definition
            for i in range(start_line, end_line):
                line = lines[i].strip()
                if function_name in line and '(' in line:
                    # Found potential function definition
                    signature_lines = []
                    j = i
                    
                    # Collect lines until we find the opening brace
                    while j < len(lines) and '{' not in lines[j]:
                        signature_lines.append(lines[j].rstrip())
                        j += 1
                    
                    # Clean up the signature
                    signature = ' '.join(signature_lines).strip()
                    signature = re.sub(r'\s+', ' ', signature)  # Normalize whitespace
                    
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
    
    def create_header_if_needed(self, header_file: str) -> bool:
        """Create header file if it doesn't exist."""
        if os.path.exists(header_file):
            return True
        
        # Create directory if needed
        os.makedirs(os.path.dirname(header_file), exist_ok=True)
        
        # Create basic header file
        header_name = os.path.basename(header_file).upper().replace('.', '_')
        
        content = f"""#ifndef {header_name}
#define {header_name}

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Function prototypes added by fix_missing_prototypes.py

#endif // {header_name}
"""
        
        with open(header_file, 'w') as f:
            f.write(content)
        
        print(f"📄 Created new header file: {header_file}")
        return True
    
    def add_prototype_to_header(self, header_file: str, prototype: str, function_name: str) -> bool:
        """Add function prototype to header file."""
        try:
            # Read existing content
            if os.path.exists(header_file):
                with open(header_file, 'r') as f:
                    content = f.read()
            else:
                self.create_header_if_needed(header_file)
                with open(header_file, 'r') as f:
                    content = f.read()
            
            # Check if prototype already exists
            if function_name in content:
                print(f"⏭️  Prototype for {function_name} already exists in {header_file}")
                return True
            
            # Find insertion point (before #endif)
            endif_pattern = r'#endif\s*//.*'
            endif_match = re.search(endif_pattern, content)
            
            if endif_match:
                # Insert before #endif
                insertion_point = endif_match.start()
                new_content = (
                    content[:insertion_point] + 
                    f"\n// {function_name}\n{prototype}\n\n" +
                    content[insertion_point:]
                )
            else:
                # Append to end
                new_content = content + f"\n// {function_name}\n{prototype}\n"
            
            # Write back
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
                
                # Create header if needed
                self.create_header_if_needed(header_file)
                
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
            return False

def main():
    """Main execution function."""
    print("🚀 Starting Missing Prototypes Fix")
    print("=" * 50)
    
    fixer = PrototypeFixer()
    
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
