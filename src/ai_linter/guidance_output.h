#ifndef ASTHRA_AI_GUIDANCE_OUTPUT_H
#define ASTHRA_AI_GUIDANCE_OUTPUT_H

#include "ai_lint_core.h"
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct AILintGuidanceIssue AILintGuidanceIssue;
typedef struct AILintStatistics AILintStatistics;
typedef struct AILintGuidanceReport AILintGuidanceReport;

// Statistics structure
struct AILintStatistics {
    size_t total_issues;
    size_t by_category[5];        // One for each AILintCategory
    size_t by_severity[3];        // One for each AILintSeverity
    size_t auto_fixable_count;
    size_t high_impact_count;     // Issues with impact_score >= 0.7
    float average_confidence;
    float average_impact;
};

// Guidance issue structure
struct AILintGuidanceIssue {
    char *rule_id;
    char *category;
    char *severity;
    char *title;
    char *description;
    char *ai_guidance;
    DiagnosticSpan location;
    char *code_snippet;
    char **fix_suggestions;
    size_t fix_count;
    float confidence;
    float impact_score;
    bool auto_fixable;
    char **related_rules;
    size_t related_count;
};

// Guidance report structure
struct AILintGuidanceReport {
    char *version;                 // Schema version
    char *file_path;              // Source file
    size_t total_issues;          // Total number of issues
    size_t auto_fixable_count;    // Number of auto-fixable issues
    AILintGuidanceIssue *issues;  // Array of issues
    AILintStatistics stats;       // Summary statistics
};

// JSON output generation functions
char *ai_linter_generate_guidance_json(AILintResult **results, size_t count);
char *ai_linter_generate_rule_catalog_json(AILinter *linter);
char *ai_linter_generate_fix_suggestions_json(AILintResult **results, size_t count);

// Helper functions for JSON generation
char *escape_json_string(const char *str);
char *format_source_location(const DiagnosticSpan *location);
AILintStatistics calculate_statistics(AILintResult **results, size_t count);

// Memory management
void ai_lint_guidance_report_destroy(AILintGuidanceReport *report);
void ai_lint_guidance_issue_destroy(AILintGuidanceIssue *issue);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_AI_GUIDANCE_OUTPUT_H 
