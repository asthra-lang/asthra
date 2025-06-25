#ifndef ASTHRA_SUGGESTION_ENGINE_H
#define ASTHRA_SUGGESTION_ENGINE_H

#include "enhanced_diagnostics.h"
#include <stddef.h>

// Suggestion generation functions
DiagnosticSuggestion *generate_undefined_variable_suggestion(const char *undefined_var,
                                                             const char **available_vars,
                                                             size_t var_count);
DiagnosticSuggestion *generate_type_mismatch_suggestion(const char *expected_type,
                                                        const char *actual_type);
DiagnosticSuggestion *generate_missing_visibility_suggestion(const char *decl_type);
DiagnosticSuggestion *generate_missing_parameter_list_suggestion(const char *func_name);
DiagnosticSuggestion *generate_missing_struct_content_suggestion(const char *struct_name);

// Grammar compliance suggestions for current PEG grammar
DiagnosticSuggestion *generate_grammar_compliance_suggestion(const char *violation_type,
                                                             const char *context);

// Utility functions for similarity analysis
int levenshtein_distance(const char *s1, const char *s2);
float calculate_similarity(const char *s1, const char *s2);
const char *find_most_similar_symbol(const char *target, const char **candidates, size_t count);

// Confidence calculation
ConfidenceLevel calculate_suggestion_confidence(float similarity, const char *suggestion_type);

// Memory management for suggestions
void suggestion_destroy(DiagnosticSuggestion *suggestion);

#endif
