#include "ai_lint_core.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Utility function implementations
const char *ai_lint_category_to_string(AILintCategory category) {
    switch (category) {
        case AI_LINT_IDIOM: return "idiom";
        case AI_LINT_PERFORMANCE: return "performance";
        case AI_LINT_SECURITY: return "security";
        case AI_LINT_CONCURRENCY: return "concurrency";
        case AI_LINT_MAINTAINABILITY: return "maintainability";
        default: return "unknown";
    }
}

const char *ai_lint_severity_to_string(AILintSeverity severity) {
    switch (severity) {
        case AI_LINT_SUGGESTION: return "suggestion";
        case AI_LINT_WARNING: return "warning";
        case AI_LINT_ERROR: return "error";
        default: return "unknown";
    }
}

AILintResult *ai_lint_result_create(const char *rule_id, AILintCategory category, 
                                   AILintSeverity severity, const char *title,
                                   const char *description, const char *ai_guidance,
                                   DiagnosticSpan location, float confidence, 
                                   float impact_score, bool auto_fixable) {
    AILintResult *result = malloc(sizeof(AILintResult));
    if (!result) return NULL;
    
    result->rule_id = strdup(rule_id);
    result->category = category;
    result->severity = severity;
    result->title = strdup(title);
    result->description = strdup(description);
    result->ai_guidance = strdup(ai_guidance);
    result->location = location;
    result->code_snippet = NULL; // Set by caller if needed
    result->fix_suggestions = NULL;
    result->fix_count = 0;
    result->confidence = confidence;
    result->impact_score = impact_score;
    result->auto_fixable = auto_fixable;
    result->related_rules = NULL;
    result->related_count = 0;
    result->category_str = strdup(ai_lint_category_to_string(category));
    
    return result;
}

void ai_lint_result_destroy(AILintResult *result) {
    if (!result) return;
    
    free(result->rule_id);
    free(result->title);
    free(result->description);
    free(result->ai_guidance);
    free(result->code_snippet);
    free(result->category_str);
    
    for (size_t i = 0; i < result->fix_count; i++) {
        free(result->fix_suggestions[i]);
    }
    free(result->fix_suggestions);
    
    for (size_t i = 0; i < result->related_count; i++) {
        free(result->related_rules[i]);
    }
    free(result->related_rules);
    
    free(result);
}

void ai_lint_results_destroy(AILintResult **results, size_t count) {
    if (!results) return;
    
    for (size_t i = 0; i < count; i++) {
        ai_lint_result_destroy(results[i]);
    }
    free(results);
}

AILinter *ai_linter_create(AsthraSemanticsAPI *semantic_api) {
    // Allow NULL semantic_api for testing purposes
    AILinter *linter = malloc(sizeof(AILinter));
    if (!linter) return NULL;
    
    linter->rules = malloc(sizeof(AILintRule) * 32); // Initial capacity
    if (!linter->rules) {
        free(linter);
        return NULL;
    }
    
    linter->rule_count = 0;
    linter->capacity = 32;
    linter->semantic_api = semantic_api; // Can be NULL for testing
    
    // Enable all categories by default
    for (int i = 0; i < 5; i++) {
        linter->enabled_categories[i] = true;
    }
    
    return linter;
}

void ai_linter_destroy(AILinter *linter) {
    if (!linter) return;
    
    // Free all registered rules
    for (size_t i = 0; i < linter->rule_count; i++) {
        free(linter->rules[i].rule_id);
        free(linter->rules[i].title);
        free(linter->rules[i].description);
        free(linter->rules[i].ai_guidance);
        
        for (size_t j = 0; j < linter->rules[i].example_count; j++) {
            free(linter->rules[i].before_examples[j]);
            free(linter->rules[i].after_examples[j]);
        }
        free(linter->rules[i].before_examples);
        free(linter->rules[i].after_examples);
    }
    
    free(linter->rules);
    free(linter);
}

bool ai_linter_register_rule(AILinter *linter, const AILintRule *rule) {
    if (!linter || !rule) return false;
    
    // Expand capacity if needed
    if (linter->rule_count >= linter->capacity) {
        size_t new_capacity = linter->capacity * 2;
        AILintRule *new_rules = realloc(linter->rules, sizeof(AILintRule) * new_capacity);
        if (!new_rules) return false;
        
        linter->rules = new_rules;
        linter->capacity = new_capacity;
    }
    
    // Deep copy the rule
    AILintRule *new_rule = &linter->rules[linter->rule_count];
    new_rule->rule_id = strdup(rule->rule_id);
    new_rule->category = rule->category;
    new_rule->default_severity = rule->default_severity;
    new_rule->title = strdup(rule->title);
    new_rule->description = strdup(rule->description);
    new_rule->ai_guidance = strdup(rule->ai_guidance);
    new_rule->check_function = rule->check_function;
    new_rule->impact_score = rule->impact_score;
    new_rule->auto_fixable = rule->auto_fixable;
    
    // Copy examples
    new_rule->example_count = rule->example_count;
    if (rule->example_count > 0) {
        new_rule->before_examples = malloc(sizeof(char*) * rule->example_count);
        new_rule->after_examples = malloc(sizeof(char*) * rule->example_count);
        
        for (size_t i = 0; i < rule->example_count; i++) {
            new_rule->before_examples[i] = strdup(rule->before_examples[i]);
            new_rule->after_examples[i] = strdup(rule->after_examples[i]);
        }
    } else {
        new_rule->before_examples = NULL;
        new_rule->after_examples = NULL;
    }
    
    linter->rule_count++;
    return true;
}

void ai_linter_enable_category(AILinter *linter, AILintCategory category, bool enabled) {
    if (!linter || category < 0 || category >= 5) return;
    linter->enabled_categories[category] = enabled;
}

// Helper function to traverse AST and apply rules
static void analyze_ast_node(AILinter *linter, ASTNode *node, AILintResult ***results, 
                            size_t *result_count, size_t *result_capacity) {
    if (!node || !linter) return;
    
    // Apply all enabled rules to this node
    for (size_t i = 0; i < linter->rule_count; i++) {
        AILintRule *rule = &linter->rules[i];
        
        // Skip if category is disabled
        if (!linter->enabled_categories[rule->category]) continue;
        
        AILintResult *result = NULL;
        if (rule->check_function(node, linter->semantic_api, &result)) {
            // Expand results array if needed
            if (*result_count >= *result_capacity) {
                size_t new_capacity = (*result_capacity == 0) ? 8 : (*result_capacity * 2);
                AILintResult **new_results = realloc(*results, sizeof(AILintResult*) * new_capacity);
                if (!new_results) {
                    ai_lint_result_destroy(result);
                    continue;
                }
                *results = new_results;
                *result_capacity = new_capacity;
            }
            
            (*results)[*result_count] = result;
            (*result_count)++;
        }
    }
    
    // Recursively analyze child nodes
    // This is a simplified traversal - in practice, you'd use the AST structure
    // For now, we'll implement basic traversal for common node types
    switch (node->type) {
        case AST_PROGRAM:
            if (node->data.program.declarations) {
                for (size_t i = 0; i < node->data.program.declarations->count; i++) {
                    analyze_ast_node(linter, node->data.program.declarations->nodes[i], 
                                   results, result_count, result_capacity);
                }
            }
            break;
        case AST_FUNCTION_DECL:
            if (node->data.function_decl.body) {
                analyze_ast_node(linter, node->data.function_decl.body, 
                               results, result_count, result_capacity);
            }
            break;
        case AST_BLOCK:
            if (node->data.block.statements) {
                for (size_t i = 0; i < node->data.block.statements->count; i++) {
                    analyze_ast_node(linter, node->data.block.statements->nodes[i], 
                                   results, result_count, result_capacity);
                }
            }
            break;
        // Add more node types as needed
        default:
            break;
    }
}

AILintResult **ai_linter_analyze_ast(AILinter *linter, ASTNode *root, size_t *result_count) {
    if (!linter || !root || !result_count) return NULL;
    
    AILintResult **results = NULL;
    size_t capacity = 0;
    *result_count = 0;
    
    analyze_ast_node(linter, root, &results, result_count, &capacity);
    
    return results;
}

AILintResult **ai_linter_analyze_code(AILinter *linter, const char *code, size_t *result_count) {
    if (!linter || !code || !result_count) return NULL;
    
    // This would need to parse the code first
    // For now, return empty results as a placeholder
    *result_count = 0;
    return NULL;
}

AILintResult **ai_linter_analyze_file(AILinter *linter, const char *file_path, size_t *result_count) {
    if (!linter || !file_path || !result_count) return NULL;
    
    // This would need to read and parse the file first
    // For now, return empty results as a placeholder
    *result_count = 0;
    return NULL;
}
