#include "../ai_lint_core.h"
#include "../../analysis/semantic_analyzer.h"
#include <string.h>
#include <stdio.h>

// Helper function to check if a variable is shared between threads
static bool is_shared_variable(const char *var_name, AsthraSemanticsAPI *api) {
    if (!var_name || !api) return false;
    
    // This would use semantic analysis to determine if a variable is shared
    // For now, use heuristics based on naming patterns
    return (strstr(var_name, "shared") != NULL ||
            strstr(var_name, "global") != NULL ||
            strstr(var_name, "counter") != NULL ||
            strstr(var_name, "state") != NULL ||
            strstr(var_name, "cache") != NULL);
}

// Helper function to check if an operation is atomic
static bool is_atomic_operation(ASTNode *node) {
    if (!node) return false;
    
    // Check for atomic operations or synchronization primitives
    if (node->type == AST_CALL_EXPR) {
        ASTNode *func = node->data.call_expr.function;
        if (func && func->type == AST_IDENTIFIER) {
            const char *func_name = func->data.identifier.name;
            return (strstr(func_name, "atomic") != NULL ||
                    strcmp(func_name, "lock") == 0 ||
                    strcmp(func_name, "unlock") == 0 ||
                    strcmp(func_name, "acquire") == 0 ||
                    strcmp(func_name, "release") == 0);
        }
    }
    
    return false;
}

// Helper function to check if code is in a synchronized block
static bool is_in_synchronized_block(ASTNode *node) {
    // This would traverse up the AST to check for synchronization
    // For now, simplified check
    (void)node;
    return false; // Placeholder
}

// Helper function to detect data race patterns
static bool has_data_race_pattern(ASTNode *node) {
    if (!node) return false;
    
    // Look for unsynchronized access to shared variables
    if (node->type == AST_ASSIGNMENT) {
        ASTNode *lvalue = node->data.assignment.target;
        if (lvalue && lvalue->type == AST_IDENTIFIER) {
            const char *var_name = lvalue->data.identifier.name;
            if (is_shared_variable(var_name, NULL) && 
                !is_in_synchronized_block(node) &&
                !is_atomic_operation(node)) {
                return true;
            }
        }
    }
    
    return false;
}

// Rule: AI_CONC_001 - Detect potential data races
bool check_data_race_risk(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result) {
    if (!node || node->type != AST_ASSIGNMENT) return false;
    
    if (has_data_race_pattern(node)) {
        ASTNode *lvalue = node->data.assignment.target;
        const char *var_name = (lvalue && lvalue->type == AST_IDENTIFIER) ? 
                               lvalue->data.identifier.name : "unknown";
        
        DiagnosticSpan location = {
            .start_line = node->location.line,
            .start_column = node->location.column,
            .end_line = node->location.line,
            .end_column = node->location.column + (int)strlen(var_name),
            .file_path = strdup("current_file")
        };
        
        char guidance[256];
        snprintf(guidance, sizeof(guidance),
                "Variable '%s' appears to be shared but accessed without synchronization",
                var_name);
        
        *result = ai_lint_result_create(
            "AI_CONC_001",
            AI_LINT_CONCURRENCY,
            AI_LINT_ERROR,
            "Potential data race detected",
            "Unsynchronized access to shared variables can cause data races",
            guidance,
            location,
            0.7f, // Good confidence
            0.9f, // Critical impact
            false // Requires manual synchronization
        );
        return true;
    }
    
    return false;
}

// Rule: AI_CONC_002 - Detect deadlock potential
bool check_deadlock_potential(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result) {
    if (!node || node->type != AST_CALL_EXPR) return false;
    
    ASTNode *func = node->data.call_expr.function;
    if (!func || func->type != AST_IDENTIFIER) return false;
    
    const char *func_name = func->data.identifier.name;
    
    // Check for nested lock acquisitions
    if (strcmp(func_name, "lock") == 0 || strcmp(func_name, "acquire") == 0) {
        // This is a simplified check - would need to analyze lock ordering
        DiagnosticSpan location = {
            .start_line = node->location.line,
            .start_column = node->location.column,
            .end_line = node->location.line,
            .end_column = node->location.column + (int)strlen(func_name),
            .file_path = strdup("current_file")
        };
        
        *result = ai_lint_result_create(
            "AI_CONC_002",
            AI_LINT_CONCURRENCY,
            AI_LINT_WARNING,
            "Potential deadlock risk detected",
            "Multiple lock acquisitions can lead to deadlocks if not ordered consistently",
            "Ensure consistent lock ordering across all threads to prevent deadlocks",
            location,
            0.5f, // Medium confidence (needs more analysis)
            0.8f, // High impact
            false // Requires manual analysis
        );
        return true;
    }
    
    return false;
}

// Rule: AI_CONC_003 - Detect improper spawn usage
bool check_improper_spawn_usage(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result) {
    if (!node || node->type != AST_SPAWN_STMT) return false;
    
    // Check for spawn without proper error handling or resource management
    // spawn_stmt only has function_name and args, no expression field
    if (!node->data.spawn_stmt.function_name) return false;
    
    // Check if spawned task has proper error handling
    // This is a simplified check - would need full analysis
    DiagnosticSpan location = {
        .start_line = node->location.line,
        .start_column = node->location.column,
        .end_line = node->location.line,
        .end_column = node->location.column + 5, // "spawn"
        .file_path = strdup("current_file")
    };
    
    *result = ai_lint_result_create(
        "AI_CONC_003",
        AI_LINT_CONCURRENCY,
        AI_LINT_SUGGESTION,
        "Spawn without error handling",
        "Spawned tasks should have proper error handling and resource cleanup",
        "Consider using spawn_with_handle for better error handling and task management",
        location,
        0.6f, // Medium confidence
        0.6f, // Medium impact
        false // Requires manual review
    );
    return true;
}

// Rule: AI_CONC_004 - Detect channel misuse
bool check_channel_misuse(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result) {
    if (!node || node->type != AST_CALL_EXPR) return false;
    
    ASTNode *func = node->data.call_expr.function;
    if (!func || func->type != AST_IDENTIFIER) return false;
    
    const char *func_name = func->data.identifier.name;
    
    // Check for potential channel deadlocks or misuse
    if (strcmp(func_name, "send") == 0 || strcmp(func_name, "recv") == 0) {
        // Check if this is in a loop without proper timeout or select
        // This is a simplified check
        DiagnosticSpan location = {
            .start_line = node->location.line,
            .start_column = node->location.column,
            .end_line = node->location.line,
            .end_column = node->location.column + (int)strlen(func_name),
            .file_path = strdup("current_file")
        };
        
        *result = ai_lint_result_create(
            "AI_CONC_004",
            AI_LINT_CONCURRENCY,
            AI_LINT_WARNING,
            "Potential channel deadlock",
            "Channel operations without timeouts can cause deadlocks",
            "Use select statements with timeouts for robust channel operations",
            location,
            0.6f, // Medium confidence
            0.7f, // Good impact
            false // Requires manual review
        );
        return true;
    }
    
    return false;
}

// Rule: AI_CONC_005 - Detect missing synchronization
bool check_missing_synchronization(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result) {
    if (!node) return false;
    
    // Check for shared data access without synchronization
    if (node->type == AST_BINARY_EXPR || node->type == AST_ASSIGNMENT) {
        // Look for operations on shared variables
        ASTNode *target = NULL;
        if (node->type == AST_ASSIGNMENT) {
            target = node->data.assignment.target;
        } else if (node->type == AST_BINARY_EXPR) {
            target = node->data.binary_expr.left;
        }
        
        if (target && target->type == AST_IDENTIFIER) {
            const char *var_name = target->data.identifier.name;
            if (is_shared_variable(var_name, api) && !is_in_synchronized_block(node)) {
                DiagnosticSpan location = {
                    .start_line = node->location.line,
                    .start_column = node->location.column,
                    .end_line = node->location.line,
                    .end_column = node->location.column + (int)strlen(var_name),
                    .file_path = strdup("current_file")
                };
                
                *result = ai_lint_result_create(
                    "AI_CONC_005",
                    AI_LINT_CONCURRENCY,
                    AI_LINT_WARNING,
                    "Missing synchronization detected",
                    "Shared variable access should be synchronized to prevent race conditions",
                    "Use mutexes, atomic operations, or channels to synchronize access",
                    location,
                    0.7f, // Good confidence
                    0.8f, // High impact
                    false // Requires manual synchronization
                );
                return true;
            }
        }
    }
    
    return false;
}

// Rule: AI_CONC_006 - Detect blocking operations in async context
bool check_blocking_in_async(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result) {
    if (!node || node->type != AST_CALL_EXPR) return false;
    
    ASTNode *func = node->data.call_expr.function;
    if (!func || func->type != AST_IDENTIFIER) return false;
    
    const char *func_name = func->data.identifier.name;
    
    // Check for blocking operations that shouldn't be in async contexts
    bool is_blocking = (strcmp(func_name, "sleep") == 0 ||
                       strcmp(func_name, "wait") == 0 ||
                       strcmp(func_name, "join") == 0 ||
                       strstr(func_name, "blocking") != NULL);
    
    if (is_blocking) {
        // Check if we're in an async context (simplified)
        // Would need to traverse up to find async function
        DiagnosticSpan location = {
            .start_line = node->location.line,
            .start_column = node->location.column,
            .end_line = node->location.line,
            .end_column = node->location.column + (int)strlen(func_name),
            .file_path = strdup("current_file")
        };
        
        *result = ai_lint_result_create(
            "AI_CONC_006",
            AI_LINT_CONCURRENCY,
            AI_LINT_WARNING,
            "Blocking operation in async context",
            "Blocking operations can prevent other tasks from running",
            "Use async alternatives or move blocking operations to separate threads",
            location,
            0.6f, // Medium confidence
            0.7f, // Good impact
            false // Requires manual refactoring
        );
        return true;
    }
    
    return false;
}

// Register all concurrency rules
void register_concurrency_rules(AILinter *linter) {
    if (!linter) return;
    
    // Rule 1: Data race detection
    AILintRule rule1 = {
        .rule_id = strdup("AI_CONC_001"),
        .category = AI_LINT_CONCURRENCY,
        .default_severity = AI_LINT_ERROR,
        .title = strdup("Potential data race"),
        .description = strdup("Unsynchronized access to shared variables"),
        .ai_guidance = strdup("Use synchronization primitives for shared variable access"),
        .check_function = check_data_race_risk,
        .before_examples = NULL,
        .after_examples = NULL,
        .example_count = 0,
        .impact_score = 0.9f,
        .auto_fixable = false
    };
    ai_linter_register_rule(linter, &rule1);
    
    // Rule 2: Deadlock potential
    AILintRule rule2 = {
        .rule_id = strdup("AI_CONC_002"),
        .category = AI_LINT_CONCURRENCY,
        .default_severity = AI_LINT_WARNING,
        .title = strdup("Deadlock potential"),
        .description = strdup("Multiple lock acquisitions can cause deadlocks"),
        .ai_guidance = strdup("Ensure consistent lock ordering to prevent deadlocks"),
        .check_function = check_deadlock_potential,
        .before_examples = NULL,
        .after_examples = NULL,
        .example_count = 0,
        .impact_score = 0.8f,
        .auto_fixable = false
    };
    ai_linter_register_rule(linter, &rule2);
    
    // Rule 3: Improper spawn usage
    AILintRule rule3 = {
        .rule_id = strdup("AI_CONC_003"),
        .category = AI_LINT_CONCURRENCY,
        .default_severity = AI_LINT_SUGGESTION,
        .title = strdup("Improper spawn usage"),
        .description = strdup("Spawned tasks need proper error handling"),
        .ai_guidance = strdup("Use spawn_with_handle for better task management"),
        .check_function = check_improper_spawn_usage,
        .before_examples = NULL,
        .after_examples = NULL,
        .example_count = 0,
        .impact_score = 0.6f,
        .auto_fixable = false
    };
    ai_linter_register_rule(linter, &rule3);
    
    // Rule 4: Channel misuse
    AILintRule rule4 = {
        .rule_id = strdup("AI_CONC_004"),
        .category = AI_LINT_CONCURRENCY,
        .default_severity = AI_LINT_WARNING,
        .title = strdup("Channel misuse"),
        .description = strdup("Channel operations without timeouts can deadlock"),
        .ai_guidance = strdup("Use select statements with timeouts"),
        .check_function = check_channel_misuse,
        .before_examples = NULL,
        .after_examples = NULL,
        .example_count = 0,
        .impact_score = 0.7f,
        .auto_fixable = false
    };
    ai_linter_register_rule(linter, &rule4);
    
    // Rule 5: Missing synchronization
    AILintRule rule5 = {
        .rule_id = strdup("AI_CONC_005"),
        .category = AI_LINT_CONCURRENCY,
        .default_severity = AI_LINT_WARNING,
        .title = strdup("Missing synchronization"),
        .description = strdup("Shared variables need synchronization"),
        .ai_guidance = strdup("Use mutexes, atomics, or channels for synchronization"),
        .check_function = check_missing_synchronization,
        .before_examples = NULL,
        .after_examples = NULL,
        .example_count = 0,
        .impact_score = 0.8f,
        .auto_fixable = false
    };
    ai_linter_register_rule(linter, &rule5);
    
    // Rule 6: Blocking in async context
    AILintRule rule6 = {
        .rule_id = strdup("AI_CONC_006"),
        .category = AI_LINT_CONCURRENCY,
        .default_severity = AI_LINT_WARNING,
        .title = strdup("Blocking in async context"),
        .description = strdup("Blocking operations prevent task scheduling"),
        .ai_guidance = strdup("Use async alternatives for non-blocking operations"),
        .check_function = check_blocking_in_async,
        .before_examples = NULL,
        .after_examples = NULL,
        .example_count = 0,
        .impact_score = 0.7f,
        .auto_fixable = false
    };
    ai_linter_register_rule(linter, &rule6);
} 
