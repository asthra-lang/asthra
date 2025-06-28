/**
 * Asthra Programming Language LLVM Debug Info
 * Implementation of debug information generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_debug.h"
#include "llvm_types.h"
#include <stdlib.h>
#include <string.h>

// DWARF type encoding constants from DWARF standard
#define DW_ATE_signed 0x05
#define DW_ATE_unsigned 0x07
#define DW_ATE_float 0x04
#define DW_ATE_boolean 0x02
#define DW_ATE_signed_char 0x06
#define DW_ATE_unsigned_char 0x08

// Initialize debug info support
void initialize_debug_info(LLVMBackendData *data, const char *filename, bool is_optimized) {
    // Create DIBuilder
    data->di_builder = LLVMCreateDIBuilder(data->module);

    // Extract directory and filename
    const char *last_slash = strrchr(filename, '/');
    char *directory = ".";
    const char *file_only = filename;

    if (last_slash) {
        size_t dir_len = last_slash - filename;
        directory = malloc(dir_len + 1);
        strncpy(directory, filename, dir_len);
        directory[dir_len] = '\0';
        file_only = last_slash + 1;
    }

    // Create file descriptor
    data->di_file = LLVMDIBuilderCreateFile(data->di_builder, file_only, strlen(file_only),
                                            directory, strlen(directory));

    // Create compile unit
    data->di_compile_unit =
        LLVMDIBuilderCreateCompileUnit(data->di_builder,
                                       LLVMDWARFSourceLanguageC, // Using C as closest language
                                       data->di_file, "Asthra Compiler", 15, // Producer
                                       is_optimized, "", 0,                  // Flags
                                       0,                                    // Runtime version
                                       "", 0,                                // Split name
                                       LLVMDWARFEmissionFull,
                                       0,     // DWOId
                                       true,  // Split debug inlining
                                       false, // Debug info for profiling
                                       "", 0, // SysRoot
                                       "", 0  // SDK
        );

    // Set as current debug scope
    data->current_debug_scope = data->di_compile_unit;

    // Cache basic debug types
    cache_basic_debug_types(data);

    // Free allocated directory if needed
    if (last_slash && strcmp(directory, ".") != 0) {
        free(directory);
    }
}

// Cache commonly used debug types
void cache_basic_debug_types(LLVMBackendData *data) {
    // Integer types
    data->di_i32_type =
        LLVMDIBuilderCreateBasicType(data->di_builder, "i32", 3, 32, DW_ATE_signed, LLVMDIFlagZero);
    data->di_i64_type =
        LLVMDIBuilderCreateBasicType(data->di_builder, "i64", 3, 64, DW_ATE_signed, LLVMDIFlagZero);

    // Floating point types
    data->di_f32_type =
        LLVMDIBuilderCreateBasicType(data->di_builder, "f32", 3, 32, DW_ATE_float, LLVMDIFlagZero);
    data->di_f64_type =
        LLVMDIBuilderCreateBasicType(data->di_builder, "f64", 3, 64, DW_ATE_float, LLVMDIFlagZero);

    // Boolean type
    data->di_bool_type = LLVMDIBuilderCreateBasicType(data->di_builder, "bool", 4, 1,
                                                      DW_ATE_boolean, LLVMDIFlagZero);

    // Void type (as unspecified type)
    data->di_void_type = LLVMDIBuilderCreateUnspecifiedType(data->di_builder, "void", 4);

    // Pointer type (void*)
    data->di_ptr_type =
        LLVMDIBuilderCreatePointerType(data->di_builder, data->di_void_type, 64, 0, 0, "", 0);
}

// Set debug location for current instruction
void set_debug_location(LLVMBackendData *data, const ASTNode *node) {
    if (!data->di_builder || !node)
        return;

    // Only set location if we have valid line info
    if (node->location.line > 0) {
        LLVMMetadataRef loc = LLVMDIBuilderCreateDebugLocation(
            data->context, node->location.line, node->location.column, data->current_debug_scope,
            NULL // No inlined location
        );
        LLVMSetCurrentDebugLocation(data->builder, loc);
    }
}

// Finalize debug info
void finalize_debug_info(LLVMBackendData *data) {
    if (data->di_builder) {
        LLVMDIBuilderFinalize(data->di_builder);

        // Add module flags for debug info
        LLVMAddModuleFlag(data->module, LLVMModuleFlagBehaviorWarning, "Debug Info Version", 18,
                          LLVMValueAsMetadata(LLVMConstInt(data->i32_type, 3, false)));

        LLVMAddModuleFlag(data->module, LLVMModuleFlagBehaviorWarning, "Dwarf Version", 13,
                          LLVMValueAsMetadata(LLVMConstInt(data->i32_type, 4, false)));
    }
}