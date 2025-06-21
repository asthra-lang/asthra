#!/usr/bin/env python3
"""
Asthra Grammar Conformance Checker

Detects grammar conformance violations across Asthra codebase.
Part of the Grammar Conformance Fix Plan v1.0.

Copyright (c) 2024 Asthra Project
Licensed under the terms specified in LICENSE
"""

import os
import re
import json
import csv
import argparse
import sys
from pathlib import Path
from typing import Dict, List, Tuple, Optional
from dataclasses import dataclass, asdict
from enum import Enum

class ViolationType(Enum):
    EMPTY_PARAMS = "empty_params"
    EMPTY_STRUCT_VOID = "empty_struct_void"
    STRING_INTERPOLATION = "string_interpolation"
    AUTO_TYPE = "auto_type"
    RETURN_VOID = "return_void"

class Priority(Enum):
    CRITICAL = "CRITICAL"
    HIGH = "HIGH"
    MEDIUM = "MEDIUM"
    LOW = "LOW"

@dataclass
class Violation:
    file_path: str
    line_number: int
    line_content: str
    violation_type: ViolationType
    priority: Priority
    description: str
    suggested_fix: str

class GrammarConformanceChecker:
    """Main checker class for grammar conformance violations."""
    
    def __init__(self):
        self.violations: List[Violation] = []
        self.violation_patterns = {
            ViolationType.EMPTY_PARAMS: {
                'pattern': r'fn\s+(\w+)\s*\(\s*\)\s*->',
                'priority': Priority.HIGH,
                'description': 'Function with empty parameter list should use (none)',
                'fix_template': 'fn {function_name}(none) ->'
            },
            ViolationType.EMPTY_STRUCT_VOID: {
                'pattern': r'struct\s+(\w+)(?:\s*<[^>]*>)?\s*\{\s*void\s*\}',
                'priority': Priority.HIGH,
                'description': 'Empty struct should use { none } instead of { void }',
                'fix_template': 'struct {struct_name} {{ none }}'
            },
            ViolationType.STRING_INTERPOLATION: {
                'pattern': r'"[^"]*\{[^}]+\}[^"]*"',
                'priority': Priority.CRITICAL,
                'description': 'String interpolation removed in v1.22, use concatenation',
                'fix_template': 'Use string concatenation: "text " + variable + " more text"'
            },
            ViolationType.AUTO_TYPE: {
                'pattern': r'let\s+(\w+)\s*:\s*auto\s*=',
                'priority': Priority.MEDIUM,
                'description': 'Auto type annotation should be explicit type',
                'fix_template': 'let {variable_name}: {explicit_type} ='
            },
            ViolationType.RETURN_VOID: {
                'pattern': r'return\s+void\s*;',
                'priority': Priority.LOW,
                'description': 'Functions returning void should use return; not return void;',
                'fix_template': 'return;'
            }
        }
    
    def scan_file(self, file_path: str) -> List[Violation]:
        """Scan a single file for grammar violations."""
        violations = []
        
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                lines = f.readlines()
        except (UnicodeDecodeError, IOError) as e:
            print(f"Warning: Could not read {file_path}: {e}")
            return violations
        
        for line_num, line in enumerate(lines, 1):
            line_stripped = line.strip()
            
            # Skip comments and empty lines
            if not line_stripped or line_stripped.startswith('//') or line_stripped.startswith('/*'):
                continue
            
            # Check each violation pattern
            for violation_type, pattern_info in self.violation_patterns.items():
                pattern = pattern_info['pattern']
                matches = re.finditer(pattern, line)
                
                for match in matches:
                    # Generate suggested fix
                    suggested_fix = self._generate_fix(violation_type, match, line_stripped)
                    
                    violation = Violation(
                        file_path=file_path,
                        line_number=line_num,
                        line_content=line_stripped,
                        violation_type=violation_type,
                        priority=pattern_info['priority'],
                        description=pattern_info['description'],
                        suggested_fix=suggested_fix
                    )
                    violations.append(violation)
        
        return violations
    
    def _generate_fix(self, violation_type: ViolationType, match: re.Match, line: str) -> str:
        """Generate a suggested fix for a specific violation."""
        pattern_info = self.violation_patterns[violation_type]
        
        if violation_type == ViolationType.EMPTY_PARAMS:
            function_name = match.group(1) if match.groups() else "function"
            return line.replace(f"fn {function_name}()", f"fn {function_name}(none)")
        
        elif violation_type == ViolationType.EMPTY_STRUCT_VOID:
            return re.sub(r'\{\s*void\s*\}', '{ none }', line)
        
        elif violation_type == ViolationType.STRING_INTERPOLATION:
            # This requires more complex analysis - provide general guidance
            return "Convert to concatenation: " + self._suggest_interpolation_fix(line)
        
        elif violation_type == ViolationType.AUTO_TYPE:
            return line.replace(': auto', ': <explicit_type>')
        
        elif violation_type == ViolationType.RETURN_VOID:
            return line.replace('return void;', 'return;')
        
        return line
    
    def _suggest_interpolation_fix(self, line: str) -> str:
        """Suggest a fix for string interpolation."""
        # Simple heuristic - extract variables from braces
        variables = re.findall(r'\{([^}]+)\}', line)
        if not variables:
            return line
        
        # Extract the string parts
        parts = re.split(r'\{[^}]+\}', line)
        
        # Build concatenation suggestion
        result_parts = []
        for i, part in enumerate(parts):
            if part.strip():
                # Remove quotes from string parts
                clean_part = part.strip().strip('"\'')
                if clean_part:
                    result_parts.append(f'"{clean_part}"')
            
            if i < len(variables):
                result_parts.append(variables[i])
        
        return ' + '.join(result_parts)
    
    def scan_directory(self, directory: str, recursive: bool = True) -> None:
        """Scan a directory for Asthra files and check for violations."""
        directory_path = Path(directory)
        
        if not directory_path.exists():
            print(f"Error: Directory {directory} does not exist")
            return
        
        pattern = "**/*.asthra" if recursive else "*.asthra"
        asthra_files = list(directory_path.glob(pattern))
        
        print(f"Scanning {len(asthra_files)} Asthra files in {directory}...")
        
        for file_path in asthra_files:
            file_violations = self.scan_file(str(file_path))
            self.violations.extend(file_violations)
    
    def generate_report(self) -> Dict:
        """Generate a comprehensive violation report."""
        # Count violations by type and priority
        type_counts = {}
        priority_counts = {}
        file_counts = {}
        
        for violation in self.violations:
            # Count by type
            type_key = violation.violation_type.value
            type_counts[type_key] = type_counts.get(type_key, 0) + 1
            
            # Count by priority
            priority_key = violation.priority.value
            priority_counts[priority_key] = priority_counts.get(priority_key, 0) + 1
            
            # Count by file
            file_counts[violation.file_path] = file_counts.get(violation.file_path, 0) + 1
        
        # Calculate summary statistics
        total_violations = len(self.violations)
        total_files_scanned = len(set(v.file_path for v in self.violations)) if self.violations else 0
        files_with_violations = len(file_counts)
        
        return {
            'summary': {
                'total_violations': total_violations,
                'total_files_scanned': total_files_scanned,
                'files_with_violations': files_with_violations,
                'violation_rate': f"{(files_with_violations / max(total_files_scanned, 1)) * 100:.1f}%"
            },
            'by_type': type_counts,
            'by_priority': priority_counts,
            'by_file': file_counts,
            'violations': [asdict(v) for v in self.violations]
        }
    
    def export_json(self, output_file: str) -> None:
        """Export violations to JSON format."""
        report = self.generate_report()
        
        with open(output_file, 'w', encoding='utf-8') as f:
            json.dump(report, f, indent=2, default=str)
        
        print(f"JSON report exported to {output_file}")
    
    def export_csv(self, output_file: str) -> None:
        """Export violations to CSV format."""
        with open(output_file, 'w', newline='', encoding='utf-8') as f:
            if not self.violations:
                f.write("No violations found\n")
                return
            
            writer = csv.writer(f)
            
            # Write header
            writer.writerow([
                'File Path', 'Line Number', 'Violation Type', 'Priority',
                'Description', 'Line Content', 'Suggested Fix'
            ])
            
            # Write violations
            for violation in self.violations:
                writer.writerow([
                    violation.file_path,
                    violation.line_number,
                    violation.violation_type.value,
                    violation.priority.value,
                    violation.description,
                    violation.line_content,
                    violation.suggested_fix
                ])
        
        print(f"CSV report exported to {output_file}")
    
    def print_summary(self) -> None:
        """Print a summary of violations to console."""
        report = self.generate_report()
        
        print("\n" + "="*80)
        print("GRAMMAR CONFORMANCE VIOLATION SUMMARY")
        print("="*80)
        
        summary = report['summary']
        print(f"Total Violations: {summary['total_violations']}")
        print(f"Files with Violations: {summary['files_with_violations']}")
        print(f"Violation Rate: {summary['violation_rate']}")
        
        print("\nBy Priority:")
        for priority in [Priority.CRITICAL, Priority.HIGH, Priority.MEDIUM, Priority.LOW]:
            count = report['by_priority'].get(priority.value, 0)
            print(f"  {priority.value}: {count}")
        
        print("\nBy Type:")
        for violation_type, count in report['by_type'].items():
            print(f"  {violation_type}: {count}")
        
        if report['by_file']:
            print(f"\nTop 10 Files with Most Violations:")
            sorted_files = sorted(report['by_file'].items(), key=lambda x: x[1], reverse=True)
            for file_path, count in sorted_files[:10]:
                print(f"  {count:3d}: {file_path}")
        
        print("="*80)

def main():
    parser = argparse.ArgumentParser(
        description="Asthra Grammar Conformance Checker",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python3 grammar_conformance_checker.py --scan examples/
  python3 grammar_conformance_checker.py --scan . --json violations.json
  python3 grammar_conformance_checker.py --scan stdlib/ --csv violations.csv --strict
        """
    )
    
    parser.add_argument('--scan', required=True,
                       help='Directory to scan for Asthra files')
    parser.add_argument('--recursive', action='store_true', default=True,
                       help='Scan directories recursively (default: True)')
    parser.add_argument('--json', metavar='FILE',
                       help='Export violations to JSON file')
    parser.add_argument('--csv', metavar='FILE',
                       help='Export violations to CSV file')
    parser.add_argument('--strict', action='store_true',
                       help='Exit with error code if violations found')
    parser.add_argument('--quiet', action='store_true',
                       help='Suppress console output except errors')
    
    args = parser.parse_args()
    
    # Create checker and scan
    checker = GrammarConformanceChecker()
    checker.scan_directory(args.scan, args.recursive)
    
    # Generate and display results
    if not args.quiet:
        checker.print_summary()
    
    # Export results if requested
    if args.json:
        checker.export_json(args.json)
    
    if args.csv:
        checker.export_csv(args.csv)
    
    # Exit with error code if strict mode and violations found
    if args.strict and checker.violations:
        print(f"\nError: {len(checker.violations)} grammar violations found!")
        sys.exit(1)
    
    if not args.quiet:
        print(f"\nScan complete. Found {len(checker.violations)} violations.")

if __name__ == '__main__':
    main() 
