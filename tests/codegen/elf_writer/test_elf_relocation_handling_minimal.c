#include "../framework/test_framework_minimal.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_relocation_section_creation", __FILE__, __LINE__, "Test relocation section creation",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_symbol_relocation", __FILE__, __LINE__, "Test symbol relocation handling",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_dynamic_relocations", __FILE__, __LINE__, "Test dynamic relocation processing",
     ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
    {"test_relocation_validation", __FILE__, __LINE__, "Test relocation validation and linking",
     ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}};

// Minimal ELF relocation structures
typedef struct {
    uint64_t offset; // Address where to apply relocation
    uint64_t info;   // Symbol and type info
    int64_t addend;  // Addend for relocation
} MinimalElfRelocation;

typedef struct {
    const char *name; // Symbol name
    uint64_t value;   // Symbol value/address
    uint64_t size;    // Symbol size
    uint8_t info;     // Symbol type and binding
    uint8_t other;    // Symbol visibility
    uint16_t shndx;   // Section index
} MinimalElfSymbol;

typedef struct {
    const char *name; // Section name
    uint32_t type;    // Section type
    uint64_t flags;   // Section flags
    uint64_t addr;    // Section address
    uint64_t offset;  // File offset
    uint64_t size;    // Section size
    uint32_t link;    // Link to other section
    uint32_t info;    // Additional info
} MinimalElfSection;

typedef struct {
    MinimalElfSection sections[16];
    int section_count;
    MinimalElfSymbol symbols[32];
    int symbol_count;
    MinimalElfRelocation relocations[64];
    int relocation_count;
} MinimalElfRelocationContext;

// ELF constants
#define SHT_NULL 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_REL 9
#define SHT_DYNSYM 11

#define STT_NOTYPE 0
#define STT_OBJECT 1
#define STT_FUNC 2
#define STT_SECTION 3
#define STT_FILE 4

#define STB_LOCAL 0
#define STB_GLOBAL 1
#define STB_WEAK 2

// Relocation types (x86_64)
#define R_X86_64_NONE 0
#define R_X86_64_64 1
#define R_X86_64_PC32 2
#define R_X86_64_GOT32 3
#define R_X86_64_PLT32 4
#define R_X86_64_32 10
#define R_X86_64_32S 11

// Helper macros
#define ELF64_R_SYM(info) ((info) >> 32)
#define ELF64_R_TYPE(info) ((info) & 0xffffffff)
#define ELF64_R_INFO(sym, type) (((uint64_t)(sym) << 32) + ((type) & 0xffffffff))

// Helper functions
static void init_relocation_context(MinimalElfRelocationContext *ctx) {
    ctx->section_count = 0;
    ctx->symbol_count = 0;
    ctx->relocation_count = 0;

    for (int i = 0; i < 16; i++) {
        ctx->sections[i] = (MinimalElfSection){0};
    }

    for (int i = 0; i < 32; i++) {
        ctx->symbols[i] = (MinimalElfSymbol){0};
    }

    for (int i = 0; i < 64; i++) {
        ctx->relocations[i] = (MinimalElfRelocation){0};
    }
}

static void add_section(MinimalElfRelocationContext *ctx, const char *name, uint32_t type,
                        uint64_t size) {
    if (ctx->section_count < 16) {
        MinimalElfSection *section = &ctx->sections[ctx->section_count++];
        section->name = name;
        section->type = type;
        section->size = size;
        section->offset = ctx->section_count * 1000;
        section->addr = 0x400000 + (ctx->section_count * 0x1000);
    }
}

static void add_symbol(MinimalElfRelocationContext *ctx, const char *name, uint64_t value,
                       uint8_t type, uint8_t bind, uint16_t section) {
    if (ctx->symbol_count < 32) {
        MinimalElfSymbol *symbol = &ctx->symbols[ctx->symbol_count++];
        symbol->name = name;
        symbol->value = value;
        symbol->size = (type == STT_FUNC) ? 32 : 8; // Default sizes
        symbol->info = (bind << 4) | (type & 0xf);
        symbol->other = 0;
        symbol->shndx = section;
    }
}

static void add_relocation(MinimalElfRelocationContext *ctx, uint64_t offset, uint32_t symbol_idx,
                           uint32_t type, int64_t addend) {
    if (ctx->relocation_count < 64) {
        MinimalElfRelocation *reloc = &ctx->relocations[ctx->relocation_count++];
        reloc->offset = offset;
        reloc->info = ELF64_R_INFO(symbol_idx, type);
        reloc->addend = addend;
    }
}

static bool validate_relocation_sections(const MinimalElfRelocationContext *ctx) {
    bool has_symtab = false;
    bool has_strtab = false;
    bool has_rela = false;

    for (int i = 0; i < ctx->section_count; i++) {
        const MinimalElfSection *section = &ctx->sections[i];

        if (section->type == SHT_SYMTAB) {
            has_symtab = true;
        } else if (section->type == SHT_STRTAB) {
            has_strtab = true;
        } else if (section->type == SHT_RELA || section->type == SHT_REL) {
            has_rela = true;
        }
    }

    return has_symtab && has_strtab && has_rela;
}

// Test functions
static AsthraTestResult test_relocation_section_creation(AsthraTestContext *context) {
    MinimalElfRelocationContext ctx;
    init_relocation_context(&ctx);

    // Create essential sections for relocation
    add_section(&ctx, ".text", SHT_PROGBITS, 1024);
    add_section(&ctx, ".data", SHT_PROGBITS, 512);
    add_section(&ctx, ".symtab", SHT_SYMTAB, 256);
    add_section(&ctx, ".strtab", SHT_STRTAB, 128);
    add_section(&ctx, ".rela.text", SHT_RELA, 192);
    add_section(&ctx, ".rela.data", SHT_RELA, 64);

    // Verify section creation
    if (ctx.section_count != 6) {
        return ASTHRA_TEST_FAIL;
    }

    // Validate that we have the required sections
    if (!validate_relocation_sections(&ctx)) {
        return ASTHRA_TEST_FAIL;
    }

    // Check section properties
    for (int i = 0; i < ctx.section_count; i++) {
        const MinimalElfSection *section = &ctx.sections[i];

        // All sections should have valid names and addresses
        if (!section->name || section->addr == 0) {
            return ASTHRA_TEST_FAIL;
        }

        // Relocation sections should have appropriate sizes
        if (section->type == SHT_RELA || section->type == SHT_REL) {
            if (section->size == 0) {
                return ASTHRA_TEST_FAIL;
            }
            // RELA sections should be larger than REL sections (due to addend)
            if (section->type == SHT_RELA && section->size < 24) {
                return ASTHRA_TEST_FAIL;
            }
        }

        // Symbol table should have reasonable size
        if (section->type == SHT_SYMTAB && section->size < 100) {
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_symbol_relocation(AsthraTestContext *context) {
    MinimalElfRelocationContext ctx;
    init_relocation_context(&ctx);

    // Create sections
    add_section(&ctx, ".text", SHT_PROGBITS, 1024);
    add_section(&ctx, ".data", SHT_PROGBITS, 512);
    add_section(&ctx, ".symtab", SHT_SYMTAB, 256);
    add_section(&ctx, ".rela.text", SHT_RELA, 192);

    // Add symbols
    add_symbol(&ctx, "", 0, STT_NOTYPE, STB_LOCAL, 0); // NULL symbol
    add_symbol(&ctx, "main", 0x401000, STT_FUNC, STB_GLOBAL, 1);
    add_symbol(&ctx, "data_var", 0x402000, STT_OBJECT, STB_GLOBAL, 2);
    add_symbol(&ctx, "helper_func", 0x401100, STT_FUNC, STB_LOCAL, 1);
    add_symbol(&ctx, "external_func", 0, STT_FUNC, STB_GLOBAL, 0); // Undefined

    // Add relocations referencing these symbols
    add_relocation(&ctx, 0x401010, 4, R_X86_64_PLT32, -4); // Call to external_func
    add_relocation(&ctx, 0x401020, 2, R_X86_64_PC32, -4);  // Reference to data_var
    add_relocation(&ctx, 0x401030, 3, R_X86_64_PC32, -4);  // Call to helper_func

    // Verify symbols were created
    if (ctx.symbol_count != 5) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify relocations were created
    if (ctx.relocation_count != 3) {
        return ASTHRA_TEST_FAIL;
    }

    // Check symbol properties
    for (int i = 0; i < ctx.symbol_count; i++) {
        const MinimalElfSymbol *symbol = &ctx.symbols[i];

        // First symbol should be NULL
        if (i == 0) {
            if (symbol->name && strlen(symbol->name) > 0) {
                return ASTHRA_TEST_FAIL;
            }
            continue;
        }

        // Other symbols should have names
        if (!symbol->name || strlen(symbol->name) == 0) {
            return ASTHRA_TEST_FAIL;
        }

        // Functions should have function type
        if (strstr(symbol->name, "func") && ((symbol->info & 0xf) != STT_FUNC)) {
            return ASTHRA_TEST_FAIL;
        }

        // Global symbols should have global binding
        if (strcmp(symbol->name, "main") == 0 || strcmp(symbol->name, "data_var") == 0) {
            if ((symbol->info >> 4) != STB_GLOBAL) {
                return ASTHRA_TEST_FAIL;
            }
        }
    }

    // Check relocation properties
    for (int i = 0; i < ctx.relocation_count; i++) {
        const MinimalElfRelocation *reloc = &ctx.relocations[i];

        // Symbol index should be valid
        uint32_t sym_idx = ELF64_R_SYM(reloc->info);
        if (sym_idx >= ctx.symbol_count) {
            return ASTHRA_TEST_FAIL;
        }

        // Relocation type should be valid
        uint32_t type = ELF64_R_TYPE(reloc->info);
        if (type != R_X86_64_PLT32 && type != R_X86_64_PC32) {
            return ASTHRA_TEST_FAIL;
        }

        // Offset should be in text section range
        if (reloc->offset < 0x401000 || reloc->offset > 0x402000) {
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_dynamic_relocations(AsthraTestContext *context) {
    MinimalElfRelocationContext ctx;
    init_relocation_context(&ctx);

    // Create sections for dynamic linking
    add_section(&ctx, ".text", SHT_PROGBITS, 1024);
    add_section(&ctx, ".got", SHT_PROGBITS, 64);
    add_section(&ctx, ".plt", SHT_PROGBITS, 128);
    add_section(&ctx, ".dynsym", SHT_DYNSYM, 256);
    add_section(&ctx, ".dynstr", SHT_STRTAB, 128);
    add_section(&ctx, ".rela.dyn", SHT_RELA, 192);
    add_section(&ctx, ".rela.plt", SHT_RELA, 96);

    // Add dynamic symbols (external functions)
    add_symbol(&ctx, "", 0, STT_NOTYPE, STB_LOCAL, 0);      // NULL symbol
    add_symbol(&ctx, "printf", 0, STT_FUNC, STB_GLOBAL, 0); // libc function
    add_symbol(&ctx, "malloc", 0, STT_FUNC, STB_GLOBAL, 0); // libc function
    add_symbol(&ctx, "strlen", 0, STT_FUNC, STB_GLOBAL, 0); // libc function

    // Add PLT relocations (procedure linkage table)
    add_relocation(&ctx, 0x401010, 1, R_X86_64_PLT32, -4); // Call to printf
    add_relocation(&ctx, 0x401020, 2, R_X86_64_PLT32, -4); // Call to malloc

    // Add GOT relocations (global offset table)
    add_relocation(&ctx, 0x600018, 1, R_X86_64_GOT32, 0); // printf GOT entry
    add_relocation(&ctx, 0x600020, 2, R_X86_64_GOT32, 0); // malloc GOT entry

    // Verify dynamic sections exist
    bool has_dynsym = false, has_dynstr = false, has_rela_dyn = false, has_rela_plt = false;
    bool has_got = false, has_plt = false;

    for (int i = 0; i < ctx.section_count; i++) {
        const MinimalElfSection *section = &ctx.sections[i];

        if (section->type == SHT_DYNSYM)
            has_dynsym = true;
        if (section->type == SHT_STRTAB && strstr(section->name, "dynstr"))
            has_dynstr = true;
        if (section->type == SHT_RELA && strstr(section->name, ".rela.dyn"))
            has_rela_dyn = true;
        if (section->type == SHT_RELA && strstr(section->name, ".rela.plt"))
            has_rela_plt = true;
        if (strcmp(section->name, ".got") == 0)
            has_got = true;
        if (strcmp(section->name, ".plt") == 0)
            has_plt = true;
    }

    if (!has_dynsym || !has_dynstr || !has_rela_dyn || !has_rela_plt || !has_got || !has_plt) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify dynamic symbols
    if (ctx.symbol_count != 4) {
        return ASTHRA_TEST_FAIL;
    }

    // Check that external functions are undefined (value = 0, section = 0)
    for (int i = 1; i < ctx.symbol_count; i++) {
        const MinimalElfSymbol *symbol = &ctx.symbols[i];
        if (symbol->value != 0 || symbol->shndx != 0) {
            return ASTHRA_TEST_FAIL;
        }
        if ((symbol->info & 0xf) != STT_FUNC) {
            return ASTHRA_TEST_FAIL;
        }
    }

    // Verify relocations
    if (ctx.relocation_count != 4) {
        return ASTHRA_TEST_FAIL;
    }

    // Check relocation types
    int plt_count = 0, got_count = 0;
    for (int i = 0; i < ctx.relocation_count; i++) {
        uint32_t type = ELF64_R_TYPE(ctx.relocations[i].info);
        if (type == R_X86_64_PLT32)
            plt_count++;
        if (type == R_X86_64_GOT32)
            got_count++;
    }

    if (plt_count != 2 || got_count != 2) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_relocation_validation(AsthraTestContext *context) {
    MinimalElfRelocationContext ctx;
    init_relocation_context(&ctx);

    // Create a complete relocation scenario
    add_section(&ctx, ".text", SHT_PROGBITS, 1024);
    add_section(&ctx, ".data", SHT_PROGBITS, 512);
    add_section(&ctx, ".symtab", SHT_SYMTAB, 256);
    add_section(&ctx, ".strtab", SHT_STRTAB, 128);
    add_section(&ctx, ".rela.text", SHT_RELA, 192);

    // Add symbols with various types and bindings
    add_symbol(&ctx, "", 0, STT_NOTYPE, STB_LOCAL, 0); // NULL symbol
    add_symbol(&ctx, "local_func", 0x401000, STT_FUNC, STB_LOCAL, 1);
    add_symbol(&ctx, "global_func", 0x401100, STT_FUNC, STB_GLOBAL, 1);
    add_symbol(&ctx, "weak_func", 0x401200, STT_FUNC, STB_WEAK, 1);
    add_symbol(&ctx, "global_var", 0x402000, STT_OBJECT, STB_GLOBAL, 2);
    add_symbol(&ctx, "extern_func", 0, STT_FUNC, STB_GLOBAL, 0); // Undefined

    // Add various relocation types
    add_relocation(&ctx, 0x401010, 2, R_X86_64_PC32, -4);  // Call to global_func
    add_relocation(&ctx, 0x401020, 4, R_X86_64_PC32, 0);   // Reference to global_var
    add_relocation(&ctx, 0x401030, 5, R_X86_64_PLT32, -4); // Call to extern_func
    add_relocation(&ctx, 0x401040, 3, R_X86_64_PC32, -4);  // Call to weak_func

    // Validation 1: All relocations should reference valid symbols
    for (int i = 0; i < ctx.relocation_count; i++) {
        uint32_t sym_idx = ELF64_R_SYM(ctx.relocations[i].info);
        if (sym_idx >= ctx.symbol_count) {
            return ASTHRA_TEST_FAIL;
        }
    }

    // Validation 2: Symbol table should have proper structure
    // First symbol should be NULL
    if (ctx.symbols[0].name && strlen(ctx.symbols[0].name) > 0) {
        return ASTHRA_TEST_FAIL;
    }

    // Validation 3: Check symbol binding consistency
    for (int i = 1; i < ctx.symbol_count; i++) {
        const MinimalElfSymbol *symbol = &ctx.symbols[i];
        uint8_t binding = symbol->info >> 4;

        // Global symbols should be globally visible
        if (binding == STB_GLOBAL && symbol->shndx == 0) {
            // Undefined global symbol - should have value 0
            if (symbol->value != 0) {
                return ASTHRA_TEST_FAIL;
            }
        } else if (binding == STB_GLOBAL && symbol->shndx != 0) {
            // Defined global symbol - should have non-zero value
            if (symbol->value == 0) {
                return ASTHRA_TEST_FAIL;
            }
        }

        // Local symbols should be in defined sections
        if (binding == STB_LOCAL && symbol->shndx == 0 && symbol->value != 0) {
            return ASTHRA_TEST_FAIL;
        }
    }

    // Validation 4: Relocation offsets should be properly aligned
    for (int i = 0; i < ctx.relocation_count; i++) {
        const MinimalElfRelocation *reloc = &ctx.relocations[i];
        uint32_t type = ELF64_R_TYPE(reloc->info);

        // PC-relative relocations should have proper alignment
        if (type == R_X86_64_PC32 || type == R_X86_64_PLT32) {
            if (reloc->offset % 4 != 0) {
                return ASTHRA_TEST_FAIL;
            }
        }

        // Verify offsets are within section bounds
        if (reloc->offset < 0x401000 || reloc->offset > 0x401100) {
            return ASTHRA_TEST_FAIL;
        }
    }

    // Validation 5: Section linking should be correct
    for (int i = 0; i < ctx.section_count; i++) {
        const MinimalElfSection *section = &ctx.sections[i];

        if (section->type == SHT_RELA) {
            // Relocation section should link to symbol table
            // In our simplified model, we don't set link, but we can verify structure
            if (section->size % 24 != 0) { // RELA entries are 24 bytes
                return ASTHRA_TEST_FAIL;
            }
        }
    }

    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    printf("=== ELF Relocation Handling Tests (Minimal Framework) ===\n");

    AsthraTestContext context = {0};
    int passed = 0, total = 0;

    // Run tests
    if (test_relocation_section_creation(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Relocation Section Creation: PASS\n");
        passed++;
    } else {
        printf("❌ Relocation Section Creation: FAIL\n");
    }
    total++;

    if (test_symbol_relocation(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Symbol Relocation: PASS\n");
        passed++;
    } else {
        printf("❌ Symbol Relocation: FAIL\n");
    }
    total++;

    if (test_dynamic_relocations(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Dynamic Relocations: PASS\n");
        passed++;
    } else {
        printf("❌ Dynamic Relocations: FAIL\n");
    }
    total++;

    if (test_relocation_validation(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Relocation Validation: PASS\n");
        passed++;
    } else {
        printf("❌ Relocation Validation: FAIL\n");
    }
    total++;

    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);

    return (passed == total) ? 0 : 1;
}
