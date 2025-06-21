#!/usr/bin/env python3
"""
Single File Grammar Validation Script
Validates Asthra code examples in a single documentation file for v1.22 grammar conformance.
"""

import re
import sys
import os

def extract_asthra_examples(file_path):
    """Extract all Asthra code blocks from a markdown file."""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
    except FileNotFoundError:
        print(f"Error: File '{file_path}' not found")
        return []
    
    # Find all asthra code blocks
    pattern = r'```asthra\n(.*?)\n```'
    matches = re.findall(pattern, content, re.DOTALL)
    return matches

def validate_example(code):
    """Validate a single code example for v1.22 grammar conformance."""
    violations = []
    
    # Check for string interpolation (removed in v1.21)
    # Look for strings with {variable} patterns on single lines
    lines = code.split('\n')
    for line_num, line in enumerate(lines, 1):
        # Match string literals with interpolation on the same line
        interpolation_matches = re.findall(r'"[^"]*\{[^}]+\}[^"]*"', line)
        if interpolation_matches:
            violations.append(f'String interpolation found (removed in v1.21) on line {line_num}: {interpolation_matches}')
    
    # Check for empty parameter lists (should be (none))
    if re.search(r'fn\s+\w+\s*\(\s*\)\s*->', code):
        violations.append('Empty parameter list should use (none)')
    
    # Check for auto type annotations (should be explicit)
    if re.search(r':\s*auto\s*=', code):
        violations.append('Auto type annotation found (should be explicit)')
    
    # Check for missing visibility modifiers on top-level declarations
    if re.search(r'^(fn|struct|enum)\s+\w+', code, re.MULTILINE):
        if not re.search(r'^(pub|priv)\s+(fn|struct|enum)', code, re.MULTILINE):
            violations.append('Missing visibility modifier (pub/priv required)')
    
    # Check for return void; (should be return;)
    if re.search(r'return\s+void\s*;', code):
        violations.append('return void; should be return;')
    
    return violations

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 validate_single_file.py <file_path>")
        sys.exit(1)
    
    file_path = sys.argv[1]
    
    if not os.path.exists(file_path):
        print(f"Error: File '{file_path}' does not exist")
        sys.exit(1)
    
    examples = extract_asthra_examples(file_path)
    
    print(f"Grammar Validation for {file_path}")
    print("=" * 50)
    print(f"Found {len(examples)} Asthra code examples")
    print()
    
    total_violations = 0
    valid_examples = 0
    
    for i, example in enumerate(examples, 1):
        violations = validate_example(example)
        if violations:
            print(f"Example {i}: INVALID")
            for violation in violations:
                print(f"  - {violation}")
            total_violations += len(violations)
        else:
            print(f"Example {i}: VALID")
            valid_examples += 1
    
    print()
    print("=" * 50)
    print(f"Validation Summary:")
    print(f"Total examples: {len(examples)}")
    print(f"Valid examples: {valid_examples}")
    print(f"Invalid examples: {len(examples) - valid_examples}")
    print(f"Total violations: {total_violations}")
    
    if total_violations == 0:
        print("✅ All examples are grammar-compliant!")
        sys.exit(0)
    else:
        print("❌ Grammar violations found - fixes needed")
        sys.exit(1)

if __name__ == '__main__':
    main() 
