#!/bin/bash

# check-claude-md-size.sh
# Script to check CLAUDE.md file size and warn if it exceeds target

# Configuration
TARGET_LINES=250
WARNING_LINES=240
CLAUDE_MD="CLAUDE.md"

# ANSI color codes
RED='\033[0;31m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Check if CLAUDE.md exists
if [ ! -f "$CLAUDE_MD" ]; then
    echo -e "${RED}Error: $CLAUDE_MD not found in current directory${NC}"
    exit 1
fi

# Get current line count
CURRENT_LINES=$(wc -l < "$CLAUDE_MD")

# Function to suggest sections to extract
suggest_extraction() {
    echo -e "\n${YELLOW}Suggested sections to extract:${NC}"
    
    # Count lines in major sections
    echo "Section line counts:"
    
    # Find sections with code blocks (often good candidates for extraction)
    echo -e "\n${YELLOW}Sections with code examples (good extraction candidates):${NC}"
    grep -n "^###.*" "$CLAUDE_MD" | while read -r line; do
        linenum=$(echo "$line" | cut -d: -f1)
        section=$(echo "$line" | cut -d: -f2-)
        
        # Check if section has code blocks
        next_section=$(grep -n "^###" "$CLAUDE_MD" | grep -A1 "^$linenum:" | tail -1 | cut -d: -f1)
        if [ -z "$next_section" ]; then
            next_section=$(wc -l < "$CLAUDE_MD")
        fi
        
        code_blocks=$(sed -n "${linenum},${next_section}p" "$CLAUDE_MD" | grep -c '```')
        if [ "$code_blocks" -gt 0 ]; then
            lines=$((next_section - linenum))
            echo "  - $section (lines $linenum-$next_section, ~$lines lines, $((code_blocks/2)) code blocks)"
        fi
    done
    
    echo -e "\n${YELLOW}Consider moving detailed content to:${NC}"
    echo "  - docs/contributor/reference/ for reference material"
    echo "  - docs/contributor/guides/ for detailed guides"
    echo "  - docs/architecture/ for design documentation"
    echo "  - docs/spec/ for language specifications"
}

# Check file size
if [ "$CURRENT_LINES" -gt "$TARGET_LINES" ]; then
    echo -e "${RED}❌ CLAUDE.md exceeds target size!${NC}"
    echo "   Current: $CURRENT_LINES lines"
    echo "   Target:  $TARGET_LINES lines"
    echo "   Excess:  $((CURRENT_LINES - TARGET_LINES)) lines"
    
    suggest_extraction
    
    exit 1
elif [ "$CURRENT_LINES" -gt "$WARNING_LINES" ]; then
    echo -e "${YELLOW}⚠️  CLAUDE.md approaching target size${NC}"
    echo "   Current: $CURRENT_LINES lines"
    echo "   Warning: $WARNING_LINES lines"
    echo "   Target:  $TARGET_LINES lines"
    echo "   Remaining: $((TARGET_LINES - CURRENT_LINES)) lines"
    
    echo -e "\n${YELLOW}Consider extracting content before reaching the limit.${NC}"
    
    exit 0
else
    echo -e "${GREEN}✅ CLAUDE.md size is good${NC}"
    echo "   Current: $CURRENT_LINES lines"
    echo "   Target:  $TARGET_LINES lines"
    echo "   Buffer:  $((TARGET_LINES - CURRENT_LINES)) lines remaining"
    
    exit 0
fi