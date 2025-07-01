#!/usr/bin/env python3
"""
Script to remove all '@wip' tags from BDD feature scenarios in bdd/features/
"""

import os
import re
import glob
from pathlib import Path

def remove_wip_tags_from_file(file_path):
    """Remove @wip tags from a single feature file"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Count @wip tags before removal
        wip_count = len(re.findall(r'@wip', content))
        
        if wip_count == 0:
            return 0
        
        # Remove @wip tags (with optional whitespace)
        # This handles cases like:
        # @wip
        # @wip 
        # and preserves the scenario structure
        content = re.sub(r'^\s*@wip\s*$', '', content, flags=re.MULTILINE)
        
        # Clean up any extra blank lines that might have been left
        content = re.sub(r'\n\n\n+', '\n\n', content)
        
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(content)
        
        print(f"Removed {wip_count} @wip tag(s) from {file_path}")
        return wip_count
        
    except Exception as e:
        print(f"Error processing {file_path}: {e}")
        return 0

def main():
    """Main function to process all feature files"""
    # Get the script directory and find bdd/features
    script_dir = Path(__file__).parent
    features_dir = script_dir / "bdd" / "features"
    
    if not features_dir.exists():
        print(f"Error: {features_dir} does not exist")
        return 1
    
    # Find all .feature files
    feature_files = list(features_dir.glob("*.feature"))
    
    if not feature_files:
        print("No .feature files found")
        return 1
    
    total_removed = 0
    processed_files = 0
    
    print(f"Processing {len(feature_files)} feature files...")
    
    for feature_file in sorted(feature_files):
        removed_count = remove_wip_tags_from_file(feature_file)
        if removed_count > 0:
            processed_files += 1
        total_removed += removed_count
    
    print(f"\nSummary:")
    print(f"- Total files scanned: {len(feature_files)}")
    print(f"- Files with @wip tags removed: {processed_files}")
    print(f"- Total @wip tags removed: {total_removed}")
    
    return 0

if __name__ == "__main__":
    exit(main())