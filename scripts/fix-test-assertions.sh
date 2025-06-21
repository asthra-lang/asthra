#!/bin/bash
# Script to fix type conversion errors in test assertions

echo "Fixing type conversion errors in test assertions..."

# Fix resolve_ai_annotation_type assertions - these should use int comparison
find tests -name "*.c" -type f -exec sed -i '' \
    's/ASTHRA_TEST_ASSERT_EQ(context, resolve_ai_annotation_type(\([^)]*\)), \([A-Z_]*\), \([^)]*\))/asthra_test_assert_int_eq(context, (int)resolve_ai_annotation_type(\1), (int)\2, \3)/g' {} \;

# Fix is_ai_annotation assertions - these should use bool
find tests -name "*.c" -type f -exec sed -i '' \
    's/ASTHRA_TEST_ASSERT_EQ(context, is_ai_annotation(\([^)]*\)), \(true\|false\), \([^)]*\))/asthra_test_assert_bool_eq(context, is_ai_annotation(\1), \2, \3)/g' {} \;

# Fix ai_annotation_valid_for_* assertions
find tests -name "*.c" -type f -exec sed -i '' \
    's/ASTHRA_TEST_ASSERT_EQ(context, ai_annotation_valid_for_\([a-z]*\)(\([^)]*\)), \(true\|false\), \([^)]*\))/asthra_test_assert_bool_eq(context, ai_annotation_valid_for_\1(\2), \3, \4)/g' {} \;

# Fix validate_ai_* assertions  
find tests -name "*.c" -type f -exec sed -i '' \
    's/ASTHRA_TEST_ASSERT_EQ(context, validate_ai_\([a-z_]*\)(\([^)]*\)), \(true\|false\), \([^)]*\))/asthra_test_assert_bool_eq(context, validate_ai_\1(\2), \3, \4)/g' {} \;

# Fix enum value comparisons (general pattern)
find tests -name "*.c" -type f -exec sed -i '' \
    's/ASTHRA_TEST_ASSERT_EQ(context, \([^,]*\), AI_\([A-Z_]*\), \([^)]*\))/asthra_test_assert_int_eq(context, (int)\1, (int)AI_\2, \3)/g' {} \;

echo "Done fixing type conversion errors!"