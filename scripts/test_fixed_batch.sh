#!/bin/bash

# Test batch of fixed codegen tests for Phase 3 completion
# Measures improvement over baseline and tracks progress

echo "==========================================================="
echo "Asthra Codegen Fixed Tests Batch Runner"
echo "Phase 3 Implementation - Systematic Test Improvements"
echo "==========================================================="

# Get baseline from current codegen results
echo "📊 Current Baseline:"
if [ -f "build/codegen_minimal_results.txt" ]; then
    baseline_working=$(grep "Successful compilations:" build/codegen_minimal_results.txt | awk '{print $3}')
    baseline_total=$(grep "Failed compilations:" build/codegen_minimal_results.txt | awk '{print $3}')
    baseline_total=$((baseline_working + baseline_total))
    baseline_rate=$(grep "Success rate:" build/codegen_minimal_results.txt | awk '{print $3}' | sed 's/%//')
    echo "   Working: $baseline_working/$baseline_total ($baseline_rate%)"
else
    echo "   No baseline found, running initial test..."
    ./scripts/test_codegen_minimal.sh > /dev/null 2>&1
    baseline_working=$(grep "Successful compilations:" build/codegen_minimal_results.txt | awk '{print $3}')
    baseline_total=$(grep "Failed compilations:" build/codegen_minimal_results.txt | awk '{print $3}')
    baseline_total=$((baseline_working + baseline_total))
    baseline_rate=$(grep "Success rate:" build/codegen_minimal_results.txt | awk '{print $3}' | sed 's/%//')
    echo "   Working: $baseline_working/$baseline_total ($baseline_rate%)"
fi

echo ""
echo "🔧 Testing Fixed Versions:"
echo "=========================="

# Directory for results
mkdir -p build/fixed_test_results
results_file="build/fixed_test_results/batch_$(date +%Y%m%d_%H%M%S).txt"

# Initialize counters
total_fixed_tests=0
successful_fixed_tests=0

# List of our fixed test files
fixed_tests=(
    "test_elf_debug_dwarf_fixed.c"
    "test_elf_debug_line_numbers_fixed.c"
    "test_elf_debug_sections_fixed.c"
    "test_linear_scan_allocation_fixed.c"
)

echo "Testing ${#fixed_tests[@]} fixed tests..." | tee -a "$results_file"
echo "Results logged to: $results_file" | tee -a "$results_file"
echo "" | tee -a "$results_file"

# Test each fixed version
for test_file in "${fixed_tests[@]}"; do
    if [ -f "tests/codegen/$test_file" ]; then
        total_fixed_tests=$((total_fixed_tests + 1))
        test_name=$(basename "$test_file" .c)
        
        echo -n "Testing $test_name... " | tee -a "$results_file"
        
        # Attempt compilation with minimal dependencies
        cd tests/codegen
        
        # Try simple compilation first
        if gcc -I../.. -I../../src -DASTHRA_MINIMAL_FRAMEWORK \
           -o "$test_name" "$test_file" \
           ../framework/test_assertions.c \
           ../framework/parser_test_utils.c \
           2>/dev/null; then
            
            # Try to run the test
            if ./"$test_name" >/dev/null 2>&1; then
                echo "✅ PASS" | tee -a "../../$results_file"
                successful_fixed_tests=$((successful_fixed_tests + 1))
            else
                echo "⚠️  COMPILE_OK_RUN_FAIL" | tee -a "../../$results_file"
            fi
            
            # Clean up
            rm -f "$test_name"
        else
            # Try with additional headers
            if gcc -I../.. -I../../src -DASTHRA_MINIMAL_FRAMEWORK \
               -DELF_COMPAT_MACOS \
               -o "$test_name" "$test_file" \
               ../framework/test_assertions.c \
               2>/dev/null; then
                
                if ./"$test_name" >/dev/null 2>&1; then
                    echo "✅ PASS (with compat)" | tee -a "../../$results_file"
                    successful_fixed_tests=$((successful_fixed_tests + 1))
                else
                    echo "⚠️  COMPAT_COMPILE_RUN_FAIL" | tee -a "../../$results_file"
                fi
                rm -f "$test_name"
            else
                echo "❌ COMPILE_FAIL" | tee -a "../../$results_file"
            fi
        fi
        
        cd ../..
    else
        echo "⚠️  File not found: tests/codegen/$test_file" | tee -a "$results_file"
    fi
done

echo "" | tee -a "$results_file"
echo "📈 Fixed Tests Summary:" | tee -a "$results_file"
echo "======================" | tee -a "$results_file"
echo "Successful: $successful_fixed_tests/$total_fixed_tests" | tee -a "$results_file"

if [ $total_fixed_tests -gt 0 ]; then
    fixed_success_rate=$(echo "scale=2; $successful_fixed_tests * 100 / $total_fixed_tests" | bc -l)
    echo "Success Rate: ${fixed_success_rate}%" | tee -a "$results_file"
else
    echo "Success Rate: 0%" | tee -a "$results_file"
    fixed_success_rate=0
fi

echo "" | tee -a "$results_file"

# Calculate potential improvement
echo "🎯 Potential Impact Analysis:" | tee -a "$results_file"
echo "=============================" | tee -a "$results_file"

# Assume each fixed test replaces 1 failing test in the original suite
potential_new_working=$((baseline_working + successful_fixed_tests))
potential_new_rate=$(echo "scale=2; $potential_new_working * 100 / $baseline_total" | bc -l)
improvement=$(echo "scale=2; $potential_new_rate - $baseline_rate" | bc -l)

echo "Baseline: $baseline_working/$baseline_total ($baseline_rate%)" | tee -a "$results_file"
echo "Potential: $potential_new_working/$baseline_total ($potential_new_rate%)" | tee -a "$results_file"
echo "Improvement: +${improvement}% (+${successful_fixed_tests} working tests)" | tee -a "$results_file"

echo "" | tee -a "$results_file"

# Phase 3 progress assessment
echo "🏁 Phase 3 Progress Assessment:" | tee -a "$results_file"
echo "==============================" | tee -a "$results_file"

phase3_target=80.0
current_with_fixes=$potential_new_rate
remaining_to_target=$(echo "scale=2; $phase3_target - $current_with_fixes" | bc -l)

echo "Phase 3 Target: 80%" | tee -a "$results_file"
echo "Current with fixes: ${current_with_fixes}%" | tee -a "$results_file"
echo "Remaining to target: ${remaining_to_target}%" | tee -a "$results_file"

if (( $(echo "$current_with_fixes >= $phase3_target" | bc -l) )); then
    echo "🎉 STATUS: Phase 3 Target ACHIEVED!" | tee -a "$results_file"
else
    needed_tests=$(echo "scale=0; ($phase3_target - $baseline_rate) * $baseline_total / 100" | bc -l)
    echo "📋 STATUS: Need ~$needed_tests more working tests for Phase 3 completion" | tee -a "$results_file"
fi

echo ""
echo "🚀 Next Steps:"
echo "============="
echo "1. Apply successful fixes to replace original failing tests"
echo "2. Create more fixed versions using proven methodology"
echo "3. Focus on categories: ELF Debug (12 tests), Register Allocation (5 tests)"
echo "4. Target Phase 3 completion at 75-80% success rate"

echo ""
echo "📁 Full results saved to: $results_file"
echo "===========================================================" 
