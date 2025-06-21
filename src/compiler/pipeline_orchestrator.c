/**
 * Asthra Programming Language Compiler
 * Pipeline Orchestrator - Include Aggregator
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This file now serves as an include aggregator for pipeline orchestration.
 * The actual implementation is split across specialized modules.
 */

#include "pipeline_orchestrator.h"
#include "pipeline_core.h"
#include "pipeline_phases.h"
#include "pipeline_utils.h"

// This file now serves as an include aggregator for pipeline orchestration.
// The actual functions are implemented in the specialized modules:
//
// - pipeline_core.h: pipeline_orchestrator_create, pipeline_orchestrator_destroy,
//                    pipeline_orchestrator_compile_file, pipeline_orchestrator_compile_files
// - pipeline_phases.h: pipeline_execute_lexer_phase, pipeline_execute_parser_phase,
//                      pipeline_execute_semantic_phase, pipeline_execute_codegen_phase,
//                      pipeline_execute_linking_phase
// - pipeline_utils.h: pipeline_get_current_time_ms, pipeline_get_phase_name,
//                     pipeline_clear_state, pipeline_is_ready_for_execution,
//                     pipeline_orchestrator_get_statistics, pipeline_orchestrator_get_phase_results
//
// All functionality is now available through their respective headers.