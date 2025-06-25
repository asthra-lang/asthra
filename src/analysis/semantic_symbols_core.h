#ifndef SEMANTIC_SYMBOLS_CORE_H
#define SEMANTIC_SYMBOLS_CORE_H

#include "semantic_symbols.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

bool symbol_table_insert_impl(SymbolTable *table, const char *name, SymbolEntry *entry);
SymbolEntry *symbol_table_lookup_impl(SymbolTable *table, const char *name);
bool symbol_table_contains(SymbolTable *table, const char *key);
size_t symbol_table_capacity(SymbolTable *table);
bool symbol_table_remove(SymbolTable *table, const char *key);

#ifdef __cplusplus
}
#endif

#endif // SEMANTIC_SYMBOLS_CORE_H
