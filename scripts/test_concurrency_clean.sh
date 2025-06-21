#!/bin/bash
# Clean concurrency test runner that filters out expected parsing errors
# 
# This script runs the concurrency tests and provides clean output by filtering
# out expected parsing errors that are part of the test framework design.

echo "Running Asthra Concurrency Tests (Clean Output)"
echo "================================================"
echo

# Run the concurrency tests and filter the output
make test-category CATEGORY=concurrency 2>&1 | \
    grep -v "Error at <string>:" | \
    grep -v "Expected parameter name" | \
    grep -v "Expected function, struct, enum, extern, impl, or const declaration" | \
    sed '/^$/N;/^\n$/d'  # Remove excessive blank lines

echo
echo "================================================"
echo "Note: Parsing errors are filtered out as they are expected behavior"
echo "in the test framework for testing error recovery mechanisms."
echo "The actual test results above show the real functionality status." 
