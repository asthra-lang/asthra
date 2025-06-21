#!/usr/bin/env python3
"""
Asthra Grammar Conformance Fixer

Automatically fixes grammar conformance violations across Asthra codebase.
Part of the Grammar Conformance Fix Plan v1.0.

Copyright (c) 2024 Asthra Project
Licensed under the terms specified in LICENSE
"""

import os
import re
import json
import argparse
import sys
import shutil
from pathlib import Path
from typing import Dict, List, Tuple, Optional, Set
from dataclasses import dataclass
from datetime import datetime
from enum import Enum

class FixType(Enum):
    EMPTY_PARAMS = "empty_params"
    EMPTY_STRUCT_VOID = "empty_struct_void"
    STRING_INTERPOLATION = "string_interpolation"
    AUTO_TYPE = "auto_type"
    RETURN_VOID = "return_void"

@dataclass
class FixResult:
    file_path: str
    fix_type: FixType
    line_number: int
    original_line: str
    fixed_line: str
    success: bool
    error_message: Optional[str] = None

class GrammarConformanceFixer:
    """Main fixer class for grammar conformance violations."""
    
    def __init__(self, backup_dir: Optional[str] = None, dry_run: bool = False):
        self.backup_dir = backup_dir
        self.dry_run = dry_run
        self.fix_results: List[FixResult] = []
        self.processed_files: Set[str] = set()
        
        # Initialize backup directory if specified
        if self.backup_dir and not self.dry_run:
            Path(self.backup_dir).mkdir(parents=True, exist_ok=True)
    
    def fix_empty_parameters(self, content: str) -> str:
        """Replace fn name() with fn name(none)"""
        pattern = r'fn(\s+\w+\s*)\(\s*\)(\s*->)'
        replacement = r'fn\1(none)\2'
        return re.sub(pattern, replacement, content)
    
    def fix_empty_struct_content(self, content: str) -> str:
        """Replace struct Name { void } with struct Name { none }"""
        pattern = r'struct(\s+\w+(?:\s*<[^>]*>)?)\s*\{\s*void\s*\}'
        replacement = r'struct\1 { none }'
        return re.sub(pattern, replacement, content)
    
    def fix_string_interpolation(self, content: str) -> str:
        """Convert string interpolation to concatenation"""
        lines = content.split('\n')
        fixed_lines = []
        
        for line in lines:
            fixed_line = self._fix_string_interpolation_line(line)
            fixed_lines.append(fixed_line)
        
        return '\n'.join(fixed_lines)
    
    def _fix_string_interpolation_line(self, line: str) -> str:
        """Fix string interpolation in a single line"""
        # Pattern to match string interpolation
        pattern = r'"([^"]*\{[^}]+\}[^"]*)"'
        
        def replace_interpolation(match):
            interpolated_string = match.group(1)
            return self._convert_to_concatenation(interpolated_string)
        
        return re.sub(pattern, replace_interpolation, line)
    
    def _convert_to_concatenation(self, interpolated_string: str) -> str:
        """Convert interpolated string to concatenation"""
        # Extract variables from braces
        variables = re.findall(r'\{([^}]+)\}', interpolated_string)
        
        # Split string by variable placeholders
        parts = re.split(r'\{[^}]+\}', interpolated_string)
        
        # Build concatenation
        result_parts = []
        
        for i, part in enumerate(parts):
            if part:  # Non-empty string part
                result_parts.append(f'"{part}"')
            
            if i < len(variables):
                var = variables[i].strip()
                # Handle complex expressions
                if '.' in var or '(' in var or '[' in var:
                    # For complex expressions, might need toString conversion
                    if self._needs_string_conversion(var):
                        result_parts.append(f'({var}).to_string()')
                    else:
                        result_parts.append(var)
                else:
                    result_parts.append(var)
        
        # Filter out empty parts and join with +
        non_empty_parts = [part for part in result_parts if part and part != '""']
        
        if not non_empty_parts:
            return '""'
        elif len(non_empty_parts) == 1:
            return non_empty_parts[0]
        else:
            return ' + '.join(non_empty_parts)
    
    def _needs_string_conversion(self, expression: str) -> bool:
        """Determine if expression needs string conversion"""
        # Heuristics for when to add .to_string()
        numeric_patterns = [
            r'\d+',  # Numbers
            r'\w+\.\w+',  # Field access that might be numeric
            r'\w+\[\d+\]',  # Array access
            r'\w+\(\)',  # Function calls that might return numbers
        ]
        
        for pattern in numeric_patterns:
            if re.search(pattern, expression):
                return True
        
        return False
    
    def fix_auto_types(self, content: str) -> str:
        """Replace auto with inferred types (enhanced with type analysis)"""
        # First, handle multi-line match expressions
        content = self._fix_multiline_match_expressions(content)
        
        # Then handle single-line auto declarations
        lines = content.split('\n')
        fixed_lines = []
        
        for line in lines:
            fixed_line = self._fix_auto_type_line(line)
            fixed_lines.append(fixed_line)
        
        return '\n'.join(fixed_lines)
    
    def _fix_multiline_match_expressions(self, content: str) -> str:
        """Fix multi-line match expressions that unwrap Result types"""
        # Pattern to match: let var: auto = match result {; Result.Ok(value) => value Result.Err(e) => return Result.Err(e) };
        pattern = r'let(\s+\w+)\s*:\s*auto\s*=\s*match\s+(\w+)\s*\{;\s*Result\.Ok\(value\)\s*=>\s*value\s*Result\.Err\([^)]+\)\s*=>\s*return\s*Result\.Err\([^)]+\)\s*\};'
        
        def replace_multiline_match(match):
            var_part = match.group(1)  # " variable_name"
            matched_var = match.group(2)  # "result_variable"
            
            # Infer type from variable name patterns
            inferred_type = self._infer_type_from_variable_name(var_part.strip(), matched_var)
            
            return f"let{var_part}: {inferred_type} = match {matched_var} {{; Result.Ok(value) => value Result.Err(e) => return Result.Err(e) }};"
        
        # Remove newlines and extra spaces for pattern matching
        normalized_content = re.sub(r'\s+', ' ', content)
        fixed_normalized = re.sub(pattern, replace_multiline_match, normalized_content)
        
        # If we made changes, we need to restore the original formatting
        if fixed_normalized != normalized_content:
            # This is a simplified approach - in practice, we'd need more sophisticated formatting preservation
            return self._restore_formatting(content, fixed_normalized)
        
        return content
    
    def _infer_type_from_variable_name(self, var_name: str, matched_var: str) -> str:
        """Infer type from variable naming patterns"""
        var_lower = var_name.lower()
        matched_lower = matched_var.lower()
        
        # Check both the variable name and what it's matching
        combined = var_lower + " " + matched_lower
        
        if any(keyword in combined for keyword in ['bool', 'boolean']):
            return 'bool'
        elif any(keyword in combined for keyword in ['num', 'number', 'int', 'float']):
            return 'f64'
        elif any(keyword in combined for keyword in ['str', 'string', 'text']):
            return 'string'
        elif any(keyword in combined for keyword in ['array', 'list']):
            return 'json::JSONArray'
        elif any(keyword in combined for keyword in ['obj', 'object']):
            return 'json::JSONObject'
        elif any(keyword in combined for keyword in ['element', 'value', 'item']):
            return 'json::JSONValue'
        else:
            return 'auto  // Match unwrap - manual review needed'
    
    def _restore_formatting(self, original: str, fixed: str) -> str:
        """Restore original formatting after fixing normalized content"""
        # This is a simplified implementation
        # In practice, we'd need more sophisticated formatting preservation
        return original  # For now, return original to avoid breaking formatting
    
    def _fix_auto_type_line(self, line: str) -> str:
        """Fix auto type annotation in a single line with type inference"""
        # Pattern to match auto type declarations - don't consume trailing semicolon for match expressions
        pattern = r'let(\s+\w+)\s*:\s*auto\s*=\s*(.+?)$'
        
        def replace_auto(match):
            var_part = match.group(1)  # " variable_name"
            value_part = match.group(2).strip()  # "value"
            
            # Check for match expressions before removing semicolon
            is_match_with_semicolon = value_part.startswith('match ') and value_part.endswith('{;')
            
            # Remove trailing semicolon if present for processing
            has_semicolon = value_part.endswith(';')
            if has_semicolon:
                value_part = value_part[:-1].strip()
            
            # Handle match expressions that start on the same line
            if value_part.startswith('match ') and value_part.endswith('{'):
                # Extract the variable being matched
                match_var_pattern = r'match\s+(\w+)\s*\{$'
                match_var_match = re.search(match_var_pattern, value_part)
                if match_var_match:
                    matched_var = match_var_match.group(1)
                    inferred_type = self._infer_type_from_variable_name(var_part.strip(), matched_var)
                    # Preserve the original ending
                    if is_match_with_semicolon:
                        return f"let{var_part}: {inferred_type} = match {matched_var} {{;"
                    else:
                        return f"let{var_part}: {inferred_type} = match {matched_var} {{"
            
            # Infer type from the value
            inferred_type = self._infer_type_from_value(value_part)
            
            # Add semicolon for regular assignments
            if has_semicolon:
                return f"let{var_part}: {inferred_type} = {value_part};"
            else:
                return f"let{var_part}: {inferred_type} = {value_part}"
        
        return re.sub(pattern, replace_auto, line)
    
    def _infer_type_from_value(self, value: str) -> str:
        """Infer type from the assigned value"""
        value = value.strip()
        
        # JSON parsing results
        if 'json::parse(' in value:
            return 'Result<json::JSONValue, json::JSONError>'
        
        # JSON conversion results
        if 'json::as_bool(' in value:
            return 'Result<bool, json::JSONError>'
        elif 'json::as_number(' in value:
            return 'Result<f64, json::JSONError>'
        elif 'json::as_string(' in value:
            return 'Result<string, json::JSONError>'
        elif 'json::as_array(' in value:
            return 'Result<json::JSONArray, json::JSONError>'
        elif 'json::as_object(' in value:
            return 'Result<json::JSONObject, json::JSONError>'
        
        # JSON access results
        if 'json::array_get(' in value:
            return 'Result<json::JSONValue, json::JSONError>'
        elif 'json::object_get(' in value:
            return 'Result<json::JSONValue, json::JSONError>'
        
        # Match expressions - analyze the match result
        if value.startswith('match ') and '{' in value:
            # For match expressions, we need to analyze the return type
            # Look for the pattern: match result { Result.Ok(value) => value, Result.Err(e) => return Result.Err(e) }
            if 'Result.Ok(value) => value' in value and 'Result.Err(' in value and 'return Result.Err(' in value:
                # This is a common pattern for unwrapping Result types
                # Extract the variable being matched
                match_var_pattern = r'match\s+(\w+)'
                match_var_match = re.search(match_var_pattern, value)
                if match_var_match:
                    var_name = match_var_match.group(1)
                    # Look for the type of the variable in the context
                    # For now, we'll use heuristics based on common patterns
                    if 'bool_' in var_name or '_bool' in var_name:
                        return 'bool'
                    elif 'num_' in var_name or '_num' in var_name or 'number_' in var_name:
                        return 'f64'
                    elif 'str_' in var_name or '_str' in var_name or 'string_' in var_name:
                        return 'string'
                    elif 'array_' in var_name or '_array' in var_name:
                        return 'json::JSONArray'
                    elif 'obj_' in var_name or '_obj' in var_name or 'object_' in var_name:
                        return 'json::JSONObject'
                    elif 'element_' in var_name or '_element' in var_name:
                        return 'json::JSONValue'
                    else:
                        return 'auto  // Match unwrap - manual review needed'
            else:
                return 'auto  // Complex match - manual review needed'
        
        # String literals
        if value.startswith('"') and value.endswith('"'):
            return 'string'
        elif value.startswith("'") and value.endswith("'"):
            return 'char'
        
        # Numeric literals
        if re.match(r'^\d+$', value):
            return 'i32'
        elif re.match(r'^\d+\.\d+$', value):
            return 'f64'
        
        # Boolean literals
        if value in ['true', 'false']:
            return 'bool'
        
        # Array literals
        if value.startswith('[') and value.endswith(']'):
            return 'auto  // Array - manual review needed'
        
        # Object literals
        if value.startswith('{') and value.endswith('}'):
            return 'auto  // Object - manual review needed'
        
        # Function calls - try to infer from common patterns
        if '::' in value and '(' in value:
            # Standard library function calls
            if 'json::array_len(' in value or 'json::object_len(' in value:
                return 'usize'
            else:
                return 'auto  // Function call - manual review needed'
        
        # Default fallback with comment for manual review
        return 'auto  // Manual review needed'
    
    def fix_return_statements(self, content: str) -> str:
        """Replace return void; with return;"""
        pattern = r'return\s+void\s*;'
        replacement = 'return;'
        return re.sub(pattern, replacement, content)
    
    def fix_file(self, file_path: str, fix_types: List[FixType]) -> List[FixResult]:
        """Fix a single file for specified violation types"""
        results = []
        
        try:
            # Read original content
            with open(file_path, 'r', encoding='utf-8') as f:
                original_content = f.read()
            
            # Create backup if not dry run
            if not self.dry_run and self.backup_dir:
                self._create_backup(file_path, original_content)
            
            # Apply fixes
            fixed_content = original_content
            
            for fix_type in fix_types:
                if fix_type == FixType.EMPTY_PARAMS:
                    fixed_content = self.fix_empty_parameters(fixed_content)
                elif fix_type == FixType.EMPTY_STRUCT_VOID:
                    fixed_content = self.fix_empty_struct_content(fixed_content)
                elif fix_type == FixType.STRING_INTERPOLATION:
                    fixed_content = self.fix_string_interpolation(fixed_content)
                elif fix_type == FixType.AUTO_TYPE:
                    fixed_content = self.fix_auto_types(fixed_content)
                elif fix_type == FixType.RETURN_VOID:
                    fixed_content = self.fix_return_statements(fixed_content)
            
            # Check if content changed
            if fixed_content != original_content:
                # Write fixed content if not dry run
                if not self.dry_run:
                    with open(file_path, 'w', encoding='utf-8') as f:
                        f.write(fixed_content)
                
                # Record changes
                original_lines = original_content.split('\n')
                fixed_lines = fixed_content.split('\n')
                
                for i, (orig, fixed) in enumerate(zip(original_lines, fixed_lines)):
                    if orig != fixed:
                        for fix_type in fix_types:
                            result = FixResult(
                                file_path=file_path,
                                fix_type=fix_type,
                                line_number=i + 1,
                                original_line=orig.strip(),
                                fixed_line=fixed.strip(),
                                success=True
                            )
                            results.append(result)
            
            self.processed_files.add(file_path)
            
        except Exception as e:
            result = FixResult(
                file_path=file_path,
                fix_type=fix_types[0] if fix_types else FixType.EMPTY_PARAMS,
                line_number=0,
                original_line="",
                fixed_line="",
                success=False,
                error_message=str(e)
            )
            results.append(result)
        
        self.fix_results.extend(results)
        return results
    
    def _create_backup(self, file_path: str, content: str) -> None:
        """Create backup of original file"""
        if not self.backup_dir:
            return
        
        # Create backup path maintaining directory structure
        rel_path = os.path.relpath(file_path)
        backup_path = os.path.join(self.backup_dir, rel_path)
        
        # Ensure backup directory exists
        backup_dir = os.path.dirname(backup_path)
        os.makedirs(backup_dir, exist_ok=True)
        
        # Write backup
        with open(backup_path, 'w', encoding='utf-8') as f:
            f.write(content)
    
    def fix_directory(self, directory: str, fix_types: List[FixType], 
                     recursive: bool = True) -> None:
        """Fix all Asthra files in a directory or a single file"""
        path = Path(directory)
        
        if not path.exists():
            print(f"Error: Path {directory} does not exist")
            return
        
        # Handle single file
        if path.is_file():
            if path.suffix == '.asthra':
                print(f"Processing single file: {directory}")
                self.fix_file(str(path), fix_types)
            else:
                print(f"Error: {directory} is not an Asthra file (.asthra)")
            return
        
        # Handle directory
        if not path.is_dir():
            print(f"Error: {directory} is not a file or directory")
            return
        
        pattern = "**/*.asthra" if recursive else "*.asthra"
        asthra_files = list(path.glob(pattern))
        
        print(f"Processing {len(asthra_files)} Asthra files in {directory}...")
        
        for file_path in asthra_files:
            self.fix_file(str(file_path), fix_types)
    
    def generate_report(self) -> Dict:
        """Generate a comprehensive fix report"""
        successful_fixes = [r for r in self.fix_results if r.success]
        failed_fixes = [r for r in self.fix_results if not r.success]
        
        # Count by type
        type_counts = {}
        for result in successful_fixes:
            type_key = result.fix_type.value
            type_counts[type_key] = type_counts.get(type_key, 0) + 1
        
        # Count by file
        file_counts = {}
        for result in successful_fixes:
            file_counts[result.file_path] = file_counts.get(result.file_path, 0) + 1
        
        return {
            'summary': {
                'total_fixes': len(successful_fixes),
                'failed_fixes': len(failed_fixes),
                'files_processed': len(self.processed_files),
                'dry_run': self.dry_run,
                'backup_dir': self.backup_dir,
                'timestamp': datetime.now().isoformat()
            },
            'by_type': type_counts,
            'by_file': file_counts,
            'successful_fixes': [
                {
                    'file_path': r.file_path,
                    'fix_type': r.fix_type.value,
                    'line_number': r.line_number,
                    'original_line': r.original_line,
                    'fixed_line': r.fixed_line
                }
                for r in successful_fixes
            ],
            'failed_fixes': [
                {
                    'file_path': r.file_path,
                    'fix_type': r.fix_type.value,
                    'error_message': r.error_message
                }
                for r in failed_fixes
            ]
        }
    
    def export_report(self, output_file: str) -> None:
        """Export fix report to JSON"""
        report = self.generate_report()
        
        with open(output_file, 'w', encoding='utf-8') as f:
            json.dump(report, f, indent=2)
        
        print(f"Fix report exported to {output_file}")
    
    def print_summary(self) -> None:
        """Print summary of fixes to console"""
        report = self.generate_report()
        summary = report['summary']
        
        print("\n" + "="*80)
        print("GRAMMAR CONFORMANCE FIX SUMMARY")
        print("="*80)
        
        if self.dry_run:
            print("DRY RUN MODE - No files were modified")
        
        print(f"Total Fixes Applied: {summary['total_fixes']}")
        print(f"Failed Fixes: {summary['failed_fixes']}")
        print(f"Files Processed: {summary['files_processed']}")
        
        if summary['backup_dir']:
            print(f"Backup Directory: {summary['backup_dir']}")
        
        print("\nFixes by Type:")
        for fix_type, count in report['by_type'].items():
            print(f"  {fix_type}: {count}")
        
        if report['failed_fixes']:
            print(f"\nFailed Fixes:")
            for failed in report['failed_fixes']:
                print(f"  {failed['file_path']}: {failed['error_message']}")
        
        print("="*80)

def main():
    parser = argparse.ArgumentParser(
        description="Asthra Grammar Conformance Fixer",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python3 grammar_conformance_fixer.py --scan examples/ --fix empty_params
  python3 grammar_conformance_fixer.py --scan . --fix all --backup backups/
  python3 grammar_conformance_fixer.py --scan stdlib/ --fix string_interpolation --dry-run
        """
    )
    
    parser.add_argument('--scan', required=True,
                       help='Directory to scan for Asthra files')
    parser.add_argument('--fix', required=True,
                       choices=['empty_params', 'empty_struct_void', 'string_interpolation', 
                               'auto_type', 'return_void', 'all'],
                       help='Type of fixes to apply')
    parser.add_argument('--backup-dir', metavar='DIR',
                       help='Directory to store backup files')
    parser.add_argument('--dry-run', action='store_true',
                       help='Show what would be fixed without making changes')
    parser.add_argument('--recursive', action='store_true', default=True,
                       help='Process directories recursively (default: True)')
    parser.add_argument('--report', metavar='FILE',
                       help='Export fix report to JSON file')
    parser.add_argument('--verbose', action='store_true',
                       help='Show detailed fix information')
    
    args = parser.parse_args()
    
    # Determine fix types
    if args.fix == 'all':
        fix_types = list(FixType)
    else:
        fix_types = [FixType(args.fix)]
    
    # Create fixer
    fixer = GrammarConformanceFixer(
        backup_dir=args.backup_dir,
        dry_run=args.dry_run
    )
    
    # Apply fixes
    fixer.fix_directory(args.scan, fix_types, args.recursive)
    
    # Generate and display results
    fixer.print_summary()
    
    if args.verbose:
        report = fixer.generate_report()
        print("\nDetailed Fix Information:")
        for fix in report['successful_fixes'][:20]:  # Show first 20
            print(f"  {fix['file_path']}:{fix['line_number']} ({fix['fix_type']})")
            print(f"    - {fix['original_line']}")
            print(f"    + {fix['fixed_line']}")
    
    # Export report if requested
    if args.report:
        fixer.export_report(args.report)
    
    print(f"\nFix complete. Applied {len([r for r in fixer.fix_results if r.success])} fixes.")

if __name__ == '__main__':
    main() 
