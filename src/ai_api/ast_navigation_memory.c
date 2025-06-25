#include "ast_navigation_internal.h"
#include <stdlib.h>

// =============================================================================
// MEMORY MANAGEMENT
// =============================================================================

void asthra_ai_free_code_location_array(AICodeLocation **locations, size_t count) {
    if (!locations)
        return;

    for (size_t i = 0; i < count; i++) {
        asthra_ai_free_code_location(locations[i]);
    }
    free(locations);
}

void asthra_ai_free_symbol_usage_array(AISymbolUsage **usages, size_t count) {
    if (!usages)
        return;

    for (size_t i = 0; i < count; i++) {
        asthra_ai_free_symbol_usage(usages[i]);
    }
    free(usages);
}

void asthra_ai_free_code_location(AICodeLocation *location) {
    if (!location)
        return;

    free(location->file_path);
    free(location->context_code);
    free(location);
}

void asthra_ai_free_symbol_usage(AISymbolUsage *usage) {
    if (!usage)
        return;

    free(usage->symbol_name);
    free(usage->usage_type);
    free(usage->scope_name);

    // Free the embedded location fields (not the struct itself)
    free(usage->location.file_path);
    free(usage->location.context_code);

    free(usage);
}