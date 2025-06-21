#!/bin/bash

# Asthra Standard Library Grammar Compliance Checker
# Scans for critical grammar violations in stdlib code

set -e

STDLIB_DIR="${1:-stdlib}"
EXIT_CODE=0

echo "🔍 Checking Asthra Grammar Compliance in: $STDLIB_DIR"
echo "=================================================="

# Check if stdlib directory exists
if [ ! -d "$STDLIB_DIR" ]; then
    echo "❌ Error: Directory $STDLIB_DIR not found"
    exit 1
fi

# Function to check for forbidden patterns
check_pattern() {
    local pattern="$1"
    local description="$2"
    local files
    
    echo "Checking for: $description"
    files=$(grep -r -n "$pattern" "$STDLIB_DIR"/ --include="*.asthra" || true)
    
    if [ -n "$files" ]; then
        echo "❌ VIOLATION: $description"
        echo "$files"
        echo ""
        EXIT_CODE=1
    else
        echo "✅ OK: No violations found"
    fi
    echo ""
}

echo "📋 Scanning for Critical Grammar Violations..."
echo ""

# 1. Critical: Trait definitions and implementations
check_pattern "trait\s+[A-Za-z_][A-Za-z0-9_]*\s*{" "Trait definitions (forbidden)"
check_pattern "impl\s+[A-Za-z_][A-Za-z0-9_]*\s+for\s+[A-Za-z_]" "Trait implementations (impl X for Y)"

# 2. Critical: Generic constraints
check_pattern "fn\s+[A-Za-z_][A-Za-z0-9_]*<[^>]*:\s*[A-Za-z_]" "Generic function constraints"
check_pattern "struct\s+[A-Za-z_][A-Za-z0-9_]*<[^>]*:\s*[A-Za-z_]" "Generic struct constraints"
check_pattern "where\s+[A-Za-z_][A-Za-z0-9_]*\s*:" "Where clauses"

# 3. Critical: Async/await definitions (basic spawn/await is allowed)
check_pattern "async\s+fn" "Async function definitions (forbidden)"

# 4. Critical: Macro definitions
check_pattern "macro_rules!" "Macro definitions"

# 5. Critical: Angle bracket imports (should use strings)
check_pattern "import\s*<[^>]+>" "Angle bracket imports"

echo "=================================================="

if [ $EXIT_CODE -eq 0 ]; then
    echo "🎉 SUCCESS: All stdlib files are grammar compliant!"
    echo "   Found 0 violations in $(find "$STDLIB_DIR" -name "*.asthra" | wc -l) files"
    echo ""
    echo "✅ Grammar compliance verification complete"
    echo "   - No trait definitions or implementations"
    echo "   - No generic constraints or where clauses"

    echo "   - No async function definitions"
    echo "   - No macro definitions"
    echo "   - No angle bracket imports"
else
    echo "🚨 FAILURE: Grammar violations found!"
    echo "   Please fix the violations above before proceeding"
fi

echo ""
echo "📊 Statistics:"
echo "   Total .asthra files: $(find "$STDLIB_DIR" -name "*.asthra" | wc -l)"
echo "   Total lines of code: $(find "$STDLIB_DIR" -name "*.asthra" -exec wc -l {} + | tail -1 | awk '{print $1}')"
echo "   Critical patterns checked: 5"

exit $EXIT_CODE 
