/**
 * Asthra Programming Language Compiler
 * ELF Compatibility Header
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Provides ELF definitions for cross-platform compilation.
 * This header is used when the system doesn't provide <elf.h>
 */

#ifndef ASTHRA_ELF_COMPAT_H
#define ASTHRA_ELF_COMPAT_H

#include <stdint.h>

// On systems that have elf.h, use it instead of this compatibility header
#ifdef __has_include
  #if __has_include(<elf.h>)
    #include <elf.h>
    #define ASTHRA_HAS_SYSTEM_ELF_H
  #endif
#elif defined(__linux__) || defined(__unix__)
  #include <elf.h>
  #define ASTHRA_HAS_SYSTEM_ELF_H
#endif

#ifndef ASTHRA_HAS_SYSTEM_ELF_H
// Only define these if we don't have the system elf.h

#ifdef __cplusplus
extern "C" {
#endif

// ELF identification
#define EI_NIDENT 16

// ELF magic number
#define ELFMAG0 0x7f
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

// ELF identification indices
#define EI_MAG0       0
#define EI_MAG1       1
#define EI_MAG2       2
#define EI_MAG3       3
#define EI_CLASS      4
#define EI_DATA       5
#define EI_VERSION    6
#define EI_OSABI      7
#define EI_ABIVERSION 8
#define EI_PAD        9

// ELF magic number macros
#define ELFMAG "\177ELF"
#define SELFMAG 4

// ELF classes
#define ELFCLASSNONE 0
#define ELFCLASS32   1
#define ELFCLASS64   2

// ELF data encoding
#define ELFDATANONE 0
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2

// ELF versions
#define EV_NONE    0
#define EV_CURRENT 1

// ELF OS/ABI
#define ELFOSABI_NONE   0
#define ELFOSABI_SYSV   0
#define ELFOSABI_LINUX  3

// ELF object file types
#define ET_NONE 0
#define ET_REL  1
#define ET_EXEC 2
#define ET_DYN  3
#define ET_CORE 4

// ELF machine types
#define EM_NONE   0
#define EM_386    3
#define EM_X86_64 62

// Section header types
#define SHT_NULL     0
#define SHT_PROGBITS 1
#define SHT_SYMTAB   2
#define SHT_STRTAB   3
#define SHT_RELA     4
#define SHT_HASH     5
#define SHT_DYNAMIC  6
#define SHT_NOTE     7
#define SHT_NOBITS   8
#define SHT_REL      9
#define SHT_SHLIB    10
#define SHT_DYNSYM   11

// Section header flags
#define SHF_WRITE     0x1
#define SHF_ALLOC     0x2
#define SHF_EXECINSTR 0x4

// Symbol table entry types
#define STT_NOTYPE  0
#define STT_OBJECT  1
#define STT_FUNC    2
#define STT_SECTION 3
#define STT_FILE    4
#define STT_COMMON  5
#define STT_TLS     6

// Symbol table entry bindings
#define STB_LOCAL  0
#define STB_GLOBAL 1
#define STB_WEAK   2

// Symbol visibility
#define STV_DEFAULT 0
#define STV_INTERNAL 1
#define STV_HIDDEN   2
#define STV_PROTECTED 3

// Special section indices
#define SHN_UNDEF 0

// x86-64 relocation types
#define R_X86_64_NONE     0
#define R_X86_64_64       1
#define R_X86_64_PC32     2
#define R_X86_64_GOT32    3
#define R_X86_64_PLT32    4
#define R_X86_64_COPY     5
#define R_X86_64_GLOB_DAT 6
#define R_X86_64_JUMP_SLOT 7
#define R_X86_64_RELATIVE 8
#define R_X86_64_GOTPCREL 9
#define R_X86_64_32       10
#define R_X86_64_32S      11
#define R_X86_64_16       12
#define R_X86_64_PC16     13
#define R_X86_64_8        14
#define R_X86_64_PC8      15

// ELF64 header
typedef struct {
    unsigned char e_ident[EI_NIDENT];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf64_Ehdr;

// ELF64 section header
typedef struct {
    uint32_t sh_name;
    uint32_t sh_type;
    uint64_t sh_flags;
    uint64_t sh_addr;
    uint64_t sh_offset;
    uint64_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint64_t sh_addralign;
    uint64_t sh_entsize;
} Elf64_Shdr;

// ELF64 symbol table entry
typedef struct {
    uint32_t st_name;
    unsigned char st_info;
    unsigned char st_other;
    uint16_t st_shndx;
    uint64_t st_value;
    uint64_t st_size;
} Elf64_Sym;

// ELF64 relocation entry with addend
typedef struct {
    uint64_t r_offset;
    uint64_t r_info;
    int64_t r_addend;
} Elf64_Rela;

// ELF64 relocation entry without addend
typedef struct {
    uint64_t r_offset;
    uint64_t r_info;
} Elf64_Rel;

// Macros for symbol table entry info
#ifndef ELF64_ST_BIND
#define ELF64_ST_BIND(info) ((info) >> 4)
#endif
#ifndef ELF64_ST_TYPE
#define ELF64_ST_TYPE(info) ((info) & 0xf)
#endif
#ifndef ELF64_ST_INFO
#define ELF64_ST_INFO(bind, type) (((bind) << 4) + ((type) & 0xf))
#endif

// Macros for relocation entry info
#ifndef ELF64_R_SYM
#define ELF64_R_SYM(info) ((info) >> 32)
#endif
#ifndef ELF64_R_TYPE
#define ELF64_R_TYPE(info) ((info) & 0xffffffff)
#endif
#ifndef ELF64_R_INFO
#define ELF64_R_INFO(sym, type) (((uint64_t)(sym) << 32) + (type))
#endif

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_HAS_SYSTEM_ELF_H

#endif // ASTHRA_ELF_COMPAT_H 
