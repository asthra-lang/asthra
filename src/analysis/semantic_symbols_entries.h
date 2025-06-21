#ifndef SEMANTIC_SYMBOLS_ENTRIES_H
#define SEMANTIC_SYMBOLS_ENTRIES_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "semantic_symbols.h"

#ifdef __cplusplus
extern "C" {
#endif

SymbolEntry *symbol_entry_copy(const SymbolEntry *entry);
SymbolEntry *semantic_resolve_symbol_impl(SemanticAnalyzer *analyzer, const char *name);

#ifdef __cplusplus
}
#endif

#endif // SEMANTIC_SYMBOLS_ENTRIES_H 
