/**
 * ELF Writer Symbols - Test Stub
 * 
 * Minimal stub for ELF writer testing
 */

#ifndef ELF_WRITER_SYMBOLS_H
#define ELF_WRITER_SYMBOLS_H

#include "elf_writer.h"

// Symbol binding types
#define STB_LOCAL   0
#define STB_GLOBAL  1
#define STB_WEAK    2

// Symbol types
#define STT_NOTYPE  0
#define STT_OBJECT  1
#define STT_FUNC    2
#define STT_SECTION 3
#define STT_FILE    4

#endif // ELF_WRITER_SYMBOLS_H