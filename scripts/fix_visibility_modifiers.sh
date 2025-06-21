#!/bin/bash

# Visibility Modifiers Fix Script
# Part of the User Manual Maintenance Plan Phase 1
# Adds missing visibility modifiers (required in v1.16+)

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

echo -e "${BLUE}Visibility Modifiers Fix Script${NC}"
echo "======================================="
echo "Adding missing visibility modifiers to user manual examples..."
echo

# Function to fix visibility modifiers in a file
fix_visibility_modifiers() {
    local file="$1"
    local backup_file="${file}.backup"
    
    echo -n "Processing $(basename "$file"): "
    
    # Create backup
    cp "$file" "$backup_file"
    
    # Count functions without visibility modifiers
    local functions_without_vis=$(grep -c "^fn " "$file" 2>/dev/null || echo "0")
    functions_without_vis=${functions_without_vis//[^0-9]/}  # Remove non-numeric characters
    functions_without_vis=${functions_without_vis:-0}        # Default to 0 if empty
    
    if [[ $functions_without_vis -eq 0 ]]; then
        echo -e "${GREEN}No functions without visibility modifiers${NC}"
        rm "$backup_file"
        return
    fi
    
    # Fix function declarations without visibility modifiers
    # Add 'pub' to functions that appear to be public examples
    # Add 'priv' to functions that appear to be internal/helper functions
    
    # Pattern 1: Functions that look like main examples or public APIs -> pub
    sed -i.tmp 's/^fn main(/pub fn main(/g' "$file"
    sed -i.tmp 's/^fn example_/pub fn example_/g' "$file"
    sed -i.tmp 's/^fn test_/pub fn test_/g' "$file"
    sed -i.tmp 's/^fn demo_/pub fn demo_/g' "$file"
    sed -i.tmp 's/^fn calculate_/pub fn calculate_/g' "$file"
    sed -i.tmp 's/^fn process_/pub fn process_/g' "$file"
    sed -i.tmp 's/^fn create_/pub fn create_/g' "$file"
    sed -i.tmp 's/^fn get_/pub fn get_/g' "$file"
    sed -i.tmp 's/^fn set_/pub fn set_/g' "$file"
    sed -i.tmp 's/^fn handle_/pub fn handle_/g' "$file"
    sed -i.tmp 's/^fn validate_/pub fn validate_/g' "$file"
    sed -i.tmp 's/^fn parse_/pub fn parse_/g' "$file"
    sed -i.tmp 's/^fn format_/pub fn format_/g' "$file"
    sed -i.tmp 's/^fn convert_/pub fn convert_/g' "$file"
    sed -i.tmp 's/^fn build_/pub fn build_/g' "$file"
    sed -i.tmp 's/^fn run_/pub fn run_/g' "$file"
    sed -i.tmp 's/^fn execute_/pub fn execute_/g' "$file"
    sed -i.tmp 's/^fn perform_/pub fn perform_/g' "$file"
    
    # Pattern 2: Functions that look like helpers or internal -> priv
    sed -i.tmp 's/^fn helper_/priv fn helper_/g' "$file"
    sed -i.tmp 's/^fn internal_/priv fn internal_/g' "$file"
    sed -i.tmp 's/^fn _/priv fn _/g' "$file"
    
    # Pattern 3: Remaining functions without clear pattern -> priv (safer default)
    sed -i.tmp 's/^fn \([a-zA-Z_][a-zA-Z0-9_]*\)(/priv fn \1(/g' "$file"
    
    # Clean up temporary file
    rm -f "${file}.tmp"
    
    # Count remaining functions without visibility modifiers
    local remaining_functions=$(grep -c "^fn " "$file" 2>/dev/null || echo "0")
    remaining_functions=${remaining_functions//[^0-9]/}  # Remove non-numeric characters
    remaining_functions=${remaining_functions:-0}        # Default to 0 if empty
    local fixed_count=$((functions_without_vis - remaining_functions))
    
    if [[ $remaining_functions -eq 0 ]]; then
        echo -e "${GREEN}Fixed $fixed_count function declarations${NC}"
        rm "$backup_file"
    else
        echo -e "${YELLOW}Fixed $fixed_count/$functions_without_vis functions, $remaining_functions remaining${NC}"
        echo "  Backup saved as $(basename "$backup_file")"
    fi
}

# Process all markdown files in user manual
echo "Processing user manual files..."
echo

for file in "$USER_MANUAL_DIR"/*.md; do
    if [[ -f "$file" ]]; then
        fix_visibility_modifiers "$file"
    fi
done

echo
echo -e "${BLUE}Visibility modifiers fix complete!${NC}"
echo "Run the validation script to verify fixes:"
echo "  bash scripts/validate_manual_examples.sh" 
