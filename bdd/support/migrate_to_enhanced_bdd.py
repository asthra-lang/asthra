#!/usr/bin/env python3
"""
Script to help migrate existing BDD tests to use the enhanced BDD framework
with automatic @wip tag skipping support.
"""

import os
import re
import sys
import argparse

def parse_feature_file(feature_path):
    """Parse a feature file to extract scenario names and their tags."""
    scenarios = {}
    current_tags = []
    
    with open(feature_path, 'r') as f:
        lines = f.readlines()
    
    for i, line in enumerate(lines):
        line = line.strip()
        
        # Check for tags
        if line.startswith('@'):
            current_tags = line.split()
        # Check for scenario
        elif line.startswith('Scenario:'):
            scenario_name = line[9:].strip()
            scenarios[scenario_name] = current_tags
            current_tags = []
    
    return scenarios

def update_c_file(c_path, scenarios):
    """Update C file to use enhanced BDD framework with tag support."""
    with open(c_path, 'r') as f:
        content = f.read()
    
    # Replace include
    content = content.replace('#include "bdd_support.h"', '#include "bdd_support_enhanced.h"')
    
    # Find test functions and update them
    test_func_pattern = r'void\s+(test_\w+)\s*\(\s*void\s*\)\s*{\s*\n\s*bdd_scenario\s*\(\s*"([^"]+)"\s*\);'
    
    def replace_test_func(match):
        func_name = match.group(1)
        scenario_name = match.group(2)
        tags = scenarios.get(scenario_name, [])
        tag_str = ' '.join(tags) if tags else 'NULL'
        
        if tags:
            return f'BDD_TEST_SCENARIO({func_name}, "{scenario_name}", "{tag_str}") {{'
        else:
            return f'BDD_TEST_SCENARIO({func_name}, "{scenario_name}", NULL) {{'
    
    content = re.sub(test_func_pattern, replace_test_func, content)
    
    # Update main function to remove manual @wip comments
    main_pattern = r'(int\s+main\s*\(\s*void\s*\)\s*{[^}]+})'
    
    def update_main(match):
        main_content = match.group(1)
        # Remove comments that disable @wip scenarios
        main_content = re.sub(r'\s*//\s*test_\w+\(\);\s*//\s*@wip.*\n', '', main_content)
        # Add note about automatic filtering
        main_content = main_content.replace(
            'bdd_init(',
            '// Note: @wip scenarios are now automatically skipped\n    bdd_init('
        )
        return main_content
    
    content = re.sub(main_pattern, update_main, content, flags=re.DOTALL)
    
    return content

def main():
    parser = argparse.ArgumentParser(description='Migrate BDD tests to enhanced framework')
    parser.add_argument('c_file', help='Path to the C test file')
    parser.add_argument('--feature', help='Path to the corresponding feature file')
    parser.add_argument('--dry-run', action='store_true', help='Show changes without writing')
    parser.add_argument('--backup', action='store_true', help='Create backup of original file')
    
    args = parser.parse_args()
    
    # Try to find feature file if not specified
    if not args.feature:
        base_name = os.path.basename(args.c_file).replace('_steps.c', '.feature')
        feature_dirs = [
            'bdd/features',
            '../features',
            '../../features',
            '../../../features'
        ]
        
        for dir in feature_dirs:
            feature_path = os.path.join(os.path.dirname(args.c_file), dir, base_name)
            if os.path.exists(feature_path):
                args.feature = feature_path
                print(f"Found feature file: {feature_path}")
                break
    
    if not args.feature or not os.path.exists(args.feature):
        print(f"Warning: Could not find feature file for {args.c_file}")
        print("Proceeding without tag information...")
        scenarios = {}
    else:
        scenarios = parse_feature_file(args.feature)
        print(f"Found {len(scenarios)} scenarios with tags")
    
    # Update C file
    updated_content = update_c_file(args.c_file, scenarios)
    
    if args.dry_run:
        print("\n=== Changes that would be made ===")
        print(updated_content)
    else:
        if args.backup:
            backup_path = args.c_file + '.bak'
            with open(args.c_file, 'r') as f:
                backup_content = f.read()
            with open(backup_path, 'w') as f:
                f.write(backup_content)
            print(f"Created backup: {backup_path}")
        
        with open(args.c_file, 'w') as f:
            f.write(updated_content)
        print(f"Updated: {args.c_file}")

if __name__ == '__main__':
    main()