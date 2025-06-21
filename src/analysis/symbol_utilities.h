#ifndef SYMBOL_UTILITIES_H
#define SYMBOL_UTILITIES_H

#include "semantic_symbols.h"
#include "semantic_types.h"
#include "../parser/ast_types.h"

// Symbol utilities function declarations
SymbolEntry *semantic_resolve_identifier(SemanticAnalyzer *analyzer, const char *name);

bool semantic_declare_symbol(SemanticAnalyzer *analyzer, 
                            const char *name, 
                            SymbolKind kind, 
                            TypeDescriptor *type, 
                            ASTNode *declaration);

#endif // SYMBOL_UTILITIES_H