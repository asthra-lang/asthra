#!/bin/bash
# Check that all variable declarations have explicit types
# Part of Type Annotation Requirement Plan - Phase 7.1

set -euo pipefail

echo "🔍 Checking for missing type annotations in Asthra files..."

# Configuration
TARGET_DIRS=("src" "stdlib" "examples" "docs/examples" "tests")
TOTAL_FILES=0
FILES_WITH_ISSUES=0
TOTAL_ISSUES=0

# Function to check a single file
check_file() {
    local file="$1"
    local issues=0
    local line_num=0
    
    while IFS= read -r line; do
        line_num=$((line_num + 1))
        
        # Skip comments and empty lines
        if [[ "$line" =~ ^[[:space:]]*// ]] || [[ -z "${line// }" ]]; then
            continue
        fi
        
        # Check for let statements without colons (missing types)
        # Pattern: let identifier = (not let identifier :)
        if [[ "$line" =~ ^[[:space:]]*let[[:space:]]+[a-zA-Z_][a-zA-Z0-9_]*[[:space:]]*= ]]; then
            echo "❌ $file:$line_num: Missing type annotation"
            echo "   Line: $line"
            echo "   Fix:  Add explicit type annotation (e.g., 'let name: Type = value;')"
            echo ""
            issues=$((issues + 1))
        fi
        
        # Check for specific problematic patterns
        if [[ "$line" =~ let[[:space:]]+[a-zA-Z_][a-zA-Z0-9_]*[[:space:]]*=[[:space:]]*[0-9]+[[:space:]]*\; ]]; then
            echo "⚠️  $file:$line_num: Numeric literal without type annotation"
            echo "   Line: $line"
            echo "   Suggestion: Specify numeric type (i32, f64, etc.)"
            echo ""
        fi
        
        if [[ "$line" =~ let[[:space:]]+[a-zA-Z_][a-zA-Z0-9_]*[[:space:]]*=[[:space:]]*\".*\"[[:space:]]*\; ]]; then
            echo "⚠️  $file:$line_num: String literal without type annotation"
            echo "   Line: $line" 
            echo "   Suggestion: Add ': string' type annotation"
            echo ""
        fi
        
        if [[ "$line" =~ let[[:space:]]+[a-zA-Z_][a-zA-Z0-9_]*[[:space:]]*=[[:space:]]*(true|false)[[:space:]]*\; ]]; then
            echo "⚠️  $file:$line_num: Boolean literal without type annotation"
            echo "   Line: $line"
            echo "   Suggestion: Add ': bool' type annotation"
            echo ""
        fi
        
    done < "$file"
    
    return $issues
}

# Function to scan directory
scan_directory() {
    local dir="$1"
    local file_count=0
    local issues_count=0
    
    if [[ ! -d "$dir" ]]; then
        echo "⚠️  Directory $dir not found, skipping..."
        return 0
    fi
    
    echo "📁 Scanning directory: $dir"
    
    while IFS= read -r -d '' file; do
        file_count=$((file_count + 1))
        TOTAL_FILES=$((TOTAL_FILES + 1))
        
        if check_file "$file"; then
            file_issues=$?
            if [[ $file_issues -gt 0 ]]; then
                issues_count=$((issues_count + file_issues))
                FILES_WITH_ISSUES=$((FILES_WITH_ISSUES + 1))
                TOTAL_ISSUES=$((TOTAL_ISSUES + file_issues))
            fi
        fi
    done < <(find "$dir" -name "*.asthra" -type f -print0 2>/dev/null)
    
    echo "   📊 Files scanned: $file_count"
    if [[ $issues_count -gt 0 ]]; then
        echo "   ❌ Issues found: $issues_count"
    else
        echo "   ✅ No issues found"
    fi
    echo ""
}

# Main execution
main() {
    echo "Type Annotation Checker for Asthra v1.15+"
    echo "========================================"
    echo ""
    
    # Check if we're in the right directory
    if [[ ! -f "grammar.txt" ]] && [[ ! -f "grammar.txt" ]]; then
        echo "❌ Error: This script should be run from the Asthra project root directory"
        exit 1
    fi
    
    # Scan all target directories
    for dir in "${TARGET_DIRS[@]}"; do
        scan_directory "$dir"
    done
    
    # Summary report
    echo "📋 SUMMARY REPORT"
    echo "=================="
    echo "Total files scanned: $TOTAL_FILES"
    echo "Files with issues: $FILES_WITH_ISSUES"
    echo "Total issues found: $TOTAL_ISSUES"
    echo ""
    
    if [[ $TOTAL_ISSUES -eq 0 ]]; then
        echo "✅ SUCCESS: All variable declarations have explicit type annotations!"
        echo ""
        echo "🎯 Benefits achieved:"
        echo "   • Eliminated AI generation ambiguity"
        echo "   • Self-documenting variable declarations"
        echo "   • Consistent code style across the project"
        echo "   • Improved IDE support and error messages"
        exit 0
    else
        echo "❌ FAILURE: Found $TOTAL_ISSUES variable declarations without type annotations"
        echo ""
        echo "🛠️  How to fix:"
        echo "   1. Run the automated migration script:"
        echo "      python3 scripts/add_type_annotations.py <files>"
        echo ""
        echo "   2. For manual fixes, add explicit type annotations:"
        echo "      BEFORE: let x = 42;"
        echo "      AFTER:  let x: i32 = 42;"
        echo ""
        echo "   3. See docs/TYPE_ANNOTATION_STYLE_GUIDE.md for complete guidance"
        echo ""
        echo "📚 Documentation:"
        echo "   • Type Annotation Style Guide: docs/TYPE_ANNOTATION_STYLE_GUIDE.md"
        echo "   • Grammar specification: grammar.txt (v1.15+)"
        echo "   • Migration plan: docs/TYPE_ANNOTATION_REQUIREMENT_PLAN.md"
        exit 1
    fi
}

# Help function
show_help() {
    echo "Type Annotation Checker for Asthra v1.15+"
    echo ""
    echo "USAGE:"
    echo "    $0 [options]"
    echo ""
    echo "OPTIONS:"
    echo "    -h, --help     Show this help message"
    echo "    -v, --verbose  Show verbose output"
    echo ""
    echo "DESCRIPTION:"
    echo "    Checks all Asthra files for missing type annotations in variable declarations."
    echo "    Starting with v1.15, all 'let' statements must include explicit type annotations."
    echo ""
    echo "DIRECTORIES SCANNED:"
    for dir in "${TARGET_DIRS[@]}"; do
        echo "    • $dir"
    done
    echo ""
    echo "EXAMPLES:"
    echo "    $0                    # Check all files"
    echo "    $0 --verbose          # Check with detailed output"
    echo ""
    echo "EXIT CODES:"
    echo "    0 - All files have proper type annotations"
    echo "    1 - Some files are missing type annotations"
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -v|--verbose)
            set -x
            shift
            ;;
        *)
            echo "❌ Unknown option: $1"
            echo "Use '$0 --help' for usage information"
            exit 1
            ;;
    esac
done

# Run main function
main 
