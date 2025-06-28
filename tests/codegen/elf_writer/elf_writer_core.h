/**
 * ELF Writer Core - Test Stub
 * 
 * Minimal stub for ELF writer testing
 */

#ifndef ELF_WRITER_CORE_H
#define ELF_WRITER_CORE_H

#include "elf_writer.h"

// Core ELF structures and functions for testing
typedef struct ELFSection {
    char name[32];
    uint32_t type;
    uint32_t flags;
} ELFSection;

// Forward declaration - implemented in elf_writer_test_common.c
ELFWriter* get_test_writer(void);

#endif // ELF_WRITER_CORE_H