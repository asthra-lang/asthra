#!/bin/bash
# Script to fix function prototype errors in test files

echo "Fixing function prototypes in test files..."

# Fix main() functions without void parameter
find tests -name "*.c" -type f -exec sed -i '' 's/int main()/int main(void)/g' {} \;

# Fix other common parameterless functions
find tests -name "*.c" -type f -exec sed -i '' 's/\([a-zA-Z_][a-zA-Z0-9_]*\)()[\s]*{/\1(void) {/g' {} \;

# Count fixes
echo "Checking for remaining issues..."
remaining=$(find tests -name "*.c" -exec grep -l "int main()" {} \; | wc -l)
echo "Remaining main() without void: $remaining"

echo "Done!"