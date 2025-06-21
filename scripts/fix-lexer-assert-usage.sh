#!/bin/bash

# Fix ASSERT_TOKEN_TYPE usage in lexer tests
# The framework version doesn't return a value, so we need to fix the usage

for file in tests/lexer/test_lexer_boundaries.c tests/lexer/test_lexer_comments.c tests/lexer/test_lexer_error_handling.c tests/lexer/test_lexer_position.c tests/lexer/test_lexer_unicode.c; do
    if [ -f "$file" ]; then
        echo "Fixing $file..."
        # Replace if (!ASSERT_TOKEN_TYPE(...)) with just ASSERT_TOKEN_TYPE(...)
        sed -i '' 's/if (!ASSERT_TOKEN_TYPE(\([^)]*\))) {/ASSERT_TOKEN_TYPE(\1);/' "$file"
        # Remove the error handling block that follows
        sed -i '' '/ASSERT_TOKEN_TYPE.*;$/,/return ASTHRA_TEST_FAIL;$/{/ASSERT_TOKEN_TYPE.*;$/!{/return ASTHRA_TEST_FAIL;$/!d;}}' "$file"
        # Clean up extra closing braces
        perl -i -0pe 's/ASSERT_TOKEN_TYPE\([^;]+\);\s*\n\s*token_free[^}]+}\s*\n\s*}/ASSERT_TOKEN_TYPE$1;/gs' "$file"
    fi
done

echo "Fixed ASSERT_TOKEN_TYPE usage in lexer tests"