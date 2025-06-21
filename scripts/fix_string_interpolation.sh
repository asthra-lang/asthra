#!/bin/bash

# String Interpolation Fix Script
# Part of the User Manual Maintenance Plan Phase 1
# Fixes string interpolation (removed in v1.21) by replacing with concatenation

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
USER_MANUAL_DIR="$PROJECT_ROOT/docs/user-manual"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}String Interpolation Fix Script${NC}"
echo "======================================"
echo "Fixing string interpolation issues in user manual..."
echo

# Function to fix string interpolation in a file
fix_string_interpolation() {
    local file="$1"
    local backup_file="${file}.backup"
    
    echo -n "Processing $(basename "$file"): "
    
    # Create backup
    cp "$file" "$backup_file"
    
    # Count original interpolations
    local original_count=$(grep -o '{[^}]*}' "$file" | wc -l || echo "0")
    
    if [[ $original_count -eq 0 ]]; then
        echo -e "${GREEN}No interpolation found${NC}"
        rm "$backup_file"
        return
    fi
    
    # Fix common string interpolation patterns
    # Pattern 1: log("LEVEL", "Message: {variable}");
    sed -i.tmp 's/log("\([^"]*\)", "\([^"]*\){\([^}]*\)}\([^"]*\)");/log("\1", "\2" + \3 + "\4");/g' "$file"
    
    # Pattern 2: log("LEVEL", "Message: {var1} and {var2}");
    sed -i.tmp 's/log("\([^"]*\)", "\([^"]*\){\([^}]*\)}\([^"]*\){\([^}]*\)}\([^"]*\)");/log("\1", "\2" + \3 + "\4" + \5 + "\6");/g' "$file"
    
    # Pattern 3: return Result.Err("Error: {variable}");
    sed -i.tmp 's/return Result\.Err("\([^"]*\){\([^}]*\)}\([^"]*\)");/return Result.Err("\1" + \2 + "\3");/g' "$file"
    
    # Pattern 4: Generic string with single interpolation
    sed -i.tmp 's/"\([^"]*\){\([^}]*\)}\([^"]*\)"/"\1" + \2 + "\3"/g' "$file"
    
    # Pattern 5: Multiple interpolations in one string
    # This is more complex, handle iteratively
    while grep -q '{[^}]*}' "$file"; do
        sed -i.tmp 's/"\([^"]*\){\([^}]*\)}\([^"]*\)"/"\1" + \2 + "\3"/g' "$file"
    done
    
    # Clean up temporary file
    rm -f "${file}.tmp"
    
    # Count remaining interpolations
    local remaining_count=$(grep -o '{[^}]*}' "$file" | wc -l || echo "0")
    local fixed_count=$((original_count - remaining_count))
    
    if [[ $remaining_count -eq 0 ]]; then
        echo -e "${GREEN}Fixed $fixed_count interpolations${NC}"
        rm "$backup_file"
    else
        echo -e "${YELLOW}Fixed $fixed_count/$original_count interpolations, $remaining_count remaining${NC}"
        echo "  Backup saved as $(basename "$backup_file")"
    fi
}

# Process all markdown files in user manual
echo "Processing user manual files..."
echo

for file in "$USER_MANUAL_DIR"/*.md; do
    if [[ -f "$file" ]]; then
        fix_string_interpolation "$file"
    fi
done

echo
echo -e "${BLUE}String interpolation fix complete!${NC}"
echo "Run the validation script to verify fixes:"
echo "  bash scripts/validate_manual_examples.sh" 
