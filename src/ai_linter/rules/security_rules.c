#include "../../analysis/semantic_analyzer.h"
#include "../ai_lint_core.h"
#include <stdio.h>
#include <string.h>

// Helper function to check if a function call is potentially unsafe
static bool is_unsafe_function(const char *func_name) {
    if (!func_name)
        return false;

    // List of potentially unsafe functions
    const char *unsafe_funcs[] = {"strcpy", "strcat",  "sprintf", "gets",   "scanf",
                                  "system", "exec",    "eval",    "malloc", "free",
                                  "memcpy", "memmove", "strncpy", "strncat"};

    size_t count = sizeof(unsafe_funcs) / sizeof(unsafe_funcs[0]);
    for (size_t i = 0; i < count; i++) {
        if (strcmp(func_name, unsafe_funcs[i]) == 0) {
            return true;
        }
    }

    return false;
}

// Helper function to check if a string contains user input patterns
static bool contains_user_input_pattern(ASTNode *node) {
    if (!node)
        return false;

    // Look for common user input patterns
    if (node->type == AST_IDENTIFIER) {
        const char *name = node->data.identifier.name;
        return (strstr(name, "input") != NULL || strstr(name, "user") != NULL ||
                strstr(name, "request") != NULL || strstr(name, "param") != NULL ||
                strstr(name, "arg") != NULL);
    }

    return false;
}

// Helper function to check if a node represents a buffer operation
static bool is_buffer_operation(ASTNode *node) {
    if (!node || node->type != AST_CALL_EXPR)
        return false;

    ASTNode *func = node->data.call_expr.function;
    if (!func || func->type != AST_IDENTIFIER)
        return false;

    const char *func_name = func->data.identifier.name;
    return (strcmp(func_name, "memcpy") == 0 || strcmp(func_name, "strcpy") == 0 ||
            strcmp(func_name, "strcat") == 0 || strcmp(func_name, "sprintf") == 0);
}

// Helper function to check if bounds checking is present
static bool has_bounds_checking(ASTNode *node) {
    if (!node)
        return false;

    // This is a simplified check - would need full analysis
    // Look for length checks, size parameters, etc.
    return false; // Placeholder - assume no bounds checking for now
}

// Rule: AI_SEC_001 - Detect unsafe function usage
bool check_unsafe_function_usage(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result) {
    if (!node || node->type != AST_CALL_EXPR)
        return false;

    ASTNode *func = node->data.call_expr.function;
    if (!func || func->type != AST_IDENTIFIER)
        return false;

    const char *func_name = func->data.identifier.name;
    if (is_unsafe_function(func_name)) {
        DiagnosticSpan location = {.start_line = node->location.line,
                                   .start_column = node->location.column,
                                   .end_line = node->location.line,
                                   .end_column = node->location.column + (int)strlen(func_name),
                                   .file_path = strdup("current_file")};

        char guidance[256];
        snprintf(guidance, sizeof(guidance),
                 "Function '%s' is potentially unsafe. Use safer alternatives with bounds checking",
                 func_name);

        *result = ai_lint_result_create(
            "AI_SEC_001", AI_LINT_SECURITY, AI_LINT_ERROR, "Unsafe function usage detected",
            "Using unsafe functions can lead to buffer overflows and security vulnerabilities",
            guidance, location,
            0.9f, // Very high confidence
            0.9f, // Critical impact
            false // Requires manual review
        );
        return true;
    }

    return false;
}

// Rule: AI_SEC_002 - Detect potential buffer overflows
bool check_buffer_overflow_risk(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result) {
    if (!node || !is_buffer_operation(node))
        return false;

    // Check if bounds checking is missing
    if (!has_bounds_checking(node)) {
        DiagnosticSpan location = {.start_line = node->location.line,
                                   .start_column = node->location.column,
                                   .end_line = node->location.line,
                                   .end_column = node->location.column + 10,
                                   .file_path = strdup("current_file")};

        *result = ai_lint_result_create(
            "AI_SEC_002", AI_LINT_SECURITY, AI_LINT_ERROR, "Potential buffer overflow detected",
            "Buffer operations without bounds checking can lead to memory corruption",
            "Add explicit bounds checking or use safer string/memory functions", location,
            0.8f, // High confidence
            0.9f, // Critical impact
            false // Requires manual analysis
        );
        return true;
    }

    return false;
}

// Rule: AI_SEC_003 - Detect unvalidated user input
bool check_unvalidated_user_input(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result) {
    if (!node || node->type != AST_CALL_EXPR)
        return false;

    // Check if function arguments contain user input patterns
    ASTNodeList *args = node->data.call_expr.args;
    if (!args)
        return false;

    // Look for user input being passed to potentially dangerous functions
    ASTNode *func = node->data.call_expr.function;
    if (!func || func->type != AST_IDENTIFIER)
        return false;

    const char *func_name = func->data.identifier.name;
    bool is_dangerous = (strcmp(func_name, "system") == 0 || strcmp(func_name, "exec") == 0 ||
                         strcmp(func_name, "eval") == 0 || strcmp(func_name, "sql_query") == 0);

    if (is_dangerous) {
        // Check if any arguments look like user input
        if (args) {
            for (size_t i = 0; i < args->count; i++) {
                ASTNode *arg = &args->nodes[i];
                if (contains_user_input_pattern(arg)) {
                    DiagnosticSpan location = {.start_line = node->location.line,
                                               .start_column = node->location.column,
                                               .end_line = node->location.line,
                                               .end_column =
                                                   node->location.column + (int)strlen(func_name),
                                               .file_path = strdup("current_file")};

                    *result = ai_lint_result_create("AI_SEC_003", AI_LINT_SECURITY, AI_LINT_ERROR,
                                                    "Unvalidated user input detected",
                                                    "Passing unvalidated user input to dangerous "
                                                    "functions can lead to injection attacks",
                                                    "Validate and sanitize all user input before "
                                                    "using in system calls or queries",
                                                    location,
                                                    0.7f, // Good confidence
                                                    0.9f, // Critical impact
                                                    false // Requires manual validation logic
                    );
                    return true;
                }
            }
        }
    }

    return false;
}

// Rule: AI_SEC_004 - Detect hardcoded secrets
bool check_hardcoded_secrets(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result) {
    if (!node || node->type != AST_STRING_LITERAL)
        return false;

    const char *str_value = node->data.string_literal.value;
    if (!str_value)
        return false;

    // Check for patterns that look like secrets
    bool looks_like_secret = false;
    const char *secret_patterns[] = {"password", "passwd", "pwd",  "secret",     "key",    "token",
                                     "api_key",  "apikey", "auth", "credential", "private"};

    size_t pattern_count = sizeof(secret_patterns) / sizeof(secret_patterns[0]);
    for (size_t i = 0; i < pattern_count; i++) {
        if (strstr(str_value, secret_patterns[i]) != NULL) {
            looks_like_secret = true;
            break;
        }
    }

    // Also check for long alphanumeric strings that might be keys
    if (!looks_like_secret && strlen(str_value) > 20) {
        int alpha_count = 0, digit_count = 0;
        for (const char *p = str_value; *p; p++) {
            if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z'))
                alpha_count++;
            else if (*p >= '0' && *p <= '9')
                digit_count++;
        }

        // If string is mostly alphanumeric and long, might be a key
        if (alpha_count + digit_count > (int)(strlen(str_value) * 0.8)) {
            looks_like_secret = true;
        }
    }

    if (looks_like_secret) {
        DiagnosticSpan location = {.start_line = node->location.line,
                                   .start_column = node->location.column,
                                   .end_line = node->location.line,
                                   .end_column = node->location.column + (int)strlen(str_value) +
                                                 2, // Include quotes
                                   .file_path = strdup("current_file")};

        *result = ai_lint_result_create(
            "AI_SEC_004", AI_LINT_SECURITY, AI_LINT_WARNING, "Potential hardcoded secret detected",
            "Hardcoded secrets in source code pose security risks",
            "Move secrets to environment variables or secure configuration files", location,
            0.6f, // Medium confidence (could be false positive)
            0.8f, // High impact
            false // Requires manual review
        );
        return true;
    }

    return false;
}

// Rule: AI_SEC_005 - Detect unsafe pointer operations
bool check_unsafe_pointer_operations(ASTNode *node, AsthraSemanticsAPI *api,
                                     AILintResult **result) {
    if (!node)
        return false;

    // Check for unsafe pointer dereferences
    if (node->type == AST_UNARY_EXPR &&
        node->data.unary_expr.operator== UNOP_DEREF) { // Dereference operator

        ASTNode *operand = node->data.unary_expr.operand;
        if (operand && operand->type == AST_IDENTIFIER) {
            // Check if this is in an unsafe block
            // For now, assume all pointer dereferences outside unsafe blocks are risky
            DiagnosticSpan location = {.start_line = node->location.line,
                                       .start_column = node->location.column,
                                       .end_line = node->location.line,
                                       .end_column = node->location.column + 5,
                                       .file_path = strdup("current_file")};

            *result = ai_lint_result_create(
                "AI_SEC_005", AI_LINT_SECURITY, AI_LINT_WARNING,
                "Unsafe pointer dereference detected",
                "Pointer dereferences should be performed in unsafe blocks with proper null checks",
                "Wrap pointer operations in unsafe blocks and add null pointer checks", location,
                0.7f, // Good confidence
                0.7f, // Good impact
                false // Requires manual safety analysis
            );
            return true;
        }
    }

    return false;
}

// Rule: AI_SEC_006 - Detect missing error handling
bool check_missing_error_handling(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result) {
    if (!node || node->type != AST_CALL_EXPR)
        return false;

    ASTNode *func = node->data.call_expr.function;
    if (!func || func->type != AST_IDENTIFIER)
        return false;

    const char *func_name = func->data.identifier.name;

    // Check for functions that typically return Result types
    bool returns_result =
        (strstr(func_name, "parse") != NULL || strstr(func_name, "read") != NULL ||
         strstr(func_name, "write") != NULL || strstr(func_name, "open") != NULL ||
         strstr(func_name, "connect") != NULL || strstr(func_name, "send") != NULL ||
         strstr(func_name, "recv") != NULL);

    if (returns_result) {
        // Check if the result is being handled (simplified check)
        // In practice, would check if this is in a match expression or has error handling
        DiagnosticSpan location = {.start_line = node->location.line,
                                   .start_column = node->location.column,
                                   .end_line = node->location.line,
                                   .end_column = node->location.column + (int)strlen(func_name),
                                   .file_path = strdup("current_file")};

        *result = ai_lint_result_create(
            "AI_SEC_006", AI_LINT_SECURITY, AI_LINT_WARNING, "Missing error handling detected",
            "Functions that can fail should have proper error handling",
            "Use match expressions or error propagation to handle Result types properly", location,
            0.6f, // Medium confidence
            0.6f, // Medium impact
            false // Requires semantic analysis
        );
        return true;
    }

    return false;
}

// Register all security rules
void register_security_rules(AILinter *linter) {
    if (!linter)
        return;

    // Rule 1: Unsafe function usage
    AILintRule rule1 = {.rule_id = strdup("AI_SEC_001"),
                        .category = AI_LINT_SECURITY,
                        .default_severity = AI_LINT_ERROR,
                        .title = strdup("Unsafe function usage"),
                        .description =
                            strdup("Using unsafe functions can lead to security vulnerabilities"),
                        .ai_guidance = strdup("Replace unsafe functions with safer alternatives"),
                        .check_function = check_unsafe_function_usage,
                        .before_examples = NULL,
                        .after_examples = NULL,
                        .example_count = 0,
                        .impact_score = 0.9f,
                        .auto_fixable = false};
    ai_linter_register_rule(linter, &rule1);

    // Rule 2: Buffer overflow risk
    AILintRule rule2 = {
        .rule_id = strdup("AI_SEC_002"),
        .category = AI_LINT_SECURITY,
        .default_severity = AI_LINT_ERROR,
        .title = strdup("Buffer overflow risk"),
        .description = strdup("Buffer operations without bounds checking are dangerous"),
        .ai_guidance = strdup("Add explicit bounds checking to prevent buffer overflows"),
        .check_function = check_buffer_overflow_risk,
        .before_examples = NULL,
        .after_examples = NULL,
        .example_count = 0,
        .impact_score = 0.9f,
        .auto_fixable = false};
    ai_linter_register_rule(linter, &rule2);

    // Rule 3: Unvalidated user input
    AILintRule rule3 = {.rule_id = strdup("AI_SEC_003"),
                        .category = AI_LINT_SECURITY,
                        .default_severity = AI_LINT_ERROR,
                        .title = strdup("Unvalidated user input"),
                        .description = strdup("User input should be validated before use"),
                        .ai_guidance = strdup("Validate and sanitize all user input"),
                        .check_function = check_unvalidated_user_input,
                        .before_examples = NULL,
                        .after_examples = NULL,
                        .example_count = 0,
                        .impact_score = 0.9f,
                        .auto_fixable = false};
    ai_linter_register_rule(linter, &rule3);

    // Rule 4: Hardcoded secrets
    AILintRule rule4 = {.rule_id = strdup("AI_SEC_004"),
                        .category = AI_LINT_SECURITY,
                        .default_severity = AI_LINT_WARNING,
                        .title = strdup("Hardcoded secrets"),
                        .description = strdup("Hardcoded secrets pose security risks"),
                        .ai_guidance = strdup("Use environment variables or secure configuration"),
                        .check_function = check_hardcoded_secrets,
                        .before_examples = NULL,
                        .after_examples = NULL,
                        .example_count = 0,
                        .impact_score = 0.8f,
                        .auto_fixable = false};
    ai_linter_register_rule(linter, &rule4);

    // Rule 5: Unsafe pointer operations
    AILintRule rule5 = {.rule_id = strdup("AI_SEC_005"),
                        .category = AI_LINT_SECURITY,
                        .default_severity = AI_LINT_WARNING,
                        .title = strdup("Unsafe pointer operations"),
                        .description = strdup("Pointer operations should be in unsafe blocks"),
                        .ai_guidance =
                            strdup("Use unsafe blocks and null checks for pointer operations"),
                        .check_function = check_unsafe_pointer_operations,
                        .before_examples = NULL,
                        .after_examples = NULL,
                        .example_count = 0,
                        .impact_score = 0.7f,
                        .auto_fixable = false};
    ai_linter_register_rule(linter, &rule5);

    // Rule 6: Missing error handling
    AILintRule rule6 = {.rule_id = strdup("AI_SEC_006"),
                        .category = AI_LINT_SECURITY,
                        .default_severity = AI_LINT_WARNING,
                        .title = strdup("Missing error handling"),
                        .description = strdup("Functions that can fail need proper error handling"),
                        .ai_guidance = strdup("Use match expressions for Result types"),
                        .check_function = check_missing_error_handling,
                        .before_examples = NULL,
                        .after_examples = NULL,
                        .example_count = 0,
                        .impact_score = 0.6f,
                        .auto_fixable = false};
    ai_linter_register_rule(linter, &rule6);
}
