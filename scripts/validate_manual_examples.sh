#!/bin/bash

# User Manual Example Validation Script
# Part of the User Manual Maintenance Plan
# Validates all code examples in user manual files

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
USER_MANUAL_DIR="$PROJECT_ROOT/docs/user-manual"
TEMP_DIR="$PROJECT_ROOT/temp/manual_validation"
RESULTS_FILE="$TEMP_DIR/validation_results.txt"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Statistics
TOTAL_EXAMPLES=0
VALID_EXAMPLES=0
INVALID_EXAMPLES=0
SKIPPED_EXAMPLES=0

echo -e "${BLUE}User Manual Example Validation${NC}"
echo "=================================="
echo "Validating code examples in user manual..."
echo

# Create temp directory
mkdir -p "$TEMP_DIR"
echo "# User Manual Example Validation Results" > "$RESULTS_FILE"
echo "**Generated:** $(date)" >> "$RESULTS_FILE"
echo >> "$RESULTS_FILE"

# Function to extract code blocks from markdown
extract_code_blocks() {
    local file="$1"
    local lang="$2"
    local output_dir="$3"
    local count=0
    
    # Extract asthra code blocks
    awk -v lang="$lang" -v output_dir="$output_dir" -v basename="$(basename "$file")" '
    BEGIN { in_block = 0; block_num = 0; }
    /^```asthra/ { 
        in_block = 1; 
        block_num++; 
        filename = output_dir "/" basename "_example_" block_num ".asthra"
        print "// Extracted from " basename " - Example " block_num > filename
        next 
    }
    /^```/ && in_block { 
        in_block = 0; 
        close(filename)
        print filename
        next 
    }
    in_block { print $0 >> filename }
    ' "$file"
}

# Function to validate a single example
validate_example() {
    local example_file="$1"
    local source_file="$2"
    local example_num="$3"
    
    TOTAL_EXAMPLES=$((TOTAL_EXAMPLES + 1))
    
    echo -n "  Example $example_num: "
    
    # Check if file exists and has content
    if [[ ! -f "$example_file" ]] || [[ ! -s "$example_file" ]]; then
        echo -e "${YELLOW}SKIPPED${NC} (empty or missing)"
        SKIPPED_EXAMPLES=$((SKIPPED_EXAMPLES + 1))
        echo "- ⚠️ **Example $example_num**: SKIPPED (empty or missing)" >> "$RESULTS_FILE"
        return
    fi
    
    # Check for basic syntax issues
    local has_package=$(grep -c "^package " "$example_file" || true)
    local has_function=$(grep -c "fn " "$example_file" || true)
    local has_braces=$(grep -c "{" "$example_file" || true)
    
    # Simple validation checks
    local issues=()
    
    # Check for removed features
    if grep -q "{[^}]*}" "$example_file" 2>/dev/null; then
        # Check if it's string interpolation (removed in v1.21)
        if grep -q "\"[^\"]*{[^}]*}[^\"]*\"" "$example_file" 2>/dev/null; then
            issues+=("Contains string interpolation (removed in v1.21)")
        fi
    fi
    
    # Check for missing type annotations (required in v1.15+)
    if grep -q "let [a-zA-Z_][a-zA-Z0-9_]* =" "$example_file" 2>/dev/null; then
        if ! grep -q "let [a-zA-Z_][a-zA-Z0-9_]*:" "$example_file" 2>/dev/null; then
            issues+=("Missing type annotations (required in v1.15+)")
        fi
    fi
    
    # Check for missing visibility modifiers (required in v1.16+)
    if grep -q "^fn " "$example_file" 2>/dev/null; then
        if ! grep -q "^pub fn \|^priv fn " "$example_file" 2>/dev/null; then
            issues+=("Missing visibility modifiers (required in v1.16+)")
        fi
    fi
    
    # Check for question mark operator (removed)
    if grep -q "?" "$example_file" 2>/dev/null; then
        issues+=("Contains question mark operator (removed)")
    fi
    
    # Report results
    if [[ ${#issues[@]} -eq 0 ]]; then
        echo -e "${GREEN}VALID${NC}"
        VALID_EXAMPLES=$((VALID_EXAMPLES + 1))
        echo "- ✅ **Example $example_num**: VALID" >> "$RESULTS_FILE"
    else
        echo -e "${RED}INVALID${NC}"
        INVALID_EXAMPLES=$((INVALID_EXAMPLES + 1))
        echo "- ❌ **Example $example_num**: INVALID" >> "$RESULTS_FILE"
        for issue in "${issues[@]}"; do
            echo "    - $issue"
            echo "  - $issue" >> "$RESULTS_FILE"
        done
    fi
}

# Process each user manual file
echo "Processing user manual files..."
echo

for manual_file in "$USER_MANUAL_DIR"/*.md; do
    if [[ ! -f "$manual_file" ]]; then
        continue
    fi
    
    filename=$(basename "$manual_file")
    echo -e "${BLUE}Processing: $filename${NC}"
    
    # Create directory for this file's examples
    file_temp_dir="$TEMP_DIR/$(basename "$filename" .md)"
    mkdir -p "$file_temp_dir"
    
    # Extract code blocks
    example_files=($(extract_code_blocks "$manual_file" "asthra" "$file_temp_dir"))
    
    if [[ ${#example_files[@]} -eq 0 ]]; then
        echo "  No Asthra code examples found"
        echo
        continue
    fi
    
    echo "  Found ${#example_files[@]} code example(s)"
    
    # Add file section to results
    echo >> "$RESULTS_FILE"
    echo "## $filename" >> "$RESULTS_FILE"
    echo >> "$RESULTS_FILE"
    
    # Validate each example
    for i in "${!example_files[@]}"; do
        validate_example "${example_files[$i]}" "$filename" "$((i + 1))"
    done
    
    echo
done

# Generate summary
echo "=================================="
echo -e "${BLUE}Validation Summary${NC}"
echo "=================================="
echo "Total examples: $TOTAL_EXAMPLES"
echo -e "Valid examples: ${GREEN}$VALID_EXAMPLES${NC}"
echo -e "Invalid examples: ${RED}$INVALID_EXAMPLES${NC}"
echo -e "Skipped examples: ${YELLOW}$SKIPPED_EXAMPLES${NC}"

if [[ $TOTAL_EXAMPLES -gt 0 ]]; then
    success_rate=$(( (VALID_EXAMPLES * 100) / TOTAL_EXAMPLES ))
    echo "Success rate: ${success_rate}%"
else
    success_rate=0
fi

# Add summary to results file
echo >> "$RESULTS_FILE"
echo "## Summary" >> "$RESULTS_FILE"
echo >> "$RESULTS_FILE"
echo "- **Total Examples**: $TOTAL_EXAMPLES" >> "$RESULTS_FILE"
echo "- **Valid Examples**: $VALID_EXAMPLES" >> "$RESULTS_FILE"
echo "- **Invalid Examples**: $INVALID_EXAMPLES" >> "$RESULTS_FILE"
echo "- **Skipped Examples**: $SKIPPED_EXAMPLES" >> "$RESULTS_FILE"
echo "- **Success Rate**: ${success_rate}%" >> "$RESULTS_FILE"
echo >> "$RESULTS_FILE"

# Add recommendations
echo "## Recommendations" >> "$RESULTS_FILE"
echo >> "$RESULTS_FILE"
if [[ $INVALID_EXAMPLES -gt 0 ]]; then
    echo "### Critical Issues to Fix" >> "$RESULTS_FILE"
    echo "1. Remove string interpolation syntax (replaced with concatenation)" >> "$RESULTS_FILE"
    echo "2. Add mandatory type annotations to all variable declarations" >> "$RESULTS_FILE"
    echo "3. Add explicit visibility modifiers (pub/priv) to all functions" >> "$RESULTS_FILE"
    echo "4. Replace question mark operators with explicit match expressions" >> "$RESULTS_FILE"
    echo >> "$RESULTS_FILE"
fi

echo "### Next Steps" >> "$RESULTS_FILE"
echo "1. Fix all invalid examples identified above" >> "$RESULTS_FILE"
echo "2. Add compilation testing for complex examples" >> "$RESULTS_FILE"
echo "3. Integrate validation into CI/CD pipeline" >> "$RESULTS_FILE"
echo "4. Set up automated example extraction and testing" >> "$RESULTS_FILE"

echo
echo "Results saved to: $RESULTS_FILE"

# Exit with appropriate code
if [[ $INVALID_EXAMPLES -gt 0 ]]; then
    echo -e "${RED}Validation failed: $INVALID_EXAMPLES invalid examples found${NC}"
    exit 1
else
    echo -e "${GREEN}All examples are valid!${NC}"
    exit 0
fi 
