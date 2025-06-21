#!/bin/bash
# Fix specific function prototypes that were missed

echo "Fixing specific function prototypes..."

# Fix functions with pattern: void function_name() {
find tests -name "*.c" -type f -exec sed -i '' 's/void \([a-zA-Z_][a-zA-Z0-9_]*\)() {/void \1(void) {/g' {} \;

# Fix functions with pattern ending in () {
find tests -name "*.c" -type f -exec sed -i '' 's/\([a-zA-Z_][a-zA-Z0-9_]*\)() {/\1(void) {/g' {} \;

# Count remaining issues
echo "Checking for remaining prototype issues..."
remaining=$(find tests -name "*.c" -exec grep -E "[a-zA-Z_][a-zA-Z0-9_]*\(\)[ ]*{" {} \; | wc -l)
echo "Remaining functions without void: $remaining"

echo "Done!"