#ifndef SEMANTIC_SYMBOLS_ALIASES_H
#define SEMANTIC_SYMBOLS_ALIASES_H

#include "semantic_symbols.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

bool semantic_register_module_alias(const char *alias, const char *module_name);
const char *semantic_resolve_module_alias(const char *alias);
bool semantic_has_module_alias(const char *alias);
void semantic_cleanup_module_aliases(void);

#ifdef __cplusplus
}
#endif

#endif // SEMANTIC_SYMBOLS_ALIASES_H
