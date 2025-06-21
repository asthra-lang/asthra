#!/bin/bash
# Register all built test executables with CTest

echo "Generating CTestTestfile.cmake for built tests..."

cat > build/tests/CTestTestfile.cmake << 'EOF'
# Generated test registration file
# This registers all built test executables with CTest

EOF

for test_exe in bin/*test*; do
    if [ -x "$test_exe" ] && [ -f "$test_exe" ]; then
        test_name=$(basename "$test_exe")
        echo "add_test(NAME $test_name COMMAND $test_exe)" >> build/tests/CTestTestfile.cmake
        echo "set_tests_properties($test_name PROPERTIES TIMEOUT 30)" >> build/tests/CTestTestfile.cmake
        echo "" >> build/tests/CTestTestfile.cmake
    fi
done

echo "Registered $(ls bin/*test* 2>/dev/null | wc -l) tests with CTest"
echo "You can now run: cd build && ctest"