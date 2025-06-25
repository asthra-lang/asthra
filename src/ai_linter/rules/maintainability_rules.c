#include "maintainability_rules.h"
#include "../../analysis/semantic_analyzer_core.h"
#include "../ai_lint_core.h"
#include <stdlib.h>
#include <string.h>

// Helper function to convert SourceLocation to DiagnosticSpan
static DiagnosticSpan source_location_to_diagnostic_span(SourceLocation location) {
    DiagnosticSpan span = {.start_line = (size_t)location.line,
                           .start_column = (size_t)location.column,
                           .end_line = (size_t)location.line,
                           .end_column = (size_t)(location.column + 10), // Approximate end
                           .file_path =
                               location.filename ? strdup(location.filename) : strdup("unknown"),
                           .label = NULL,
                           .snippet = NULL};
    return span;
}

// Helper function declarations
static bool contains_error_handling(ASTNode *node);
static bool has_documentation_comment(ASTNode *node);
static int count_function_parameters(ASTNode *func_node);
static bool is_complex_function(ASTNode *func_node);
static bool has_magic_numbers(ASTNode *node);
static bool is_deeply_nested(ASTNode *node, int max_depth);
static bool has_descriptive_name(const char *name, ASTNodeType context);
static bool should_have_error_handling(ASTNode *node);

// Rule: AI_MAINT_001 - Functions should have proper error handling
bool check_missing_error_handling(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result) {
    if (node->type != AST_FUNCTION_DECL)
        return false;

    // Check if function should have error handling but doesn't
    if (should_have_error_handling(node) &&
        !contains_error_handling(node->data.function_decl.body)) {
        DiagnosticSpan location = source_location_to_diagnostic_span(node->location);
        *result = ai_lint_result_create(
            "AI_MAINT_001", AI_LINT_MAINTAINABILITY, AI_LINT_WARNING, "Missing error handling",
            "Functions that can fail should return Result<T, E> and handle errors properly",
            "Add proper error handling with Result<T, E> return type and error propagation",
            location,
            0.75f, // Good confidence
            0.75f, // Good impact
            false  // Not auto-fixable (requires semantic analysis)
        );
        return true;
    }
    return false;
}

// Rule: AI_MAINT_002 - Public functions should have documentation
bool check_missing_documentation(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result) {
    if (node->type != AST_FUNCTION_DECL)
        return false;

    // Check if this is a public function
    if (node->data.function_decl.visibility == VISIBILITY_PUBLIC) {
        if (!has_documentation_comment(node)) {
            DiagnosticSpan location = source_location_to_diagnostic_span(node->location);
            *result = ai_lint_result_create(
                "AI_MAINT_002", AI_LINT_MAINTAINABILITY, AI_LINT_SUGGESTION,
                "Missing documentation for public function",
                "Public functions should have documentation comments explaining their purpose, "
                "parameters, and return values",
                "Add documentation comment above function: /// Description of function purpose",
                location,
                0.90f, // Very high confidence
                0.60f, // Medium impact
                false  // Not auto-fixable (requires human input)
            );
            return true;
        }
    }
    return false;
}

// Rule: AI_MAINT_003 - Functions should not have too many parameters
bool check_too_many_parameters(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result) {
    if (node->type != AST_FUNCTION_DECL)
        return false;

    int param_count = count_function_parameters(node);
    if (param_count > 5) { // Threshold for too many parameters
        DiagnosticSpan location = source_location_to_diagnostic_span(node->location);
        *result = ai_lint_result_create(
            "AI_MAINT_003", AI_LINT_MAINTAINABILITY, AI_LINT_WARNING,
            "Too many function parameters",
            "Functions with many parameters are hard to understand and maintain. Consider using a "
            "struct or builder pattern",
            "Refactor to use a parameter struct or builder pattern to reduce parameter count",
            location,
            0.80f, // High confidence
            0.70f, // High impact
            false  // Not auto-fixable (requires refactoring)
        );
        return true;
    }
    return false;
}

// Rule: AI_MAINT_004 - Avoid magic numbers
bool check_magic_numbers(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result) {
    if (node->type != AST_INTEGER_LITERAL && node->type != AST_FLOAT_LITERAL)
        return false;

    // Skip common non-magic numbers (0, 1, 2, etc.)
    if (node->type == AST_INTEGER_LITERAL) {
        long value = node->data.integer_literal.value;
        if (value >= 0 && value <= 2)
            return false; // 0, 1, 2 are usually not magic
    }

    // Check if this number appears in a context where it should be a constant
    if (has_magic_numbers(node)) {
        DiagnosticSpan location = source_location_to_diagnostic_span(node->location);
        *result = ai_lint_result_create(
            "AI_MAINT_004", AI_LINT_MAINTAINABILITY, AI_LINT_SUGGESTION, "Magic number detected",
            "Numeric literals should be replaced with named constants for better maintainability",
            "Replace magic number with a named constant: const BUFFER_SIZE: usize = 1024;",
            location,
            0.70f, // Medium-high confidence
            0.50f, // Medium impact
            false  // Not auto-fixable (requires semantic understanding)
        );
        return true;
    }
    return false;
}

// Rule: AI_MAINT_005 - Avoid deeply nested code
bool check_deep_nesting(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result) {
    if (node->type != AST_FUNCTION_DECL)
        return false;

    if (is_deeply_nested(node->data.function_decl.body, 4)) { // Max 4 levels
        DiagnosticSpan location = source_location_to_diagnostic_span(node->location);
        *result = ai_lint_result_create("AI_MAINT_005", AI_LINT_MAINTAINABILITY, AI_LINT_WARNING,
                                        "Deeply nested code detected",
                                        "Deeply nested code is hard to read and maintain. Consider "
                                        "extracting functions or using early returns",
                                        "Refactor deeply nested code using early returns, guard "
                                        "clauses, or extracted functions",
                                        location,
                                        0.85f, // High confidence
                                        0.80f, // High impact
                                        false  // Not auto-fixable (requires refactoring)
        );
        return true;
    }
    return false;
}

// Rule: AI_MAINT_006 - Use descriptive variable and function names
bool check_descriptive_names(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result) {
    const char *name = NULL;
    ASTNodeType context = node->type;

    switch (node->type) {
    case AST_FUNCTION_DECL:
        name = node->data.function_decl.name;
        break;
    case AST_LET_STMT:
        name = node->data.let_stmt.name;
        break;
    case AST_PARAM_DECL:
        name = node->data.param_decl.name;
        break;
    default:
        return false;
    }

    if (name && !has_descriptive_name(name, context)) {
        DiagnosticSpan location = source_location_to_diagnostic_span(node->location);
        *result = ai_lint_result_create(
            "AI_MAINT_006", AI_LINT_MAINTAINABILITY, AI_LINT_SUGGESTION,
            "Non-descriptive name detected",
            "Variable and function names should be descriptive and self-documenting",
            "Use descriptive names that clearly indicate purpose: 'user_count' instead of 'n'",
            location,
            0.60f, // Medium confidence (subjective)
            0.60f, // Medium impact
            false  // Not auto-fixable (requires human judgment)
        );
        return true;
    }
    return false;
}

// Helper function implementations
static bool contains_error_handling(ASTNode *node) {
    if (!node)
        return false;

    // Look for Result types, match expressions, or error propagation
    switch (node->type) {
    case AST_MATCH_STMT:
        // Check if matching on Result type
        return true; // Simplified - would need semantic analysis
    case AST_CALL_EXPR:
        // Check for .unwrap(), .expect(), or ? operator usage
        return true; // Simplified
    default:
        break;
    }

    // Recursively check child nodes
    // This is a simplified implementation - real version would traverse all children
    return false;
}

static bool has_documentation_comment(ASTNode *node) {
    // Check if node has preceding documentation comments
    // This would need to be implemented with comment tracking in the parser
    return false; // Simplified - would check for /// comments
}

static int count_function_parameters(ASTNode *func_node) {
    if (!func_node || func_node->type != AST_FUNCTION_DECL)
        return 0;

    ASTNodeList *params = func_node->data.function_decl.params;
    if (!params)
        return 0;

    return (int)params->count;
}

static bool is_complex_function(ASTNode *func_node) {
    // Check various complexity metrics
    int param_count = count_function_parameters(func_node);
    bool has_deep_nesting = is_deeply_nested(func_node->data.function_decl.body, 3);

    return param_count > 3 || has_deep_nesting;
}

static bool has_magic_numbers(ASTNode *node) {
    // Check if this number appears in a context where it should be a constant
    // Look at parent context to determine if it's likely a magic number
    return true; // Simplified - would analyze context
}

static bool is_deeply_nested(ASTNode *node, int max_depth) {
    if (!node || max_depth <= 0)
        return max_depth < 0;

    switch (node->type) {
    case AST_IF_STMT:
    case AST_FOR_STMT:
    case AST_MATCH_STMT:
    case AST_BLOCK:
        // These constructs add nesting depth
        max_depth--;
        break;
    default:
        break;
    }

    // Recursively check child nodes (simplified)
    return max_depth < 0;
}

static bool has_descriptive_name(const char *name, ASTNodeType context) {
    if (!name)
        return false;

    size_t len = strlen(name);

    // Very short names are usually not descriptive (except for common cases)
    if (len <= 2) {
        // Allow common short names in certain contexts
        if (context == AST_PARAM_DECL &&
            (strcmp(name, "i") == 0 || strcmp(name, "j") == 0 || strcmp(name, "k") == 0)) {
            return true; // Loop indices are acceptable
        }
        return false;
    }

    // Check for common non-descriptive patterns
    if (strcmp(name, "temp") == 0 || strcmp(name, "tmp") == 0 || strcmp(name, "data") == 0 ||
        strcmp(name, "val") == 0 || strcmp(name, "var") == 0 || strcmp(name, "obj") == 0) {
        return false;
    }

    return true; // Assume longer names are descriptive
}

static bool should_have_error_handling(ASTNode *node) {
    if (!node || node->type != AST_FUNCTION_DECL)
        return false;

    // Functions that interact with external resources should have error handling
    // This is a simplified heuristic - real implementation would be more sophisticated
    const char *name = node->data.function_decl.name;
    if (!name)
        return false;

    // Check for common patterns that suggest error handling is needed
    if (strstr(name, "read") || strstr(name, "write") || strstr(name, "open") ||
        strstr(name, "connect") || strstr(name, "parse") || strstr(name, "load")) {
        return true;
    }

    return false;
}

// Registration function for all maintainability rules
void register_maintainability_rules(AILinter *linter) {
    if (!linter)
        return;

    // AI_MAINT_001: Missing error handling
    AILintRule error_handling_rule = {
        .rule_id = "AI_MAINT_001",
        .category = AI_LINT_MAINTAINABILITY,
        .default_severity = AI_LINT_WARNING,
        .title = "Missing error handling",
        .description =
            "Functions that can fail should return Result<T, E> and handle errors properly",
        .ai_guidance =
            "Add proper error handling with Result<T, E> return type and error propagation",
        .check_function = check_missing_error_handling,
        .before_examples = NULL,
        .after_examples = NULL,
        .example_count = 0,
        .impact_score = 0.75f,
        .auto_fixable = false};
    ai_linter_register_rule(linter, &error_handling_rule);

    // AI_MAINT_002: Missing documentation
    AILintRule documentation_rule = {
        .rule_id = "AI_MAINT_002",
        .category = AI_LINT_MAINTAINABILITY,
        .default_severity = AI_LINT_SUGGESTION,
        .title = "Missing documentation for public function",
        .description = "Public functions should have documentation comments explaining their "
                       "purpose, parameters, and return values",
        .ai_guidance =
            "Add documentation comment above function: /// Description of function purpose",
        .check_function = check_missing_documentation,
        .before_examples = NULL,
        .after_examples = NULL,
        .example_count = 0,
        .impact_score = 0.60f,
        .auto_fixable = false};
    ai_linter_register_rule(linter, &documentation_rule);

    // AI_MAINT_003: Too many parameters
    AILintRule parameters_rule = {
        .rule_id = "AI_MAINT_003",
        .category = AI_LINT_MAINTAINABILITY,
        .default_severity = AI_LINT_WARNING,
        .title = "Too many function parameters",
        .description = "Functions with many parameters are hard to understand and maintain. "
                       "Consider using a struct or builder pattern",
        .ai_guidance =
            "Refactor to use a parameter struct or builder pattern to reduce parameter count",
        .check_function = check_too_many_parameters,
        .before_examples = NULL,
        .after_examples = NULL,
        .example_count = 0,
        .impact_score = 0.70f,
        .auto_fixable = false};
    ai_linter_register_rule(linter, &parameters_rule);

    // AI_MAINT_004: Magic numbers
    AILintRule magic_numbers_rule = {
        .rule_id = "AI_MAINT_004",
        .category = AI_LINT_MAINTAINABILITY,
        .default_severity = AI_LINT_SUGGESTION,
        .title = "Magic number detected",
        .description =
            "Numeric literals should be replaced with named constants for better maintainability",
        .ai_guidance =
            "Replace magic number with a named constant: const BUFFER_SIZE: usize = 1024;",
        .check_function = check_magic_numbers,
        .before_examples = NULL,
        .after_examples = NULL,
        .example_count = 0,
        .impact_score = 0.50f,
        .auto_fixable = false};
    ai_linter_register_rule(linter, &magic_numbers_rule);

    // AI_MAINT_005: Deep nesting
    AILintRule nesting_rule = {.rule_id = "AI_MAINT_005",
                               .category = AI_LINT_MAINTAINABILITY,
                               .default_severity = AI_LINT_WARNING,
                               .title = "Deeply nested code detected",
                               .description =
                                   "Deeply nested code is hard to read and maintain. Consider "
                                   "extracting functions or using early returns",
                               .ai_guidance = "Refactor deeply nested code using early returns, "
                                              "guard clauses, or extracted functions",
                               .check_function = check_deep_nesting,
                               .before_examples = NULL,
                               .after_examples = NULL,
                               .example_count = 0,
                               .impact_score = 0.80f,
                               .auto_fixable = false};
    ai_linter_register_rule(linter, &nesting_rule);

    // AI_MAINT_006: Descriptive names
    AILintRule naming_rule = {
        .rule_id = "AI_MAINT_006",
        .category = AI_LINT_MAINTAINABILITY,
        .default_severity = AI_LINT_SUGGESTION,
        .title = "Non-descriptive name detected",
        .description = "Variable and function names should be descriptive and self-documenting",
        .ai_guidance =
            "Use descriptive names that clearly indicate purpose: 'user_count' instead of 'n'",
        .check_function = check_descriptive_names,
        .before_examples = NULL,
        .after_examples = NULL,
        .example_count = 0,
        .impact_score = 0.60f,
        .auto_fixable = false};
    ai_linter_register_rule(linter, &naming_rule);
}
