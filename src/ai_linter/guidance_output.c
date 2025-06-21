#include "guidance_output.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

// Helper function to escape JSON strings
char *escape_json_string(const char *str) {
    if (!str) return strdup("null");
    
    size_t len = strlen(str);
    size_t escaped_len = len * 2 + 3; // Worst case: every char escaped + quotes + null
    char *escaped = malloc(escaped_len);
    if (!escaped) return NULL;
    
    char *dst = escaped;
    *dst++ = '"';
    
    for (const char *src = str; *src; src++) {
        switch (*src) {
            case '"':  *dst++ = '\\'; *dst++ = '"'; break;
            case '\\': *dst++ = '\\'; *dst++ = '\\'; break;
            case '\b': *dst++ = '\\'; *dst++ = 'b'; break;
            case '\f': *dst++ = '\\'; *dst++ = 'f'; break;
            case '\n': *dst++ = '\\'; *dst++ = 'n'; break;
            case '\r': *dst++ = '\\'; *dst++ = 'r'; break;
            case '\t': *dst++ = '\\'; *dst++ = 't'; break;
            default:   *dst++ = *src; break;
        }
    }
    
    *dst++ = '"';
    *dst = '\0';
    
    return escaped;
}

// Format source location as JSON object
char *format_source_location(const DiagnosticSpan *location) {
    if (!location) return strdup("null");
    
    char *file_path = escape_json_string(location->file_path);
    char *result = malloc(256);
    if (!result) {
        free(file_path);
        return NULL;
    }
    
    snprintf(result, 256,
        "{"
        "\"file\":%s,"
        "\"start_line\":%zu,"
        "\"start_column\":%zu,"
        "\"end_line\":%zu,"
        "\"end_column\":%zu"
        "}",
        file_path,
        location->start_line,
        location->start_column,
        location->end_line,
        location->end_column
    );
    
    free(file_path);
    return result;
}

// Calculate statistics from lint results
AILintStatistics calculate_statistics(AILintResult **results, size_t count) {
    AILintStatistics stats = {0};
    
    if (!results || count == 0) return stats;
    
    stats.total_issues = count;
    float total_confidence = 0.0f;
    float total_impact = 0.0f;
    
    for (size_t i = 0; i < count; i++) {
        AILintResult *result = results[i];
        if (!result) continue;
        
        // Count by category
        if (result->category < 5) {
            stats.by_category[result->category]++;
        }
        
        // Count by severity
        if (result->severity < 3) {
            stats.by_severity[result->severity]++;
        }
        
        // Count auto-fixable
        if (result->auto_fixable) {
            stats.auto_fixable_count++;
        }
        
        // Count high impact
        if (result->impact_score >= 0.7f) {
            stats.high_impact_count++;
        }
        
        // Accumulate for averages
        total_confidence += result->confidence;
        total_impact += result->impact_score;
    }
    
    // Calculate averages
    stats.average_confidence = total_confidence / (float)count;
    stats.average_impact = total_impact / (float)count;
    
    return stats;
}

// Generate comprehensive guidance JSON
char *ai_linter_generate_guidance_json(AILintResult **results, size_t count) {
    if (!results) return strdup("{\"error\":\"No results provided\"}");
    
    // Calculate buffer size estimate
    size_t buffer_size = 1024 + (count * 512); // Base + estimated per result
    char *json = malloc(buffer_size);
    if (!json) return NULL;
    
    char *ptr = json;
    size_t remaining = buffer_size;
    
    // Start JSON object
    int written = snprintf(ptr, remaining,
        "{"
        "\"schema_version\":\"1.0\","
        "\"timestamp\":\"%ld\","
        "\"total_issues\":%zu,"
        "\"issues\":[",
        time(NULL), count
    );
    
    if (written < 0 || (size_t)written >= remaining) {
        free(json);
        return NULL;
    }
    ptr += written;
    remaining -= (size_t)written;
    
    // Add each issue
    for (size_t i = 0; i < count; i++) {
        AILintResult *result = results[i];
        if (!result) continue;
        
        char *rule_id = escape_json_string(result->rule_id);
        char *category = escape_json_string(ai_lint_category_to_string(result->category));
        char *severity = escape_json_string(ai_lint_severity_to_string(result->severity));
        char *title = escape_json_string(result->title);
        char *description = escape_json_string(result->description);
        char *ai_guidance = escape_json_string(result->ai_guidance);
        char *location = format_source_location(&result->location);
        
        written = snprintf(ptr, remaining,
            "%s{"
            "\"rule_id\":%s,"
            "\"category\":%s,"
            "\"severity\":%s,"
            "\"title\":%s,"
            "\"description\":%s,"
            "\"ai_guidance\":%s,"
            "\"location\":%s,"
            "\"confidence\":%.2f,"
            "\"impact_score\":%.2f,"
            "\"auto_fixable\":%s"
            "}",
            (i > 0) ? "," : "",
            rule_id, category, severity, title, description, ai_guidance, location,
            result->confidence, result->impact_score,
            result->auto_fixable ? "true" : "false"
        );
        
        free(rule_id);
        free(category);
        free(severity);
        free(title);
        free(description);
        free(ai_guidance);
        free(location);
        
        if (written < 0 || (size_t)written >= remaining) {
            // Buffer too small, reallocate
            size_t used = (size_t)(ptr - json);
            buffer_size *= 2;
            char *new_json = realloc(json, buffer_size);
            if (!new_json) {
                free(json);
                return NULL;
            }
            json = new_json;
            ptr = json + used;
            remaining = buffer_size - used;
            i--; // Retry this iteration
            continue;
        }
        
        ptr += written;
        remaining -= (size_t)written;
    }
    
    // Add statistics
    AILintStatistics stats = calculate_statistics(results, count);
    written = snprintf(ptr, remaining,
        "],"
        "\"statistics\":{"
        "\"total_issues\":%zu,"
        "\"auto_fixable_count\":%zu,"
        "\"high_impact_count\":%zu,"
        "\"average_confidence\":%.2f,"
        "\"average_impact\":%.2f,"
        "\"by_category\":{"
        "\"idiom\":%zu,"
        "\"performance\":%zu,"
        "\"security\":%zu,"
        "\"concurrency\":%zu,"
        "\"maintainability\":%zu"
        "},"
        "\"by_severity\":{"
        "\"suggestion\":%zu,"
        "\"warning\":%zu,"
        "\"error\":%zu"
        "}"
        "}"
        "}",
        stats.total_issues,
        stats.auto_fixable_count,
        stats.high_impact_count,
        stats.average_confidence,
        stats.average_impact,
        stats.by_category[0], stats.by_category[1], stats.by_category[2],
        stats.by_category[3], stats.by_category[4],
        stats.by_severity[0], stats.by_severity[1], stats.by_severity[2]
    );
    
    if (written < 0 || (size_t)written >= remaining) {
        free(json);
        return NULL;
    }
    
    return json;
}

// Generate rule catalog JSON for AI tools
char *ai_linter_generate_rule_catalog_json(AILinter *linter) {
    if (!linter) return strdup("{\"error\":\"No linter provided\"}");
    
    size_t buffer_size = 2048 + (linter->rule_count * 256);
    char *json = malloc(buffer_size);
    if (!json) return NULL;
    
    char *ptr = json;
    size_t remaining = buffer_size;
    
    int written = snprintf(ptr, remaining,
        "{"
        "\"schema_version\":\"1.0\","
        "\"rule_count\":%zu,"
        "\"rules\":[",
        linter->rule_count
    );
    
    if (written < 0 || (size_t)written >= remaining) {
        free(json);
        return NULL;
    }
    ptr += written;
    remaining -= (size_t)written;
    
    for (size_t i = 0; i < linter->rule_count; i++) {
        AILintRule *rule = &linter->rules[i];
        
        char *rule_id = escape_json_string(rule->rule_id);
        char *category = escape_json_string(ai_lint_category_to_string(rule->category));
        char *severity = escape_json_string(ai_lint_severity_to_string(rule->default_severity));
        char *title = escape_json_string(rule->title);
        char *description = escape_json_string(rule->description);
        char *ai_guidance = escape_json_string(rule->ai_guidance);
        
        written = snprintf(ptr, remaining,
            "%s{"
            "\"rule_id\":%s,"
            "\"category\":%s,"
            "\"default_severity\":%s,"
            "\"title\":%s,"
            "\"description\":%s,"
            "\"ai_guidance\":%s,"
            "\"impact_score\":%.2f,"
            "\"auto_fixable\":%s"
            "}",
            (i > 0) ? "," : "",
            rule_id, category, severity, title, description, ai_guidance,
            rule->impact_score,
            rule->auto_fixable ? "true" : "false"
        );
        
        free(rule_id);
        free(category);
        free(severity);
        free(title);
        free(description);
        free(ai_guidance);
        
        if (written < 0 || (size_t)written >= remaining) {
            free(json);
            return NULL;
        }
        
        ptr += written;
        remaining -= (size_t)written;
    }
    
    written = snprintf(ptr, remaining, "]}");
    if (written < 0 || (size_t)written >= remaining) {
        free(json);
        return NULL;
    }
    
    return json;
}

// Generate fix suggestions JSON
char *ai_linter_generate_fix_suggestions_json(AILintResult **results, size_t count) {
    if (!results) return strdup("{\"error\":\"No results provided\"}");
    
    size_t buffer_size = 1024 + (count * 256);
    char *json = malloc(buffer_size);
    if (!json) return NULL;
    
    char *ptr = json;
    size_t remaining = buffer_size;
    
    int written = snprintf(ptr, remaining,
        "{"
        "\"schema_version\":\"1.0\","
        "\"fix_suggestions\":["
    );
    
    if (written < 0 || (size_t)written >= remaining) {
        free(json);
        return NULL;
    }
    ptr += written;
    remaining -= (size_t)written;
    
    bool first = true;
    for (size_t i = 0; i < count; i++) {
        AILintResult *result = results[i];
        if (!result || !result->auto_fixable) continue;
        
        char *rule_id = escape_json_string(result->rule_id);
        char *ai_guidance = escape_json_string(result->ai_guidance);
        char *location = format_source_location(&result->location);
        
        written = snprintf(ptr, remaining,
            "%s{"
            "\"rule_id\":%s,"
            "\"ai_guidance\":%s,"
            "\"location\":%s,"
            "\"confidence\":%.2f"
            "}",
            first ? "" : ",",
            rule_id, ai_guidance, location,
            result->confidence
        );
        
        free(rule_id);
        free(ai_guidance);
        free(location);
        
        if (written < 0 || (size_t)written >= remaining) {
            free(json);
            return NULL;
        }
        
        ptr += written;
        remaining -= (size_t)written;
        first = false;
    }
    
    written = snprintf(ptr, remaining, "]}");
    if (written < 0 || (size_t)written >= remaining) {
        free(json);
        return NULL;
    }
    
    return json;
}

// Memory management functions
void ai_lint_guidance_report_destroy(AILintGuidanceReport *report) {
    if (!report) return;
    
    free(report->version);
    free(report->file_path);
    
    if (report->issues) {
        for (size_t i = 0; i < report->total_issues; i++) {
            ai_lint_guidance_issue_destroy(&report->issues[i]);
        }
        free(report->issues);
    }
    
    free(report);
}

void ai_lint_guidance_issue_destroy(AILintGuidanceIssue *issue) {
    if (!issue) return;
    
    free(issue->rule_id);
    free(issue->category);
    free(issue->severity);
    free(issue->title);
    free(issue->description);
    free(issue->ai_guidance);
    free(issue->code_snippet);
    
    if (issue->fix_suggestions) {
        for (size_t i = 0; i < issue->fix_count; i++) {
            free(issue->fix_suggestions[i]);
        }
        free(issue->fix_suggestions);
    }
    
    if (issue->related_rules) {
        for (size_t i = 0; i < issue->related_count; i++) {
            free(issue->related_rules[i]);
        }
        free(issue->related_rules);
    }
} 
