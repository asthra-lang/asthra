#!/bin/bash

# Script to help fix failing BDD scenarios
# This script identifies common patterns that need fixing in the BDD test files

echo "=== BDD Test Fixing Helper Script ==="
echo "This script helps identify patterns that need fixing in BDD test files"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Find all step files
STEP_FILES=$(find steps/unit -name "*_steps.c" -type f)

echo "Analyzing BDD step files for common issues..."
echo ""

# Pattern 1: Missing type annotations
echo -e "${YELLOW}1. Checking for missing type annotations:${NC}"
echo "   Pattern: 'let variable = expression;' (missing ': Type')"
echo ""

for file in $STEP_FILES; do
    # Look for let statements without type annotations
    if grep -n "let [a-zA-Z_][a-zA-Z0-9_]* = " "$file" | grep -v ": [a-zA-Z]" > /dev/null; then
        echo -e "${RED}Found in $file:${NC}"
        grep -n "let [a-zA-Z_][a-zA-Z0-9_]* = " "$file" | grep -v ": [a-zA-Z]" | head -5
        echo ""
    fi
done

# Pattern 2: Missing struct visibility modifiers  
echo -e "${YELLOW}2. Checking for structs without visibility modifiers:${NC}"
echo "   Pattern: 'struct Name {' (missing 'pub' or 'priv')"
echo ""

for file in $STEP_FILES; do
    # Look for struct declarations without pub/priv
    if grep -n "^[[:space:]]*struct [A-Z]" "$file" | grep -v "pub struct\|priv struct" > /dev/null; then
        echo -e "${RED}Found in $file:${NC}"
        grep -n "^[[:space:]]*struct [A-Z]" "$file" | grep -v "pub struct\|priv struct" | head -5
        echo ""
    fi
done

# Pattern 3: sizeof without type annotations
echo -e "${YELLOW}3. Checking for sizeof usage without type annotations:${NC}"
echo "   Pattern: 'let var = sizeof(Type);' (should be 'let var: usize = sizeof(Type);')"
echo ""

for file in $STEP_FILES; do
    if grep -n "let.*= sizeof" "$file" | grep -v ": usize" > /dev/null; then
        echo -e "${RED}Found in $file:${NC}"
        grep -n "let.*= sizeof" "$file" | grep -v ": usize" | head -5
        echo ""
    fi
done

# Summary of files that need attention
echo -e "${YELLOW}=== Summary of Files Needing Updates ===${NC}"
echo ""

FILES_WITH_ISSUES=()

for file in $STEP_FILES; do
    has_issue=false
    
    # Check each pattern
    if grep -q "let [a-zA-Z_][a-zA-Z0-9_]* = " "$file" | grep -v ": [a-zA-Z]"; then
        has_issue=true
    fi
    
    if grep -q "^[[:space:]]*struct [A-Z]" "$file" | grep -v "pub struct\|priv struct"; then
        has_issue=true
    fi
    
    if [ "$has_issue" = true ]; then
        FILES_WITH_ISSUES+=("$file")
    fi
done

if [ ${#FILES_WITH_ISSUES[@]} -eq 0 ]; then
    echo -e "${GREEN}No issues found! All BDD tests appear to follow current syntax requirements.${NC}"
else
    echo -e "${RED}Files requiring updates:${NC}"
    for file in "${FILES_WITH_ISSUES[@]}"; do
        echo "  - $file"
    done
fi

echo ""
echo "=== Quick Fix Examples ==="
echo ""
echo "1. For missing type annotations:"
echo "   sed -i '' 's/let \([a-zA-Z_][a-zA-Z0-9_]*\) = sizeof(\([^)]*\));/let \1: usize = sizeof(\2);/g' FILE"
echo ""
echo "2. For missing struct visibility:"
echo "   sed -i '' 's/^[[:space:]]*struct \([A-Z]\)/pub struct \1/g' FILE"
echo ""
echo "Note: Always review changes before committing!"