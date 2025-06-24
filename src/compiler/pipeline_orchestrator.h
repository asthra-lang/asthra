/**
 * Asthra Programming Language Compiler
 * Pipeline Orchestrator - Unified Compilation Workflow Coordination
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This module provides the orchestration layer for coordinating all compiler phases
 * into a unified end-to-end compilation pipeline that transforms Asthra source code
 * into executable binaries.
 */

#ifndef ASTHRA_PIPELINE_ORCHESTRATOR_H
#define ASTHRA_PIPELINE_ORCHESTRATOR_H

#include "../compiler.h"
#include "../parser/lexer.h"
#include "../parser/parser.h"
#include "../parser/ast.h"
#include "../analysis/semantic_analyzer.h"
#include "../codegen/backend_interface.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct PipelineOrchestrator PipelineOrchestrator;
typedef struct PipelinePhaseResult PipelinePhaseResult;
typedef struct LexerPhaseData LexerPhaseData;
typedef struct ParserPhaseData ParserPhaseData;
typedef struct SemanticPhaseData SemanticPhaseData;
typedef struct CodegenPhaseData CodegenPhaseData;
typedef struct LinkingPhaseData LinkingPhaseData;
typedef struct SourceFileContext SourceFileContext;

// =============================================================================
// PHASE RESULT STRUCTURES
// =============================================================================

/**
 * Results from lexical analysis phase
 */
struct LexerPhaseData {
    Lexer *lexer;
    char *source_code;
    size_t source_length;
    const char *filename;
    bool success;
    size_t token_count;
    double execution_time_ms;
    char *error_message;
};

/**
 * Results from parsing phase
 */
struct ParserPhaseData {
    Parser *parser;
    ASTNode *program_ast;
    bool success;
    size_t ast_node_count;
    size_t parse_error_count;
    double execution_time_ms;
    char *error_message;
};

/**
 * Results from semantic analysis phase
 */
struct SemanticPhaseData {
    SemanticAnalyzer *analyzer;
    bool success;
    size_t symbol_count;
    size_t type_count;
    size_t semantic_error_count;
    double execution_time_ms;
    char *error_message;
};

/**
 * Results from code generation phase
 */
struct CodegenPhaseData {
    AsthraBackend *backend;
    bool success;
    size_t lines_generated;
    size_t functions_generated;
    double execution_time_ms;
    char *output_path;
    char *error_message;
};

/**
 * Results from linking phase
 */
struct LinkingPhaseData {
    bool success;
    char *executable_path;
    size_t executable_size;
    double execution_time_ms;
    char *error_message;
    char *linker_command;
};

/**
 * Comprehensive phase result structure
 */
struct PipelinePhaseResult {
    AsthraCompilerPhase phase;
    bool success;
    double start_time;
    double end_time;
    double execution_time_ms;
    
    // Phase-specific data (only one will be valid)
    union {
        LexerPhaseData lexer_data;
        ParserPhaseData parser_data;
        SemanticPhaseData semantic_data;
        CodegenPhaseData codegen_data;
        LinkingPhaseData linking_data;
    } data;
    
    char *error_message;
    char *warning_message;
};

/**
 * Source file context for multi-file compilation
 */
struct SourceFileContext {
    const char *input_path;
    const char *relative_path;
    char *source_code;
    size_t source_length;
    PipelinePhaseResult phases[6]; // One for each compilation phase
    bool processed;
    struct SourceFileContext *next;
};

/**
 * Main pipeline orchestrator structure
 */
struct PipelineOrchestrator {
    AsthraCompilerContext *compiler_context;
    
    // Multi-file compilation support
    SourceFileContext *source_files;
    size_t source_file_count;
    
    // Pipeline state
    AsthraCompilerPhase current_phase;
    bool pipeline_active;
    bool error_occurred;
    
    // Performance profiling
    double total_start_time;
    double total_end_time;
    double phase_times[6]; // Times for each phase
    
    // Resource management
    Lexer **lexers;
    Parser **parsers;
    SemanticAnalyzer *global_analyzer;
    AsthraBackend *backend;
    
    // Output management
    char *output_executable_path;
    char *intermediate_dir;
    char **object_files;
    size_t object_file_count;
    
    // Configuration
    struct {
        bool generate_debug_info;
        bool optimize_pipeline;
        bool parallel_compilation;
        bool save_intermediates;
        bool verbose_output;
        size_t max_errors;
        double timeout_seconds;
    } config;
    
    // Statistics
    struct {
        size_t total_lines_processed;
        size_t total_tokens_processed;
        size_t total_ast_nodes_created;
        size_t total_symbols_created;
        size_t total_instructions_generated;
        size_t total_errors;
        size_t total_warnings;
    } statistics;
};

// =============================================================================
// PIPELINE ORCHESTRATOR FUNCTIONS
// =============================================================================

/**
 * Create a new pipeline orchestrator
 */
PipelineOrchestrator *pipeline_orchestrator_create(AsthraCompilerContext *context);

/**
 * Destroy pipeline orchestrator and free all resources
 */
void pipeline_orchestrator_destroy(PipelineOrchestrator *orchestrator);

/**
 * Execute complete compilation pipeline for a single file
 */
bool pipeline_orchestrator_compile_file(PipelineOrchestrator *orchestrator,
                                       const char *input_file,
                                       const char *output_file);

/**
 * Execute complete compilation pipeline for multiple files
 */
bool pipeline_orchestrator_compile_files(PipelineOrchestrator *orchestrator,
                                        const char **input_files,
                                        size_t file_count,
                                        const char *output_file);

/**
 * Get detailed phase results for analysis
 */
const PipelinePhaseResult *pipeline_orchestrator_get_phase_results(
    PipelineOrchestrator *orchestrator, 
    const char *filename);

/**
 * Get comprehensive compilation statistics
 */
void pipeline_orchestrator_get_statistics(PipelineOrchestrator *orchestrator,
                                         size_t *total_files,
                                         size_t *total_lines,
                                         size_t *total_errors,
                                         double *total_time_ms);

// =============================================================================
// PHASE EXECUTION FUNCTIONS
// =============================================================================

/**
 * Execute lexical analysis phase
 */
bool pipeline_execute_lexer_phase(PipelineOrchestrator *orchestrator,
                                 SourceFileContext *source_context);

/**
 * Execute parsing phase
 */
bool pipeline_execute_parser_phase(PipelineOrchestrator *orchestrator,
                                  SourceFileContext *source_context);

/**
 * Execute semantic analysis phase
 */
bool pipeline_execute_semantic_phase(PipelineOrchestrator *orchestrator,
                                    SourceFileContext *source_context);

/**
 * Execute code generation phase
 */
bool pipeline_execute_codegen_phase(PipelineOrchestrator *orchestrator,
                                   SourceFileContext *source_context);

/**
 * Execute linking phase
 */
bool pipeline_execute_linking_phase(PipelineOrchestrator *orchestrator,
                                   const char *output_executable);

// =============================================================================
// PIPELINE VALIDATION AND OPTIMIZATION
// =============================================================================

/**
 * Validate pipeline state between phases
 */
bool pipeline_validate_phase_transition(PipelineOrchestrator *orchestrator,
                                       AsthraCompilerPhase from_phase,
                                       AsthraCompilerPhase to_phase);

/**
 * Optimize pipeline execution order
 */
bool pipeline_optimize_execution_order(PipelineOrchestrator *orchestrator);

/**
 * Clear pipeline state for next compilation
 */
void pipeline_clear_state(PipelineOrchestrator *orchestrator);

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * Get current timestamp in milliseconds
 */
double pipeline_get_current_time_ms(void);

/**
 * Format phase results for human-readable output
 */
char *pipeline_format_phase_results(const PipelinePhaseResult *result);

/**
 * Get phase name as string
 */
const char *pipeline_get_phase_name(AsthraCompilerPhase phase);

/**
 * Check if pipeline is in valid state for execution
 */
bool pipeline_is_ready_for_execution(PipelineOrchestrator *orchestrator);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_PIPELINE_ORCHESTRATOR_H 
