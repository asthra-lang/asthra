#!/bin/bash
# Run all successfully built test executables

echo "=== Running Built Tests ==="
echo

passed=0
failed=0
total=0

for test_exe in bin/*test*; do
    if [ -x "$test_exe" ] && [ -f "$test_exe" ]; then
        test_name=$(basename "$test_exe")
        echo "Running: $test_name"
        
        if $test_exe > /tmp/${test_name}.log 2>&1; then
            echo "  ✅ PASSED"
            passed=$((passed + 1))
        else
            echo "  ❌ FAILED (exit code: $?)"
            echo "  Output:"
            head -20 /tmp/${test_name}.log | sed 's/^/    /'
            failed=$((failed + 1))
        fi
        total=$((total + 1))
        echo
    fi
done

echo "=== Summary ==="
echo "Total tests run: $total"
echo "Passed: $passed"
echo "Failed: $failed"
echo "Success rate: $(( passed * 100 / total ))%"