/**
 * ELF Relocation Manager - Test Stub
 * 
 * Minimal stub for ELF writer testing
 */

#ifndef ELF_RELOCATION_MANAGER_H
#define ELF_RELOCATION_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct ELFRelocationManager {
    void *internal;  // Opaque pointer for test purposes
} ELFRelocationManager;

// Stub functions for testing
static inline ELFRelocationManager* elf_relocation_manager_create(void) {
    return NULL;
}

static inline void elf_relocation_manager_destroy(ELFRelocationManager* mgr) {
    (void)mgr;
}

#endif // ELF_RELOCATION_MANAGER_H