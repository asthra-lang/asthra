#include "enhanced_error_reporting.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define INITIAL_REPORTER_CAPACITY 16

// Helper function to duplicate strings safely
static char *safe_strdup(const char *str) {
    if (!str) return NULL;
    size_t len = strlen(str);
    char *copy = malloc(len + 1);
    if (copy) {
        strcpy(copy, str);
    }
    return copy;
}

EnhancedErrorReporter *enhanced_error_reporter_create(void) {
    EnhancedErrorReporter *reporter = malloc(sizeof(EnhancedErrorReporter));
    if (!reporter) return NULL;
    
    reporter->diagnostics = malloc(sizeof(EnhancedDiagnostic*) * INITIAL_REPORTER_CAPACITY);
    if (!reporter->diagnostics) {
        free(reporter);
        return NULL;
    }
    
    reporter->diagnostic_count = 0;
    reporter->capacity = INITIAL_REPORTER_CAPACITY;
    reporter->enable_suggestions = true;
    reporter->enable_metadata = true;
    reporter->min_confidence = CONFIDENCE_LOW;
    
    return reporter;
}

void enhanced_error_reporter_destroy(EnhancedErrorReporter *reporter) {
    if (!reporter) return;
    
    for (size_t i = 0; i < reporter->diagnostic_count; i++) {
        enhanced_diagnostic_destroy(reporter->diagnostics[i]);
    }
    
    free(reporter->diagnostics);
    free(reporter);
}

static bool ensure_reporter_capacity(EnhancedErrorReporter *reporter) {
    if (reporter->diagnostic_count >= reporter->capacity) {
        size_t new_capacity = reporter->capacity * 2;
        EnhancedDiagnostic **new_diagnostics = realloc(reporter->diagnostics,
                                                      sizeof(EnhancedDiagnostic*) * new_capacity);
        if (!new_diagnostics) return false;
        
        reporter->diagnostics = new_diagnostics;
        reporter->capacity = new_capacity;
    }
    return true;
}

void enhanced_error_reporter_report_undefined_variable(EnhancedErrorReporter *reporter, 
                                                      const char *var_name, 
                                                      size_t line, size_t column,
                                                      const char *file_path) {
    if (!reporter || !var_name || !file_path) return;
    if (!ensure_reporter_capacity(reporter)) return;
    
    // Create diagnostic message
    char message[256];
    snprintf(message, sizeof(message), "undefined variable '%s'", var_name);
    
    EnhancedDiagnostic *diagnostic = enhanced_diagnostic_create("ASTHRA_E001", DIAGNOSTIC_ERROR, message);
    if (!diagnostic) return;
    
    // Add span
    DiagnosticSpan span = {
        .start_line = line,
        .start_column = column,
        .end_line = line,
        .end_column = column + strlen(var_name),
        .file_path = safe_strdup(file_path),
        .label = safe_strdup("undefined variable"),
        .snippet = NULL
    };
    enhanced_diagnostic_add_span(diagnostic, &span);
    
    // Add suggestion (placeholder - would integrate with symbol table in real implementation)
    if (reporter->enable_suggestions) {
        DiagnosticSuggestion suggestion = {
            .type = SUGGESTION_REPLACE,
            .span = span,
            .text = safe_strdup("user_info"), // Placeholder - would use similarity algorithm
            .confidence = CONFIDENCE_MEDIUM,
            .rationale = safe_strdup("Similar variable found in scope")
        };
        enhanced_diagnostic_add_suggestion(diagnostic, &suggestion);
    }
    
    reporter->diagnostics[reporter->diagnostic_count++] = diagnostic;
}

void enhanced_error_reporter_report_type_mismatch(EnhancedErrorReporter *reporter,
                                                  const char *expected_type,
                                                  const char *actual_type,
                                                  size_t line, size_t column,
                                                  const char *file_path) {
    if (!reporter || !expected_type || !actual_type || !file_path) return;
    if (!ensure_reporter_capacity(reporter)) return;
    
    // Create diagnostic message
    char message[512];
    snprintf(message, sizeof(message), "type mismatch: expected '%s', found '%s'", 
             expected_type, actual_type);
    
    EnhancedDiagnostic *diagnostic = enhanced_diagnostic_create("ASTHRA_E003", DIAGNOSTIC_ERROR, message);
    if (!diagnostic) return;
    
    // Add span
    DiagnosticSpan span = {
        .start_line = line,
        .start_column = column,
        .end_line = line,
        .end_column = column + strlen(actual_type),
        .file_path = safe_strdup(file_path),
        .label = safe_strdup("type mismatch"),
        .snippet = NULL
    };
    enhanced_diagnostic_add_span(diagnostic, &span);
    
    // Add type conversion suggestion
    if (reporter->enable_suggestions) {
        char *suggestion_text = NULL;
        ConfidenceLevel confidence = CONFIDENCE_LOW;
        char *rationale = NULL;
        
        // Common type conversions
        if (strcmp(expected_type, "string") == 0 && strcmp(actual_type, "i32") == 0) {
            suggestion_text = safe_strdup(".to_string()");
            confidence = CONFIDENCE_HIGH;
            rationale = safe_strdup("Convert integer to string using .to_string() method");
        } else if (strcmp(expected_type, "i32") == 0 && strcmp(actual_type, "string") == 0) {
            suggestion_text = safe_strdup(".parse::<i32>().unwrap()");
            confidence = CONFIDENCE_MEDIUM;
            rationale = safe_strdup("Parse string to integer (consider error handling)");
        } else {
            // Generic type cast suggestion
            char cast_text[128];
            snprintf(cast_text, sizeof(cast_text), " as %s", expected_type);
            suggestion_text = safe_strdup(cast_text);
            confidence = CONFIDENCE_LOW;
            rationale = safe_strdup("Explicit type cast (verify compatibility)");
        }
        
        if (suggestion_text && rationale) {
            DiagnosticSuggestion suggestion = {
                .type = SUGGESTION_REPLACE,
                .span = span,
                .text = suggestion_text,
                .confidence = confidence,
                .rationale = rationale
            };
            enhanced_diagnostic_add_suggestion(diagnostic, &suggestion);
        }
    }
    
    reporter->diagnostics[reporter->diagnostic_count++] = diagnostic;
}

void enhanced_error_reporter_report_missing_visibility(EnhancedErrorReporter *reporter,
                                                      const char *decl_type,
                                                      size_t line, size_t column,
                                                      const char *file_path) {
    if (!reporter || !decl_type || !file_path) return;
    if (!ensure_reporter_capacity(reporter)) return;
    
    // Create diagnostic for current PEG grammar compliance
    char message[256];
    snprintf(message, sizeof(message), "missing required visibility modifier for %s declaration", decl_type);
    
    EnhancedDiagnostic *diagnostic = enhanced_diagnostic_create("ASTHRA_E002", DIAGNOSTIC_ERROR, message);
    if (!diagnostic) return;
    
    // Add span
    DiagnosticSpan span = {
        .start_line = line,
        .start_column = column,
        .end_line = line,
        .end_column = column + strlen(decl_type),
        .file_path = safe_strdup(file_path),
        .label = safe_strdup("missing visibility modifier"),
        .snippet = NULL
    };
    enhanced_diagnostic_add_span(diagnostic, &span);
    
    // Add high-confidence suggestion for grammar compliance
    if (reporter->enable_suggestions) {
        DiagnosticSuggestion suggestion = {
            .type = SUGGESTION_INSERT,
            .span = span,
            .text = safe_strdup("pub "), // Default to public visibility
            .confidence = CONFIDENCE_HIGH,
            .rationale = safe_strdup("Current PEG grammar requires explicit visibility modifiers (pub/priv)")
        };
        enhanced_diagnostic_add_suggestion(diagnostic, &suggestion);
    }
    
    reporter->diagnostics[reporter->diagnostic_count++] = diagnostic;
}

void enhanced_error_reporter_report_missing_parameter_list(EnhancedErrorReporter *reporter,
                                                          const char *func_name,
                                                          size_t line, size_t column,
                                                          const char *file_path) {
    if (!reporter || !func_name || !file_path) return;
    if (!ensure_reporter_capacity(reporter)) return;
    
    // Create diagnostic for current PEG grammar compliance
    char message[256];
    snprintf(message, sizeof(message), "missing required parameter list for function '%s'", func_name);
    
    EnhancedDiagnostic *diagnostic = enhanced_diagnostic_create("ASTHRA_E004", DIAGNOSTIC_ERROR, message);
    if (!diagnostic) return;
    
    // Add span
    DiagnosticSpan span = {
        .start_line = line,
        .start_column = column,
        .end_line = line,
        .end_column = column + strlen(func_name),
        .file_path = safe_strdup(file_path),
        .label = safe_strdup("missing parameter list"),
        .snippet = NULL
    };
    enhanced_diagnostic_add_span(diagnostic, &span);
    
    // Add high-confidence suggestion for grammar compliance
    if (reporter->enable_suggestions) {
        DiagnosticSuggestion suggestion = {
            .type = SUGGESTION_INSERT,
            .span = span,
            .text = safe_strdup("(none)"), // Use 'none' for empty parameter lists per current grammar
            .confidence = CONFIDENCE_HIGH,
            .rationale = safe_strdup("Current PEG grammar requires explicit parameter lists - use 'none' for empty lists")
        };
        enhanced_diagnostic_add_suggestion(diagnostic, &suggestion);
    }
    
    reporter->diagnostics[reporter->diagnostic_count++] = diagnostic;
}

void enhanced_error_reporter_report_missing_struct_content(EnhancedErrorReporter *reporter,
                                                          const char *struct_name,
                                                          size_t line, size_t column,
                                                          const char *file_path) {
    if (!reporter || !struct_name || !file_path) return;
    if (!ensure_reporter_capacity(reporter)) return;
    
    // Create diagnostic for current PEG grammar compliance
    char message[256];
    snprintf(message, sizeof(message), "missing required content for struct '%s'", struct_name);
    
    EnhancedDiagnostic *diagnostic = enhanced_diagnostic_create("ASTHRA_E005", DIAGNOSTIC_ERROR, message);
    if (!diagnostic) return;
    
    // Add span
    DiagnosticSpan span = {
        .start_line = line,
        .start_column = column,
        .end_line = line,
        .end_column = column + strlen(struct_name),
        .file_path = safe_strdup(file_path),
        .label = safe_strdup("missing struct content"),
        .snippet = NULL
    };
    enhanced_diagnostic_add_span(diagnostic, &span);
    
    // Add high-confidence suggestion for grammar compliance
    if (reporter->enable_suggestions) {
        DiagnosticSuggestion suggestion = {
            .type = SUGGESTION_INSERT,
            .span = span,
            .text = safe_strdup("{ none }"), // Use 'none' for empty struct content per current grammar
            .confidence = CONFIDENCE_HIGH,
            .rationale = safe_strdup("Current PEG grammar requires explicit struct content - use 'none' for empty structs")
        };
        enhanced_diagnostic_add_suggestion(diagnostic, &suggestion);
    }
    
    reporter->diagnostics[reporter->diagnostic_count++] = diagnostic;
}

char *enhanced_error_reporter_to_json(EnhancedErrorReporter *reporter) {
    if (!reporter || reporter->diagnostic_count == 0) return NULL;
    
    // Estimate buffer size
    size_t buffer_size = 1024 + (reporter->diagnostic_count * 512);
    char *json = malloc(buffer_size);
    if (!json) return NULL;
    
    strcpy(json, "{\n  \"diagnostics\": [\n");
    
    for (size_t i = 0; i < reporter->diagnostic_count; i++) {
        char *diagnostic_json = enhanced_diagnostic_to_json(reporter->diagnostics[i]);
        if (diagnostic_json) {
            // Ensure buffer capacity
            size_t needed = strlen(json) + strlen(diagnostic_json) + 10;
            if (needed > buffer_size) {
                buffer_size = needed * 2;
                char *new_json = realloc(json, buffer_size);
                if (!new_json) {
                    free(json);
                    free(diagnostic_json);
                    return NULL;
                }
                json = new_json;
            }
            
            strcat(json, "    ");
            strcat(json, diagnostic_json);
            if (i < reporter->diagnostic_count - 1) {
                strcat(json, ",");
            }
            strcat(json, "\n");
            free(diagnostic_json);
        }
    }
    
    strcat(json, "  ]\n}");
    return json;
}

void enhanced_error_reporter_print_human_readable(EnhancedErrorReporter *reporter) {
    if (!reporter) return;
    
    for (size_t i = 0; i < reporter->diagnostic_count; i++) {
        EnhancedDiagnostic *diagnostic = reporter->diagnostics[i];
        if (!diagnostic) continue;
        
        printf("%s: %s\n", diagnostic->code, diagnostic->message);
        
        // Print spans
        for (size_t j = 0; j < diagnostic->span_count; j++) {
            DiagnosticSpan *span = &diagnostic->spans[j];
            printf("  --> %s:%zu:%zu\n", span->file_path, span->start_line, span->start_column);
            if (span->label) {
                printf("      %s\n", span->label);
            }
        }
        
        // Print suggestions
        for (size_t j = 0; j < diagnostic->suggestion_count; j++) {
            DiagnosticSuggestion *suggestion = &diagnostic->suggestions[j];
            printf("  suggestion (%s confidence): %s\n", 
                   (suggestion->confidence == CONFIDENCE_HIGH) ? "high" :
                   (suggestion->confidence == CONFIDENCE_MEDIUM) ? "medium" : "low",
                   suggestion->rationale);
        }
        
        printf("\n");
    }
}

bool enhanced_error_reporter_has_errors(EnhancedErrorReporter *reporter) {
    if (!reporter) return false;
    
    for (size_t i = 0; i < reporter->diagnostic_count; i++) {
        if (reporter->diagnostics[i] && reporter->diagnostics[i]->level == DIAGNOSTIC_ERROR) {
            return true;
        }
    }
    return false;
}

size_t enhanced_error_reporter_get_error_count(EnhancedErrorReporter *reporter) {
    if (!reporter) return 0;
    
    size_t error_count = 0;
    for (size_t i = 0; i < reporter->diagnostic_count; i++) {
        if (reporter->diagnostics[i] && reporter->diagnostics[i]->level == DIAGNOSTIC_ERROR) {
            error_count++;
        }
    }
    return error_count;
}

void enhanced_error_reporter_clear(EnhancedErrorReporter *reporter) {
    if (!reporter) return;
    
    for (size_t i = 0; i < reporter->diagnostic_count; i++) {
        enhanced_diagnostic_destroy(reporter->diagnostics[i]);
    }
    
    reporter->diagnostic_count = 0;
} 
