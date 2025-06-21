#ifndef SEMANTIC_SYMBOLS_STATS_H
#define SEMANTIC_SYMBOLS_STATS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "semantic_symbols.h"

#ifdef __cplusplus
extern "C" {
#endif

void semantic_print_symbol_statistics(SemanticAnalyzer *analyzer);

#ifdef __cplusplus
}
#endif

#endif // SEMANTIC_SYMBOLS_STATS_H
