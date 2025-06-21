/**
 * Asthra Programming Language Compiler
 * Symbol Management Operations
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "symbol_resolution.h"
#include "../platform.h"
#include <stdlib.h>
#include <string.h>
#include <strings.h>

bool asthra_symbol_resolver_add_symbol(Asthra_SymbolResolver *resolver,
                                      const Asthra_ObjectSymbol *symbol,
                                      const char *source_file) {
    if (!resolver || !symbol || !symbol->name || !source_file) {
        if (resolver) {
            resolver->last_error = asthra_strdup("Invalid parameters for symbol addition");
            resolver->error_occurred = true;
        }
        return false;
    }
    
    // Check load factor and resize if needed
    double load_factor = (double)resolver->symbol_count / (double)resolver->table_size;
    if (load_factor > 0.7) {
        if (!resize_symbol_table(resolver)) {
            return false;
        }
    }
    
    size_t hash = asthra_symbol_hash(symbol->name, resolver->table_size);
    
    // Check for existing symbol
    Asthra_SymbolEntry *existing = resolver->symbol_table[hash];
    while (existing) {
        bool names_match = resolver->config.case_sensitive_symbols ? 
            (strcmp(existing->name, symbol->name) == 0) :
            (strcasecmp(existing->name, symbol->name) == 0);
            
        if (names_match) {
            // Symbol already exists - handle conflict
            if (existing->status == ASTHRA_RESOLVE_DEFINED && 
                symbol->binding != ASTHRA_SYMBOL_UNDEFINED) {
                // Multiple definitions - this is an error
                resolver->last_error = asthra_strdup("Multiple definitions of symbol");
                resolver->error_occurred = true;
                return false;
            } else if (existing->status == ASTHRA_RESOLVE_UNDEFINED ||
                symbol->binding != ASTHRA_SYMBOL_UNDEFINED) {
                existing->address = symbol->address;
                existing->size = symbol->size;
                existing->status = ASTHRA_RESOLVE_DEFINED;
                existing->defining_file = asthra_strdup(source_file);
                existing->section_name = asthra_strdup(symbol->section_name);
                existing->resolved = true;
                return true;
            }
            
            return true; // Symbol already handled properly
        }
        existing = existing->next;
    }
    
    // Create new symbol entry
    Asthra_SymbolEntry *entry = calloc(1, sizeof(Asthra_SymbolEntry));
    if (!entry) {
        resolver->last_error = asthra_strdup("Failed to allocate memory for symbol entry");
        resolver->error_occurred = true;
        return false;
    }
    
    entry->name = asthra_strdup(symbol->name);
    entry->address = symbol->address;
    entry->size = symbol->size;
    entry->defining_file = asthra_strdup(source_file);
    entry->section_name = asthra_strdup(symbol->section_name);
    entry->file_offset = 0; // Set during relocation
    
    // Determine symbol status and type
    switch (symbol->binding) {
        case ASTHRA_SYMBOL_UNDEFINED:
            entry->status = ASTHRA_RESOLVE_UNDEFINED;
            entry->resolved = false;
            break;
        case ASTHRA_SYMBOL_GLOBAL:
            entry->status = ASTHRA_RESOLVE_DEFINED;
            entry->resolved = true;
            break;
        case ASTHRA_SYMBOL_WEAK:
            entry->status = ASTHRA_RESOLVE_WEAK;
            entry->resolved = true;
            break;
        case ASTHRA_SYMBOL_LOCAL:
            entry->status = ASTHRA_RESOLVE_DEFINED;
            entry->resolved = true;
            break;
        default:
            entry->status = ASTHRA_RESOLVE_UNDEFINED;
            entry->resolved = false;
            break;
    }
    
    // Set symbol type
    if (strstr(symbol->name, "func_") == symbol->name) {
        entry->type = ASTHRA_RESOLVE_FUNC;
    } else if (strstr(symbol->name, "var_") == symbol->name) {
        entry->type = ASTHRA_RESOLVE_VAR;
    } else {
        entry->type = ASTHRA_RESOLVE_UNKNOWN;
    }
    
    // Insert into hash table
    entry->next = resolver->symbol_table[hash];
    resolver->symbol_table[hash] = entry;
    resolver->symbol_count++;
    
    // Update statistics
    if (entry->next) {
        resolver->hash_stats.hash_collisions++;
    }
    
    return true;
}

size_t asthra_symbol_resolver_add_object_file(Asthra_SymbolResolver *resolver,
                                             const Asthra_ObjectFile *object_file) {
    if (!resolver || !object_file) return 0;
    
    size_t added_count = 0;
    
    // Add all symbols from the object file
    for (size_t i = 0; i < object_file->symbol_count; i++) {
        if (asthra_symbol_resolver_add_symbol(resolver, &object_file->symbols[i], 
                                             object_file->file_path)) {
            added_count++;
        }
    }
    
    return added_count;
}

bool asthra_symbol_resolver_add_reference(Asthra_SymbolResolver *resolver,
                                         const char *symbol_name,
                                         const char *referencing_file,
                                         const char *section_name,
                                         uint64_t offset,
                                         int reference_type,
                                         int64_t addend) {
    if (!resolver || !symbol_name || !referencing_file) {
        if (resolver) {
            resolver->last_error = asthra_strdup("Invalid parameters for reference addition");
            resolver->error_occurred = true;
        }
        return false;
    }
    
    // Find symbol entry
    size_t hash = asthra_symbol_hash(symbol_name, resolver->table_size);
    Asthra_SymbolEntry *entry = resolver->symbol_table[hash];
    
    while (entry) {
        bool names_match = resolver->config.case_sensitive_symbols ? 
            (strcmp(entry->name, symbol_name) == 0) :
            (strcasecmp(entry->name, symbol_name) == 0);
            
        if (names_match) {
            break;
        }
        entry = entry->next;
    }
    
    // If symbol doesn't exist, create undefined entry
    if (!entry) {
        entry = calloc(1, sizeof(Asthra_SymbolEntry));
        if (!entry) {
            resolver->last_error = asthra_strdup("Failed to allocate memory for undefined symbol");
            resolver->error_occurred = true;
            return false;
        }
        
        entry->name = asthra_strdup(symbol_name);
        entry->status = ASTHRA_RESOLVE_UNDEFINED;
        entry->resolved = false;
        entry->type = ASTHRA_RESOLVE_UNKNOWN;
        
        // Insert into hash table
        entry->next = resolver->symbol_table[hash];
        resolver->symbol_table[hash] = entry;
        resolver->symbol_count++;
    }
    
    // Create reference
    Asthra_SymbolReference *ref = calloc(1, sizeof(Asthra_SymbolReference));
    if (!ref) {
        resolver->last_error = asthra_strdup("Failed to allocate memory for symbol reference");
        resolver->error_occurred = true;
        return false;
    }
    
    ref->referencing_file = asthra_strdup(referencing_file);
    ref->section_name = asthra_strdup(section_name);
    ref->offset = offset;
    ref->type = (int)reference_type;
    ref->addend = addend;
    ref->resolved = false;
    
    // Add to symbol's reference list
    ref->next = entry->references;
    entry->references = ref;
    entry->reference_count++;
    
    return true;
}

Asthra_SymbolEntry *asthra_symbol_resolver_find_symbol(Asthra_SymbolResolver *resolver,
                                                       const char *symbol_name) {
    if (!resolver || !symbol_name) return NULL;
    
    size_t hash = asthra_symbol_hash(symbol_name, resolver->table_size);
    Asthra_SymbolEntry *entry = resolver->symbol_table[hash];
    
    while (entry) {
        if (strcmp(entry->name, symbol_name) == 0) {
            return entry;
        }
        entry = entry->next;
    }
    
    return NULL;
}