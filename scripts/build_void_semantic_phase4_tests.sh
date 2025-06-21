#!/usr/bin/env bash

# Build script for Void Semantic Overloading Fix Phase 4 tests
# Properly links all necessary object files for compilation

set -e

echo "🔨 Building Void Semantic Fix Phase 4 Tests..."

# Define directories
PARSER_SRC_DIR="src/parser"
SEMANTIC_SRC_DIR="src/analysis"
CODEGEN_SRC_DIR="src/codegen"
TEST_FRAMEWORK_DIR="tests/framework"

# Define common object files needed for all tests
PARSER_OBJECTS=(
    "$PARSER_SRC_DIR/parser_core.o"
    "$PARSER_SRC_DIR/lexer_scan.o"
    "$PARSER_SRC_DIR/lexer_core.o"
    "$PARSER_SRC_DIR/lexer_util.o"
    "$PARSER_SRC_DIR/token.o"
    "$PARSER_SRC_DIR/keyword.o"
    "$PARSER_SRC_DIR/ast_node_creation.o"
    "$PARSER_SRC_DIR/ast_node_destruction.o"
    "$PARSER_SRC_DIR/ast_destruction_expressions.o"
    "$PARSER_SRC_DIR/ast_destruction_declarations.o"
    "$PARSER_SRC_DIR/ast_destruction_literals.o"
    "$PARSER_SRC_DIR/ast_destruction_patterns.o"
    "$PARSER_SRC_DIR/ast_destruction_statements.o"
    "$PARSER_SRC_DIR/ast_destruction_types.o"
    "$PARSER_SRC_DIR/ast_destruction_concurrency.o"
    "$PARSER_SRC_DIR/ast_node_list.o"
    "$PARSER_SRC_DIR/ast_utility.o"
    "$PARSER_SRC_DIR/ast_traversal.o"
    "$PARSER_SRC_DIR/parser_errors.o"
    "$PARSER_SRC_DIR/parser_ast_helpers.o"
    "$PARSER_SRC_DIR/grammar_toplevel_program.o"
    "$PARSER_SRC_DIR/grammar_toplevel_function.o"
    "$PARSER_SRC_DIR/grammar_toplevel_struct.o"
    "$PARSER_SRC_DIR/grammar_toplevel_extern.o"
    "$PARSER_SRC_DIR/grammar_toplevel_method.o"
    "$PARSER_SRC_DIR/grammar_toplevel_enum.o"
    "$PARSER_SRC_DIR/grammar_expressions_primary.o"
    "$PARSER_SRC_DIR/grammar_expressions_precedence.o"
    "$PARSER_SRC_DIR/grammar_expressions_unary.o"
    "$PARSER_SRC_DIR/grammar_expressions_special.o"
    "$PARSER_SRC_DIR/grammar_expressions_struct.o"
    "$PARSER_SRC_DIR/grammar_statements_core.o"
    "$PARSER_SRC_DIR/grammar_statements_control.o"
    "$PARSER_SRC_DIR/grammar_statements_concurrency.o"
    "$PARSER_SRC_DIR/grammar_statements_types.o"
    "$PARSER_SRC_DIR/grammar_patterns.o"
    "$PARSER_SRC_DIR/grammar_annotations.o"
)

SEMANTIC_OBJECTS=(
    "$SEMANTIC_SRC_DIR/semantic_core.o"
    "$SEMANTIC_SRC_DIR/semantic_builtins.o"
    "$SEMANTIC_SRC_DIR/semantic_types.o"
    "$SEMANTIC_SRC_DIR/semantic_statements.o"
    "$SEMANTIC_SRC_DIR/semantic_expressions.o"
    "$SEMANTIC_SRC_DIR/semantic_declarations.o"
    "$SEMANTIC_SRC_DIR/semantic_methods.o"
    "$SEMANTIC_SRC_DIR/semantic_errors.o"
    "$SEMANTIC_SRC_DIR/semantic_enums.o"
    "$SEMANTIC_SRC_DIR/semantic_structs.o"
    "$SEMANTIC_SRC_DIR/semantic_symbols_core.o"
    "$SEMANTIC_SRC_DIR/semantic_symbols_entries.o"
    "$SEMANTIC_SRC_DIR/semantic_symbols_aliases.o"
    "$SEMANTIC_SRC_DIR/semantic_symbols_utils.o"
    "$SEMANTIC_SRC_DIR/semantic_symbols_stats.o"
    "$SEMANTIC_SRC_DIR/semantic_symbols.o"
    "$SEMANTIC_SRC_DIR/semantic_scopes.o"
    "$SEMANTIC_SRC_DIR/semantic_statistics.o"
    "$SEMANTIC_SRC_DIR/semantic_functions.o"
    "$SEMANTIC_SRC_DIR/semantic_imports.o"
    "$SEMANTIC_SRC_DIR/semantic_annotations_validation.o"
    "$SEMANTIC_SRC_DIR/semantic_annotations_ffi.o"
    "$SEMANTIC_SRC_DIR/semantic_annotations_registry.o"
    "$SEMANTIC_SRC_DIR/semantic_annotations_errors.o"
    "$SEMANTIC_SRC_DIR/semantic_annotations_inheritance.o"
    "$SEMANTIC_SRC_DIR/semantic_annotations_conflicts.o"
    "$SEMANTIC_SRC_DIR/semantic_annotations_parameters.o"
    "$SEMANTIC_SRC_DIR/semantic_annotations_concurrency.o"
)

CODEGEN_OBJECTS=(
    "$CODEGEN_SRC_DIR/code_generator_core.o"
    "$CODEGEN_SRC_DIR/code_generator_expressions.o"
    "$CODEGEN_SRC_DIR/code_generator_statements.o"
    "$CODEGEN_SRC_DIR/code_generator_utils.o"
    "$CODEGEN_SRC_DIR/generic_instantiation.o"
    "$CODEGEN_SRC_DIR/instruction_buffer.o"
    "$CODEGEN_SRC_DIR/instruction_factory.o"
    "$CODEGEN_SRC_DIR/register_allocator.o"
    "$CODEGEN_SRC_DIR/label_manager.o"
    "$CODEGEN_SRC_DIR/symbol_table.o"
    "$CODEGEN_SRC_DIR/code_generator_abi.o"
)

# Common compiler flags
CFLAGS="-std=c17 -Wall -Wextra -Wpedantic -Wformat=2 -Wstrict-prototypes -Wmissing-prototypes -Wold-style-definition -Wredundant-decls -Wnested-externs -Wmissing-include-dirs -Wnull-dereference -Wshadow -Wcast-align -Wcast-qual -Wwrite-strings -Wconversion -Wsign-conversion -Wfloat-equal -Wundef -Wunreachable-code -Wunused-macros -Wdate-time -Wstack-protector -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fPIE -DASTHRA_COMPILER_CLANG=1 -Wthread-safety -Wthread-safety-beta -Wdocumentation -Wno-missing-field-initializers -Wno-unused-parameter -I/opt/homebrew/opt/llvm/include"
LDFLAGS="-lpthread -lm -Wl,-pie"

# Function to build object files if they don't exist
build_objects() {
    local objects=("$@")
    for obj in "${objects[@]}"; do
        if [[ ! -f "$obj" ]]; then
            local src="${obj%.o}.c"
            if [[ -f "$src" ]]; then
                echo "🔨 Building missing object: $obj"
                clang $CFLAGS -c "$src" -o "$obj"
            else
                echo "⚠️  Warning: Source file $src not found for $obj"
            fi
        fi
    done
}

# Build necessary object files
echo "📦 Building required object files..."
build_objects "${PARSER_OBJECTS[@]}"
build_objects "${SEMANTIC_OBJECTS[@]}"
build_objects "${CODEGEN_OBJECTS[@]}"

# Function to build a test executable
build_test() {
    local test_file="$1"
    local test_name="${test_file%.c}"
    local test_type="$2"
    
    echo "🔗 Building $test_name..."
    
    # Compile the test source
    clang $CFLAGS -c "$test_file" -o "${test_name}.o"
    
    # Link based on test type
    case "$test_type" in
        "parser")
            clang "${test_name}.o" "${PARSER_OBJECTS[@]}" -o "$test_name" $LDFLAGS
            ;;
        "semantic")
            clang "${test_name}.o" "${PARSER_OBJECTS[@]}" "${SEMANTIC_OBJECTS[@]}" -o "$test_name" $LDFLAGS
            ;;
        "codegen")
            clang "${test_name}.o" "${PARSER_OBJECTS[@]}" "${SEMANTIC_OBJECTS[@]}" "${CODEGEN_OBJECTS[@]}" -o "$test_name" $LDFLAGS
            ;;
        *)
            echo "❌ Unknown test type: $test_type"
            return 1
            ;;
    esac
    
    if [[ $? -eq 0 ]]; then
        echo "✅ Successfully built $test_name"
    else
        echo "❌ Failed to build $test_name"
        return 1
    fi
}

# Build Phase 4 tests
echo ""
echo "🧪 Building Phase 4 Test Suite..."

# Build parser comprehensive test (includes semantic analysis integration)
if [[ -f "tests/parser/test_void_semantic_fix_phase4_comprehensive.c" ]]; then
    build_test "tests/parser/test_void_semantic_fix_phase4_comprehensive.c" "semantic"
fi

# Build semantic analysis test
if [[ -f "tests/semantic/test_void_semantic_fix_phase4_semantic.c" ]]; then
    build_test "tests/semantic/test_void_semantic_fix_phase4_semantic.c" "semantic"
fi

# Build code generation test
if [[ -f "tests/codegen/test_void_semantic_fix_phase4_codegen.c" ]]; then
    build_test "tests/codegen/test_void_semantic_fix_phase4_codegen.c" "codegen"
fi

# Build Phase 3 regression test
if [[ -f "tests/parser/test_void_semantic_fix_phase3.c" ]]; then
    build_test "tests/parser/test_void_semantic_fix_phase3.c" "parser"
fi

echo ""
echo "✅ Phase 4 test build completed!"
echo ""
echo "📋 Available test executables:"
echo "  - tests/parser/test_void_semantic_fix_phase4_comprehensive"
echo "  - tests/semantic/test_void_semantic_fix_phase4_semantic"
echo "  - tests/codegen/test_void_semantic_fix_phase4_codegen"
echo "  - tests/parser/test_void_semantic_fix_phase3"
echo ""
echo "🚀 Run tests with: ./tests/parser/test_void_semantic_fix_phase4_comprehensive" 
