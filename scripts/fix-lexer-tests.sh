#!/bin/bash

# Fix ASSERT_TOKEN_TYPE macro redefinition in lexer tests
for file in tests/lexer/test_lexer_boundaries.c tests/lexer/test_lexer_comments.c tests/lexer/test_lexer_error_handling.c tests/lexer/test_lexer_position.c tests/lexer/test_lexer_unicode.c; do
    if [ -f "$file" ]; then
        echo "Fixing $file..."
        # Remove the duplicate ASSERT_TOKEN_TYPE definition
        sed -i '' '/^#define ASSERT_TOKEN_TYPE.*$/,/.*expected_type), (token)->type)$/d' "$file"
        # Add a comment indicating we're using the framework version
        sed -i '' 's/Helper macro for token type assertions/Using ASSERT_TOKEN_TYPE from test_assertions.h/' "$file"
    fi
done

echo "Fixed macro redefinition issues in lexer tests"