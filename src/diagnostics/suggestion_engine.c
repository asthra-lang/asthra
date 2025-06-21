#include "suggestion_engine.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Helper function to safely duplicate strings
static char *safe_strdup(const char *str) {
    if (!str) return NULL;
    size_t len = strlen(str);
    char *copy = malloc(len + 1);
    if (copy) {
        strcpy(copy, str);
    }
    return copy;
}

// Helper function to find minimum of three integers
static int min3(int a, int b, int c) {
    int min_val = a;
    if (b < min_val) min_val = b;
    if (c < min_val) min_val = c;
    return min_val;
}

int levenshtein_distance(const char *s1, const char *s2) {
    if (!s1 || !s2) return -1;
    
    int len1 = strlen(s1), len2 = strlen(s2);
    
    // Handle edge cases
    if (len1 == 0) return len2;
    if (len2 == 0) return len1;
    
    // Create matrix
    int **matrix = malloc((len1 + 1) * sizeof(int*));
    if (!matrix) return -1;
    
    for (int i = 0; i <= len1; i++) {
        matrix[i] = malloc((len2 + 1) * sizeof(int));
        if (!matrix[i]) {
            // Cleanup on failure
            for (int j = 0; j < i; j++) {
                free(matrix[j]);
            }
            free(matrix);
            return -1;
        }
        matrix[i][0] = i;
    }
    
    for (int j = 0; j <= len2; j++) {
        matrix[0][j] = j;
    }
    
    // Fill matrix using dynamic programming
    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            int cost = (s1[i-1] == s2[j-1]) ? 0 : 1;
            matrix[i][j] = min3(
                matrix[i-1][j] + 1,      // deletion
                matrix[i][j-1] + 1,      // insertion
                matrix[i-1][j-1] + cost  // substitution
            );
        }
    }
    
    int result = matrix[len1][len2];
    
    // Cleanup
    for (int i = 0; i <= len1; i++) {
        free(matrix[i]);
    }
    free(matrix);
    
    return result;
}

float calculate_similarity(const char *s1, const char *s2) {
    if (!s1 || !s2) return 0.0f;
    
    int distance = levenshtein_distance(s1, s2);
    if (distance < 0) return 0.0f;
    
    int max_len = strlen(s1) > strlen(s2) ? strlen(s1) : strlen(s2);
    if (max_len == 0) return 1.0f;
    
    return 1.0f - ((float)distance / max_len);
}

const char *find_most_similar_symbol(const char *target, const char **candidates, size_t count) {
    if (!target || !candidates || count == 0) return NULL;
    
    const char *best_match = NULL;
    float best_similarity = 0.0f;
    
    for (size_t i = 0; i < count; i++) {
        if (!candidates[i]) continue;
        
        float similarity = calculate_similarity(target, candidates[i]);
        if (similarity > best_similarity && similarity >= 0.4f) { // Minimum 40% similarity
            best_similarity = similarity;
            best_match = candidates[i];
        }
    }
    
    return best_match;
}

ConfidenceLevel calculate_suggestion_confidence(float similarity, const char *suggestion_type) {
    if (!suggestion_type) return CONFIDENCE_LOW;
    
    // Grammar compliance suggestions are always high confidence
    if (strcmp(suggestion_type, "grammar_compliance") == 0) {
        return CONFIDENCE_HIGH;
    }
    
    // Similarity-based confidence
    if (similarity >= 0.8f) {
        return CONFIDENCE_HIGH;
    } else if (similarity >= 0.6f) {
        return CONFIDENCE_MEDIUM;
    } else {
        return CONFIDENCE_LOW;
    }
}

DiagnosticSuggestion *generate_undefined_variable_suggestion(const char *undefined_var, 
                                                           const char **available_vars,
                                                           size_t var_count) {
    if (!undefined_var || !available_vars || var_count == 0) return NULL;
    
    const char *best_match = find_most_similar_symbol(undefined_var, available_vars, var_count);
    if (!best_match) return NULL;
    
    float similarity = calculate_similarity(undefined_var, best_match);
    
    DiagnosticSuggestion *suggestion = malloc(sizeof(DiagnosticSuggestion));
    if (!suggestion) return NULL;
    
    suggestion->type = SUGGESTION_REPLACE;
    suggestion->text = safe_strdup(best_match);
    suggestion->confidence = calculate_suggestion_confidence(similarity, "variable_similarity");
    
    // Generate rationale
    char *rationale = malloc(256);
    if (rationale) {
        snprintf(rationale, 256, "Similar variable '%s' found in scope (%.1f%% similarity)", 
                 best_match, similarity * 100);
        suggestion->rationale = rationale;
    } else {
        suggestion->rationale = safe_strdup("Similar variable found in scope");
    }
    
    // Initialize span (will be set by caller)
    memset(&suggestion->span, 0, sizeof(DiagnosticSpan));
    
    return suggestion;
}

DiagnosticSuggestion *generate_type_mismatch_suggestion(const char *expected_type,
                                                       const char *actual_type) {
    if (!expected_type || !actual_type) return NULL;
    
    DiagnosticSuggestion *suggestion = malloc(sizeof(DiagnosticSuggestion));
    if (!suggestion) return NULL;
    
    suggestion->type = SUGGESTION_REPLACE;
    
    char *suggestion_text = NULL;
    ConfidenceLevel confidence = CONFIDENCE_LOW;
    char *rationale = NULL;
    
    // Common type conversions based on Asthra's type system
    if (strcmp(expected_type, "string") == 0 && strcmp(actual_type, "i32") == 0) {
        suggestion_text = safe_strdup(".to_string()");
        confidence = CONFIDENCE_HIGH;
        rationale = safe_strdup("Convert integer to string using .to_string() method");
    } else if (strcmp(expected_type, "i32") == 0 && strcmp(actual_type, "string") == 0) {
        suggestion_text = safe_strdup(".parse::<i32>().unwrap()");
        confidence = CONFIDENCE_MEDIUM;
        rationale = safe_strdup("Parse string to integer (consider error handling)");
    } else if (strcmp(expected_type, "f64") == 0 && strcmp(actual_type, "i32") == 0) {
        suggestion_text = safe_strdup(" as f64");
        confidence = CONFIDENCE_HIGH;
        rationale = safe_strdup("Convert integer to float using type cast");
    } else if (strcmp(expected_type, "i32") == 0 && strcmp(actual_type, "f64") == 0) {
        suggestion_text = safe_strdup(" as i32");
        confidence = CONFIDENCE_MEDIUM;
        rationale = safe_strdup("Convert float to integer (may lose precision)");
    } else if (strcmp(expected_type, "bool") == 0 && strcmp(actual_type, "i32") == 0) {
        suggestion_text = safe_strdup(" != 0");
        confidence = CONFIDENCE_MEDIUM;
        rationale = safe_strdup("Convert integer to boolean (0 = false, non-zero = true)");
    } else {
        // Generic type cast suggestion
        char cast_text[128];
        snprintf(cast_text, sizeof(cast_text), " as %s", expected_type);
        suggestion_text = safe_strdup(cast_text);
        confidence = CONFIDENCE_LOW;
        rationale = safe_strdup("Explicit type cast (verify compatibility)");
    }
    
    suggestion->text = suggestion_text;
    suggestion->confidence = confidence;
    suggestion->rationale = rationale;
    
    // Initialize span (will be set by caller)
    memset(&suggestion->span, 0, sizeof(DiagnosticSpan));
    
    return suggestion;
}

DiagnosticSuggestion *generate_missing_visibility_suggestion(const char *decl_type) {
    if (!decl_type) return NULL;
    
    DiagnosticSuggestion *suggestion = malloc(sizeof(DiagnosticSuggestion));
    if (!suggestion) return NULL;
    
    suggestion->type = SUGGESTION_INSERT;
    suggestion->text = safe_strdup("pub "); // Default to public visibility
    suggestion->confidence = CONFIDENCE_HIGH;
    
    char *rationale = malloc(256);
    if (rationale) {
        snprintf(rationale, 256, "Current PEG grammar requires explicit visibility modifiers for %s declarations", decl_type);
        suggestion->rationale = rationale;
    } else {
        suggestion->rationale = safe_strdup("Grammar requires explicit visibility modifiers");
    }
    
    // Initialize span (will be set by caller)
    memset(&suggestion->span, 0, sizeof(DiagnosticSpan));
    
    return suggestion;
}

DiagnosticSuggestion *generate_missing_parameter_list_suggestion(const char *func_name) {
    if (!func_name) return NULL;
    
    DiagnosticSuggestion *suggestion = malloc(sizeof(DiagnosticSuggestion));
    if (!suggestion) return NULL;
    
    suggestion->type = SUGGESTION_INSERT;
    suggestion->text = safe_strdup("(none)"); // Use 'none' for empty parameter lists per current grammar
    suggestion->confidence = CONFIDENCE_HIGH;
    
    char *rationale = malloc(256);
    if (rationale) {
        snprintf(rationale, 256, "Current PEG grammar requires explicit parameter lists - use 'none' for empty lists");
        suggestion->rationale = rationale;
    } else {
        suggestion->rationale = safe_strdup("Grammar requires explicit parameter lists");
    }
    
    // Initialize span (will be set by caller)
    memset(&suggestion->span, 0, sizeof(DiagnosticSpan));
    
    return suggestion;
}

DiagnosticSuggestion *generate_missing_struct_content_suggestion(const char *struct_name) {
    if (!struct_name) return NULL;
    
    DiagnosticSuggestion *suggestion = malloc(sizeof(DiagnosticSuggestion));
    if (!suggestion) return NULL;
    
    suggestion->type = SUGGESTION_INSERT;
    suggestion->text = safe_strdup("{ none }"); // Use 'none' for empty struct content per current grammar
    suggestion->confidence = CONFIDENCE_HIGH;
    
    char *rationale = malloc(256);
    if (rationale) {
        snprintf(rationale, 256, "Current PEG grammar requires explicit struct content - use 'none' for empty structs");
        suggestion->rationale = rationale;
    } else {
        suggestion->rationale = safe_strdup("Grammar requires explicit struct content");
    }
    
    // Initialize span (will be set by caller)
    memset(&suggestion->span, 0, sizeof(DiagnosticSpan));
    
    return suggestion;
}

DiagnosticSuggestion *generate_grammar_compliance_suggestion(const char *violation_type,
                                                           const char *context) {
    if (!violation_type) return NULL;
    
    DiagnosticSuggestion *suggestion = malloc(sizeof(DiagnosticSuggestion));
    if (!suggestion) return NULL;
    
    suggestion->confidence = CONFIDENCE_HIGH; // Grammar compliance is always high confidence
    
    // Handle different grammar violations
    if (strcmp(violation_type, "missing_visibility") == 0) {
        suggestion->type = SUGGESTION_INSERT;
        suggestion->text = safe_strdup("pub ");
        suggestion->rationale = safe_strdup("Current PEG grammar requires explicit visibility modifiers");
    } else if (strcmp(violation_type, "missing_parameter_list") == 0) {
        suggestion->type = SUGGESTION_INSERT;
        suggestion->text = safe_strdup("(none)");
        suggestion->rationale = safe_strdup("Current PEG grammar requires explicit parameter lists");
    } else if (strcmp(violation_type, "missing_struct_content") == 0) {
        suggestion->type = SUGGESTION_INSERT;
        suggestion->text = safe_strdup("{ none }");
        suggestion->rationale = safe_strdup("Current PEG grammar requires explicit struct content");
    } else if (strcmp(violation_type, "missing_enum_content") == 0) {
        suggestion->type = SUGGESTION_INSERT;
        suggestion->text = safe_strdup("{ none }");
        suggestion->rationale = safe_strdup("Current PEG grammar requires explicit enum content");
    } else if (strcmp(violation_type, "missing_import_alias") == 0) {
        suggestion->type = SUGGESTION_INSERT;
        suggestion->text = safe_strdup(" as ModuleName");
        suggestion->rationale = safe_strdup("Consider adding import alias to avoid naming conflicts");
    } else {
        // Generic grammar compliance suggestion
        suggestion->type = SUGGESTION_REPLACE;
        suggestion->text = safe_strdup("/* fix grammar violation */");
        suggestion->rationale = safe_strdup("Code violates current PEG grammar requirements");
    }
    
    // Initialize span (will be set by caller)
    memset(&suggestion->span, 0, sizeof(DiagnosticSpan));
    
    return suggestion;
}

void suggestion_destroy(DiagnosticSuggestion *suggestion) {
    if (!suggestion) return;
    
    free(suggestion->text);
    free(suggestion->rationale);
    free(suggestion->span.file_path);
    free(suggestion->span.label);
    free(suggestion->span.snippet);
    free(suggestion);
} 
