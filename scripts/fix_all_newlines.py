#!/usr/bin/env python3
"""
Comprehensive newline fix script for all source files.
Fixes all -Wnewline-eof warnings by adding missing newlines to source files.
"""

import os
import glob
import sys
from pathlib import Path

def find_files_missing_newlines():
    """Find all source files missing newlines."""
    print("Scanning for files missing newlines...")
    
    patterns = ['**/*.c', '**/*.h', '**/*.cpp', '**/*.hpp']
    files_to_fix = []
    total_scanned = 0
    
    for pattern in patterns:
        for file_path in glob.glob(pattern, recursive=True):
            total_scanned += 1
            if should_process_file(file_path):
                if file_missing_newline(file_path):
                    files_to_fix.append(file_path)
    
    print(f"Scanned {total_scanned} files, found {len(files_to_fix)} files missing newlines")
    return files_to_fix

def should_process_file(file_path):
    """Check if file should be processed."""
    exclude_dirs = ['build/', '.git/', 'third-party/', '__pycache__/']
    exclude_files = ['.gitignore', 'Makefile']
    
    # Skip excluded directories
    if any(exclude in file_path for exclude in exclude_dirs):
        return False
    
    # Skip excluded files
    if any(file_path.endswith(exclude) for exclude in exclude_files):
        return False
    
    # Skip if file doesn't exist or is not readable
    if not os.path.isfile(file_path) or not os.access(file_path, os.R_OK):
        return False
    
    return True

def file_missing_newline(file_path):
    """Check if file is missing final newline."""
    try:
        with open(file_path, 'rb') as f:
            content = f.read()
        
        # Empty files don't need newlines
        if not content:
            return False
            
        # Check if file ends with newline
        return not content.endswith(b'\n')
    except Exception as e:
        print(f"Warning: Could not read {file_path}: {e}")
        return False

def fix_newlines(files, dry_run=False):
    """Add newlines to all files."""
    print(f"\n{'DRY RUN: Would fix' if dry_run else 'Fixing'} {len(files)} files:")
    
    fixed_count = 0
    error_count = 0
    
    for file_path in files:
        try:
            if dry_run:
                print(f"  Would fix: {file_path}")
                fixed_count += 1
            else:
                with open(file_path, 'ab') as f:
                    f.write(b'\n')
                print(f"  Fixed: {file_path}")
                fixed_count += 1
        except Exception as e:
            print(f"  Error fixing {file_path}: {e}")
            error_count += 1
    
    print(f"\nResults:")
    print(f"  {'Would fix' if dry_run else 'Fixed'}: {fixed_count} files")
    if error_count > 0:
        print(f"  Errors: {error_count} files")
    
    return fixed_count, error_count

def get_files_by_directory(files):
    """Group files by directory for reporting."""
    dirs = {}
    for file_path in files:
        dir_name = os.path.dirname(file_path) or '.'
        if dir_name not in dirs:
            dirs[dir_name] = []
        dirs[dir_name].append(os.path.basename(file_path))
    return dirs

def print_detailed_report(files):
    """Print detailed report of files to be fixed."""
    if not files:
        print("No files need newline fixes!")
        return
    
    print(f"\nDetailed Report - {len(files)} files missing newlines:")
    print("=" * 60)
    
    dirs = get_files_by_directory(files)
    
    for dir_name in sorted(dirs.keys()):
        print(f"\n{dir_name}/ ({len(dirs[dir_name])} files):")
        for file_name in sorted(dirs[dir_name]):
            print(f"  - {file_name}")

def verify_fixes(files):
    """Verify that fixes were applied correctly."""
    print("\nVerifying fixes...")
    
    still_missing = []
    for file_path in files:
        if file_missing_newline(file_path):
            still_missing.append(file_path)
    
    if still_missing:
        print(f"Warning: {len(still_missing)} files still missing newlines:")
        for file_path in still_missing:
            print(f"  - {file_path}")
        return False
    else:
        print("✅ All fixes verified successfully!")
        return True

def main():
    """Main function."""
    print("Asthra Comprehensive Newline Fix Script")
    print("=" * 50)
    
    # Change to project root if we're in scripts directory
    if os.path.basename(os.getcwd()) == 'scripts':
        os.chdir('..')
        print("Changed to project root directory")
    
    # Verify we're in the right directory
    if not os.path.exists("runtime") or not os.path.exists("src"):
        print("Error: Not in Asthra project root directory")
        sys.exit(1)
    
    # Parse command line arguments
    dry_run = '--dry-run' in sys.argv or '-n' in sys.argv
    verbose = '--verbose' in sys.argv or '-v' in sys.argv
    
    if dry_run:
        print("Running in DRY RUN mode - no files will be modified")
    
    try:
        # Find files missing newlines
        files_to_fix = find_files_missing_newlines()
        
        if not files_to_fix:
            print("\n✅ No files missing newlines found!")
            return
        
        # Print detailed report if verbose or dry run
        if verbose or dry_run:
            print_detailed_report(files_to_fix)
        
        # Ask for confirmation unless dry run
        if not dry_run:
            print(f"\nReady to fix {len(files_to_fix)} files.")
            response = input("Continue? (y/N): ").strip().lower()
            if response not in ['y', 'yes']:
                print("Aborted by user.")
                return
        
        # Fix the files
        fixed_count, error_count = fix_newlines(files_to_fix, dry_run)
        
        # Verify fixes if not dry run
        if not dry_run and fixed_count > 0:
            verify_fixes(files_to_fix)
        
        # Final summary
        print("\n" + "=" * 50)
        if dry_run:
            print(f"DRY RUN COMPLETE: Would fix {fixed_count} files")
            print("Run without --dry-run to apply fixes")
        else:
            print(f"NEWLINE FIXES COMPLETE: Fixed {fixed_count} files")
            if error_count == 0:
                print("✅ All fixes applied successfully!")
                print("Run 'make clean && make' to verify build")
            else:
                print(f"⚠️  {error_count} files had errors")
        
    except KeyboardInterrupt:
        print("\nInterrupted by user.")
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main() 
