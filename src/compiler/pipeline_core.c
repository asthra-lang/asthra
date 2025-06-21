/**
 * Asthra Programming Language Compiler
 * Pipeline Orchestrator - Core Orchestrator Functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Core orchestrator lifecycle and main compilation API
 */

#include "pipeline_core.h"
#include "pipeline_utils.h"
#include "pipeline_phases.h"
#include "../codegen/code_generator_core.h"
#ifdef __APPLE__
#include "../codegen/macho_writer.h"
#else
#include "../codegen/elf_writer.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <unistd.h>

// =============================================================================
// PIPELINE ORCHESTRATOR CORE FUNCTIONS
// =============================================================================

/**
 * Create a new pipeline orchestrator
 */
PipelineOrchestrator *pipeline_orchestrator_create(AsthraCompilerContext *context) {
    if (!context) return NULL;
    
    PipelineOrchestrator *orchestrator = calloc(1, sizeof(PipelineOrchestrator));
    if (!orchestrator) return NULL;
    
    orchestrator->compiler_context = context;
    orchestrator->current_phase = ASTHRA_PHASE_LEXING;
    orchestrator->pipeline_active = false;
    orchestrator->error_occurred = false;
    
    // Initialize configuration with defaults
    orchestrator->config.generate_debug_info = true;
    orchestrator->config.optimize_pipeline = true;
    orchestrator->config.parallel_compilation = false; // Start with sequential
    orchestrator->config.save_intermediates = false;
    orchestrator->config.verbose_output = false;
    orchestrator->config.max_errors = 100;
    orchestrator->config.timeout_seconds = 300.0; // 5 minutes
    
    // Create intermediate directory
    orchestrator->intermediate_dir = strdup("build/intermediate");
    // Create the directory with parent directories
    struct stat st = {0};
    if (stat("build", &st) == -1) {
        mkdir("build", 0755);
    }
    if (stat(orchestrator->intermediate_dir, &st) == -1) {
        mkdir(orchestrator->intermediate_dir, 0755);
    }
    
    return orchestrator;
}

/**
 * Destroy pipeline orchestrator and free all resources
 */
void pipeline_orchestrator_destroy(PipelineOrchestrator *orchestrator) {
    if (!orchestrator) return;
    
    // Clean up source file contexts
    SourceFileContext *current = orchestrator->source_files;
    while (current) {
        SourceFileContext *next = current->next;
        
        // Free source code
        free(current->source_code);
        
        // Free phase results
        for (int i = 0; i < 6; i++) {
            PipelinePhaseResult *result = &current->phases[i];
            free(result->error_message);
            free(result->warning_message);
            
            // Free phase-specific data
            switch (result->phase) {
                case ASTHRA_PHASE_LEXING:
                    if (result->data.lexer_data.lexer) {
                        lexer_destroy(result->data.lexer_data.lexer);
                    }
                    free(result->data.lexer_data.error_message);
                    break;
                    
                case ASTHRA_PHASE_PARSING:
                    if (result->data.parser_data.parser) {
                        parser_destroy(result->data.parser_data.parser);
                    }
                    if (result->data.parser_data.program_ast) {
                        ast_free_node(result->data.parser_data.program_ast);
                    }
                    free(result->data.parser_data.error_message);
                    break;
                    
                case ASTHRA_PHASE_SEMANTIC_ANALYSIS:
                    if (result->data.semantic_data.analyzer) {
                        semantic_analyzer_destroy(result->data.semantic_data.analyzer);
                    }
                    free(result->data.semantic_data.error_message);
                    break;
                    
                case ASTHRA_PHASE_CODE_GENERATION:
                    if (result->data.codegen_data.generator) {
                        code_generator_destroy(result->data.codegen_data.generator);
                    }
                    if (result->data.codegen_data.ffi_generator) {
                        ffi_assembly_generator_destroy(result->data.codegen_data.ffi_generator);
                    }
#ifdef __APPLE__
                    if (result->data.codegen_data.macho_writer) {
                        macho_writer_destroy(result->data.codegen_data.macho_writer);
                    }
#else
                    if (result->data.codegen_data.elf_writer) {
                        elf_writer_destroy(result->data.codegen_data.elf_writer);
                    }
#endif
                    free(result->data.codegen_data.c_code_output);
                    free(result->data.codegen_data.elf_output_path);
                    free(result->data.codegen_data.error_message);
                    break;
                    
                case ASTHRA_PHASE_LINKING:
                    free(result->data.linking_data.executable_path);
                    free(result->data.linking_data.error_message);
                    free(result->data.linking_data.linker_command);
                    break;
                    
                default:
                    break;
            }
        }
        
        free(current);
        current = next;
    }
    
    // Free global resources
    if (orchestrator->global_analyzer) {
        semantic_analyzer_destroy(orchestrator->global_analyzer);
    }
    if (orchestrator->global_generator) {
        code_generator_destroy(orchestrator->global_generator);
    }
    if (orchestrator->ffi_generator) {
        ffi_assembly_generator_destroy(orchestrator->ffi_generator);
    }
#ifdef __APPLE__
    if (orchestrator->macho_writer) {
        macho_writer_destroy(orchestrator->macho_writer);
    }
#else
    if (orchestrator->elf_writer) {
        elf_writer_destroy(orchestrator->elf_writer);
    }
#endif
    
    // Free output paths
    free(orchestrator->output_executable_path);
    free(orchestrator->intermediate_dir);
    
    // Free object file list
    if (orchestrator->object_files) {
        for (size_t i = 0; i < orchestrator->object_file_count; i++) {
            free(orchestrator->object_files[i]);
        }
        free(orchestrator->object_files);
    }
    
    free(orchestrator);
}

// =============================================================================
// MAIN COMPILATION API FUNCTIONS
// =============================================================================

/**
 * Execute complete compilation pipeline for a single file
 */
bool pipeline_orchestrator_compile_file(PipelineOrchestrator *orchestrator,
                                       const char *input_file,
                                       const char *output_file) {
    printf("DEBUG: pipeline_orchestrator_compile_file called with input=%s, output=%s\n", 
           input_file ? input_file : "NULL", output_file ? output_file : "NULL");
    fflush(stdout);
    
    if (!orchestrator || !input_file || !output_file) return false;
    
    printf("DEBUG: Clearing previous state\n");
    fflush(stdout);
    // Clear previous state
    pipeline_clear_state(orchestrator);
    printf("DEBUG: State cleared\n");
    fflush(stdout);
    
    orchestrator->pipeline_active = true;
    orchestrator->total_start_time = pipeline_get_current_time_ms();
    
    // Create source file context
    SourceFileContext *source_context = calloc(1, sizeof(SourceFileContext));
    if (!source_context) {
        orchestrator->error_occurred = true;
        return false;
    }
    
    source_context->input_path = input_file;
    source_context->relative_path = input_file; // For single file, same as input
    
    // Read source code
    source_context->source_code = read_source_file(input_file, &source_context->source_length);
    if (!source_context->source_code) {
        free(source_context);
        orchestrator->error_occurred = true;
        return false;
    }
    
    // Add to orchestrator
    orchestrator->source_files = source_context;
    orchestrator->source_file_count = 1;
    orchestrator->output_executable_path = strdup(output_file);
    
    bool success = true;
    
    // Execute compilation phases sequentially
    if (success) {
        printf("DEBUG: Starting lexer phase\n");
        fflush(stdout);
        success = pipeline_execute_lexer_phase(orchestrator, source_context);
        orchestrator->phase_times[0] = source_context->phases[0].execution_time_ms;
        printf("DEBUG: Lexer phase completed, success=%d\n", success);
        fflush(stdout);
    }
    
    if (success) {
        printf("DEBUG: Starting parser phase\n");
        fflush(stdout);
        success = pipeline_execute_parser_phase(orchestrator, source_context);
        orchestrator->phase_times[1] = source_context->phases[1].execution_time_ms;
        printf("DEBUG: Parser phase completed, success=%d\n", success);
        fflush(stdout);
    }
    
    if (success) {
        printf("DEBUG: Starting semantic phase\n");
        fflush(stdout);
        success = pipeline_execute_semantic_phase(orchestrator, source_context);
        orchestrator->phase_times[2] = source_context->phases[2].execution_time_ms;
        printf("DEBUG: Semantic phase completed, success=%d\n", success);
        fflush(stdout);
    }
    
    if (success) {
        printf("DEBUG: Starting codegen phase\n");
        fflush(stdout);
        success = pipeline_execute_codegen_phase(orchestrator, source_context);
        orchestrator->phase_times[3] = source_context->phases[3].execution_time_ms;
        printf("DEBUG: Codegen phase completed, success=%d\n", success);
        fflush(stdout);
    }
    
    if (success) {
        printf("DEBUG: Starting linking phase\n");
        fflush(stdout);
        success = pipeline_execute_linking_phase(orchestrator, output_file);
        orchestrator->phase_times[4] = source_context->phases[4].execution_time_ms;
        printf("DEBUG: Linking phase completed, success=%d\n", success);
        fflush(stdout);
    }
    
    // Mark as processed
    source_context->processed = true;
    
    // Finalize timing
    orchestrator->total_end_time = pipeline_get_current_time_ms();
    orchestrator->pipeline_active = false;
    
    if (!success) {
        orchestrator->error_occurred = true;
    }
    
    if (orchestrator->config.verbose_output) {
        double total_time = orchestrator->total_end_time - orchestrator->total_start_time;
        printf("Compilation %s in %.2f ms\n", 
               success ? "completed successfully" : "failed", total_time);
    }
    
    return success;
}

/**
 * Execute complete compilation pipeline for multiple files
 */
bool pipeline_orchestrator_compile_files(PipelineOrchestrator *orchestrator,
                                        const char **input_files,
                                        size_t file_count,
                                        const char *output_file) {
    if (!orchestrator || !input_files || file_count == 0 || !output_file) return false;
    
    // Clear previous state
    pipeline_clear_state(orchestrator);
    
    orchestrator->pipeline_active = true;
    orchestrator->total_start_time = pipeline_get_current_time_ms();
    orchestrator->output_executable_path = strdup(output_file);
    
    // Process all source files
    SourceFileContext *file_list_head = NULL;
    SourceFileContext *file_list_tail = NULL;
    bool overall_success = true;
    
    for (size_t i = 0; i < file_count; i++) {
        // Create source file context
        SourceFileContext *source_context = calloc(1, sizeof(SourceFileContext));
        if (!source_context) {
            overall_success = false;
            break;
        }
        
        source_context->input_path = input_files[i];
        source_context->relative_path = input_files[i];
        
        // Read source code
        source_context->source_code = read_source_file(input_files[i], &source_context->source_length);
        if (!source_context->source_code) {
            free(source_context);
            overall_success = false;
            break;
        }
        
        // Add to linked list
        if (!file_list_head) {
            file_list_head = source_context;
            file_list_tail = source_context;
        } else {
            file_list_tail->next = source_context;
            file_list_tail = source_context;
        }
        
        // Execute phases for this file
        bool file_success = true;
        
        if (file_success) {
            file_success = pipeline_execute_lexer_phase(orchestrator, source_context);
        }
        
        if (file_success) {
            file_success = pipeline_execute_parser_phase(orchestrator, source_context);
        }
        
        if (file_success) {
            file_success = pipeline_execute_semantic_phase(orchestrator, source_context);
        }
        
        if (file_success) {
            file_success = pipeline_execute_codegen_phase(orchestrator, source_context);
        }
        
        source_context->processed = file_success;
        
        if (!file_success) {
            overall_success = false;
            if (orchestrator->config.verbose_output) {
                printf("Failed to compile file: %s\n", input_files[i]);
            }
        }
    }
    
    // Set up the file list in orchestrator
    orchestrator->source_files = file_list_head;
    orchestrator->source_file_count = file_count;
    
    // Execute final linking phase if all files compiled successfully
    if (overall_success) {
        overall_success = pipeline_execute_linking_phase(orchestrator, output_file);
    }
    
    // Finalize timing
    orchestrator->total_end_time = pipeline_get_current_time_ms();
    orchestrator->pipeline_active = false;
    
    if (!overall_success) {
        orchestrator->error_occurred = true;
    }
    
    if (orchestrator->config.verbose_output) {
        double total_time = orchestrator->total_end_time - orchestrator->total_start_time;
        printf("Multi-file compilation %s in %.2f ms (%zu files)\n", 
               overall_success ? "completed successfully" : "failed", 
               total_time, file_count);
    }
    
    return overall_success;
}