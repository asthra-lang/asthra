#include "../framework/test_framework_minimal.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_debug_section_creation", __FILE__, __LINE__, "Test debug section creation and structure",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_debug_info_generation", __FILE__, __LINE__, "Test debug info generation",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_debug_line_numbers", __FILE__, __LINE__, "Test debug line number information",
     ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
    {"test_debug_variable_info", __FILE__, __LINE__, "Test debug variable information",
     ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}};

// Minimal ELF debug section structures
typedef struct {
    const char *name;
    uint32_t type;      // Section type (SHT_PROGBITS, etc.)
    uint64_t flags;     // Section flags
    uint64_t offset;    // File offset
    uint64_t size;      // Section size
    uint32_t info;      // Additional info
    uint32_t link;      // Link to other section
    uint64_t addralign; // Address alignment
} MinimalElfSection;

typedef struct {
    uint32_t line_number;
    uint64_t address;
    const char *file_name;
    uint32_t column;
} MinimalDebugLine;

typedef struct {
    const char *name;
    uint64_t address;
    uint32_t type;        // Variable type encoding
    uint32_t scope_start; // Start of scope
    uint32_t scope_end;   // End of scope
} MinimalDebugVariable;

typedef struct {
    MinimalElfSection sections[16];
    int section_count;
    MinimalDebugLine lines[32];
    int line_count;
    MinimalDebugVariable variables[16];
    int variable_count;
} MinimalElfDebugContext;

// ELF section types (simplified)
#define SHT_NULL 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_NOBITS 8

// Debug section flags
#define SHF_ALLOC 0x2
#define SHF_EXECINSTR 0x4

// Helper functions
static void init_debug_context(MinimalElfDebugContext *ctx) {
    ctx->section_count = 0;
    ctx->line_count = 0;
    ctx->variable_count = 0;

    for (int i = 0; i < 16; i++) {
        ctx->sections[i] = (MinimalElfSection){0};
        ctx->variables[i] = (MinimalDebugVariable){0};
    }

    for (int i = 0; i < 32; i++) {
        ctx->lines[i] = (MinimalDebugLine){0};
    }
}

static void add_debug_section(MinimalElfDebugContext *ctx, const char *name, uint32_t type,
                              uint64_t size) {
    if (ctx->section_count < 16) {
        MinimalElfSection *section = &ctx->sections[ctx->section_count++];
        section->name = name;
        section->type = type;
        section->size = size;
        section->offset = ctx->section_count * 1000; // Simulate file offsets
        section->addralign = 1;

        // Set appropriate flags for debug sections
        if (strstr(name, ".debug_") == name) {
            section->flags = 0; // Debug sections typically have no special flags
        } else if (strcmp(name, ".text") == 0) {
            section->flags = SHF_ALLOC | SHF_EXECINSTR;
        } else {
            section->flags = SHF_ALLOC;
        }
    }
}

static void add_debug_line(MinimalElfDebugContext *ctx, uint32_t line, uint64_t addr,
                           const char *file) {
    if (ctx->line_count < 32) {
        MinimalDebugLine *debug_line = &ctx->lines[ctx->line_count++];
        debug_line->line_number = line;
        debug_line->address = addr;
        debug_line->file_name = file;
        debug_line->column = 1; // Default column
    }
}

static void add_debug_variable(MinimalElfDebugContext *ctx, const char *name, uint64_t addr,
                               uint32_t type) {
    if (ctx->variable_count < 16) {
        MinimalDebugVariable *var = &ctx->variables[ctx->variable_count++];
        var->name = name;
        var->address = addr;
        var->type = type;
        var->scope_start = 0;
        var->scope_end = 1000; // Default scope
    }
}

static bool validate_debug_sections(const MinimalElfDebugContext *ctx) {
    bool has_debug_info = false;
    bool has_debug_line = false;
    bool has_debug_str = false;

    for (int i = 0; i < ctx->section_count; i++) {
        const MinimalElfSection *section = &ctx->sections[i];

        if (strcmp(section->name, ".debug_info") == 0) {
            has_debug_info = true;
            if (section->type != SHT_PROGBITS || section->size == 0) {
                return false;
            }
        } else if (strcmp(section->name, ".debug_line") == 0) {
            has_debug_line = true;
            if (section->type != SHT_PROGBITS || section->size == 0) {
                return false;
            }
        } else if (strcmp(section->name, ".debug_str") == 0) {
            has_debug_str = true;
            if (section->type != SHT_PROGBITS) {
                return false;
            }
        }
    }

    return has_debug_info && has_debug_line && has_debug_str;
}

// Test functions using minimal framework approach
static AsthraTestResult test_debug_section_creation(AsthraTestContext *context) {
    MinimalElfDebugContext ctx;
    init_debug_context(&ctx);

    // Create essential debug sections
    add_debug_section(&ctx, ".debug_info", SHT_PROGBITS, 256);
    add_debug_section(&ctx, ".debug_line", SHT_PROGBITS, 128);
    add_debug_section(&ctx, ".debug_str", SHT_PROGBITS, 64);
    add_debug_section(&ctx, ".debug_abbrev", SHT_PROGBITS, 32);

    // Add some regular sections for context
    add_debug_section(&ctx, ".text", SHT_PROGBITS, 1024);
    add_debug_section(&ctx, ".data", SHT_PROGBITS, 512);

    // Validate section creation
    if (ctx.section_count != 6) {
        return ASTHRA_TEST_FAIL;
    }

    // Check that debug sections were created correctly
    if (!validate_debug_sections(&ctx)) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify section properties
    for (int i = 0; i < ctx.section_count; i++) {
        const MinimalElfSection *section = &ctx.sections[i];

        // All sections should have valid names and sizes
        if (!section->name || section->size == 0) {
            return ASTHRA_TEST_FAIL;
        }

        // Debug sections should have no special flags
        if (strstr(section->name, ".debug_") == section->name) {
            if (section->flags != 0) {
                return ASTHRA_TEST_FAIL;
            }
        }

        // Text section should be executable
        if (strcmp(section->name, ".text") == 0) {
            if (!(section->flags & SHF_EXECINSTR)) {
                return ASTHRA_TEST_FAIL;
            }
        }
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_debug_info_generation(AsthraTestContext *context) {
    MinimalElfDebugContext ctx;
    init_debug_context(&ctx);

    // Create debug sections with simulated DWARF info
    add_debug_section(&ctx, ".debug_info", SHT_PROGBITS, 512);
    add_debug_section(&ctx, ".debug_abbrev", SHT_PROGBITS, 128);
    add_debug_section(&ctx, ".debug_str", SHT_PROGBITS, 256);

    // Simulate debug info content by checking sizes and relationships
    MinimalElfSection *debug_info = &ctx.sections[0];
    MinimalElfSection *debug_abbrev = &ctx.sections[1];
    MinimalElfSection *debug_str = &ctx.sections[2];

    // Debug info should be the largest section
    if (debug_info->size <= debug_abbrev->size || debug_info->size <= debug_str->size) {
        return ASTHRA_TEST_FAIL;
    }

    // Set up proper section linking (debug_info links to debug_abbrev)
    debug_info->link = 1; // Index of debug_abbrev section

    // Verify linking is correct
    if (debug_info->link != 1) {
        return ASTHRA_TEST_FAIL;
    }

    // Simulate compilation unit information
    // In real DWARF, this would contain type information, function definitions, etc.
    uint32_t compilation_units = 2;
    uint32_t estimated_size = compilation_units * 200; // Estimate 200 bytes per CU

    if (debug_info->size < estimated_size) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_debug_line_numbers(AsthraTestContext *context) {
    MinimalElfDebugContext ctx;
    init_debug_context(&ctx);

    // Create debug line section
    add_debug_section(&ctx, ".debug_line", SHT_PROGBITS, 256);

    // Add line number information for a simple function
    add_debug_line(&ctx, 1, 0x1000, "main.c");
    add_debug_line(&ctx, 2, 0x1008, "main.c");
    add_debug_line(&ctx, 3, 0x1010, "main.c");
    add_debug_line(&ctx, 5, 0x1020, "main.c");   // Line 4 was empty
    add_debug_line(&ctx, 1, 0x1030, "helper.c"); // Different file

    // Verify line information was recorded
    if (ctx.line_count != 5) {
        return ASTHRA_TEST_FAIL;
    }

    // Check line number progression for main.c
    bool main_c_lines_valid = true;
    uint64_t last_addr = 0;

    for (int i = 0; i < ctx.line_count; i++) {
        const MinimalDebugLine *line = &ctx.lines[i];

        if (strcmp(line->file_name, "main.c") == 0) {
            // Addresses should be increasing
            if (line->address <= last_addr && last_addr != 0) {
                main_c_lines_valid = false;
            }
            last_addr = line->address;

            // Line numbers should be reasonable
            if (line->line_number == 0 || line->line_number > 1000) {
                main_c_lines_valid = false;
            }
        }
    }

    if (!main_c_lines_valid) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify we have entries for both files
    bool has_main_c = false, has_helper_c = false;
    for (int i = 0; i < ctx.line_count; i++) {
        if (strcmp(ctx.lines[i].file_name, "main.c") == 0)
            has_main_c = true;
        if (strcmp(ctx.lines[i].file_name, "helper.c") == 0)
            has_helper_c = true;
    }

    if (!has_main_c || !has_helper_c) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_debug_variable_info(AsthraTestContext *context) {
    MinimalElfDebugContext ctx;
    init_debug_context(&ctx);

    // Create debug sections for variable information
    add_debug_section(&ctx, ".debug_info", SHT_PROGBITS, 512);
    add_debug_section(&ctx, ".debug_loc", SHT_PROGBITS, 128);

    // Add variable debug information
    add_debug_variable(&ctx, "x", 0x1000, 1);       // Type 1 = int
    add_debug_variable(&ctx, "y", 0x1008, 1);       // Type 1 = int
    add_debug_variable(&ctx, "result", 0x1010, 1);  // Type 1 = int
    add_debug_variable(&ctx, "message", 0x1020, 2); // Type 2 = string

    // Verify variable information
    if (ctx.variable_count != 4) {
        return ASTHRA_TEST_FAIL;
    }

    // Check variable properties
    for (int i = 0; i < ctx.variable_count; i++) {
        const MinimalDebugVariable *var = &ctx.variables[i];

        // All variables should have names and valid addresses
        if (!var->name || var->address == 0) {
            return ASTHRA_TEST_FAIL;
        }

        // Addresses should be in reasonable range
        if (var->address < 0x1000 || var->address > 0x2000) {
            return ASTHRA_TEST_FAIL;
        }

        // Types should be valid
        if (var->type == 0 || var->type > 10) {
            return ASTHRA_TEST_FAIL;
        }

        // Scope should be valid
        if (var->scope_end <= var->scope_start) {
            return ASTHRA_TEST_FAIL;
        }
    }

    // Verify specific variables exist
    bool has_x = false, has_message = false;
    for (int i = 0; i < ctx.variable_count; i++) {
        if (strcmp(ctx.variables[i].name, "x") == 0)
            has_x = true;
        if (strcmp(ctx.variables[i].name, "message") == 0)
            has_message = true;
    }

    if (!has_x || !has_message) {
        return ASTHRA_TEST_FAIL;
    }

    // Check that different types are used
    bool has_type_1 = false, has_type_2 = false;
    for (int i = 0; i < ctx.variable_count; i++) {
        if (ctx.variables[i].type == 1)
            has_type_1 = true;
        if (ctx.variables[i].type == 2)
            has_type_2 = true;
    }

    if (!has_type_1 || !has_type_2) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    printf("=== ELF Debug Sections Tests (Minimal Framework) ===\n");

    AsthraTestContext context = {0}; // Initialize to zero
    int passed = 0, total = 0;

    // Run tests
    if (test_debug_section_creation(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Debug Section Creation: PASS\n");
        passed++;
    } else {
        printf("❌ Debug Section Creation: FAIL\n");
    }
    total++;

    if (test_debug_info_generation(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Debug Info Generation: PASS\n");
        passed++;
    } else {
        printf("❌ Debug Info Generation: FAIL\n");
    }
    total++;

    if (test_debug_line_numbers(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Debug Line Numbers: PASS\n");
        passed++;
    } else {
        printf("❌ Debug Line Numbers: FAIL\n");
    }
    total++;

    if (test_debug_variable_info(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Debug Variable Info: PASS\n");
        passed++;
    } else {
        printf("❌ Debug Variable Info: FAIL\n");
    }
    total++;

    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);

    return (passed == total) ? 0 : 1;
}
