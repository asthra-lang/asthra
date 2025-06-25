/*
 * =============================================================================
 * PATTERN MATCHING TEST COMMON UTILITIES
 * =============================================================================
 *
 * This header contains shared types, macros, and utility functions used
 * across all pattern matching test files.
 *
 * Part of Phase 3.1: Advanced Pattern Matching
 *
 * =============================================================================
 */

#ifndef TEST_PATTERN_COMMON_H
#define TEST_PATTERN_COMMON_H

#include "ast_node.h"
#include "parser_core.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// =============================================================================
// TEST FRAMEWORK MACROS
// =============================================================================

#define TEST_ASSERT(condition, message)                                                            \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            printf("FAIL: %s\n", message);                                                         \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

#define TEST_SUCCESS()                                                                             \
    do {                                                                                           \
        printf("PASS\n");                                                                          \
        return true;                                                                               \
    } while (0)

#define TEST_START(name) printf("Testing %s... ", name)
#define TEST_END() TEST_SUCCESS()
#define ASSERT_NOT_NULL(ptr) TEST_ASSERT((ptr) != NULL, "Pointer should not be NULL")
#define ASSERT_TRUE(condition) TEST_ASSERT((condition), "Condition should be true")
#define ASSERT_FALSE(condition) TEST_ASSERT(!(condition), "Condition should be false")
#define ASSERT_EQ(a, b) TEST_ASSERT((a) == (b), "Values should be equal")
#define ASSERT_LT(a, b) TEST_ASSERT((a) < (b), "First value should be less than second")
#define ASSERT_GT(a, b) TEST_ASSERT((a) > (b), "First value should be greater than second")
#define ASSERT_STRING_EQ(a, b) TEST_ASSERT(strcmp((a), (b)) == 0, "Strings should be equal")
#define ASSERT_STRING_CONTAINS(str, substr)                                                        \
    TEST_ASSERT(strstr((str), (substr)) != NULL, "String should contain substring")

// =============================================================================
// MOCK TYPES FOR DEMONSTRATION
// =============================================================================

typedef struct {
    bool success;
    int pattern_count;
    bool is_exhaustive;
    int unreachable_patterns;
    bool handles_explicit_values;
    bool handles_tagged_unions;
    bool extracts_values;
    bool handles_nested_patterns;
    int max_nesting_depth;
    bool handles_mixed_patterns;
    bool handles_nested_structs;
    int missing_patterns;
    char *missing_pattern_names[10];
    bool uses_jump_table;
    int jump_table_size;

    // Additional fields for optimization tests
    bool optimizes_range_checks;
    bool eliminates_redundant_checks;
    int optimization_level;
    bool optimizes_guard_evaluation;
    bool has_function_calls_in_guards;
    double jump_table_density;
    bool uses_hash_table;
    double hash_table_load_factor;
    bool uses_binary_search;
    bool uses_decision_tree;
    int decision_tree_depth;
    double average_comparisons;
    int runtime_memory_usage;
    bool uses_compact_representation;
    bool optimizes_field_access_order;
    bool minimizes_memory_jumps;
    double cache_miss_estimate;
    int guard_count;
    int instruction_count;

    // Guard-specific fields for pattern matching tests
    bool has_guards;
    bool has_complex_guards;
    bool has_logical_operators;
    bool has_mathematical_guards;
    bool has_enum_guards;
    bool has_struct_guards;
    bool has_multiple_guards;
    bool extracts_enum_values;
    bool extracts_nested_fields;
    bool compiles_guard_expressions;
    bool has_errors;
    char *error_message;
} PatternCompilationResult;

typedef struct {
    bool success;
    bool has_errors;
    char *error_message;
} SemanticResult;

typedef struct {
    char *data;
    size_t length;
} String;

typedef struct {
    char *enum_name;
    char *variant_name;
    int value;
} EnumValue;

typedef struct {
    char *enum_name;
    char *variant_name;
    void *value;
} TaggedUnionValue;

typedef int i32;

typedef struct {
    char *field_name;
    void *value;
} StructValue;

// Generic value type for flexibility
typedef void *Value;

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

// Mock functions for demonstration
extern ASTNode *parse_source(const char *source);
extern SemanticResult *analyze_semantics(ASTNode *ast);
extern PatternCompilationResult *compile_enum_patterns(ASTNode *ast);
extern EnumValue create_enum_value(const char *enum_name, const char *variant_name, int value);
extern TaggedUnionValue create_tagged_union_value(const char *enum_name, const char *variant_name,
                                                  void *value);
extern void *create_i32_value(int value);
extern void *create_string_value(const char *str);
extern void *create_f64_value(double value);
extern void *create_bool_value(bool value);
extern void *create_i8_value(int value);
extern void *create_i64_value(long long value);
extern Value create_tuple_value(Value *values, int count);
extern String execute_pattern_match(PatternCompilationResult *result, EnumValue value);
extern String execute_pattern_match_with_value(PatternCompilationResult *result, void *value);
extern i32 execute_pattern_match_i32(PatternCompilationResult *result, TaggedUnionValue value);
extern StructValue create_struct_value(StructValue *fields, int count);
extern StructValue *create_struct_value_ptr(StructValue *fields, int count);
extern void cleanup_pattern_compilation_result(PatternCompilationResult *result);
extern void cleanup_semantic_result(SemanticResult *result);
extern void cleanup_ast(ASTNode *ast);

// Additional utility function for TaggedUnionValue compatibility
extern String execute_pattern_match_tagged_union(PatternCompilationResult *result,
                                                 TaggedUnionValue value);

#endif // TEST_PATTERN_COMMON_H
