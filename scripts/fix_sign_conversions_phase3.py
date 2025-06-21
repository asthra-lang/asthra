#!/usr/bin/env python3
"""
Phase 3: Sign Conversion Warning Fix Script
Systematically analyze and fix sign conversion warnings with type safety priority.
"""

import os
import re
import subprocess
import sys
from pathlib import Path
from collections import defaultdict, namedtuple
from typing import List, Dict, Tuple, Optional

# Data structures
SignConversionWarning = namedtuple('SignConversionWarning', [
    'file_path', 'line_number', 'column', 'warning_text', 'context'
])

FixSuggestion = namedtuple('FixSuggestion', [
    'category', 'safety_level', 'fix_type', 'original_code', 'suggested_fix', 'explanation'
])

class SignConversionAnalyzer:
    def __init__(self):
        self.warnings = []
        self.categorized_warnings = defaultdict(list)
        self.fix_suggestions = []
        
    def capture_warnings(self) -> List[SignConversionWarning]:
        """Capture all sign conversion warnings from build output."""
        print("🔍 Capturing sign conversion warnings...")
        
        try:
            # Clean build to get fresh warnings
            subprocess.run(['make', 'clean'], capture_output=True, check=True)
            
            # Build and capture warnings
            result = subprocess.run(['make'], capture_output=True, text=True)
            build_output = result.stderr + result.stdout
            
            # Parse warnings
            warning_pattern = r'([^:]+):(\d+):(\d+):\s*warning:\s*.*sign-conversion.*?\[(.*?)\]\s*(.*?)(?=\n|$)'
            
            for match in re.finditer(warning_pattern, build_output, re.MULTILINE):
                file_path = match.group(1).strip()
                line_number = int(match.group(2))
                column = int(match.group(3))
                warning_text = match.group(4).strip()
                context = match.group(5).strip() if match.group(5) else ""
                
                warning = SignConversionWarning(
                    file_path=file_path,
                    line_number=line_number,
                    column=column,
                    warning_text=warning_text,
                    context=context
                )
                self.warnings.append(warning)
                
            print(f"📊 Found {len(self.warnings)} sign conversion warnings")
            return self.warnings
            
        except subprocess.CalledProcessError as e:
            print(f"❌ Error capturing warnings: {e}")
            return []
    
    def categorize_warnings(self):
        """Categorize warnings by type and safety level."""
        print("📋 Categorizing warnings by safety level...")
        
        for warning in self.warnings:
            category = self._determine_category(warning)
            self.categorized_warnings[category].append(warning)
            
        # Print categorization summary
        for category, warnings in self.categorized_warnings.items():
            print(f"  {category}: {len(warnings)} warnings")
    
    def _determine_category(self, warning: SignConversionWarning) -> str:
        """Determine the category of a sign conversion warning."""
        file_path = warning.file_path.lower()
        context = warning.context.lower()
        
        # Category A: Safe Index/Size Conversions
        if any(keyword in context for keyword in ['strlen', 'sizeof', 'size_t', 'index', 'length']):
            return "A_SAFE_SIZE_INDEX"
        
        # Category B: API Boundary Conversions  
        if any(keyword in file_path for keyword in ['json', 'ffi', 'platform', 'third-party']):
            return "B_API_BOUNDARY"
            
        # Category C: Arithmetic/Logic Operations
        if any(keyword in context for keyword in ['+', '-', '*', '/', '%', '<<', '>>', '&', '|', '^']):
            return "C_ARITHMETIC_LOGIC"
            
        # Category D: Potential Unsafe Conversions
        if any(keyword in context for keyword in ['malloc', 'realloc', 'calloc', 'free']):
            return "D_MEMORY_OPERATIONS"
            
        # Default category
        return "E_GENERAL_CONVERSION"
    
    def analyze_source_context(self, warning: SignConversionWarning) -> Optional[str]:
        """Extract source code context around the warning."""
        try:
            with open(warning.file_path, 'r', encoding='utf-8', errors='ignore') as f:
                lines = f.readlines()
                
            if warning.line_number <= len(lines):
                # Get context lines (3 before, target, 3 after)
                start_line = max(0, warning.line_number - 4)
                end_line = min(len(lines), warning.line_number + 3)
                
                context_lines = []
                for i in range(start_line, end_line):
                    line_num = i + 1
                    marker = ">>> " if line_num == warning.line_number else "    "
                    context_lines.append(f"{marker}{line_num:4d}: {lines[i].rstrip()}")
                
                return "\n".join(context_lines)
                
        except Exception as e:
            print(f"⚠️  Could not read context from {warning.file_path}: {e}")
            
        return None
    
    def generate_fix_suggestions(self):
        """Generate fix suggestions for each category."""
        print("💡 Generating fix suggestions...")
        
        for category, warnings in self.categorized_warnings.items():
            for warning in warnings:
                suggestion = self._generate_fix_for_warning(warning, category)
                if suggestion:
                    self.fix_suggestions.append(suggestion)
    
    def _generate_fix_for_warning(self, warning: SignConversionWarning, category: str) -> Optional[FixSuggestion]:
        """Generate a specific fix suggestion for a warning."""
        context = self.analyze_source_context(warning)
        
        if category == "A_SAFE_SIZE_INDEX":
            return FixSuggestion(
                category="Safe Size/Index",
                safety_level="HIGH",
                fix_type="EXPLICIT_CAST",
                original_code=warning.context,
                suggested_fix="Add explicit cast: (int)size_value or (size_t)int_value",
                explanation="Safe conversion for size/index operations with known bounds"
            )
            
        elif category == "B_API_BOUNDARY":
            return FixSuggestion(
                category="API Boundary",
                safety_level="MEDIUM",
                fix_type="API_ADAPTATION",
                original_code=warning.context,
                suggested_fix="Use appropriate API types or add boundary checks",
                explanation="Ensure API compatibility with proper type handling"
            )
            
        elif category == "C_ARITHMETIC_LOGIC":
            return FixSuggestion(
                category="Arithmetic/Logic",
                safety_level="LOW",
                fix_type="MANUAL_REVIEW",
                original_code=warning.context,
                suggested_fix="Review arithmetic operation for overflow safety",
                explanation="Manual review required for arithmetic operations"
            )
            
        elif category == "D_MEMORY_OPERATIONS":
            return FixSuggestion(
                category="Memory Operations",
                safety_level="CRITICAL",
                fix_type="MANUAL_REVIEW",
                original_code=warning.context,
                suggested_fix="Review memory operation for safety",
                explanation="Critical review required for memory operations"
            )
            
        else:
            return FixSuggestion(
                category="General",
                safety_level="MEDIUM",
                fix_type="REVIEW_NEEDED",
                original_code=warning.context,
                suggested_fix="Review conversion context",
                explanation="General sign conversion requiring review"
            )
    
    def apply_safe_fixes(self, dry_run: bool = True):
        """Apply automated fixes for safe conversions."""
        print(f"🔧 {'Simulating' if dry_run else 'Applying'} safe automated fixes...")
        
        safe_fixes = [s for s in self.fix_suggestions if s.safety_level == "HIGH"]
        
        print(f"📊 Found {len(safe_fixes)} safe fixes to apply")
        
        if not dry_run:
            # TODO: Implement actual file modifications
            print("⚠️  Actual file modification not implemented yet - use manual review")
        
        return safe_fixes
    
    def generate_report(self, output_file: str = "sign_conversion_analysis_report.md"):
        """Generate comprehensive analysis report."""
        print(f"📄 Generating analysis report: {output_file}")
        
        with open(output_file, 'w') as f:
            f.write("# Sign Conversion Warning Analysis Report\n\n")
            f.write(f"**Generated**: {subprocess.check_output(['date']).decode().strip()}\n")
            f.write(f"**Total Warnings**: {len(self.warnings)}\n\n")
            
            # Summary by category
            f.write("## Warning Categories\n\n")
            for category, warnings in self.categorized_warnings.items():
                f.write(f"### {category.replace('_', ' ').title()}\n")
                f.write(f"**Count**: {len(warnings)}\n\n")
                
                for i, warning in enumerate(warnings[:5]):  # Show first 5 examples
                    f.write(f"#### Example {i+1}\n")
                    f.write(f"**File**: `{warning.file_path}:{warning.line_number}`\n")
                    f.write(f"**Warning**: {warning.warning_text}\n")
                    f.write(f"**Context**: {warning.context}\n\n")
                    
                    # Add source context if available
                    context = self.analyze_source_context(warning)
                    if context:
                        f.write("**Source Context**:\n```c\n")
                        f.write(context)
                        f.write("\n```\n\n")
                
                if len(warnings) > 5:
                    f.write(f"*... and {len(warnings) - 5} more warnings in this category*\n\n")
            
            # Fix suggestions summary
            f.write("## Fix Suggestions Summary\n\n")
            safety_counts = defaultdict(int)
            for suggestion in self.fix_suggestions:
                safety_counts[suggestion.safety_level] += 1
                
            for safety_level, count in safety_counts.items():
                f.write(f"- **{safety_level}**: {count} fixes\n")
            
            f.write("\n## Recommended Action Plan\n\n")
            f.write("1. **HIGH Safety**: Apply automated fixes immediately\n")
            f.write("2. **MEDIUM Safety**: Review and apply with testing\n")
            f.write("3. **LOW Safety**: Manual review required\n")
            f.write("4. **CRITICAL Safety**: Thorough analysis required\n\n")
            
        print(f"✅ Report generated: {output_file}")

def main():
    """Main execution function."""
    print("🚀 Starting Phase 3: Sign Conversion Warning Analysis")
    print("=" * 60)
    
    analyzer = SignConversionAnalyzer()
    
    # Step 1: Capture warnings
    warnings = analyzer.capture_warnings()
    if not warnings:
        print("✅ No sign conversion warnings found!")
        return 0
    
    # Step 2: Categorize warnings
    analyzer.categorize_warnings()
    
    # Step 3: Generate fix suggestions
    analyzer.generate_fix_suggestions()
    
    # Step 4: Apply safe fixes (dry run first)
    safe_fixes = analyzer.apply_safe_fixes(dry_run=True)
    
    # Step 5: Generate comprehensive report
    analyzer.generate_report()
    
    print("\n" + "=" * 60)
    print("📊 Phase 3 Analysis Complete!")
    print(f"📈 Total warnings analyzed: {len(warnings)}")
    print(f"🔧 Safe fixes identified: {len(safe_fixes)}")
    print("📄 Detailed report: sign_conversion_analysis_report.md")
    print("\n🎯 Next Steps:")
    print("1. Review the generated report")
    print("2. Apply safe fixes manually or with automation")
    print("3. Conduct manual review for complex cases")
    print("4. Test and validate all changes")
    
    return 0

if __name__ == "__main__":
    sys.exit(main()) 
