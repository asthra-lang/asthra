/*
 * =============================================================================
 * GUARD EXPRESSION TEST COMMON UTILITIES
 * =============================================================================
 *
 * This header contains shared types, macros, and utility functions used
 * across all guard expression test files.
 *
 * Part of Phase 3.2: Advanced Pattern Matching
 *
 * =============================================================================
 */

#ifndef TEST_GUARD_COMMON_H
#define TEST_GUARD_COMMON_H

#include "test_pattern_common.h"

// =============================================================================
// GUARD-SPECIFIC TYPES (using base types from test_pattern_common.h)
// =============================================================================

typedef struct {
    Value *values;
    size_t count;
} TupleValue;

// =============================================================================
// GUARD-SPECIFIC UTILITY FUNCTIONS
// =============================================================================

// Guard test environment
extern void setup_guard_test_environment(void);
extern void cleanup_guard_test_environment(void);
extern void init_guard_expression_compiler(void);
extern void cleanup_guard_expression_compiler(void);

// Guard compilation
extern PatternCompilationResult *compile_patterns_with_guards(ASTNode *ast);

// Value creation functions (using base implementations from test_pattern_common.h)
extern String create_string(const char *str);
extern TupleValue create_tuple_value_extended(Value *values, size_t count);

// Pattern execution with guards (using base implementation from test_pattern_common.h)

#endif // TEST_GUARD_COMMON_H
