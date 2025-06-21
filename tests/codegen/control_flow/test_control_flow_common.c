/**
 * Common implementation for control flow analysis tests
 * Shared fixtures and utilities
 */

#include "test_control_flow_common.h"

/**
 * Setup test fixture for control flow analysis
 */
ControlFlowAnalysisTestFixture* setup_control_flow_analysis_fixture(void) {
    ControlFlowAnalysisTestFixture* fixture = calloc(1, sizeof(ControlFlowAnalysisTestFixture));
    if (!fixture) return NULL;
    
    fixture->cfg = control_flow_graph_create();
    if (!fixture->cfg) {
        free(fixture);
        return NULL;
    }
    
    fixture->instruction_buffer = instruction_buffer_create(1024);
    if (!fixture->instruction_buffer) {
        control_flow_graph_destroy(fixture->cfg);
        free(fixture);
        return NULL;
    }
    
    fixture->dominator_analysis = dominator_analysis_create();
    if (!fixture->dominator_analysis) {
        instruction_buffer_destroy(fixture->instruction_buffer);
        control_flow_graph_destroy(fixture->cfg);
        free(fixture);
        return NULL;
    }
    
    fixture->loop_analysis = loop_analysis_create();
    if (!fixture->loop_analysis) {
        dominator_analysis_destroy(fixture->dominator_analysis);
        instruction_buffer_destroy(fixture->instruction_buffer);
        control_flow_graph_destroy(fixture->cfg);
        free(fixture);
        return NULL;
    }
    
    return fixture;
}

/**
 * Cleanup test fixture
 */
void cleanup_control_flow_analysis_fixture(ControlFlowAnalysisTestFixture* fixture) {
    if (!fixture) return;
    
    if (fixture->test_ast) {
        ast_free_node(fixture->test_ast);
    }
    if (fixture->loop_analysis) {
        loop_analysis_destroy(fixture->loop_analysis);
    }
    if (fixture->dominator_analysis) {
        dominator_analysis_destroy(fixture->dominator_analysis);
    }
    if (fixture->instruction_buffer) {
        instruction_buffer_destroy(fixture->instruction_buffer);
    }
    if (fixture->cfg) {
        control_flow_graph_destroy(fixture->cfg);
    }
    free(fixture);
}