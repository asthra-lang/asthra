#!/bin/bash

# Add missing includes to symbol_export_test_common.h
sed -i '' '/#include "..\/..\/src\/codegen\/code_generator.h"/a\
#include "../../src/parser/parser_core.h"' tests/codegen/symbol_export_test_common.h

# Fix the semantic_analyze function call - it should be semantic_analyzer_analyze
find tests/codegen -name "*.c" -type f -exec sed -i '' \
    's/semantic_analyze(/semantic_analyzer_analyze(/g' {} \;

# Fix code_generator_generate to use proper function name
find tests/codegen -name "*.c" -type f -exec sed -i '' \
    's/code_generator_generate(/code_generate(/g' {} \;

# Fix code_generator_get_output
find tests/codegen -name "*.c" -type f -exec sed -i '' \
    's/code_generator_get_output(/code_generator_get_assembly(/g' {} \;

# Remove parser_get_lexer calls as parsers own their lexers
sed -i '' '/Lexer\* lexer = parser_get_lexer(parser);/d' tests/codegen/symbol_export_test_common.c
sed -i '' 's/if (lexer) lexer_destroy(lexer);/\/\/ Parser owns lexer, no need to destroy separately/g' tests/codegen/symbol_export_test_common.c

echo "Fixed symbol export test issues"