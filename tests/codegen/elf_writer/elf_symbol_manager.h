/**
 * ELF Symbol Manager - Test Stub
 * 
 * Minimal stub for ELF writer testing
 */

#ifndef ELF_SYMBOL_MANAGER_H
#define ELF_SYMBOL_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct ELFSymbolManager {
    void *internal;  // Opaque pointer for test purposes
} ELFSymbolManager;

// Stub functions for testing
static inline ELFSymbolManager* elf_symbol_manager_create(void) {
    return NULL;
}

static inline void elf_symbol_manager_destroy(ELFSymbolManager* mgr) {
    (void)mgr;
}

#endif // ELF_SYMBOL_MANAGER_H