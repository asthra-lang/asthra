/**
 * Common header for control flow analysis tests
 * Shared fixtures and utilities for CFG, dominator, and loop tests
 */

#ifndef TEST_CONTROL_FLOW_COMMON_H
#define TEST_CONTROL_FLOW_COMMON_H

#include "../framework/compiler_test_utils.h"
#include "../framework/test_framework.h"
#include "ast.h"
#include "code_generator_instructions.h"
// #include "codegen_test_stubs.h" - removed, header deleted
#include "optimizer.h"
#include "optimizer_cfg.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test fixture for control flow analysis testing
typedef struct {
    ControlFlowGraph *cfg;
    InstructionBuffer *instruction_buffer;
    DominatorAnalysis *dominator_analysis;
    LoopAnalysis *loop_analysis;
    ASTNode *test_ast;
} ControlFlowAnalysisTestFixture;

// Function prototypes
ControlFlowAnalysisTestFixture *setup_control_flow_analysis_fixture(void);
void cleanup_control_flow_analysis_fixture(ControlFlowAnalysisTestFixture *fixture);

#endif // TEST_CONTROL_FLOW_COMMON_H