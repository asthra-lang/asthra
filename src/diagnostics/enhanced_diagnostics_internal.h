/**
 * Asthra Programming Language
 * Enhanced Diagnostics Internal Functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_ENHANCED_DIAGNOSTICS_INTERNAL_H
#define ASTHRA_ENHANCED_DIAGNOSTICS_INTERNAL_H

#include "enhanced_diagnostics.h"
#include <stdbool.h>

// Utility functions (enhanced_diagnostics_utils.c)
char *duplicate_string(const char *str);
char *escape_json_string(const char *str);
const char *diagnostic_level_to_string(DiagnosticLevel level);
const char *suggestion_type_to_string(SuggestionType type);
const char *confidence_level_to_string(ConfidenceLevel confidence);

// Capacity management functions (enhanced_diagnostics_capacity.c)
bool ensure_span_capacity(EnhancedDiagnostic *diagnostic);
bool ensure_suggestion_capacity(EnhancedDiagnostic *diagnostic);

#endif