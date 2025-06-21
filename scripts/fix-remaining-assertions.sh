#!/bin/bash
# Fix remaining assertion issues

echo "Fixing remaining assertion issues..."

# Fix integer literal comparisons (like 0)
find tests -name "*.c" -type f -exec sed -i '' \
    's/ASTHRA_TEST_ASSERT_EQ(context, \([^,]*\), 0, \([^)]*\))/asthra_test_assert_int_eq(context, \1, 0, \2)/g' {} \;

# Fix ASTHRA_TEST_ASSERT_EQ with numeric literals
find tests -name "*.c" -type f -exec sed -i '' \
    's/ASTHRA_TEST_ASSERT_EQ(context, \([^,]*\), \([0-9][0-9]*\), \([^)]*\))/asthra_test_assert_int_eq(context, \1, \2, \3)/g' {} \;

# Replace remaining ASTHRA_TEST_ASSERT_EQ that return enum/int types with int comparison
find tests -name "*.c" -type f -exec sed -i '' \
    's/ASTHRA_TEST_ASSERT_EQ(context, resolve_ai_annotation_type(\([^)]*\)), \([^,]*\), \([^)]*\))/asthra_test_assert_int_eq(context, resolve_ai_annotation_type(\1), \2, \3)/g' {} \;

echo "Done!"