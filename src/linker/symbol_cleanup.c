/**
 * Asthra Programming Language Compiler
 * Symbol Cleanup Utilities
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../platform.h"
#include "symbol_resolution.h"
#include <stdlib.h>
#include <string.h>

void asthra_symbol_entry_free(Asthra_SymbolEntry *entry) {
    if (!entry)
        return;

    if (entry->name)
        free(entry->name);
    if (entry->defining_file)
        free(entry->defining_file);
    if (entry->section_name)
        free(entry->section_name);

    // Free all references
    Asthra_SymbolReference *ref = entry->references;
    while (ref) {
        Asthra_SymbolReference *next = ref->next;
        if (ref->referencing_file)
            free(ref->referencing_file);
        if (ref->section_name)
            free(ref->section_name);
        free(ref);
        ref = next;
    }

    free(entry);
}

void asthra_symbol_conflict_free(Asthra_SymbolConflict *conflict) {
    if (!conflict)
        return;

    if (conflict->symbol_name)
        free(conflict->symbol_name);
    if (conflict->description)
        free(conflict->description);

    free(conflict);
}

void asthra_resolution_result_cleanup(Asthra_ResolutionResult *result) {
    if (!result)
        return;

    // Free undefined symbol names
    if (result->undefined_symbol_names) {
        for (size_t i = 0; i < result->undefined_count; i++) {
            if (result->undefined_symbol_names[i]) {
                free(result->undefined_symbol_names[i]);
            }
        }
        free(result->undefined_symbol_names);
    }

    // Free conflicts
    Asthra_SymbolConflict *conflict = result->conflicts;
    while (conflict) {
        Asthra_SymbolConflict *next = conflict->next;
        asthra_symbol_conflict_free(conflict);
        conflict = next;
    }

    // Free warnings
    if (result->warnings) {
        for (size_t i = 0; i < result->warning_count; i++) {
            if (result->warnings[i]) {
                free(result->warnings[i]);
            }
        }
        free(result->warnings);
    }

    memset(result, 0, sizeof(Asthra_ResolutionResult));
}