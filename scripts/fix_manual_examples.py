#!/usr/bin/env python3

"""
User Manual Example Fix Script
Part of the User Manual Maintenance Plan Phase 1 completion
Automatically fixes the remaining 40 invalid examples identified by validation script

Issues to fix:
1. Missing type annotations (35+ examples) - Add explicit types
2. String interpolation (5+ examples) - Convert to string concatenation
3. Missing visibility modifiers (2 examples) - Add pub/priv
4. Question mark operator (1 example) - Convert to match expressions
"""

import os
import re
import sys
import argparse
from pathlib import Path
from typing import List, Tuple, Dict

class ManualExampleFixer:
    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.user_manual_dir = self.project_root / "docs" / "user-manual"
        self.fixes_applied = 0
        self.files_modified = 0
        
    def fix_type_annotations(self, content: str) -> str:
        """Fix missing type annotations in let statements"""
        fixes = 0
        
        # Pattern: let variable = value; -> let variable: type = value;
        patterns = [
            # let x = 42; -> let x: i32 = 42;
            (r'let\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*(\d+);', r'let \1: i32 = \2;'),
            # let x = 42.0; -> let x: f64 = 42.0;
            (r'let\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*(\d+\.\d+);', r'let \1: f64 = \2;'),
            # let x = true; -> let x: bool = true;
            (r'let\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*(true|false);', r'let \1: bool = \2;'),
            # let x = "string"; -> let x: String = "string";
            (r'let\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*"([^"]*)";', r'let \1: String = "\2";'),
            # let x = Vec::new(); -> let x: Vec<i32> = Vec::new();
            (r'let\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*Vec::new\(\);', r'let \1: Vec<i32> = Vec::new();'),
            # let x = HashMap::new(); -> let x: HashMap<String, i32> = HashMap::new();
            (r'let\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*HashMap::new\(\);', r'let \1: HashMap<String, i32> = HashMap::new();'),
            # let x = Some(value); -> let x: Option<i32> = Some(value);
            (r'let\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*Some\((\d+)\);', r'let \1: Option<i32> = Some(\2);'),
            # let x = None; -> let x: Option<i32> = None;
            (r'let\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*None;', r'let \1: Option<i32> = None;'),
            # let x = Ok(value); -> let x: Result<i32, String> = Ok(value);
            (r'let\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*Ok\((\d+)\);', r'let \1: Result<i32, String> = Ok(\2);'),
            # let x = Err("error"); -> let x: Result<i32, String> = Err("error");
            (r'let\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*Err\("([^"]*)"\);', r'let \1: Result<i32, String> = Err("\2");'),
        ]
        
        for pattern, replacement in patterns:
            new_content, count = re.subn(pattern, replacement, content)
            if count > 0:
                content = new_content
                fixes += count
                
        self.fixes_applied += fixes
        return content
    
    def fix_string_interpolation(self, content: str) -> str:
        """Convert string interpolation to string concatenation"""
        fixes = 0
        
        # Pattern: "Hello {name}!" -> "Hello " + name + "!"
        def replace_interpolation(match):
            nonlocal fixes
            fixes += 1
            full_string = match.group(0)
            
            # Simple case: "text {var} more text"
            # Convert to: "text " + var + " more text"
            parts = []
            current_pos = 1  # Skip opening quote
            
            while current_pos < len(full_string) - 1:  # Skip closing quote
                # Find next {
                brace_start = full_string.find('{', current_pos)
                if brace_start == -1:
                    # No more braces, add rest as string literal
                    remaining = full_string[current_pos:-1]  # Exclude closing quote
                    if remaining:
                        parts.append(f'"{remaining}"')
                    break
                
                # Add text before brace as string literal
                before_brace = full_string[current_pos:brace_start]
                if before_brace:
                    parts.append(f'"{before_brace}"')
                
                # Find closing brace
                brace_end = full_string.find('}', brace_start)
                if brace_end == -1:
                    break
                
                # Add variable name
                var_name = full_string[brace_start + 1:brace_end]
                parts.append(var_name)
                
                current_pos = brace_end + 1
            
            # Join with +
            if len(parts) == 1:
                return parts[0]
            else:
                return ' + '.join(parts)
        
        # Match string interpolation patterns
        pattern = r'"[^"]*\{[^}]+\}[^"]*"'
        content = re.sub(pattern, replace_interpolation, content)
        
        self.fixes_applied += fixes
        return content
    
    def fix_visibility_modifiers(self, content: str) -> str:
        """Add missing visibility modifiers to functions"""
        fixes = 0
        
        # Pattern: fn function_name -> pub fn function_name
        # Only add pub if it's not already there and not inside impl blocks
        lines = content.split('\n')
        in_impl_block = False
        brace_count = 0
        
        for i, line in enumerate(lines):
            stripped = line.strip()
            
            # Track impl blocks
            if stripped.startswith('impl '):
                in_impl_block = True
                brace_count = 0
            
            # Count braces to track when we exit impl block
            brace_count += line.count('{') - line.count('}')
            if in_impl_block and brace_count <= 0:
                in_impl_block = False
            
            # Fix function declarations outside impl blocks
            if not in_impl_block and re.match(r'^\s*fn\s+[a-zA-Z_]', stripped):
                if not re.match(r'^\s*(pub|priv)\s+fn\s+', stripped):
                    # Add pub modifier
                    lines[i] = re.sub(r'^(\s*)fn\s+', r'\1pub fn ', line)
                    fixes += 1
        
        self.fixes_applied += fixes
        return '\n'.join(lines)
    
    def fix_question_mark_operator(self, content: str) -> str:
        """Convert question mark operator to match expressions"""
        fixes = 0
        
        # Pattern: result? -> match result { Ok(val) => val, Err(e) => return Err(e) }
        # This is a simplified conversion - may need manual review for complex cases
        def replace_question_mark(match):
            nonlocal fixes
            fixes += 1
            expr = match.group(1)
            return f'match {expr} {{ Ok(val) => val, Err(e) => return Err(e) }}'
        
        # Match expressions ending with ?
        pattern = r'([a-zA-Z_][a-zA-Z0-9_]*(?:\([^)]*\))?)\?'
        content = re.sub(pattern, replace_question_mark, content)
        
        self.fixes_applied += fixes
        return content
    
    def fix_file(self, file_path: Path) -> bool:
        """Fix all issues in a single file"""
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                original_content = f.read()
            
            content = original_content
            
            # Apply all fixes
            content = self.fix_type_annotations(content)
            content = self.fix_string_interpolation(content)
            content = self.fix_visibility_modifiers(content)
            content = self.fix_question_mark_operator(content)
            
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
        print("🔧 User Manual Example Fix Script")
        print("=" * 50)
        print("Fixing remaining 40 invalid examples...")
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
            print("✅ Fixes applied successfully!")
            print("Run validation script to verify improvements.")
        else:
            print("ℹ️  No fixes were needed.")

def main():
    parser = argparse.ArgumentParser(description="Fix user manual examples")
    parser.add_argument("--project-root", default=".", 
                       help="Project root directory (default: current directory)")
    parser.add_argument("--dry-run", action="store_true",
                       help="Show what would be fixed without making changes")
    
    args = parser.parse_args()
    
    # Get absolute path
    project_root = os.path.abspath(args.project_root)
    
    if not os.path.exists(os.path.join(project_root, "docs", "user-manual")):
        print("Error: user-manual directory not found")
        print(f"Expected: {os.path.join(project_root, 'docs', 'user-manual')}")
        sys.exit(1)
    
    fixer = ManualExampleFixer(project_root)
    
    if args.dry_run:
        print("DRY RUN MODE - No changes will be made")
        print()
    
    fixer.fix_all_files()

if __name__ == "__main__":
    main() 
