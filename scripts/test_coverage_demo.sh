#!/bin/bash
# Test Coverage Demonstration Script for Asthra
# This script demonstrates how to calculate and analyze test coverage for C programs

echo "========================================"
echo "Asthra Test Coverage Demonstration"
echo "========================================"
echo ""

# Check if we're in the right directory
if [ ! -f "Makefile" ] || [ ! -d "src" ]; then
    echo "❌ Error: Please run this script from the Asthra project root directory"
    exit 1
fi

echo "1. Checking coverage tool availability..."
echo "----------------------------------------"

# Check for required tools
TOOLS_MISSING=0

if command -v gcov >/dev/null 2>&1; then
    echo "✅ gcov: $(gcov --version | head -1)"
else
    echo "❌ gcov: Not found"
    TOOLS_MISSING=1
fi

if command -v lcov >/dev/null 2>&1; then
    echo "✅ lcov: $(lcov --version | head -1)"
else
    echo "⚠️  lcov: Not found (optional for HTML reports)"
fi

if command -v llvm-profdata >/dev/null 2>&1; then
    echo "✅ llvm-profdata: Available"
else
    echo "⚠️  llvm-profdata: Not found (optional for Clang)"
fi

if command -v llvm-cov >/dev/null 2>&1; then
    echo "✅ llvm-cov: Available"
else
    echo "⚠️  llvm-cov: Not found (optional for Clang)"
fi

if command -v bc >/dev/null 2>&1; then
    echo "✅ bc: Available"
else
    echo "⚠️  bc: Not found (needed for threshold checking)"
fi

if [ $TOOLS_MISSING -eq 1 ]; then
    echo ""
    echo "❌ Critical tools missing. Please install:"
    echo "   Ubuntu/Debian: sudo apt-get install gcc gcov lcov bc"
    echo "   macOS: brew install lcov bc"
    exit 1
fi

echo ""
echo "2. Showing coverage system information..."
echo "----------------------------------------"
make coverage-info

echo ""
echo "3. Cleaning any existing coverage data..."
echo "----------------------------------------"
make coverage-clean

echo ""
echo "4. Running basic test compilation to verify build system..."
echo "-----------------------------------------------------------"
if make directories; then
    echo "✅ Build directories created successfully"
else
    echo "❌ Failed to create build directories"
    exit 1
fi

# Try building without coverage first to check basic compilation
echo ""
echo "5. Testing basic compilation (without coverage)..."
echo "---------------------------------------------------"
if make all >/dev/null 2>&1; then
    echo "✅ Basic compilation successful"
else
    echo "❌ Basic compilation failed. Please fix build issues first."
    exit 1
fi

echo ""
echo "6. Running quick coverage analysis..."
echo "--------------------------------------"
if make coverage-quick; then
    echo "✅ Quick coverage analysis completed"
else
    echo "❌ Coverage analysis failed"
    echo "This might be due to:"
    echo "  - Missing test executables"
    echo "  - Compilation errors with coverage flags"
    echo "  - Missing test targets"
    echo ""
    echo "Try running individual steps:"
    echo "  make coverage-compile-gcov"
    echo "  make test-all"
    echo "  make coverage-report-gcov"
fi

echo ""
echo "7. Generating coverage summary..."
echo "--------------------------------"
if make coverage-summary; then
    echo "✅ Coverage summary generated"
else
    echo "⚠️  Coverage summary generation had issues"
fi

echo ""
echo "8. Checking coverage quality threshold..."
echo "----------------------------------------"
if make coverage-check; then
    echo "✅ Coverage meets quality threshold"
else
    echo "⚠️  Coverage below recommended threshold"
    echo "Consider adding more tests to improve coverage"
fi

echo ""
echo "9. Module-specific coverage demonstration..."
echo "--------------------------------------------"

echo "Parser module coverage:"
if make coverage-parser 2>/dev/null; then
    echo "✅ Parser coverage analysis completed"
else
    echo "⚠️  Parser coverage analysis skipped (compile first)"
fi

echo ""
echo "Code generation module coverage:"
if make coverage-codegen 2>/dev/null; then
    echo "✅ Codegen coverage analysis completed"
else
    echo "⚠️  Codegen coverage analysis skipped (compile first)"
fi

echo ""
echo "Analysis module coverage:"
if make coverage-analysis 2>/dev/null; then
    echo "✅ Analysis coverage analysis completed"
else
    echo "⚠️  Analysis coverage analysis skipped (compile first)"
fi

echo ""
echo "10. Full HTML coverage report generation..."
echo "-------------------------------------------"
if command -v lcov >/dev/null 2>&1; then
    if make coverage; then
        echo "✅ Full HTML coverage report generated!"
        echo ""
        echo "Coverage reports generated:"
        if [ -d "coverage_output" ]; then
            echo "  📁 coverage_output/"
            echo "    📁 html/ - HTML reports"
            echo "    📁 data/ - Raw coverage data"
            echo ""
            if [ -f "coverage_output/html/index.html" ]; then
                echo "🌐 Open coverage_output/html/index.html in your browser to view the report"
            fi
        fi
    else
        echo "⚠️  HTML coverage report generation had issues"
    fi
else
    echo "⚠️  lcov not available - skipping HTML report generation"
    echo "Install lcov for beautiful HTML coverage reports:"
    echo "  Ubuntu/Debian: sudo apt-get install lcov"
    echo "  macOS: brew install lcov"
fi

echo ""
echo "11. Platform-specific coverage considerations..."
echo "------------------------------------------------"
echo "Current platform: $(uname)"
echo "Current compiler: ${CC:-gcc}"

case "$(uname)" in
    Darwin)
        echo "🍎 macOS detected"
        echo "   - Use Clang for LLVM coverage features"
        echo "   - Install tools: brew install lcov llvm"
        ;;
    Linux)
        echo "🐧 Linux detected"
        echo "   - GCC/gcov works well"
        echo "   - Install tools: sudo apt-get install lcov gcovr"
        ;;
    MINGW*|CYGWIN*|MSYS*)
        echo "🪟 Windows detected"
        echo "   - MinGW/MSYS2 environment"
        echo "   - Coverage support may be limited"
        ;;
esac

echo ""
echo "12. Coverage analysis best practices..."
echo "--------------------------------------"
echo "📋 Coverage Guidelines:"
echo "   • Minimum acceptable: 80% line coverage"
echo "   • Good target: 90% line coverage + 80% branch coverage"
echo "   • Excellent: 95% line coverage + 90% branch coverage"
echo ""
echo "📊 What to focus on:"
echo "   • Business logic and algorithms"
echo "   • Error handling paths"
echo "   • Boundary conditions"
echo "   • Module interfaces"
echo ""
echo "🚫 What to exclude:"
echo "   • Generated code"
echo "   • Third-party libraries"
echo "   • Platform-specific code (when not testing that platform)"

echo ""
echo "13. Troubleshooting tips..."
echo "---------------------------"
echo "🔧 Common issues and solutions:"
echo ""
echo "Missing .gcda files:"
echo "  → Ensure tests run to completion and exit normally"
echo "  → Check that programs don't crash during testing"
echo ""
echo "Permission errors:"
echo "  → Verify write permissions in project directory"
echo "  → Check that coverage files aren't owned by root"
echo ""
echo "Compiler mismatches:"
echo "  → Use same compiler for coverage compilation and reporting"
echo "  → Clear build cache: make clean && make coverage-clean"
echo ""
echo "Path issues:"
echo "  → Use absolute paths for complex build systems"
echo "  → Ensure GCOV_PREFIX_STRIP is set correctly if needed"

echo ""
echo "14. Next steps and automation..."
echo "-------------------------------"
echo "🚀 To integrate coverage into your workflow:"
echo ""
echo "Daily development:"
echo "  make coverage-quick          # Fast coverage check"
echo "  make coverage-summary        # View coverage summary"
echo "  make coverage-check          # Verify quality threshold"
echo ""
echo "Before commits:"
echo "  make coverage                # Full HTML report"
echo ""
echo "CI/CD integration:"
echo "  Add coverage-check to your test pipeline"
echo "  Archive HTML reports as build artifacts"
echo "  Set up coverage trend tracking"

echo ""
echo "=========================================="
echo "✅ Test Coverage Demonstration Complete!"
echo "=========================================="
echo ""
echo "Available make targets:"
echo "  make coverage              # Full coverage analysis"
echo "  make coverage-quick        # Quick coverage report"
echo "  make coverage-summary      # Coverage summary"
echo "  make coverage-check        # Quality threshold check"
echo "  make coverage-clean        # Clean coverage data"
echo "  make coverage-info         # Show coverage information"
echo ""
echo "For detailed documentation, see: docs/test_coverage_guide.md" 
