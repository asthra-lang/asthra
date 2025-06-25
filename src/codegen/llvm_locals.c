/**
 * Asthra Programming Language LLVM Local Variables
 * Implementation of local variable tracking
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_locals.h"
#include <stdlib.h>
#include <string.h>

// Register a local variable with its alloca instruction
void register_local_var(LLVMBackendData *data, const char *name, LLVMValueRef alloca,
                        LLVMTypeRef type) {
    LocalVar *var = malloc(sizeof(LocalVar));
    var->name = strdup(name);
    var->alloca = alloca;
    var->type = type;
    var->next = data->local_vars;
    data->local_vars = var;
}

// Look up a local variable entry by name
LocalVar *lookup_local_var_entry(LLVMBackendData *data, const char *name) {
    LocalVar *var = data->local_vars;
    while (var) {
        if (strcmp(var->name, name) == 0) {
            return var;
        }
        var = var->next;
    }
    return NULL;
}

// Look up a local variable's alloca by name
LLVMValueRef lookup_local_var(LLVMBackendData *data, const char *name) {
    LocalVar *var = lookup_local_var_entry(data, name);
    return var ? var->alloca : NULL;
}

// Clear all local variables (typically at end of function)
void clear_local_vars(LLVMBackendData *data) {
    LocalVar *var = data->local_vars;
    while (var) {
        LocalVar *next = var->next;
        free((char *)var->name);
        free(var);
        var = next;
    }
    data->local_vars = NULL;
}