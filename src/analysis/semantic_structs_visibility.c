/*
 * Field Visibility and Access Module
 * Field visibility checking and access permission validation
 * 
 * Part of semantic_structs.c split (578 lines -> 5 focused modules)
 * Handles field access validation and visibility enforcement
 */

#include "semantic_structs_common.h"

// ============================================================================
// FIELD VISIBILITY CHECKING
// ============================================================================

bool check_field_visibility(SemanticAnalyzer *analyzer, const char *struct_name, const char *field_name, SourceLocation location) {
    if (!analyzer || !struct_name || !field_name) {
        return false;
    }

    // Get the struct symbol
    SymbolEntry *struct_symbol = symbol_table_lookup_safe(analyzer->current_scope, struct_name);
    if (!struct_symbol || !struct_symbol->type) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL,
                             location,
                             "Unknown struct '%s' in field access", struct_name);
        return false;
    }

    TypeDescriptor *struct_type = struct_symbol->type;
    if (struct_type->category != TYPE_STRUCT || !struct_type->data.struct_type.fields) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH,
                             location,
                             "Symbol '%s' has no fields", struct_name);
        return false;
    }

    // Look up the field
    SymbolEntry *field_symbol = symbol_table_lookup_local(struct_type->data.struct_type.fields, field_name);
    if (!field_symbol) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL,
                             location,
                             "Struct '%s' has no field '%s'", struct_name, field_name);
        return false;
    }

    // Check visibility
    if (field_symbol->visibility == VISIBILITY_PRIVATE) {
        // TODO: Check if we're in the same package/module
        // For now, we'll allow private access (should be restricted in real implementation)
    }

    return true;
} 
