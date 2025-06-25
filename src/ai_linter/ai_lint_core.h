#ifndef ASTHRA_AI_LINT_CORE_H
#define ASTHRA_AI_LINT_CORE_H

#include "../ai_api/semantic_api_core.h"
#include "../diagnostics/enhanced_diagnostics.h"
#include "../parser/ast.h"

typedef enum {
    AI_LINT_SUGGESTION, // Improvement suggestion
    AI_LINT_WARNING,    // Potential issue
    AI_LINT_ERROR       // Definite problem
} AILintSeverity;

typedef enum {
    AI_LINT_IDIOM,          // Language idioms and conventions
    AI_LINT_PERFORMANCE,    // Performance optimizations
    AI_LINT_SECURITY,       // Security best practices
    AI_LINT_CONCURRENCY,    // Concurrency patterns
    AI_LINT_MAINTAINABILITY // Code maintainability
} AILintCategory;

typedef struct AILintResult {
    char *rule_id;           // "AI_IDIOM_001", "AI_PERF_002"
    AILintCategory category; // Category classification
    AILintSeverity severity; // Severity level
    char *title;             // Short description
    char *description;       // Detailed description
    char *ai_guidance;       // Specific guidance for AI models
    DiagnosticSpan location; // Where the issue occurs
    char *code_snippet;      // Code that triggered the issue
    char **fix_suggestions;  // Array of fix suggestions
    size_t fix_count;        // Number of fix suggestions
    float confidence;        // Confidence score (0.0-1.0)
    float impact_score;      // Impact score (0.0-1.0)
    bool auto_fixable;       // Can be automatically applied
    char **related_rules;    // Related rule IDs
    size_t related_count;    // Number of related rules
    char *category_str;      // String representation of category
} AILintResult;

typedef struct {
    char *rule_id;
    AILintCategory category;
    AILintSeverity default_severity;
    char *title;
    char *description;
    char *ai_guidance;
    bool (*check_function)(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result);
    char **before_examples;
    char **after_examples;
    size_t example_count;
    float impact_score;
    bool auto_fixable;
} AILintRule;

typedef struct {
    AILintRule *rules;
    size_t rule_count;
    size_t capacity;
    AsthraSemanticsAPI *semantic_api;
    bool enabled_categories[5]; // One for each AILintCategory
} AILinter;

// Core API functions
AILinter *ai_linter_create(AsthraSemanticsAPI *semantic_api);
void ai_linter_destroy(AILinter *linter);
bool ai_linter_register_rule(AILinter *linter, const AILintRule *rule);
void ai_linter_enable_category(AILinter *linter, AILintCategory category, bool enabled);

// Analysis functions
AILintResult **ai_linter_analyze_file(AILinter *linter, const char *file_path,
                                      size_t *result_count);
AILintResult **ai_linter_analyze_code(AILinter *linter, const char *code, size_t *result_count);
AILintResult **ai_linter_analyze_ast(AILinter *linter, ASTNode *root, size_t *result_count);

// Result management
void ai_lint_result_destroy(AILintResult *result);
void ai_lint_results_destroy(AILintResult **results, size_t count);

// Utility functions
const char *ai_lint_category_to_string(AILintCategory category);
const char *ai_lint_severity_to_string(AILintSeverity severity);
AILintResult *ai_lint_result_create(const char *rule_id, AILintCategory category,
                                    AILintSeverity severity, const char *title,
                                    const char *description, const char *ai_guidance,
                                    DiagnosticSpan location, float confidence, float impact_score,
                                    bool auto_fixable);

#endif
