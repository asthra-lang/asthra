/**
 * Asthra Programming Language Compiler
 * ELF Optimization Functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Functions for optimizing ELF metadata and testing C linkage.
 */

#include "elf_writer.h"
#include "elf_writer_core.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// No external declarations needed - functions are in elf_writer.h

// =============================================================================
// OPTIMIZATION FUNCTIONS
// =============================================================================

bool elf_optimize_metadata(ELFWriter *writer) {
    if (!writer) return false;
    
    // Optimize Asthra metadata sections for size and access patterns
    for (size_t i = 0; i < writer->section_count; i++) {
        ELFSection *section = writer->sections[i];
        
        if (section->is_asthra_section && section->data_size > 0) {
            // Compress metadata if beneficial
            // This is a placeholder for actual compression logic
            
            // Align data for optimal access
            if (section->data_size % 8 != 0) {
                size_t aligned_size = (section->data_size + (size_t)7) & ~(size_t)7;
                if (elf_expand_section_data(section, aligned_size)) {
                    // Zero-fill padding
                    memset((char*)section->data + section->data_size, 0, 
                           aligned_size - section->data_size);
                    section->data_size = aligned_size;
                }
            }
        }
    }
    
    return true;
}

bool elf_test_c_linkage(const char *elf_filename, const char *test_c_file) {
    if (!elf_filename || !test_c_file) return false;
    
    // Create a simple test C file if not provided
    const char *test_code = 
        "#include <stdio.h>\n"
        "extern int _Asthra_init(void);\n"
        "int main() {\n"
        "    if (_Asthra_init()) {\n"
        "        printf(\"Asthra runtime initialized successfully\\n\");\n"
        "        return 0;\n"
        "    } else {\n"
        "        printf(\"Asthra runtime initialization failed\\n\");\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    // Write test file
    FILE *test_file = fopen("test_linkage.c", "w");
    if (!test_file) return false;
    
    fprintf(test_file, "%s", test_code);
    fclose(test_file);
    
    // Try to compile and link
    char command[512];
    snprintf(command, sizeof(command), 
             "gcc -o test_linkage test_linkage.c %s 2>/dev/null", elf_filename);
    
    int result = system(command);
    
    // Clean up
    unlink("test_linkage.c");
    unlink("test_linkage");
    
    if (result == 0) {
        printf("C linkage test passed\n");
        return true;
    } else {
        printf("C linkage test failed\n");
        return false;
    }
}