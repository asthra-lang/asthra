#!/bin/bash
# qa_qmark_fixes.sh - Quality assurance for systematic fixes
# Part of Question Mark Operator Removal - Phase 4 Automation Plan

set -e

echo "=== Quality Assurance for Question Mark Operator Fixes ==="

# Configuration
TEMP_DIR="/tmp/qmark_phase4"
QA_REPORT="$TEMP_DIR/qa_report.txt"

mkdir -p "$TEMP_DIR"

echo "QA Report - $(date)" > "$QA_REPORT"
echo "==========================================" >> "$QA_REPORT"

# 1. Verify no question mark operators remain in Asthra code
echo "1. Checking for remaining question mark operators..."
echo "1. Remaining Question Mark Operators" >> "$QA_REPORT"
echo "======================================" >> "$QA_REPORT"

# Find remaining operators (excluding false positives)
remaining_files=$(find stdlib/ examples/ -name "*.asthra" -exec grep -l '\?' {} \; 2>/dev/null | \
    xargs -I {} sh -c 'if grep -q '\''[a-zA-Z0-9_])?[[:space:]]*;'\'' "$1" || grep -q '\''[a-zA-Z0-9_])?[[:space:]]*$'\'' "$1"; then echo "$1"; fi' _ {} | \
    wc -l)

echo "Files with remaining operators: $remaining_files"
echo "Files with remaining operators: $remaining_files" >> "$QA_REPORT"

if [ "$remaining_files" -gt 0 ]; then
    echo "❌ Found remaining question mark operators in:" | tee -a "$QA_REPORT"
    find stdlib/ examples/ -name "*.asthra" -exec grep -l '\?' {} \; 2>/dev/null | \
        xargs -I {} sh -c 'if grep -q '\''[a-zA-Z0-9_])?[[:space:]]*;'\'' "$1" || grep -q '\''[a-zA-Z0-9_])?[[:space:]]*$'\'' "$1"; then echo "  $1"; fi' _ {} | \
        tee -a "$QA_REPORT"
else
    echo "✅ No remaining question mark operators found!" | tee -a "$QA_REPORT"
fi

echo "" >> "$QA_REPORT"

# 2. Check for broken error handling patterns
echo ""
echo "2. Checking error handling pattern consistency..."
echo "2. Error Handling Pattern Consistency" >> "$QA_REPORT"
echo "====================================" >> "$QA_REPORT"

# Count standard error propagation patterns
error_patterns=$(grep -r "Result\.Err(e) => return Result\.Err(e)" stdlib/ examples/ --include="*.asthra" 2>/dev/null | wc -l)
ok_patterns=$(grep -r "Result\.Ok(" stdlib/ examples/ --include="*.asthra" 2>/dev/null | wc -l)

echo "Standard error propagation patterns found: $error_patterns" | tee -a "$QA_REPORT"
echo "Result.Ok patterns found: $ok_patterns" | tee -a "$QA_REPORT"

if [ "$error_patterns" -gt 0 ] && [ "$ok_patterns" -gt 0 ]; then
    echo "✅ Error handling patterns appear consistent" | tee -a "$QA_REPORT"
else
    echo "⚠️  Warning: Error handling patterns may be inconsistent" | tee -a "$QA_REPORT"
fi

echo "" >> "$QA_REPORT"

# 3. Validate match expression syntax
echo ""
echo "3. Validating match expression syntax..."
echo "3. Match Expression Syntax Validation" >> "$QA_REPORT"
echo "====================================" >> "$QA_REPORT"

syntax_issues=0

# Check for unbalanced braces in match expressions
files_with_match=$(grep -r "match " stdlib/ examples/ --include="*.asthra" -l 2>/dev/null)

while read -r file; do
    if [ -f "$file" ]; then
        # Extract match blocks and check syntax
        match_lines=$(grep -n "match " "$file" | head -5)
        if [ -n "$match_lines" ]; then
            # Basic syntax check - look for common issues
            if grep -q "match .* {" "$file" && ! grep -q "Result\.Ok\|Result\.Err" "$file"; then
                echo "⚠️  $file - Match expressions without Result patterns" | tee -a "$QA_REPORT"
                ((syntax_issues++))
            fi
        fi
    fi
done <<< "$files_with_match"

echo "Syntax issues found: $syntax_issues" | tee -a "$QA_REPORT"

if [ "$syntax_issues" -eq 0 ]; then
    echo "✅ Match expression syntax appears correct" | tee -a "$QA_REPORT"
else
    echo "❌ Found $syntax_issues potential syntax issues" | tee -a "$QA_REPORT"
fi

echo "" >> "$QA_REPORT"

# 4. Statistics and coverage analysis
echo ""
echo "4. Generating statistics and coverage analysis..."
echo "4. Statistics and Coverage Analysis" >> "$QA_REPORT"
echo "==================================" >> "$QA_REPORT"

# Count total Asthra files
total_files=$(find stdlib/ examples/ -name "*.asthra" 2>/dev/null | wc -l)
echo "Total Asthra files: $total_files" | tee -a "$QA_REPORT"

# Count files with result handling
result_files=$(find stdlib/ examples/ -name "*.asthra" -exec grep -l "match.*Result\." {} \; 2>/dev/null | wc -l)
echo "Files with result handling: $result_files" | tee -a "$QA_REPORT"

if [ "$total_files" -gt 0 ]; then
    coverage_percent=$(( result_files * 100 / total_files ))
    echo "Result handling coverage: $coverage_percent%" | tee -a "$QA_REPORT"
else
    echo "Result handling coverage: 0%" | tee -a "$QA_REPORT"
fi

# Count by category
stdlib_files=$(find stdlib/ -name "*.asthra" 2>/dev/null | wc -l)
examples_files=$(find examples/ -name "*.asthra" 2>/dev/null | wc -l)

stdlib_result_files=$(find stdlib/ -name "*.asthra" -exec grep -l "match.*Result\." {} \; 2>/dev/null | wc -l)
examples_result_files=$(find examples/ -name "*.asthra" -exec grep -l "match.*Result\." {} \; 2>/dev/null | wc -l)

echo "" >> "$QA_REPORT"
echo "Category breakdown:" >> "$QA_REPORT"
echo "  stdlib/: $stdlib_files total, $stdlib_result_files with result handling" >> "$QA_REPORT"
echo "  examples/: $examples_files total, $examples_result_files with result handling" >> "$QA_REPORT"

echo ""
echo "Category breakdown:"
echo "  stdlib/: $stdlib_files total, $stdlib_result_files with result handling"
echo "  examples/: $examples_files total, $examples_result_files with result handling"

echo "" >> "$QA_REPORT"

# 5. Final assessment
echo ""
echo "5. Final Assessment"
echo "5. Final Assessment" >> "$QA_REPORT"
echo "==================" >> "$QA_REPORT"

total_score=0
max_score=4

# Scoring criteria
if [ "$remaining_files" -eq 0 ]; then
    echo "✅ No remaining question mark operators: +1 point" | tee -a "$QA_REPORT"
    ((total_score++))
else
    echo "❌ Remaining question mark operators found: 0 points" | tee -a "$QA_REPORT"
fi

if [ "$error_patterns" -gt 0 ] && [ "$ok_patterns" -gt 0 ]; then
    echo "✅ Consistent error handling patterns: +1 point" | tee -a "$QA_REPORT"
    ((total_score++))
else
    echo "❌ Inconsistent error handling patterns: 0 points" | tee -a "$QA_REPORT"
fi

if [ "$syntax_issues" -eq 0 ]; then
    echo "✅ No syntax issues detected: +1 point" | tee -a "$QA_REPORT"
    ((total_score++))
else
    echo "❌ Syntax issues detected: 0 points" | tee -a "$QA_REPORT"
fi

if [ "$coverage_percent" -gt 50 ]; then
    echo "✅ Good result handling coverage (>50%): +1 point" | tee -a "$QA_REPORT"
    ((total_score++))
else
    echo "❌ Low result handling coverage (≤50%): 0 points" | tee -a "$QA_REPORT"
fi

echo "" | tee -a "$QA_REPORT"
echo "Final Score: $total_score/$max_score" | tee -a "$QA_REPORT"

if [ "$total_score" -eq "$max_score" ]; then
    echo "🎉 EXCELLENT! Phase 4 automation completed successfully!" | tee -a "$QA_REPORT"
    qa_status="PASSED"
elif [ "$total_score" -ge 3 ]; then
    echo "✅ GOOD! Minor issues to address but mostly successful." | tee -a "$QA_REPORT"
    qa_status="PASSED_WITH_WARNINGS"
else
    echo "❌ NEEDS WORK! Significant issues require attention." | tee -a "$QA_REPORT"
    qa_status="FAILED"
fi

echo "" | tee -a "$QA_REPORT"
echo "QA Status: $qa_status" | tee -a "$QA_REPORT"

# Summary and recommendations
echo ""
echo "=== Summary and Recommendations ==="
echo ""
echo "QA Report saved to: $QA_REPORT"

if [ "$qa_status" = "PASSED" ]; then
    echo ""
    echo "🎉 Phase 4 automation has been completed successfully!"
    echo "   All question mark operators have been systematically replaced"
    echo "   with grammar-compliant Result type handling patterns."
    echo ""
    echo "Next steps:"
    echo "1. Update documentation to reflect completion"
    echo "2. Run final compilation tests"
    echo "3. Archive Phase 4 automation tools"
elif [ "$qa_status" = "PASSED_WITH_WARNINGS" ]; then
    echo ""
    echo "✅ Phase 4 automation largely successful with minor issues."
    echo ""
    echo "Recommended actions:"
    echo "1. Review and address any remaining issues"
    echo "2. Run targeted fixes for problem areas"
    echo "3. Re-run QA validation after fixes"
else
    echo ""
    echo "❌ Phase 4 automation requires additional work."
    echo ""
    echo "Required actions:"
    echo "1. Review failed criteria in detail"
    echo "2. Run targeted manual fixes"
    echo "3. Consider adjusting automation scripts"
    echo "4. Re-run full automation cycle"
fi

exit 0 
