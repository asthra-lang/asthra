#ifndef SEMANTIC_SYMBOLS_UTILS_H
#define SEMANTIC_SYMBOLS_UTILS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "semantic_symbols.h"

#ifdef __cplusplus
extern "C" {
#endif

const char *symbol_kind_name(SymbolKind kind);
bool symbol_kind_is_callable(SymbolKind kind);
bool symbol_kind_is_type(SymbolKind kind);
bool symbol_kind_is_value(SymbolKind kind);
bool semantic_validate_symbol_name(const char *name);
bool semantic_is_reserved_keyword(const char *name);
bool semantic_check_symbol_shadowing(SemanticAnalyzer *analyzer, const char *name);

#ifdef __cplusplus
}
#endif

#endif // SEMANTIC_SYMBOLS_UTILS_H
