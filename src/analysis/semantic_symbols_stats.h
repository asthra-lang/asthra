#ifndef SEMANTIC_SYMBOLS_STATS_H
#define SEMANTIC_SYMBOLS_STATS_H

#include "semantic_symbols.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void semantic_print_symbol_statistics(SemanticAnalyzer *analyzer);

#ifdef __cplusplus
}
#endif

#endif // SEMANTIC_SYMBOLS_STATS_H
