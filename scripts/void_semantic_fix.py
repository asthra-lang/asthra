#!/usr/bin/env python3
"""
Void Semantic Overloading Fix Migration Script
Asthra Programming Language v1.18 -> v1.19

This script transforms Asthra source files to use the new semantic clarity system:
- 'void' is preserved ONLY as a return type for functions returning nothing
- 'none' is used for all structural absence markers (empty parameters, fields, etc.)

Transformations Applied:
- Function parameters: fn name(void) -> fn name(none)  
- Struct content: struct S { void } -> struct S { none }
- Enum content: enum E { void } -> enum E { none }
- Array elements: [void] -> [none]
- Function arguments: func(void) -> func(none)
- Pattern arguments: Pattern.Variant(void) -> Pattern.Variant(none)
- Annotation parameters: #[attr(void)] -> #[attr(none)]
- Preserve return types: -> void (unchanged)

Usage:
    python3 scripts/void_semantic_fix.py file1.asthra file2.asthra ...
    python3 scripts/void_semantic_fix.py stdlib/**/*.asthra
    python3 scripts/void_semantic_fix.py examples/**/*.asthra

Author: Asthra Language Team
Version: 1.0.0 (Grammar v1.19)
"""

import re
import sys
import os
import argparse
from pathlib import Path
from typing import List, Tuple, Dict

class VoidSemanticMigrator:
    """
    Comprehensive migration tool for void->none semantic transformation.
    Handles all 8 structural contexts while preserving legitimate void return types.
    """
    
    def __init__(self, verbose: bool = False, dry_run: bool = False):
        self.verbose = verbose
        self.dry_run = dry_run
        self.stats = {
            'files_processed': 0,
            'files_changed': 0,
            'transformations_applied': 0,
            'backups_created': 0,
            'errors': 0
        }
        
        # Transformation patterns with detailed matching
        self.patterns = self._build_transformation_patterns()
    
    def _build_transformation_patterns(self) -> List[Tuple[str, str, str]]:
        """Build comprehensive regex patterns for all void->none transformations."""
        return [
            # 1. Function parameters: fn name(void) -> fn name(none)
            (
                r'(\b(?:pub|priv)\s+)?fn\s+(\w+)\s*\(\s*void\s*\)',
                r'\1fn \2(none)',
                'function_parameters'
            ),
            
            # 2. External function parameters: extern fn name(void) -> extern fn name(none)
            (
                r'extern\s+(?:"[^"]*")?\s*fn\s+(\w+)\s*\(\s*void\s*\)',
                r'extern fn \1(none)',
                'extern_function_parameters'
            ),
            
            # 3. Method parameters: fn method_name(void) in impl blocks
            (
                r'(\s+)fn\s+(\w+)\s*\(\s*void\s*\)',
                r'\1fn \2(none)',
                'method_parameters'
            ),
            
            # 4. Struct content: struct Name { void } -> struct Name { none }
            (
                r'struct\s+(\w+)(?:\s*<[^>]*>)?\s*\{\s*void\s*\}',
                r'struct \1 { none }',
                'struct_content'
            ),
            
            # 5. Enum content: enum Name { void } -> enum Name { none }
            (
                r'enum\s+(\w+)(?:\s*<[^>]*>)?\s*\{\s*void\s*\}',
                r'enum \1 { none }',
                'enum_content'
            ),
            
            # 6. Array literals: [void] -> [none]
            (
                r'\[\s*void\s*\]',
                r'[none]',
                'array_literals'
            ),
            
            # 7. Function calls: function_name(void) -> function_name(none)
            (
                r'(\w+)\s*\(\s*void\s*\)(?=\s*[;}.,\s])',
                r'\1(none)',
                'function_calls'
            ),
            
            # 8. Pattern arguments: EnumName.VariantName(void) -> EnumName.VariantName(none)
            (
                r'(\w+)\.(\w+)\s*\(\s*void\s*\)',
                r'\1.\2(none)',
                'pattern_arguments'
            ),
            
            # 9. Annotation parameters: #[annotation_name(void)] -> #[annotation_name(none)]
            (
                r'#\[(\w+)\s*\(\s*void\s*\)\]',
                r'#[\1(none)]',
                'annotation_parameters'
            ),
            
            # 10. Method self parameters edge case: self, void -> self, none (rare but possible)
            (
                r'(\bself\s*,\s*)void\s*\)',
                r'\1none)',
                'method_self_parameters'
            ),
        ]
    
    def _preserve_return_types(self, content: str) -> str:
        """
        Defensive preservation of return types.
        Ensure -> void is never changed to -> none.
        """
        # Fix any accidental transformations of return types
        content = re.sub(r'->\s*none(?=\s*\{)', r'-> void', content)
        return content
    
    def _apply_transformations(self, content: str) -> Tuple[str, Dict[str, int]]:
        """Apply all void->none transformations and track statistics."""
        transformation_counts = {}
        modified_content = content
        
        for pattern, replacement, category in self.patterns:
            # Count matches before transformation
            matches = re.findall(pattern, modified_content)
            match_count = len(matches)
            
            if match_count > 0:
                transformation_counts[category] = match_count
                modified_content = re.sub(pattern, replacement, modified_content)
                
                if self.verbose:
                    print(f"    - {category}: {match_count} transformations")
        
        # Defensive preservation of return types
        modified_content = self._preserve_return_types(modified_content)
        
        return modified_content, transformation_counts
    
    def _create_backup(self, filepath: Path) -> Path:
        """Create backup file with v1.18 suffix."""
        backup_path = filepath.with_suffix(f"{filepath.suffix}.v118_backup")
        backup_path.write_text(filepath.read_text(encoding='utf-8'), encoding='utf-8')
        return backup_path
    
    def _validate_asthra_file(self, filepath: Path) -> bool:
        """Validate that this is an Asthra source file."""
        if not filepath.suffix == '.asthra':
            if self.verbose:
                print(f"Skipping non-Asthra file: {filepath}")
            return False
        
        if not filepath.exists():
            print(f"❌ File not found: {filepath}")
            return False
        
        return True
    
    def process_file(self, filepath: Path) -> bool:
        """Process a single Asthra file and apply void->none transformations."""
        if not self._validate_asthra_file(filepath):
            return False
        
        try:
            if self.verbose:
                print(f"Processing: {filepath}")
            
            # Read original content
            original_content = filepath.read_text(encoding='utf-8')
            
            # Apply transformations
            modified_content, transformation_counts = self._apply_transformations(original_content)
            
            # Check if any changes were made
            if original_content == modified_content:
                if self.verbose:
                    print(f"    ✅ No changes needed")
                self.stats['files_processed'] += 1
                return True
            
            # Create backup if not dry run
            if not self.dry_run:
                backup_path = self._create_backup(filepath)
                self.stats['backups_created'] += 1
                if self.verbose:
                    print(f"    📁 Created backup: {backup_path}")
            
            # Write modified content if not dry run
            if not self.dry_run:
                filepath.write_text(modified_content, encoding='utf-8')
            
            # Update statistics
            total_transformations = sum(transformation_counts.values())
            self.stats['files_processed'] += 1
            self.stats['files_changed'] += 1
            self.stats['transformations_applied'] += total_transformations
            
            if self.verbose or self.dry_run:
                action = "Would apply" if self.dry_run else "Applied"
                print(f"    ✅ {action} {total_transformations} transformations")
                for category, count in transformation_counts.items():
                    print(f"       • {category}: {count}")
            
            return True
            
        except Exception as e:
            print(f"    ❌ Error processing {filepath}: {e}")
            self.stats['errors'] += 1
            return False
    
    def process_files(self, filepaths: List[Path]) -> None:
        """Process multiple Asthra files."""
        print("Asthra Void Semantic Overloading Fix (v1.18 -> v1.19)")
        print("=" * 60)
        
        if self.dry_run:
            print("🔍 DRY RUN MODE - No files will be modified")
        
        print(f"Processing {len(filepaths)} files...")
        print()
        
        for filepath in filepaths:
            self.process_file(filepath)
            if self.verbose:
                print()
        
        self._print_summary()
    
    def _print_summary(self) -> None:
        """Print comprehensive migration summary."""
        print("\n" + "=" * 60)
        print("MIGRATION SUMMARY")
        print("=" * 60)
        
        print(f"Files processed: {self.stats['files_processed']}")
        print(f"Files changed: {self.stats['files_changed']}")
        print(f"Total transformations: {self.stats['transformations_applied']}")
        print(f"Backups created: {self.stats['backups_created']}")
        print(f"Errors encountered: {self.stats['errors']}")
        
        if self.stats['errors'] == 0 and not self.dry_run:
            print("\n🎉 All files successfully migrated to v1.19 void semantic clarity!")
            print("✅ Grammar now enforces clear semantic boundaries:")
            print("   • 'void' = type absence (function return types)")
            print("   • 'none' = structural absence (parameters, fields, etc.)")
        elif self.dry_run:
            print(f"\n🔍 Dry run completed. {self.stats['files_changed']} files would be modified.")
        else:
            print(f"\n⚠️  {self.stats['errors']} files require manual attention")

def expand_glob_patterns(patterns: List[str]) -> List[Path]:
    """Expand glob patterns to actual file paths."""
    filepaths = []
    
    for pattern in patterns:
        path = Path(pattern)
        
        if path.is_file():
            filepaths.append(path)
        elif '*' in pattern or '?' in pattern:
            # Handle glob patterns
            if '**' in pattern:
                # Recursive glob
                base_parts = pattern.split('**')
                base_path = Path(base_parts[0]) if base_parts[0] else Path('.')
                suffix_pattern = base_parts[1].lstrip('/')
                filepaths.extend(base_path.rglob(suffix_pattern))
            else:
                # Simple glob
                base_path = Path(pattern).parent
                pattern_name = Path(pattern).name
                filepaths.extend(base_path.glob(pattern_name))
        else:
            # Regular path or directory
            if path.is_dir():
                filepaths.extend(path.rglob('*.asthra'))
            else:
                print(f"Warning: Path not found: {pattern}")
    
    # Filter to only .asthra files and remove duplicates
    asthra_files = []
    seen = set()
    for filepath in filepaths:
        if filepath.suffix == '.asthra' and filepath not in seen:
            asthra_files.append(filepath)
            seen.add(filepath)
    
    return asthra_files

def main():
    """Main entry point for the migration script."""
    parser = argparse.ArgumentParser(
        description="Asthra Void Semantic Overloading Fix Migration Tool",
        epilog="""
Examples:
  python3 scripts/void_semantic_fix.py file1.asthra file2.asthra
  python3 scripts/void_semantic_fix.py stdlib/**/*.asthra
  python3 scripts/void_semantic_fix.py examples/ docs/examples/
  python3 scripts/void_semantic_fix.py --dry-run --verbose stdlib/
        """,
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    
    parser.add_argument(
        'files',
        nargs='+',
        help='Asthra files or directories to process (supports glob patterns)'
    )
    
    parser.add_argument(
        '--dry-run',
        action='store_true',
        help='Show what would be changed without modifying files'
    )
    
    parser.add_argument(
        '--verbose', '-v',
        action='store_true',
        help='Show detailed processing information'
    )
    
    args = parser.parse_args()
    
    # Expand file patterns
    filepaths = expand_glob_patterns(args.files)
    
    if not filepaths:
        print("❌ No Asthra files found to process")
        sys.exit(1)
    
    # Create migrator and process files
    migrator = VoidSemanticMigrator(verbose=args.verbose, dry_run=args.dry_run)
    migrator.process_files(filepaths)
    
    # Exit with error code if there were errors
    if migrator.stats['errors'] > 0:
        sys.exit(1)

if __name__ == "__main__":
    main() 
