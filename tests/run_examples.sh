#!/bin/bash
# Integration test: compile and run all example .ast files
set -euo pipefail

COMPILER="./zig-out/bin/asthra"
EXAMPLES_DIR="examples"
PASS=0
FAIL=0
FAILURES=""

if [ ! -f "$COMPILER" ]; then
    echo "ERROR: Compiler not found at $COMPILER. Run 'zig build' first."
    exit 1
fi

for src in "$EXAMPLES_DIR"/*.ast; do
    name=$(basename "$src" .ast)
    out="/tmp/asthra_test_${name}_$$"

    # Skip library files (non-main packages) — they are imported by other examples
    if ! grep -q '^package main;' "$src"; then
        printf "%-20s SKIP (library)\n" "$name"
        continue
    fi

    printf "%-20s " "$name"

    # Compile
    if ! compile_output=$("$COMPILER" "$src" -o "$out" 2>&1); then
        echo "FAIL (compile error)"
        FAILURES="$FAILURES\n--- $name (compile) ---\n$compile_output"
        FAIL=$((FAIL + 1))
        continue
    fi

    # Run
    if ! run_output=$("$out" 2>&1); then
        echo "FAIL (runtime error)"
        FAILURES="$FAILURES\n--- $name (runtime) ---\n$run_output"
        FAIL=$((FAIL + 1))
        rm -f "$out"
        continue
    fi

    echo "PASS"
    PASS=$((PASS + 1))
    rm -f "$out"
done

echo ""
echo "Results: $PASS passed, $FAIL failed, $((PASS + FAIL)) total"

if [ $FAIL -gt 0 ]; then
    echo ""
    echo "Failures:"
    echo -e "$FAILURES"
    exit 1
fi
