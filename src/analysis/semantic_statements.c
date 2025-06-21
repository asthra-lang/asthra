/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Statement Analysis Module Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Statement analysis dispatcher - delegates to specialized modules
 */

#include "semantic_statements.h"
#include "semantic_variables.h"
#include "semantic_basic_statements.h"
#include "semantic_control_flow.h"
#include "semantic_loops.h"
#include "semantic_concurrency.h"
#include "semantic_patterns.h"
#include "semantic_core.h"
#include "semantic_symbols.h"
#include "semantic_types.h"
#include "semantic_utilities.h"
#include "../parser/ast_node_list.h"
#include <stdlib.h>
#include <string.h>

// This file now serves as an include aggregator for statement analysis.
// The actual semantic_analyze_statement function is implemented in semantic_core.c
// and calls the specialized functions from the included modules.

// All statement analysis functions are now available through their respective headers:
// - semantic_variables.h: analyze_let_statement, analyze_assignment_validation
// - semantic_basic_statements.h: analyze_block_statement, analyze_expression_statement
// - semantic_control_flow.h: analyze_return_statement, analyze_if_statement, analyze_if_let_statement, analyze_match_statement
// - semantic_loops.h: analyze_for_statement, analyze_while_statement
// - semantic_concurrency.h: analyze_spawn_statement, analyze_spawn_with_handle_statement, analyze_await_statement
// - semantic_patterns.h: analyze_unsafe_block_statement, semantic_validate_pattern_types