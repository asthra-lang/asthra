#include "../../analysis/semantic_analyzer.h"
#include "../../parser/ast.h"
#include "../ai_lint_core.h"
#include <stdio.h>
#include <string.h>

// Helper functions for pattern detection
static bool contains_index_access_pattern(ASTNode *body, const char *loop_var);
static ASTNode *find_parent_function(ASTNode *node);
static bool is_last_statement_in_function(ASTNode *stmt, ASTNode *func);
static int count_if_else_depth(ASTNode *if_stmt);
static bool could_be_match_expression(ASTNode *if_stmt, AsthraSemanticsAPI *api);

// Rule: AI_IDIOM_001 - Prefer iterator patterns over manual indexing
static bool check_manual_indexing(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result) {
    if (!node || node->type != AST_FOR_STMT)
        return false;

    // Check if this is a manual indexing pattern: for i in range(0, vec.len())
    ASTNode *iterable = node->data.for_stmt.iterable;
    if (iterable && iterable->type == AST_CALL_EXPR) {
        ASTNode *func_name = iterable->data.call_expr.function;
        if (func_name && func_name->type == AST_IDENTIFIER &&
            strcmp(func_name->data.identifier.name, "range") == 0) {
            // Check if body uses vec[i] pattern
            ASTNode *body = node->data.for_stmt.body;
            const char *loop_var = node->data.for_stmt.variable;
            if (contains_index_access_pattern(body, loop_var)) {
                DiagnosticSpan location = {
                    .start_line = node->location.line,
                    .start_column = node->location.column,
                    .end_line = node->location.line,
                    .end_column = node->location.column + 10,
                    .file_path =
                        strdup("current_file") // Would be set properly in real implementation
                };

                *result = ai_lint_result_create("AI_IDIOM_001", AI_LINT_IDIOM, AI_LINT_SUGGESTION,
                                                "Manual indexing detected",
                                                "Use iterator patterns instead of manual indexing "
                                                "for better readability and safety",
                                                "Replace 'for i in range(0, vec.len()) { vec[i] }' "
                                                "with 'for item in vec { item }'",
                                                location,
                                                0.8f, // High confidence
                                                0.7f, // Good impact
                                                true  // Auto-fixable
                );
                return true;
            }
        }
    }
    return false;
}

// Rule: AI_IDIOM_002 - Prefer implicit returns in expressions
static bool check_explicit_return(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result) {
    if (!node || node->type != AST_RETURN_STMT)
        return false;

    // Check if this is the last statement in a function
    ASTNode *parent = find_parent_function(node);
    if (parent && is_last_statement_in_function(node, parent)) {
        DiagnosticSpan location = {.start_line = node->location.line,
                                   .start_column = node->location.column,
                                   .end_line = node->location.line,
                                   .end_column = node->location.column + 6, // "return"
                                   .file_path = strdup("current_file")};

        *result = ai_lint_result_create(
            "AI_IDIOM_002", AI_LINT_IDIOM, AI_LINT_SUGGESTION,
            "Explicit return in expression context",
            "Asthra functions can use implicit returns for the last expression",
            "Remove 'return' keyword from the last expression in function body", location,
            0.7f, // Medium-high confidence
            0.6f, // Medium impact
            true  // Auto-fixable
        );
        return true;
    }
    return false;
}

// Rule: AI_IDIOM_003 - Use pattern matching instead of nested if-else
static bool check_nested_if_else(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result) {
    if (!node || node->type != AST_IF_STMT)
        return false;

    // Count nested if-else depth
    int depth = count_if_else_depth(node);
    if (depth >= 3) {
        // Check if this could be a match expression
        if (could_be_match_expression(node, api)) {
            DiagnosticSpan location = {.start_line = node->location.line,
                                       .start_column = node->location.column,
                                       .end_line = node->location.line,
                                       .end_column = node->location.column + 2, // "if"
                                       .file_path = strdup("current_file")};

            *result = ai_lint_result_create(
                "AI_IDIOM_003", AI_LINT_IDIOM, AI_LINT_SUGGESTION, "Deep if-else nesting detected",
                "Consider using match expressions for complex conditional logic",
                "Replace nested if-else with match expression for better readability", location,
                0.6f, // Medium confidence
                0.8f, // High impact for readability
                false // Not auto-fixable (requires semantic analysis)
            );
            return true;
        }
    }
    return false;
}

// Helper function implementations
static bool contains_index_access_pattern(ASTNode *body, const char *loop_var) {
    if (!body || !loop_var)
        return false;

    // Simplified check - in practice would traverse the AST looking for array access
    // patterns like vec[i] where i is the loop variable
    switch (body->type) {
    case AST_BLOCK:
        if (body->data.block.statements) {
            for (size_t i = 0; i < body->data.block.statements->count; i++) {
                if (contains_index_access_pattern(&body->data.block.statements->nodes[i],
                                                  loop_var)) {
                    return true;
                }
            }
        }
        break;
    case AST_INDEX_ACCESS:
        // Check if the index is our loop variable
        if (body->data.index_access.index &&
            body->data.index_access.index->type == AST_IDENTIFIER &&
            strcmp(body->data.index_access.index->data.identifier.name, loop_var) == 0) {
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}

static ASTNode *find_parent_function(ASTNode *node) {
    // In practice, this would traverse up the AST to find the containing function
    // For now, return NULL as a placeholder
    return NULL;
}

static bool is_last_statement_in_function(ASTNode *stmt, ASTNode *func) {
    // In practice, this would check if the statement is the last one in the function body
    // For now, return true as a placeholder
    return true;
}

static int count_if_else_depth(ASTNode *if_stmt) {
    if (!if_stmt || if_stmt->type != AST_IF_STMT)
        return 0;

    int depth = 1;
    ASTNode *else_part = if_stmt->data.if_stmt.else_block;
    if (else_part && else_part->type == AST_IF_STMT) {
        depth += count_if_else_depth(else_part);
    }
    return depth;
}

static bool could_be_match_expression(ASTNode *if_stmt, AsthraSemanticsAPI *api) {
    // Simplified check - in practice would analyze the condition patterns
    // to see if they're suitable for match expressions
    return true; // Placeholder
}

// Function to register all idiom rules
void register_idiom_rules(AILinter *linter) {
    if (!linter)
        return;

    // Register AI_IDIOM_001
    AILintRule rule1 = {
        .rule_id = strdup("AI_IDIOM_001"),
        .category = AI_LINT_IDIOM,
        .default_severity = AI_LINT_SUGGESTION,
        .title = strdup("Manual indexing detected"),
        .description = strdup("Use iterator patterns instead of manual indexing"),
        .ai_guidance = strdup(
            "Replace 'for i in range(0, vec.len()) { vec[i] }' with 'for item in vec { item }'"),
        .check_function = check_manual_indexing,
        .before_examples = NULL,
        .after_examples = NULL,
        .example_count = 0,
        .impact_score = 0.7f,
        .auto_fixable = true};
    ai_linter_register_rule(linter, &rule1);

    // Register AI_IDIOM_002
    AILintRule rule2 = {
        .rule_id = strdup("AI_IDIOM_002"),
        .category = AI_LINT_IDIOM,
        .default_severity = AI_LINT_SUGGESTION,
        .title = strdup("Explicit return in expression context"),
        .description = strdup("Asthra functions can use implicit returns"),
        .ai_guidance = strdup("Remove 'return' keyword from the last expression in function body"),
        .check_function = check_explicit_return,
        .before_examples = NULL,
        .after_examples = NULL,
        .example_count = 0,
        .impact_score = 0.6f,
        .auto_fixable = true};
    ai_linter_register_rule(linter, &rule2);

    // Register AI_IDIOM_003
    AILintRule rule3 = {
        .rule_id = strdup("AI_IDIOM_003"),
        .category = AI_LINT_IDIOM,
        .default_severity = AI_LINT_SUGGESTION,
        .title = strdup("Deep if-else nesting detected"),
        .description = strdup("Consider using match expressions for complex conditional logic"),
        .ai_guidance =
            strdup("Replace nested if-else with match expression for better readability"),
        .check_function = check_nested_if_else,
        .before_examples = NULL,
        .after_examples = NULL,
        .example_count = 0,
        .impact_score = 0.8f,
        .auto_fixable = false};
    ai_linter_register_rule(linter, &rule3);
}
