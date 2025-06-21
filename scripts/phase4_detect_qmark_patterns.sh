#!/bin/bash
# detect_qmark_patterns.sh - Identify all question mark operator patterns
# Part of Question Mark Operator Removal - Phase 4 Automation Plan

set -e

echo "=== Question Mark Operator Pattern Detection ==="
echo "Scanning stdlib/, examples/, and docs/ directories..."

# Create temporary files for analysis
TEMP_DIR="/tmp/qmark_phase4"
mkdir -p "$TEMP_DIR"
FILES_LIST="$TEMP_DIR/qmark_files.txt"
PATTERNS_REPORT="$TEMP_DIR/patterns_report.txt"
STATS_REPORT="$TEMP_DIR/stats_report.txt"

# Find all files with question mark operators (excluding false positives)
echo "Finding files with question mark operators..."
find stdlib/ examples/ docs/ -name "*.asthra" -exec grep -l '\?' {} \; | \
    grep -v '\.backup\|\.tmp\|\.bak' > "$FILES_LIST" || true

# Count total files found
total_files=$(wc -l < "$FILES_LIST")
echo "Found $total_files files with question mark operators"

if [ "$total_files" -eq 0 ]; then
    echo "✅ No question mark operators found! Phase 4 may already be complete."
    exit 0
fi

# Analyze patterns in each file
echo ""
echo "=== Pattern Analysis Report ===" > "$PATTERNS_REPORT"
echo "Generated: $(date)" >> "$PATTERNS_REPORT"
echo "" >> "$PATTERNS_REPORT"

# Initialize counters (compatible with older bash)
simple_operation_count=0
assignment_count=0
await_count=0
chained_count=0
stdlib_count=0
examples_count=0
docs_count=0

while read -r file; do
    if [ ! -f "$file" ]; then
        continue
    fi
    
    echo "Analyzing: $file"
    echo "File: $file" >> "$PATTERNS_REPORT"
    echo "----------------------------------------" >> "$PATTERNS_REPORT"
    
    # Extract question mark patterns (exclude false positives like "?" in strings)
    patterns=$(grep -n '\?' "$file" | grep -v 'format!\|println!\|".*?.*"' | head -10)
    
    if [ -n "$patterns" ]; then
        echo "$patterns" >> "$PATTERNS_REPORT"
        
        # Count pattern types
        if echo "$patterns" | grep -q '\..*()?\s*;'; then
            simple_operation_count=$((simple_operation_count + 1))
        fi
        if echo "$patterns" | grep -q 'let.*=.*?\s*;'; then
            assignment_count=$((assignment_count + 1))
        fi
        if echo "$patterns" | grep -q 'await.*?'; then
            await_count=$((await_count + 1))
        fi
        if echo "$patterns" | grep -q '\..*()?\s*\.'; then
            chained_count=$((chained_count + 1))
        fi
        
        # Count by file category
        if [[ "$file" == stdlib/* ]]; then
            stdlib_count=$((stdlib_count + 1))
        elif [[ "$file" == examples/* ]]; then
            examples_count=$((examples_count + 1))
        elif [[ "$file" == docs/* ]]; then
            docs_count=$((docs_count + 1))
        fi
    else
        echo "No question mark patterns found" >> "$PATTERNS_REPORT"
    fi
    
    echo "" >> "$PATTERNS_REPORT"
done < "$FILES_LIST"

# Generate statistics report
echo ""
echo "=== Statistics Report ===" > "$STATS_REPORT"
echo "Generated: $(date)" >> "$STATS_REPORT"
echo "" >> "$STATS_REPORT"

echo "File Categories:" >> "$STATS_REPORT"
echo "  stdlib: $stdlib_count files" >> "$STATS_REPORT"
echo "  examples: $examples_count files" >> "$STATS_REPORT"
echo "  docs: $docs_count files" >> "$STATS_REPORT"

echo "" >> "$STATS_REPORT"
echo "Pattern Types:" >> "$STATS_REPORT"
echo "  simple_operation: $simple_operation_count occurrences" >> "$STATS_REPORT"
echo "  assignment: $assignment_count occurrences" >> "$STATS_REPORT"
echo "  await: $await_count occurrences" >> "$STATS_REPORT"
echo "  chained: $chained_count occurrences" >> "$STATS_REPORT"

# Count total instances
total_instances=$(grep -r '\?' stdlib/ examples/ docs/ --include="*.asthra" | \
    grep -v 'format!\|println!\|".*?.*"' | wc -l)

echo "" >> "$STATS_REPORT"
echo "Total question mark operator instances: $total_instances" >> "$STATS_REPORT"
echo "Total files requiring fixes: $total_files" >> "$STATS_REPORT"

# Display summary
echo ""
echo "=== Summary ==="
echo "Files requiring fixes: $total_files"
echo "Estimated total instances: $total_instances"
echo ""
echo "Pattern breakdown:"
echo "  simple_operation: $simple_operation_count"
echo "  assignment: $assignment_count"
echo "  await: $await_count"
echo "  chained: $chained_count"

echo ""
echo "Category breakdown:"
echo "  stdlib: $stdlib_count files"
echo "  examples: $examples_count files"
echo "  docs: $docs_count files"

echo ""
echo "Reports generated:"
echo "  Pattern analysis: $PATTERNS_REPORT"
echo "  Statistics: $STATS_REPORT"
echo "  File list: $FILES_LIST"

echo ""
echo "Next steps:"
echo "1. Review pattern analysis report"
echo "2. Run systematic replacement script"
echo "3. Validate fixes with QA script" 
