/**
 * Asthra Programming Language Compiler
 * Pipeline Orchestrator - Phase Execution
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Individual phase execution implementations for the compilation pipeline
 */

#include "pipeline_phases.h"
#include "pipeline_utils.h"
#include "../codegen/code_generator_core.h"
#ifdef __APPLE__
#include "../codegen/macho_writer.h"
#else
#include "../codegen/elf_writer.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// =============================================================================
// PHASE EXECUTION FUNCTIONS
// =============================================================================

/**
 * Execute lexical analysis phase
 */
bool pipeline_execute_lexer_phase(PipelineOrchestrator *orchestrator,
                                 SourceFileContext *source_context) {
    if (!orchestrator || !source_context) return false;
    
    PipelinePhaseResult *result = &source_context->phases[ASTHRA_PHASE_LEXING];
    initialize_phase_result(result, ASTHRA_PHASE_LEXING);
    
    orchestrator->current_phase = ASTHRA_PHASE_LEXING;
    
    // Create lexer
    Lexer *lexer = lexer_create(source_context->source_code, 
                               source_context->source_length,
                               source_context->input_path);
    
    if (!lexer) {
        finalize_phase_result(result, false, "Failed to create lexer");
        return false;
    }
    
    // Tokenize source to count tokens and validate lexer
    size_t token_count = 0;
    Token token;
    
    // Get tokens until EOF
    do {
        token = lexer_next_token(lexer);
        if (token.type != TOKEN_EOF && token.type != TOKEN_ERROR) {
            token_count++;
        }
    } while (token.type != TOKEN_EOF && token.type != TOKEN_ERROR);
    
    result->data.lexer_data.token_count = token_count;
    
    // Reset lexer position for subsequent parsing
    // Note: lexer_reset is not available, so we'll accept this limitation
    
    // Store lexer data
    result->data.lexer_data.lexer = lexer;
    result->data.lexer_data.source_code = source_context->source_code;
    result->data.lexer_data.source_length = source_context->source_length;
    result->data.lexer_data.filename = source_context->input_path;
    
    const char *error_msg = (token.type == TOKEN_ERROR) ? "Lexical analysis encountered errors" : NULL;
    bool success = (token.type != TOKEN_ERROR);
    finalize_phase_result(result, success, error_msg);
    
    return success;
}

/**
 * Execute parsing phase
 */
bool pipeline_execute_parser_phase(PipelineOrchestrator *orchestrator,
                                  SourceFileContext *source_context) {
    if (!orchestrator || !source_context) return false;
    
    PipelinePhaseResult *result = &source_context->phases[ASTHRA_PHASE_PARSING];
    initialize_phase_result(result, ASTHRA_PHASE_PARSING);
    
    orchestrator->current_phase = ASTHRA_PHASE_PARSING;
    
    // Get source info from previous phase to create a fresh lexer
    // The lexer from the previous phase has been exhausted by token counting
    const char *source_code = source_context->phases[ASTHRA_PHASE_LEXING].data.lexer_data.source_code;
    size_t source_length = source_context->phases[ASTHRA_PHASE_LEXING].data.lexer_data.source_length;
    const char *filename = source_context->phases[ASTHRA_PHASE_LEXING].data.lexer_data.filename;
    
    if (!source_code) {
        finalize_phase_result(result, false, "No source code available from previous phase");
        return false;
    }
    
    // Create a fresh lexer for parsing
    Lexer *lexer = lexer_create(source_code, source_length, filename);
    if (!lexer) {
        finalize_phase_result(result, false, "Failed to create lexer for parsing");
        return false;
    }
    
    // Create parser
    Parser *parser = parser_create(lexer);
    if (!parser) {
        finalize_phase_result(result, false, "Failed to create parser");
        return false;
    }
    
    // Parse the program
    ASTNode *program_ast = parser_parse_program(parser);
    bool parse_success = (program_ast != NULL && !parser_had_error(parser));
    
    // Store parser data
    result->data.parser_data.parser = parser;
    result->data.parser_data.program_ast = program_ast;
    result->data.parser_data.parse_error_count = parser_had_error(parser) ? parser_get_error_count(parser) : 0;
    
    if (program_ast) {
        // Note: ast_count_nodes function not available, using simplified count
        result->data.parser_data.ast_node_count = 1; // Simplified - just count the root node
    }
    
    const char *error_msg = parse_success ? NULL : "Parsing failed";
    finalize_phase_result(result, parse_success, error_msg);
    
    // Destroy the lexer we created for parsing
    lexer_destroy(lexer);
    
    return parse_success;
}

/**
 * Execute semantic analysis phase
 */
bool pipeline_execute_semantic_phase(PipelineOrchestrator *orchestrator,
                                    SourceFileContext *source_context) {
    if (!orchestrator || !source_context) return false;
    
    PipelinePhaseResult *result = &source_context->phases[ASTHRA_PHASE_SEMANTIC_ANALYSIS];
    initialize_phase_result(result, ASTHRA_PHASE_SEMANTIC_ANALYSIS);
    
    orchestrator->current_phase = ASTHRA_PHASE_SEMANTIC_ANALYSIS;
    
    // Get AST from previous phase
    ASTNode *program_ast = source_context->phases[ASTHRA_PHASE_PARSING].data.parser_data.program_ast;
    if (!program_ast) {
        finalize_phase_result(result, false, "No AST available from parsing phase");
        return false;
    }
    
    // Create or reuse global semantic analyzer
    if (!orchestrator->global_analyzer) {
        orchestrator->global_analyzer = semantic_analyzer_create();
        if (!orchestrator->global_analyzer) {
            finalize_phase_result(result, false, "Failed to create semantic analyzer");
            return false;
        }
    }
    
    // Perform semantic analysis
    printf("DEBUG: About to call semantic_analyze_program\n");
    fflush(stdout);
    bool semantic_success = semantic_analyze_program(orchestrator->global_analyzer, program_ast);
    printf("DEBUG: semantic_analyze_program returned %d\n", semantic_success);
    fflush(stdout);
    
    // Store semantic data
    result->data.semantic_data.analyzer = orchestrator->global_analyzer;
    result->data.semantic_data.semantic_error_count = semantic_get_error_count(orchestrator->global_analyzer);
    
    // Get symbol count if available
    // Note: Direct access to symbol_table not available, using simplified approach
    result->data.semantic_data.symbol_count = 0; // Simplified - would need proper symbol table access
    result->data.semantic_data.type_count = 0;   // Simplified - would need proper type registry access
    
    const char *error_msg = semantic_success ? NULL : "Semantic analysis failed";
    finalize_phase_result(result, semantic_success, error_msg);
    
    return semantic_success;
}

/**
 * Execute code generation phase
 */
bool pipeline_execute_codegen_phase(PipelineOrchestrator *orchestrator,
                                   SourceFileContext *source_context) {
    printf("DEBUG: pipeline_execute_codegen_phase - Entry\n");
    fflush(stdout);
    
    if (!orchestrator || !source_context) {
        printf("DEBUG: pipeline_execute_codegen_phase - Invalid parameters\n");
        fflush(stdout);
        return false;
    }
    
    printf("DEBUG: pipeline_execute_codegen_phase - Initializing phase result\n");
    fflush(stdout);
    
    PipelinePhaseResult *result = &source_context->phases[ASTHRA_PHASE_CODE_GENERATION];
    initialize_phase_result(result, ASTHRA_PHASE_CODE_GENERATION);
    
    orchestrator->current_phase = ASTHRA_PHASE_CODE_GENERATION;
    
    printf("DEBUG: pipeline_execute_codegen_phase - Getting AST from parsing phase\n");
    fflush(stdout);
    
    // Get AST from parsing phase
    ASTNode *program_ast = source_context->phases[ASTHRA_PHASE_PARSING].data.parser_data.program_ast;
    if (!program_ast) {
        printf("DEBUG: pipeline_execute_codegen_phase - No AST available\n");
        fflush(stdout);
        finalize_phase_result(result, false, "No AST available for code generation");
        return false;
    }
    
    printf("DEBUG: pipeline_execute_codegen_phase - Creating code generator\n");
    fflush(stdout);
    
    // Create code generator if needed
    if (!orchestrator->global_generator) {
        printf("DEBUG: pipeline_execute_codegen_phase - About to call code_generator_create\n");
        fflush(stdout);
        
        // Choose target architecture based on platform
#ifdef __arm64__
        TargetArchitecture target_arch = TARGET_ARCH_AARCH64;
        CallingConvention calling_conv = CALLING_CONV_AARCH64_AAPCS;
        printf("DEBUG: Using ARM64/AArch64 target architecture\n");
#elif defined(__x86_64__)
        TargetArchitecture target_arch = TARGET_ARCH_X86_64;
        CallingConvention calling_conv = CALLING_CONV_SYSTEM_V_AMD64;
        printf("DEBUG: Using x86_64 target architecture\n");
#else
        TargetArchitecture target_arch = TARGET_ARCH_X86_64;
        CallingConvention calling_conv = CALLING_CONV_SYSTEM_V_AMD64;
        printf("DEBUG: Using default x86_64 target architecture\n");
#endif
        
        orchestrator->global_generator = code_generator_create(target_arch, calling_conv);
        printf("DEBUG: pipeline_execute_codegen_phase - code_generator_create returned\n");
        fflush(stdout);
        if (!orchestrator->global_generator) {
            printf("DEBUG: pipeline_execute_codegen_phase - Failed to create code generator\n");
            fflush(stdout);
            finalize_phase_result(result, false, "Failed to create code generator");
            return false;
        }
    }
    
    printf("DEBUG: pipeline_execute_codegen_phase - Creating FFI assembly generator\n");
    fflush(stdout);
    
    // Create FFI assembly generator if needed
    if (!orchestrator->ffi_generator) {
        printf("DEBUG: pipeline_execute_codegen_phase - About to call ffi_assembly_generator_create\n");
        fflush(stdout);
        
        // Use the same target architecture as the code generator
#ifdef __arm64__
        TargetArchitecture target_arch = TARGET_ARCH_AARCH64;
        CallingConvention calling_conv = CALLING_CONV_AARCH64_AAPCS;
#elif defined(__x86_64__)
        TargetArchitecture target_arch = TARGET_ARCH_X86_64;
        CallingConvention calling_conv = CALLING_CONV_SYSTEM_V_AMD64;
#else
        TargetArchitecture target_arch = TARGET_ARCH_X86_64;
        CallingConvention calling_conv = CALLING_CONV_SYSTEM_V_AMD64;
#endif
        
        orchestrator->ffi_generator = ffi_assembly_generator_create(target_arch, calling_conv);
        printf("DEBUG: pipeline_execute_codegen_phase - ffi_assembly_generator_create returned\n");
        fflush(stdout);
        if (!orchestrator->ffi_generator) {
            printf("DEBUG: pipeline_execute_codegen_phase - Failed to create FFI assembly generator\n");
            fflush(stdout);
            finalize_phase_result(result, false, "Failed to create FFI assembly generator");
            return false;
        }
    }
    
    printf("DEBUG: pipeline_execute_codegen_phase - Setting semantic analyzer\n");
    fflush(stdout);
    
    // Set semantic analyzer on code generators to satisfy architectural requirements
    if (orchestrator->global_analyzer) {
        printf("DEBUG: pipeline_execute_codegen_phase - About to call code_generator_set_semantic_analyzer\n");
        fflush(stdout);
        code_generator_set_semantic_analyzer(orchestrator->global_generator, orchestrator->global_analyzer);
        printf("DEBUG: pipeline_execute_codegen_phase - code_generator_set_semantic_analyzer returned\n");
        fflush(stdout);
        // Also set on FFI generator's base generator
        if (orchestrator->ffi_generator && orchestrator->ffi_generator->base_generator) {
            printf("DEBUG: pipeline_execute_codegen_phase - Setting semantic analyzer on FFI base generator\n");
            fflush(stdout);
            code_generator_set_semantic_analyzer(orchestrator->ffi_generator->base_generator, orchestrator->global_analyzer);
            printf("DEBUG: pipeline_execute_codegen_phase - FFI base generator semantic analyzer set\n");
            fflush(stdout);
        }
    }
    
#ifdef __APPLE__
    printf("DEBUG: pipeline_execute_codegen_phase - Creating Mach-O writer\n");
    fflush(stdout);
    
    // Create Mach-O writer if needed
    if (!orchestrator->macho_writer) {
        printf("DEBUG: pipeline_execute_codegen_phase - About to call macho_writer_create\n");
        fflush(stdout);
        orchestrator->macho_writer = macho_writer_create(orchestrator->ffi_generator);
        printf("DEBUG: pipeline_execute_codegen_phase - macho_writer_create returned\n");
        fflush(stdout);
        if (!orchestrator->macho_writer) {
            printf("DEBUG: pipeline_execute_codegen_phase - Failed to create Mach-O writer\n");
            fflush(stdout);
            finalize_phase_result(result, false, "Failed to create Mach-O writer");
            return false;
        }
    }
#else
    printf("DEBUG: pipeline_execute_codegen_phase - Creating ELF writer\n");
    fflush(stdout);
    
    // Create ELF writer if needed
    if (!orchestrator->elf_writer) {
        printf("DEBUG: pipeline_execute_codegen_phase - About to call elf_writer_create\n");
        fflush(stdout);
        orchestrator->elf_writer = elf_writer_create(orchestrator->ffi_generator);
        printf("DEBUG: pipeline_execute_codegen_phase - elf_writer_create returned\n");
        fflush(stdout);
        if (!orchestrator->elf_writer) {
            printf("DEBUG: pipeline_execute_codegen_phase - Failed to create ELF writer\n");
            fflush(stdout);
            finalize_phase_result(result, false, "Failed to create ELF writer");
            return false;
        }
    }
#endif
    
    printf("DEBUG: pipeline_execute_codegen_phase - Generating object file path\n");
    fflush(stdout);
    
    // Generate object file path
    char object_path[512];
    snprintf(object_path, sizeof(object_path), "%s/%s.o", 
             orchestrator->intermediate_dir, 
             strrchr(source_context->input_path, '/') ? 
             strrchr(source_context->input_path, '/') + 1 : source_context->input_path);
    
    printf("DEBUG: pipeline_execute_codegen_phase - Object path: %s\n", object_path);
    fflush(stdout);
    
    printf("DEBUG: pipeline_execute_codegen_phase - About to call object file generation\n");
    fflush(stdout);
    
    // Generate object file (platform-specific format)
    bool codegen_success;
#ifdef __APPLE__
    codegen_success = macho_generate_object_file(orchestrator->macho_writer, program_ast, object_path);
#else
    codegen_success = elf_generate_object_file(orchestrator->elf_writer, program_ast, object_path);
#endif
    
    printf("DEBUG: pipeline_execute_codegen_phase - object file generation returned: %d\n", codegen_success);
    fflush(stdout);
    
    // Store codegen data
    result->data.codegen_data.generator = orchestrator->global_generator;
    result->data.codegen_data.ffi_generator = orchestrator->ffi_generator;
#ifdef __APPLE__
    result->data.codegen_data.macho_writer = orchestrator->macho_writer;
#else
    result->data.codegen_data.elf_writer = orchestrator->elf_writer;
#endif
    result->data.codegen_data.elf_output_path = strdup(object_path);
    
    if (codegen_success) {
        // Add object file to list for linking
        orchestrator->object_file_count++;
        orchestrator->object_files = realloc(orchestrator->object_files, 
                                           orchestrator->object_file_count * sizeof(char*));
        orchestrator->object_files[orchestrator->object_file_count - 1] = strdup(object_path);
    }
    
    const char *error_msg = codegen_success ? NULL : "Code generation failed";
    finalize_phase_result(result, codegen_success, error_msg);
    
    printf("DEBUG: pipeline_execute_codegen_phase - Returning %d\n", codegen_success);
    fflush(stdout);
    return codegen_success;
}

/**
 * Execute linking phase
 */
bool pipeline_execute_linking_phase(PipelineOrchestrator *orchestrator,
                                   const char *output_executable) {
    if (!orchestrator || !output_executable) return false;
    
    // Find the first source file to store linking results
    if (!orchestrator->source_files) return false;
    
    PipelinePhaseResult *result = &orchestrator->source_files->phases[ASTHRA_PHASE_LINKING];
    initialize_phase_result(result, ASTHRA_PHASE_LINKING);
    
    orchestrator->current_phase = ASTHRA_PHASE_LINKING;
    
    // Ensure output directory exists
    if (!create_intermediate_directory(output_executable)) {
        finalize_phase_result(result, false, "Failed to create output directory");
        return false;
    }
    
    // Build linker command
    char linker_command[2048];
    int cmd_len = snprintf(linker_command, sizeof(linker_command), "cc");
    
    // Add all object files
    for (size_t i = 0; i < orchestrator->object_file_count; i++) {
        cmd_len += snprintf(linker_command + cmd_len, 
                           (size_t)(sizeof(linker_command) - cmd_len), 
                           " %s", orchestrator->object_files[i]);
    }
    
    // Add output executable
    // Note: Skipping -lasthra_runtime for now as it doesn't exist yet
    cmd_len += snprintf(linker_command + cmd_len, 
                       (size_t)(sizeof(linker_command) - cmd_len), 
                       " -o %s", output_executable);
    
    // Execute linker command
    result->data.linking_data.linker_command = strdup(linker_command);
    printf("DEBUG: Executing linker command: %s\n", linker_command);
    
    // Execute linker command (now works with Mach-O object files)
    int link_result = system(linker_command);
    
    if (link_result == 0) {
        // Get executable size
        struct stat st;
        if (stat(output_executable, &st) == 0) {
            result->data.linking_data.executable_size = (size_t)st.st_size;
        }
        result->data.linking_data.executable_path = strdup(output_executable);
    }
    
    bool success = (link_result == 0);
    const char *error_msg = success ? NULL : "Linking failed";
    finalize_phase_result(result, success, error_msg);
    
    return success;
}