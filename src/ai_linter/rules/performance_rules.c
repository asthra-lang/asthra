#include "../../analysis/semantic_analyzer_core.h"
#include "../../parser/ast.h"
#include "../ai_lint_core.h"
#include <stdio.h>
#include <string.h>

// Helper function to check if a node contains string concatenation
static bool contains_string_concatenation(ASTNode *node, const char *var_name) {
    if (!node)
        return false;

    // Look for assignment patterns like: var = var + other
    if (node->type == AST_ASSIGNMENT) {
        ASTNode *lvalue = node->data.assignment.target;
        ASTNode *rvalue = node->data.assignment.value;

        if (lvalue && lvalue->type == AST_IDENTIFIER &&
            strcmp(lvalue->data.identifier.name, var_name) == 0) {
            // Check if rvalue is a binary operation with +
            if (rvalue && rvalue->type == AST_BINARY_EXPR &&
                rvalue->data.binary_expr.operator== BINOP_ADD) {
                ASTNode *left = rvalue->data.binary_expr.left;
                if (left && left->type == AST_IDENTIFIER &&
                    strcmp(left->data.identifier.name, var_name) == 0) {
                    return true;
                }
            }
        }
    }

    // Recursively check child nodes
    // This is simplified - in practice would traverse all child nodes
    return false;
}

// Helper function to check if a variable is of string type
static bool is_string_type(const char *var_name, AsthraSemanticsAPI *api) {
    if (!var_name || !api)
        return false;

    // This would use the semantic API to check variable type
    // For now, assume variables with "str", "message", "text" are strings
    return (strstr(var_name, "str") != NULL || strstr(var_name, "message") != NULL ||
            strstr(var_name, "text") != NULL || strstr(var_name, "content") != NULL);
}

// Helper function to check if a call is Vec::new()
static bool is_vec_new_call(ASTNode *node) {
    if (!node || node->type != AST_CALL_EXPR)
        return false;

    ASTNode *func = node->data.call_expr.function;
    if (!func || func->type != AST_ASSOCIATED_FUNC_CALL)
        return false;

    // Check for Vec::new() pattern
    const char *struct_name = func->data.associated_func_call.struct_name;
    const char *func_name = func->data.associated_func_call.function_name;

    if (struct_name && strcmp(struct_name, "Vec") == 0 && func_name &&
        strcmp(func_name, "new") == 0) {
        return true;
    }

    return false;
}

// Helper function to find a following loop that uses a variable
static ASTNode *find_following_loop_using_var(ASTNode *var_decl, const char *var_name) {
    // This is a simplified implementation
    // In practice, would traverse the AST to find subsequent loops
    (void)var_decl; // Suppress unused parameter warning
    (void)var_name;
    return NULL; // Placeholder
}

// Helper function to predict loop iterations
static bool can_predict_loop_iterations(ASTNode *loop, AsthraSemanticsAPI *api) {
    if (!loop || !api)
        return false;

    // Check for range(0, n) patterns or iterating over known collections
    if (loop->type == AST_FOR_STMT) {
        ASTNode *iterable = loop->data.for_stmt.iterable;
        if (iterable && iterable->type == AST_CALL_EXPR) {
            ASTNode *func = iterable->data.call_expr.function;
            if (func && func->type == AST_IDENTIFIER &&
                strcmp(func->data.identifier.name, "range") == 0) {
                return true; // range() calls are predictable
            }
        }
    }

    return false;
}

// Rule: AI_PERF_001 - Avoid repeated string concatenation in loops
bool check_string_concat_in_loop(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result) {
    if (!node || node->type != AST_FOR_STMT) {
        return false;
    }

    // Look for string concatenation patterns in loop body
    ASTNode *body = node->data.for_stmt.body;
    if (!body)
        return false;

    // Look for assignment statements in the loop body
    if (body->type == AST_BLOCK && body->data.block.statements) {
        for (size_t i = 0; i < body->data.block.statements->count; i++) {
            ASTNode *stmt = &body->data.block.statements->nodes[i];
            if (stmt->type == AST_ASSIGNMENT) {
                ASTNode *lvalue = stmt->data.assignment.target;
                if (lvalue && lvalue->type == AST_IDENTIFIER) {
                    const char *var_name = lvalue->data.identifier.name;

                    if (contains_string_concatenation(stmt, var_name) &&
                        is_string_type(var_name, api)) {
                        DiagnosticSpan location = {.start_line = (size_t)node->location.line,
                                                   .start_column = (size_t)node->location.column,
                                                   .end_line = (size_t)node->location.line,
                                                   .end_column =
                                                       (size_t)(node->location.column + 10),
                                                   .file_path = strdup("current_file")};

                        *result = ai_lint_result_create(
                            "AI_PERF_001", AI_LINT_PERFORMANCE, AI_LINT_WARNING,
                            "String concatenation in loop detected",
                            "Repeated string concatenation has O(n²) complexity. Use StringBuilder "
                            "for better performance",
                            "Replace 'str = str + other' with StringBuilder.append() for efficient "
                            "concatenation",
                            location,
                            0.9f, // Very high confidence
                            0.8f, // High impact
                            false // Requires semantic transformation
                        );
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

// Rule: AI_PERF_002 - Pre-allocate collections when size is known
bool check_collection_preallocation(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result) {
    if (!node || node->type != AST_LET_STMT)
        return false;

    // Check for Vec::new() without capacity when size is predictable
    ASTNode *init = node->data.let_stmt.initializer;
    if (!init || !is_vec_new_call(init))
        return false;

    const char *var_name = node->data.let_stmt.name;
    if (!var_name)
        return false;

    // Look for subsequent loop that adds to this vector
    ASTNode *following_loop = find_following_loop_using_var(node, var_name);
    if (following_loop && can_predict_loop_iterations(following_loop, api)) {
        DiagnosticSpan location = {.start_line = (size_t)node->location.line,
                                   .start_column = (size_t)node->location.column,
                                   .end_line = (size_t)node->location.line,
                                   .end_column = (size_t)(node->location.column + 20),
                                   .file_path = strdup("current_file")};

        *result = ai_lint_result_create(
            "AI_PERF_002", AI_LINT_PERFORMANCE, AI_LINT_SUGGESTION,
            "Vector allocation without capacity hint",
            "Pre-allocate collections when size is predictable to avoid repeated reallocations",
            "Use Vec::with_capacity(size) instead of Vec::new() when iteration count is known",
            location,
            0.7f, // Good confidence
            0.6f, // Medium impact
            false // Requires size analysis
        );
        return true;
    }

    return false;
}

// Rule: AI_PERF_003 - Avoid unnecessary cloning in loops
bool check_unnecessary_cloning(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result) {
    if (!node || node->type != AST_FOR_STMT) {
        return false;
    }

    // Look for .clone() calls in loop body
    ASTNode *body = node->data.for_stmt.body;
    if (!body)
        return false;

    // Look for method calls in the loop body
    if (body->type == AST_BLOCK && body->data.block.statements) {
        for (size_t i = 0; i < body->data.block.statements->count; i++) {
            ASTNode *stmt = &body->data.block.statements->nodes[i];
            if (stmt->type == AST_EXPR_STMT) {
                ASTNode *expr = stmt->data.expr_stmt.expression;
                if (expr && expr->type == AST_CALL_EXPR) {
                    // Check if this is a .clone() method call
                    ASTNode *func = expr->data.call_expr.function;
                    if (func && func->type == AST_FIELD_ACCESS) {
                        const char *method_name = func->data.field_access.field_name;
                        if (method_name && strcmp(method_name, "clone") == 0) {
                            DiagnosticSpan location = {
                                .start_line = (size_t)stmt->location.line,
                                .start_column = (size_t)stmt->location.column,
                                .end_line = (size_t)stmt->location.line,
                                .end_column = (size_t)(stmt->location.column + 15),
                                .file_path = strdup("current_file")};

                            *result = ai_lint_result_create(
                                "AI_PERF_003", AI_LINT_PERFORMANCE, AI_LINT_WARNING,
                                "Unnecessary cloning in loop detected",
                                "Cloning objects in loops can be expensive. Consider borrowing or "
                                "moving instead",
                                "Use references (&item) or move semantics instead of .clone() in "
                                "performance-critical loops",
                                location,
                                0.8f, // High confidence
                                0.7f, // High impact
                                false // Requires semantic analysis
                            );
                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}

// Rule: AI_PERF_004 - Manual indexing vs iterator patterns
bool check_manual_indexing_performance(ASTNode *node, AsthraSemanticsAPI *api,
                                       AILintResult **result) {
    if (!node || node->type != AST_FOR_STMT)
        return false;

    // Check if this is a manual indexing pattern: for i in range(0, vec.len())
    ASTNode *iterator = node->data.for_stmt.iterable;
    if (iterator && iterator->type == AST_CALL_EXPR) {
        ASTNode *func_name = iterator->data.call_expr.function;
        if (func_name && func_name->type == AST_IDENTIFIER &&
            strcmp(func_name->data.identifier.name, "range") == 0) {
            // Check if body uses vec[i] pattern
            ASTNode *body = node->data.for_stmt.body;
            if (body && body->type == AST_BLOCK && body->data.block.statements) {
                for (size_t i = 0; i < body->data.block.statements->count; i++) {
                    ASTNode *stmt = &body->data.block.statements->nodes[i];
                    // Look for index access patterns
                    if (stmt->type == AST_EXPR_STMT) {
                        ASTNode *expr = stmt->data.expr_stmt.expression;
                        if (expr && expr->type == AST_INDEX_ACCESS) {
                            DiagnosticSpan location = {
                                .start_line = (size_t)node->location.line,
                                .start_column = (size_t)node->location.column,
                                .end_line = (size_t)node->location.line,
                                .end_column = (size_t)(node->location.column + 10),
                                .file_path = strdup("current_file")};

                            *result = ai_lint_result_create(
                                "AI_PERF_004", AI_LINT_PERFORMANCE, AI_LINT_SUGGESTION,
                                "Manual indexing detected",
                                "Iterator patterns are often more efficient and safer than manual "
                                "indexing",
                                "Replace 'for i in range(0, vec.len()) { vec[i] }' with 'for item "
                                "in vec { item }'",
                                location,
                                0.8f, // High confidence
                                0.6f, // Medium impact
                                true  // Auto-fixable
                            );
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

// Registration function for all performance rules
void register_performance_rules(AILinter *linter) {
    if (!linter)
        return;

    // AI_PERF_001: String concatenation in loops
    AILintRule string_concat_rule = {
        .rule_id = "AI_PERF_001",
        .category = AI_LINT_PERFORMANCE,
        .default_severity = AI_LINT_WARNING,
        .title = "String concatenation in loop detected",
        .description = "Repeated string concatenation has O(n²) complexity. Use StringBuilder for "
                       "better performance",
        .ai_guidance =
            "Replace 'str = str + other' with StringBuilder.append() for efficient concatenation",
        .check_function = check_string_concat_in_loop,
        .before_examples = NULL,
        .after_examples = NULL,
        .example_count = 0,
        .impact_score = 0.8f,
        .auto_fixable = false};
    ai_linter_register_rule(linter, &string_concat_rule);

    // AI_PERF_002: Collection pre-allocation
    AILintRule preallocation_rule = {
        .rule_id = "AI_PERF_002",
        .category = AI_LINT_PERFORMANCE,
        .default_severity = AI_LINT_SUGGESTION,
        .title = "Vector allocation without capacity hint",
        .description =
            "Pre-allocate collections when size is predictable to avoid repeated reallocations",
        .ai_guidance =
            "Use Vec::with_capacity(size) instead of Vec::new() when iteration count is known",
        .check_function = check_collection_preallocation,
        .before_examples = NULL,
        .after_examples = NULL,
        .example_count = 0,
        .impact_score = 0.6f,
        .auto_fixable = false};
    ai_linter_register_rule(linter, &preallocation_rule);

    // AI_PERF_003: Unnecessary cloning
    AILintRule cloning_rule = {
        .rule_id = "AI_PERF_003",
        .category = AI_LINT_PERFORMANCE,
        .default_severity = AI_LINT_WARNING,
        .title = "Unnecessary cloning in loop detected",
        .description =
            "Cloning objects in loops can be expensive. Consider borrowing or moving instead",
        .ai_guidance = "Use references (&item) or move semantics instead of .clone() in "
                       "performance-critical loops",
        .check_function = check_unnecessary_cloning,
        .before_examples = NULL,
        .after_examples = NULL,
        .example_count = 0,
        .impact_score = 0.7f,
        .auto_fixable = false};
    ai_linter_register_rule(linter, &cloning_rule);

    // AI_PERF_004: Manual indexing
    AILintRule indexing_rule = {
        .rule_id = "AI_PERF_004",
        .category = AI_LINT_PERFORMANCE,
        .default_severity = AI_LINT_SUGGESTION,
        .title = "Manual indexing detected",
        .description = "Iterator patterns are often more efficient and safer than manual indexing",
        .ai_guidance =
            "Replace 'for i in range(0, vec.len()) { vec[i] }' with 'for item in vec { item }'",
        .check_function = check_manual_indexing_performance,
        .before_examples = NULL,
        .after_examples = NULL,
        .example_count = 0,
        .impact_score = 0.6f,
        .auto_fixable = true};
    ai_linter_register_rule(linter, &indexing_rule);
}
