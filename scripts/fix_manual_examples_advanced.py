#!/usr/bin/env python3

"""
Advanced User Manual Example Fix Script
Handles complex cases that the basic script missed

Remaining issues (34 examples):
1. Complex string concatenation patterns
2. Multi-line let statements
3. Complex function signatures
4. Nested expressions with missing types
"""

import os
import re
import sys
import argparse
from pathlib import Path
from typing import List, Tuple, Dict

class AdvancedManualExampleFixer:
    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.user_manual_dir = self.project_root / "docs" / "user-manual"
        self.fixes_applied = 0
        self.files_modified = 0
        
    def fix_complex_string_interpolation(self, content: str) -> str:
        """Fix complex string interpolation patterns that the basic script missed"""
        fixes = 0
        
        # Fix patterns like: "text {var} more {var2} text"
        def replace_complex_interpolation(match):
            nonlocal fixes
            fixes += 1
            full_string = match.group(0)
            
            # Handle complex interpolation with multiple variables
            # Convert "User {user_id} logged in at {timestamp}" 
            # to "User " + user_id + " logged in at " + timestamp
            
            parts = []
            i = 1  # Skip opening quote
            
            while i < len(full_string) - 1:  # Skip closing quote
                # Find next {
                brace_start = full_string.find('{', i)
                if brace_start == -1:
                    # No more braces, add rest
                    remaining = full_string[i:-1]
                    if remaining:
                        parts.append(f'"{remaining}"')
                    break
                
                # Add text before brace
                before = full_string[i:brace_start]
                if before:
                    parts.append(f'"{before}"')
                
                # Find closing brace
                brace_end = full_string.find('}', brace_start)
                if brace_end == -1:
                    break
                
                # Add variable
                var = full_string[brace_start + 1:brace_end]
                parts.append(var)
                
                i = brace_end + 1
            
            return ' + '.join(parts) if len(parts) > 1 else (parts[0] if parts else '""')
        
        # Match complex interpolation patterns
        patterns = [
            r'"[^"]*\{[^}]+\}[^"]*\{[^}]+\}[^"]*"',  # Multiple interpolations
            r'"[^"]*\{[^}]+\.[^}]+\}[^"]*"',         # Property access in interpolation
            r'"[^"]*\{[^}]+\([^}]*\)\}[^"]*"',       # Function calls in interpolation
        ]
        
        for pattern in patterns:
            content = re.sub(pattern, replace_complex_interpolation, content)
        
        self.fixes_applied += fixes
        return content
    
    def fix_multiline_let_statements(self, content: str) -> str:
        """Fix let statements that span multiple lines"""
        fixes = 0
        
        # Pattern: let var = \n    expression;
        def fix_multiline_let(match):
            nonlocal fixes
            fixes += 1
            var_name = match.group(1)
            expression = match.group(2).strip()
            
            # Determine type based on expression
            if 'Result.Ok' in expression or 'Result.Err' in expression:
                type_annotation = ': Result<i32, String>'
            elif 'Some(' in expression or expression == 'None':
                type_annotation = ': Option<i32>'
            elif expression.startswith('"') and expression.endswith('"'):
                type_annotation = ': String'
            elif expression.isdigit():
                type_annotation = ': i32'
            elif '.' in expression and expression.replace('.', '').isdigit():
                type_annotation = ': f64'
            elif expression in ['true', 'false']:
                type_annotation = ': bool'
            else:
                type_annotation = ': String'  # Default fallback
            
            return f'let {var_name}{type_annotation} = {expression};'
        
        # Match multiline let statements
        pattern = r'let\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*\n\s*([^;]+);'
        content = re.sub(pattern, fix_multiline_let, content, flags=re.MULTILINE)
        
        self.fixes_applied += fixes
        return content
    
    def fix_complex_expressions(self, content: str) -> str:
        """Fix complex expressions that need type annotations"""
        fixes = 0
        
        # Fix function call assignments
        patterns = [
            # let x = function_call();
            (r'let\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*([a-zA-Z_][a-zA-Z0-9_:]*)\([^)]*\);', 
             r'let \1: String = \2();'),
            
            # let x = object.method();
            (r'let\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*([a-zA-Z_][a-zA-Z0-9_]*\.[a-zA-Z_][a-zA-Z0-9_]*\([^)]*\));', 
             r'let \1: String = \2;'),
            
            # let x = array[index];
            (r'let\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*([a-zA-Z_][a-zA-Z0-9_]*\[[^\]]*\]);', 
             r'let \1: String = \2;'),
        ]
        
        for pattern, replacement in patterns:
            new_content, count = re.subn(pattern, replacement, content)
            if count > 0:
                content = new_content
                fixes += count
        
        self.fixes_applied += fixes
        return content
    
    def fix_string_concatenation_artifacts(self, content: str) -> str:
        """Fix artifacts from string concatenation conversion"""
        fixes = 0
        
        # Fix patterns like: " + " that should be concatenated
        patterns = [
            # Fix broken concatenation: " + " -> proper concatenation
            (r'"\s*\+\s*"', ''),
            
            # Fix extra quotes: ""text"" -> "text"
            (r'""([^"]+)""', r'"\1"'),
            
            # Fix space issues: "text " +"more" -> "text " + "more"
            (r'"\s*\+\s*"([^"]*)"', r' + "\1"'),
        ]
        
        for pattern, replacement in patterns:
            new_content, count = re.subn(pattern, replacement, content)
            if count > 0:
                content = new_content
                fixes += count
        
        self.fixes_applied += fixes
        return content
    
    def fix_question_mark_artifacts(self, content: str) -> str:
        """Fix question mark operator artifacts"""
        fixes = 0
        
        # Fix the weird "match Asthra { Ok(val) => val, Err(e) => return Err(e) }" pattern
        # This seems to be from a broken replacement
        content = content.replace(
            'match Asthra { Ok(val) => val, Err(e) => return Err(e) }',
            'Asthra'
        )
        
        # Fix other question mark artifacts
        patterns = [
            (r'match\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\{\s*Ok\(val\)\s*=>\s*val,\s*Err\(e\)\s*=>\s*return\s*Err\(e\)\s*\}', r'\1'),
        ]
        
        for pattern, replacement in patterns:
            new_content, count = re.subn(pattern, replacement, content)
            if count > 0:
                content = new_content
                fixes += count
        
        self.fixes_applied += fixes
        return content
    
    def fix_broken_syntax_patterns(self, content: str) -> str:
        """Fix various broken syntax patterns"""
        fixes = 0
        
        # Fix broken function signatures
        patterns = [
            # Fix: fn name() -> type " + -> fn name() -> type {
            (r'(\w+)\s*\(\s*\)\s*->\s*(\w+)\s*"\s*\+\s*', r'\1() -> \2 {'),
            
            # Fix: -> Result<type, type> " + -> -> Result<type, type> {
            (r'->\s*(Result<[^>]+>)\s*"\s*\+\s*', r'-> \1 {'),
            
            # Fix broken braces: " + " -> {
            (r'"\s*\+\s*"', ''),
            
            # Fix broken returns: return 0; + " -> return 0;
            (r'return\s+(\d+);\s*\+\s*"', r'return \1;'),
        ]
        
        for pattern, replacement in patterns:
            new_content, count = re.subn(pattern, replacement, content)
            if count > 0:
                content = new_content
                fixes += count
        
        self.fixes_applied += fixes
        return content
    
    def fix_file(self, file_path: Path) -> bool:
        """Fix all issues in a single file"""
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                original_content = f.read()
            
            content = original_content
            
            # Apply all advanced fixes
            content = self.fix_question_mark_artifacts(content)
            content = self.fix_complex_string_interpolation(content)
            content = self.fix_multiline_let_statements(content)
            content = self.fix_complex_expressions(content)
            content = self.fix_string_concatenation_artifacts(content)
            content = self.fix_broken_syntax_patterns(content)
            
            # Only write if changes were made
            if content != original_content:
                with open(file_path, 'w', encoding='utf-8') as f:
                    f.write(content)
                self.files_modified += 1
                return True
            
            return False
            
        except Exception as e:
            print(f"Error processing {file_path}: {e}")
            return False
    
    def fix_all_files(self) -> None:
        """Fix all markdown files in the user manual directory"""
        print("🔧 Advanced User Manual Example Fix Script")
        print("=" * 50)
        print("Fixing remaining 34 complex invalid examples...")
        print()
        
        # Get all markdown files
        md_files = list(self.user_manual_dir.glob("*.md"))
        
        for file_path in md_files:
            print(f"Processing: {file_path.name}")
            if self.fix_file(file_path):
                print(f"  ✅ Modified")
            else:
                print(f"  ⚪ No changes needed")
        
        print()
        print("=" * 50)
        print(f"Summary:")
        print(f"Files processed: {len(md_files)}")
        print(f"Files modified: {self.files_modified}")
        print(f"Total fixes applied: {self.fixes_applied}")
        print()
        
        if self.fixes_applied > 0:
            print("✅ Advanced fixes applied successfully!")
            print("Run validation script to verify improvements.")
        else:
            print("ℹ️  No advanced fixes were needed.")

def main():
    parser = argparse.ArgumentParser(description="Fix complex user manual examples")
    parser.add_argument("--project-root", default=".", 
                       help="Project root directory (default: current directory)")
    
    args = parser.parse_args()
    
    # Get absolute path
    project_root = os.path.abspath(args.project_root)
    
    if not os.path.exists(os.path.join(project_root, "docs", "user-manual")):
        print("Error: user-manual directory not found")
        print(f"Expected: {os.path.join(project_root, 'docs', 'user-manual')}")
        sys.exit(1)
    
    fixer = AdvancedManualExampleFixer(project_root)
    fixer.fix_all_files()

if __name__ == "__main__":
    main() 
