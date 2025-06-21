#!/bin/bash

# Fix the remaining lexer tests that have syntax errors from ASSERT_TOKEN_TYPE usage

# For test_lexer_comments.c
if [ -f "tests/lexer/test_lexer_comments.c" ]; then
    echo "Fixing test_lexer_comments.c..."
    sed -i '' 's/ASSERT_TOKEN_TYPE;/ASSERT_TOKEN_TYPE(context, \&token, expected);/' tests/lexer/test_lexer_comments.c
    sed -i '' '/token_free(&token);/{N;/lexer_destroy(lexer);/{N;/}/d;}}' tests/lexer/test_lexer_comments.c
fi

# For test_lexer_error_handling.c
if [ -f "tests/lexer/test_lexer_error_handling.c" ]; then
    echo "Fixing test_lexer_error_handling.c..."
    # Remove the orphaned code blocks after ASSERT_TOKEN_TYPE
    sed -i '' '/ASSERT_TOKEN_TYPE(context, &token, TOKEN_ERROR);/{n;/token_free/,/}/d;}' tests/lexer/test_lexer_error_handling.c
    sed -i '' '/ASSERT_TOKEN_TYPE(context, &token, TOKEN_EOF);/{n;/token_free/,/}/d;}' tests/lexer/test_lexer_error_handling.c
    sed -i '' '/ASSERT_TOKEN_TYPE(context, &token2, TOKEN_EOF);/{n;/token_free/,/}/d;}' tests/lexer/test_lexer_error_handling.c
fi

# For test_lexer_position.c
if [ -f "tests/lexer/test_lexer_position.c" ]; then
    echo "Fixing test_lexer_position.c..."
    # Remove the orphaned code after ASSERT_TOKEN_TYPE
    sed -i '' '/ASSERT_TOKEN_TYPE(context, &token, expectations\[current_expectation\].expected_type);/{n;/token_free/,/}/d;}' tests/lexer/test_lexer_position.c
fi

# For test_lexer_unicode.c
if [ -f "tests/lexer/test_lexer_unicode.c" ]; then
    echo "Fixing test_lexer_unicode.c..."
    # This file seems empty, let's check
    if [ ! -s "tests/lexer/test_lexer_unicode.c" ]; then
        echo "test_lexer_unicode.c is empty, skipping..."
    fi
fi

echo "Fixes applied to lexer tests"