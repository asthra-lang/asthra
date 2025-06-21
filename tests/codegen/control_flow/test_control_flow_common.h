/**
 * Common header for control flow analysis tests
 * Shared fixtures and utilities for CFG, dominator, and loop tests
 */

#ifndef TEST_CONTROL_FLOW_COMMON_H
#define TEST_CONTROL_FLOW_COMMON_H

#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#include "optimizer.h"
#include "optimizer_cfg.h"
#include "code_generator_instructions.h"
#include "ast.h"
#include "codegen_test_stubs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Test fixture for control flow analysis testing
typedef struct {
    ControlFlowGraph* cfg;
    InstructionBuffer* instruction_buffer;
    DominatorAnalysis* dominator_analysis;
    LoopAnalysis* loop_analysis;
    ASTNode* test_ast;
} ControlFlowAnalysisTestFixture;

// Function prototypes
ControlFlowAnalysisTestFixture* setup_control_flow_analysis_fixture(void);
void cleanup_control_flow_analysis_fixture(ControlFlowAnalysisTestFixture* fixture);

#endif // TEST_CONTROL_FLOW_COMMON_H