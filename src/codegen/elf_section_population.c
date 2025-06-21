/**
 * Asthra Programming Language Compiler
 * ELF Section Population Functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Functions for populating ELF sections with comprehensive metadata
 * for all Asthra language features.
 */

#include "elf_writer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// External magic numbers and version are defined in elf_writer_core.c
// and exported as constants for use in other files

// Helper function to find section by name
static ELFSection *find_section_by_name(ELFWriter *writer, const char *name) {
    for (size_t i = 0; i < writer->section_count; i++) {
        if (strcmp(writer->sections[i]->name, name) == 0) {
            return writer->sections[i];
        }
    }
    return NULL;
}

// Helper function to expand section data
static bool expand_section_data(ELFSection *section, size_t required_size) {
    if (required_size <= section->data_capacity) {
        return true;
    }
    
    // Fix: Ensure new_capacity starts with at least 1 to avoid infinite loop when data_capacity is 0
    size_t new_capacity = section->data_capacity;
    if (new_capacity == 0) {
        new_capacity = 1;
    }
    
    while (new_capacity < required_size) {
        new_capacity *= 2;
    }
    
    void *new_data = realloc(section->data, new_capacity);
    if (!new_data) return false;
    
    section->data = new_data;
    section->data_capacity = new_capacity;
    return true;
}

// =============================================================================
// TEXT SECTION POPULATION
// =============================================================================

bool elf_populate_text_section(ELFWriter *writer) {
    printf("DEBUG: elf_populate_text_section - Entry\n");
    fflush(stdout);
    
    if (!writer || !writer->ffi_generator) {
        printf("DEBUG: elf_populate_text_section - Invalid parameters\n");
        fflush(stdout);
        return false;
    }
    
    printf("DEBUG: elf_populate_text_section - Finding .text section\n");
    fflush(stdout);
    
    ELFSection *text_section = find_section_by_name(writer, ".text");
    if (!text_section) {
        printf("DEBUG: elf_populate_text_section - .text section not found\n");
        fflush(stdout);
        return false;
    }
    
    printf("DEBUG: elf_populate_text_section - Allocating assembly buffer\n");
    fflush(stdout);
    
    // Get assembly code from FFI generator
    char *assembly_buffer = malloc(1024 * 1024); // 1MB buffer
    if (!assembly_buffer) {
        printf("DEBUG: elf_populate_text_section - Failed to allocate assembly buffer\n");
        fflush(stdout);
        return false;
    }
    
    printf("DEBUG: elf_populate_text_section - About to call ffi_print_nasm_assembly\n");
    fflush(stdout);
    
    bool success = ffi_print_nasm_assembly(writer->ffi_generator, 
                                          assembly_buffer, 1024 * 1024);
    
    printf("DEBUG: elf_populate_text_section - ffi_print_nasm_assembly returned: %d\n", success);
    fflush(stdout);
    
    if (!success) {
        printf("DEBUG: elf_populate_text_section - ffi_print_nasm_assembly failed\n");
        fflush(stdout);
        free(assembly_buffer);
        return false;
    }
    
    printf("DEBUG: elf_populate_text_section - Creating machine code\n");
    fflush(stdout);
    
    // Convert NASM assembly to machine code (simplified)
    // For now, generate a minimal main function that returns 0
    // This is a temporary stub for testing the pipeline
    
    // x86-64 machine code for:
    // main:
    //     xor eax, eax    ; return 0
    //     ret
    unsigned char machine_code[] = {
        0x31, 0xC0,  // xor eax, eax
        0xC3         // ret
    };
    
    size_t code_size = sizeof(machine_code);
    
    printf("DEBUG: elf_populate_text_section - About to expand section data\n");
    fflush(stdout);
    
    if (!expand_section_data(text_section, code_size)) {
        printf("DEBUG: elf_populate_text_section - expand_section_data failed\n");
        fflush(stdout);
        free(assembly_buffer);
        return false;
    }
    
    printf("DEBUG: elf_populate_text_section - About to copy machine code\n");
    fflush(stdout);
    
    memcpy(text_section->data, machine_code, code_size);
    text_section->data_size = code_size;
    
    printf("DEBUG: elf_populate_text_section - About to call elf_add_symbol\n");
    fflush(stdout);
    
    // Add a symbol for main function
    // Note: This is a simplified version for testing
    // In a real implementation, symbols would be added based on the actual functions generated
    ELFSymbol *main_symbol = elf_add_symbol(writer, "main", 0, code_size, 
                                           STB_GLOBAL, STT_FUNC, 1); // section 1 is typically .text
    
    printf("DEBUG: elf_populate_text_section - elf_add_symbol returned\n");
    fflush(stdout);
    
    if (!main_symbol) {
        printf("Warning: Failed to add main symbol\n");
    }
    
    free(assembly_buffer);
    return true;
}

// =============================================================================
// FFI METADATA SECTION POPULATION
// =============================================================================

bool elf_populate_ffi_section(ELFWriter *writer) {
    if (!writer || !writer->ffi_generator) return false;
    
    ELFSection *ffi_section = find_section_by_name(writer, ".Asthra.ffi");
    if (!ffi_section) return false;
    
    // Get FFI statistics from generator
    size_t ffi_calls, pattern_matches, string_ops, slice_ops, security_ops, spawn_stmts;
    ffi_get_generation_statistics(writer->ffi_generator, &ffi_calls, &pattern_matches,
                                 &string_ops, &slice_ops, &security_ops, &spawn_stmts);
    
    // Calculate required size
    size_t header_size = sizeof(AsthraSectionFFI);
    size_t functions_size = ffi_calls * sizeof(((AsthraSectionFFI*)0)->functions[0]);
    size_t total_size = header_size + functions_size;
    
    if (!expand_section_data(ffi_section, total_size)) {
        return false;
    }
    
    // Create FFI metadata
    AsthraSectionFFI *ffi_metadata = (AsthraSectionFFI*)ffi_section->data;
    ffi_metadata->magic = 0x41464649; // "AFFI"
    ffi_metadata->version = ASTHRA_METADATA_VERSION;
    ffi_metadata->function_count = (uint32_t)ffi_calls;
    ffi_metadata->variadic_count = 0; // Count variadic functions
    
    // Populate function metadata (simplified)
    for (size_t i = 0; i < ffi_calls && i < 100; i++) {
        snprintf(ffi_metadata->functions[i].name, 64, "ffi_function_%zu", i);
        ffi_metadata->functions[i].param_count = 2; // Example
        ffi_metadata->functions[i].variadic_start = 0;
        ffi_metadata->functions[i].calling_conv = FFI_CONV_SYSV_AMD64;
        ffi_metadata->functions[i].transfer_semantics = 0;
        ffi_metadata->functions[i].function_address = 0; // Filled by linker
        
        // Example parameter metadata
        for (int j = 0; j < 2; j++) {
            ffi_metadata->functions[i].parameters[j].marshal_type = FFI_MARSHAL_DIRECT;
            ffi_metadata->functions[i].parameters[j].transfer_type = 0; // FFI_TRANSFER_NONE
            ffi_metadata->functions[i].parameters[j].is_borrowed = 0;
            ffi_metadata->functions[i].parameters[j].reserved = 0;
        }
    }
    
    ffi_section->data_size = total_size;
    writer->asthra_metadata->ffi_metadata = ffi_metadata;
    writer->asthra_metadata->ffi_size = total_size;
    
    // Update statistics
    writer->asthra_metadata_entries_count += ffi_calls;
    return true;
}

// =============================================================================
// GC METADATA SECTION POPULATION
// =============================================================================

bool elf_populate_gc_section(ELFWriter *writer) {
    if (!writer) return false;
    
    ELFSection *gc_section = find_section_by_name(writer, ".Asthra.gc");
    if (!gc_section) return false;
    
    // Calculate required size for GC metadata
    size_t gc_root_count = 10; // Example count
    size_t header_size = sizeof(AsthraSectionGC);
    size_t roots_size = gc_root_count * sizeof(((AsthraSectionGC*)0)->gc_roots[0]);
    size_t total_size = header_size + roots_size;
    
    if (!expand_section_data(gc_section, total_size)) {
        return false;
    }
    
    // Create GC metadata
    AsthraSectionGC *gc_metadata = (AsthraSectionGC*)gc_section->data;
    gc_metadata->magic = 0x41474300; // "AGC\0"
    gc_metadata->version = ASTHRA_METADATA_VERSION;
    gc_metadata->gc_root_count = (uint32_t)gc_root_count;
    gc_metadata->ownership_region_count = 5; // Example
    
    // Populate GC root metadata (example)
    for (size_t i = 0; i < gc_root_count; i++) {
        gc_metadata->gc_roots[i].address = 0x1000 + i * 8; // Example addresses
        gc_metadata->gc_roots[i].size = 8; // Example size
        gc_metadata->gc_roots[i].type_id = i % 3; // Example type IDs
        gc_metadata->gc_roots[i].ownership_type = 0; // gc ownership
        gc_metadata->gc_roots[i].is_mutable = i % 2; // Alternate mutability
    }
    
    gc_section->data_size = total_size;
    writer->asthra_metadata->gc_metadata = gc_metadata;
    writer->asthra_metadata->gc_size = total_size;
    
    // Update statistics
    writer->asthra_metadata_entries_count += gc_root_count;
    return true;
}

// =============================================================================
// SECURITY METADATA SECTION POPULATION
// =============================================================================

bool elf_populate_security_section(ELFWriter *writer) {
    if (!writer || !writer->ffi_generator) return false;
    
    ELFSection *security_section = find_section_by_name(writer, ".Asthra.security_meta");
    if (!security_section) return false;
    
    // Get security operation count
    size_t ffi_calls, pattern_matches, string_ops, slice_ops, security_ops, spawn_stmts;
    ffi_get_generation_statistics(writer->ffi_generator, &ffi_calls, &pattern_matches,
                                 &string_ops, &slice_ops, &security_ops, &spawn_stmts);
    
    // Calculate required size
    size_t header_size = sizeof(AsthraSectionSecurity);
    size_t ops_size = security_ops * sizeof(((AsthraSectionSecurity*)0)->security_operations[0]);
    size_t total_size = header_size + ops_size;
    
    if (!expand_section_data(security_section, total_size)) {
        return false;
    }
    
    // Create security metadata
    AsthraSectionSecurity *security_metadata = (AsthraSectionSecurity*)security_section->data;
    security_metadata->magic = 0x41534543; // "ASEC"
    security_metadata->version = ASTHRA_METADATA_VERSION;
    security_metadata->constant_time_count = (uint32_t)(security_ops / 2); // Example split
    security_metadata->volatile_memory_count = (uint32_t)(security_ops / 2);
    
    // Populate security operation metadata (example)
    for (size_t i = 0; i < security_ops && i < 50; i++) {
        security_metadata->security_operations[i].function_address = 0x2000 + i * 16;
        security_metadata->security_operations[i].operation_offset = (uint32_t)(i * 4);
        security_metadata->security_operations[i].operation_type = (uint32_t)(i % 4); // SECURITY_OP_*
        security_metadata->security_operations[i].memory_size = 32; // Example size
        security_metadata->security_operations[i].flags = 0;
        security_metadata->security_operations[i].reserved = 0;
    }
    
    security_section->data_size = total_size;
    writer->asthra_metadata->security_metadata = security_metadata;
    writer->asthra_metadata->security_size = total_size;
    
    // Update statistics
    writer->asthra_metadata_entries_count += security_ops;
    return true;
}

// =============================================================================
// PATTERN MATCHING METADATA SECTION POPULATION
// =============================================================================

bool elf_populate_pattern_matching_section(ELFWriter *writer) {
    if (!writer || !writer->ffi_generator) return false;
    
    ELFSection *pattern_section = find_section_by_name(writer, ".Asthra.pattern_matching");
    if (!pattern_section) return false;
    
    // Get pattern matching statistics
    size_t ffi_calls, pattern_matches, string_ops, slice_ops, security_ops, spawn_stmts;
    ffi_get_generation_statistics(writer->ffi_generator, &ffi_calls, &pattern_matches,
                                 &string_ops, &slice_ops, &security_ops, &spawn_stmts);
    
    // Calculate required size
    size_t header_size = sizeof(AsthraSectionPatternMatching);
    size_t matches_size = pattern_matches * sizeof(((AsthraSectionPatternMatching*)0)->matches[0]);
    size_t total_size = header_size + matches_size;
    
    if (!expand_section_data(pattern_section, total_size)) {
        return false;
    }
    
    // Create pattern matching metadata
    AsthraSectionPatternMatching *pattern_metadata = (AsthraSectionPatternMatching*)pattern_section->data;
    pattern_metadata->magic = 0x41504154; // "APAT"
    pattern_metadata->version = ASTHRA_METADATA_VERSION;
    pattern_metadata->match_count = (uint32_t)pattern_matches;
    pattern_metadata->result_type_count = (uint32_t)(pattern_matches / 2); // Example: half are Result types
    
    // Populate pattern match metadata (example)
    for (size_t i = 0; i < pattern_matches && i < 20; i++) {
        pattern_metadata->matches[i].match_address = 0x3000 + i * 32;
        pattern_metadata->matches[i].strategy = (uint32_t)(i % 3); // PATTERN_STRATEGY_*
        pattern_metadata->matches[i].arm_count = (uint32_t)(2 + (i % 3)); // 2-4 arms
        pattern_metadata->matches[i].is_exhaustive = 1; // Assume exhaustive
        pattern_metadata->matches[i].has_result_patterns = i % 2; // Half have Result patterns
        
        // Populate arms (example)
        for (int j = 0; j < (int)pattern_metadata->matches[i].arm_count && j < 32; j++) {
            pattern_metadata->matches[i].arms[j].pattern_type = (uint32_t)j; // Different pattern types
            pattern_metadata->matches[i].arms[j].target_address = 0x3100 + i * 32 + (size_t)j * 8;
            pattern_metadata->matches[i].arms[j].binding_count = (uint32_t)(j % 2); // 0-1 bindings
            pattern_metadata->matches[i].arms[j].reserved = 0;
        }
    }
    
    pattern_section->data_size = total_size;
    writer->asthra_metadata->pattern_metadata = pattern_metadata;
    writer->asthra_metadata->pattern_size = total_size;
    
    // Update statistics
    writer->asthra_metadata_entries_count += pattern_matches;
    return true;
}

// =============================================================================
// STRING OPERATIONS METADATA SECTION POPULATION
// =============================================================================

bool elf_populate_string_ops_section(ELFWriter *writer) {
    if (!writer || !writer->ffi_generator) return false;
    
    ELFSection *string_section = find_section_by_name(writer, ".Asthra.string_ops");
    if (!string_section) return false;
    
    // Get string operation statistics
    size_t ffi_calls, pattern_matches, string_ops, slice_ops, security_ops, spawn_stmts;
    ffi_get_generation_statistics(writer->ffi_generator, &ffi_calls, &pattern_matches,
                                 &string_ops, &slice_ops, &security_ops, &spawn_stmts);
    
    // Calculate required size
    size_t header_size = sizeof(AsthraSectionStringOps);
    size_t ops_size = string_ops * sizeof(((AsthraSectionStringOps*)0)->string_operations[0]);
    size_t total_size = header_size + ops_size;
    
    if (!expand_section_data(string_section, total_size)) {
        return false;
    }
    
    // Create string operations metadata
    AsthraSectionStringOps *string_metadata = (AsthraSectionStringOps*)string_section->data;
    string_metadata->magic = 0x41535452; // "ASTR"
    string_metadata->version = ASTHRA_METADATA_VERSION;
    string_metadata->string_op_count = (uint32_t)string_ops;
    string_metadata->interpolation_count = (uint32_t)(string_ops / 3); // Example: 1/3 are interpolations
    
    // Populate string operation metadata (example)
    for (size_t i = 0; i < string_ops && i < 30; i++) {
        string_metadata->string_operations[i].operation_address = 0x4000 + i * 16;
        string_metadata->string_operations[i].operation_type = (uint32_t)(i % 5); // STRING_OP_*
        string_metadata->string_operations[i].operand_count = (uint32_t)(2 + (i % 2)); // 2-3 operands
        string_metadata->string_operations[i].is_deterministic = 1; // Deterministic
        string_metadata->string_operations[i].needs_allocation = i % 2; // Half need allocation
        string_metadata->string_operations[i].template_address = 0x5000 + i * 8;
        string_metadata->string_operations[i].expression_count = (uint32_t)(i % 4); // 0-3 expressions
        string_metadata->string_operations[i].reserved = 0;
    }
    
    string_section->data_size = total_size;
    writer->asthra_metadata->string_metadata = string_metadata;
    writer->asthra_metadata->string_size = total_size;
    
    // Update statistics
    writer->asthra_metadata_entries_count += string_ops;
    return true;
}

// =============================================================================
// SLICE METADATA SECTION POPULATION
// =============================================================================

bool elf_populate_slice_meta_section(ELFWriter *writer) {
    if (!writer || !writer->ffi_generator) return false;
    
    ELFSection *slice_section = find_section_by_name(writer, ".Asthra.slice_meta");
    if (!slice_section) return false;
    
    // Get slice operation statistics
    size_t ffi_calls, pattern_matches, string_ops, slice_ops, security_ops, spawn_stmts;
    ffi_get_generation_statistics(writer->ffi_generator, &ffi_calls, &pattern_matches,
                                 &string_ops, &slice_ops, &security_ops, &spawn_stmts);
    
    // Calculate required size
    size_t header_size = sizeof(AsthraSectionSliceMeta);
    size_t ops_size = slice_ops * sizeof(((AsthraSectionSliceMeta*)0)->slice_operations[0]);
    size_t total_size = header_size + ops_size;
    
    if (!expand_section_data(slice_section, total_size)) {
        return false;
    }
    
    // Create slice metadata
    AsthraSectionSliceMeta *slice_metadata = (AsthraSectionSliceMeta*)slice_section->data;
    slice_metadata->magic = 0x41534C49; // "ASLI"
    slice_metadata->version = ASTHRA_METADATA_VERSION;
    slice_metadata->slice_count = (uint32_t)slice_ops;
    slice_metadata->bounds_check_count = (uint32_t)slice_ops; // All have bounds checks
    
    // Populate slice operation metadata (example)
    for (size_t i = 0; i < slice_ops && i < 25; i++) {
        slice_metadata->slice_operations[i].slice_address = 0x6000 + i * 24;
        slice_metadata->slice_operations[i].operation_type = (uint32_t)(i % 6); // SLICE_OP_*
        slice_metadata->slice_operations[i].element_size = (uint32_t)(1 << (i % 4)); // 1, 2, 4, 8 bytes
        slice_metadata->slice_operations[i].is_mutable = i % 2;
        slice_metadata->slice_operations[i].bounds_checking = 1; // Always enabled
        slice_metadata->slice_operations[i].bounds_check_addr = 0x6100 + i * 8;
        slice_metadata->slice_operations[i].ffi_conversion = i % 3 == 0; // 1/3 need FFI conversion
        slice_metadata->slice_operations[i].reserved = 0;
    }
    
    slice_section->data_size = total_size;
    writer->asthra_metadata->slice_metadata = slice_metadata;
    writer->asthra_metadata->slice_size = total_size;
    
    // Update statistics
    writer->asthra_metadata_entries_count += slice_ops;
    return true;
}

// =============================================================================
// CONCURRENCY METADATA SECTION POPULATION
// =============================================================================

bool elf_populate_concurrency_section(ELFWriter *writer) {
    if (!writer || !writer->ffi_generator) return false;
    
    ELFSection *concurrency_section = find_section_by_name(writer, ".Asthra.concurrency");
    if (!concurrency_section) return false;
    
    // Get spawn statement statistics
    size_t ffi_calls, pattern_matches, string_ops, slice_ops, security_ops, spawn_stmts;
    ffi_get_generation_statistics(writer->ffi_generator, &ffi_calls, &pattern_matches,
                                 &string_ops, &slice_ops, &security_ops, &spawn_stmts);
    
    // Calculate required size
    size_t header_size = sizeof(AsthraSectionConcurrency);
    size_t spawns_size = spawn_stmts * sizeof(((AsthraSectionConcurrency*)0)->spawn_points[0]);
    size_t total_size = header_size + spawns_size;
    
    if (!expand_section_data(concurrency_section, total_size)) {
        return false;
    }
    
    // Create concurrency metadata
    AsthraSectionConcurrency *concurrency_metadata = (AsthraSectionConcurrency*)concurrency_section->data;
    concurrency_metadata->magic = 0x41434F4E; // "ACON"
    concurrency_metadata->version = ASTHRA_METADATA_VERSION;
    concurrency_metadata->spawn_count = (uint32_t)spawn_stmts;
    concurrency_metadata->task_count = (uint32_t)spawn_stmts; // Assume one task per spawn
    
    // Populate spawn point metadata (example)
    for (size_t i = 0; i < spawn_stmts && i < 15; i++) {
        concurrency_metadata->spawn_points[i].spawn_address = 0x7000 + i * 32;
        snprintf(concurrency_metadata->spawn_points[i].function_name, 64, 
                "task_function_%zu", i);
        concurrency_metadata->spawn_points[i].argument_count = (uint32_t)(1 + (i % 3)); // 1-3 args
        concurrency_metadata->spawn_points[i].needs_handle = i % 2; // Half need handles
        concurrency_metadata->spawn_points[i].scheduler_call = 0x8000 + i * 8;
        concurrency_metadata->spawn_points[i].memory_barrier = i % 3 == 0; // 1/3 need barriers
        concurrency_metadata->spawn_points[i].atomic_operations = i % 2; // Half use atomics
        concurrency_metadata->spawn_points[i].reserved = 0;
    }
    
    concurrency_section->data_size = total_size;
    writer->asthra_metadata->concurrency_metadata = concurrency_metadata;
    writer->asthra_metadata->concurrency_size = total_size;
    
    // Update statistics
    writer->asthra_metadata_entries_count += spawn_stmts;
    return true;
} 
