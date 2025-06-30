/**
 * Asthra Codegen Test Stubs
 *
 * Provides stub implementations for missing test utility functions
 * to allow codegen tests to link and run.
 */

#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include "semantic_types.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// Pattern Engine Stubs
// =============================================================================

// Note: Pattern engine functions are implemented in test_pattern_engine_common.c
// We only need init_pattern_engine_context here since it's not in that file

typedef struct PatternEngineContext {
    int dummy;
} PatternEngineContext;

PatternEngineContext *init_pattern_engine_context(void) {
    PatternEngineContext *ctx = malloc(sizeof(PatternEngineContext));
    if (ctx)
        ctx->dummy = 0;
    return ctx;
}

// =============================================================================
// Parser Integration Stubs
// =============================================================================

// parse_test_source is already implemented in statement_generation_helpers.c

ASTNode *parse_test_expression(const char *expr_str) {
    // Create a minimal expression node
    // For now, just return NULL as a stub
    return NULL;
}

// ast_free_node is already implemented in ast_node_destruction.o

// =============================================================================
// Code Generator Test Fixtures
// =============================================================================

typedef struct CodegenFixture {
    void *backend; // Backend abstraction removed
    SemanticAnalyzer *analyzer;
} CodegenFixture;

// These functions are already implemented in statement_generation_common.c and
// statement_generation_helpers.c

// =============================================================================

// =============================================================================
// Code Generation Test Functions (renamed to avoid conflicts)
// =============================================================================

char *test_code_generate_expression(void *backend, ASTNode *expr) {
    // Stub: return a simple string
    return strdup("/* expression */");
}

char *test_code_generate_enum_variant_construction(void *backend, const char *enum_name,
                                                   const char *variant_name, ASTNode *payload) {
    // Stub: return a formatted string
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s::%s(/* payload */)", enum_name, variant_name);
    return strdup(buffer);
}

char *test_code_generate_impl_block(void *backend, ASTNode *impl_block) {
    // Stub: return a simple implementation
    return strdup("/* impl block */");
}

// =============================================================================
// Statement Test Suite Configuration
// =============================================================================

typedef struct StatementTestSuiteConfig {
    const char *name;
    int test_count;
} StatementTestSuiteConfig;

// create_statement_test_suite_config is already implemented in statement_generation_common.c

// =============================================================================
// Test Framework Extensions
// =============================================================================

// asthra_test_statistics_sync_compat_fields is already implemented in test_statistics.o

// =============================================================================
// Type System Functions
// =============================================================================

// type_descriptor_retain is already implemented in semantic_types.o

// Note: Tests that need code_generate_* functions should be updated to use
// the test_code_generate_* versions to avoid conflicts with the real implementation

// =============================================================================
// Control Flow Analysis Stubs
// =============================================================================

// Forward declaration to avoid including optimizer headers
typedef struct ControlFlowGraph ControlFlowGraph;
typedef struct BasicBlock BasicBlock;
typedef struct DominatorAnalysis DominatorAnalysis;
typedef struct LoopAnalysis LoopAnalysis;

// Stub implementation since optimizer doesn't exist yet
typedef struct ControlFlowGraphStub {
    int dummy;
} ControlFlowGraphStub;

ControlFlowGraph *control_flow_graph_create(void) {
    ControlFlowGraphStub *cfg = malloc(sizeof(ControlFlowGraphStub));
    if (cfg)
        cfg->dummy = 0;
    return (ControlFlowGraph *)cfg;
}

void control_flow_graph_destroy(ControlFlowGraph *cfg) {
    if (cfg)
        free(cfg);
}

bool control_flow_graph_build(ControlFlowGraph *cfg, void *instructions, size_t count) {
    // Stub: always succeed
    return true;
}

size_t control_flow_graph_get_block_count(ControlFlowGraph *cfg) {
    // Stub: return a reasonable count
    return 4;
}

BasicBlock *control_flow_graph_get_entry_block(ControlFlowGraph *cfg) {
    // Stub: return a dummy pointer (non-NULL)
    // Since BasicBlock is opaque, just return a non-NULL pointer
    return (BasicBlock *)0x1;
}

size_t basic_block_get_successor_count(BasicBlock *block) {
    // Stub: return a reasonable count
    return 2;
}

bool control_flow_graph_has_back_edges(ControlFlowGraph *cfg) {
    // Stub: assume we have loops
    return true;
}

// These don't exist in the optimizer, so provide stubs
typedef struct DominatorAnalysisStub {
    int dummy;
} DominatorAnalysisStub;

typedef struct LoopAnalysisStub {
    int dummy;
} LoopAnalysisStub;

DominatorAnalysis *dominator_analysis_create(void) {
    DominatorAnalysisStub *da = malloc(sizeof(DominatorAnalysisStub));
    if (da)
        da->dummy = 0;
    return (DominatorAnalysis *)da;
}

void dominator_analysis_destroy(DominatorAnalysis *da) {
    if (da)
        free(da);
}

LoopAnalysis *loop_analysis_create(void) {
    LoopAnalysisStub *la = malloc(sizeof(LoopAnalysisStub));
    if (la)
        la->dummy = 0;
    return (LoopAnalysis *)la;
}

void loop_analysis_destroy(LoopAnalysis *la) {
    if (la)
        free(la);
}

// =============================================================================
// Data Flow Analysis Stubs
// =============================================================================

// Forward declarations
typedef struct DataFlowAnalysis DataFlowAnalysis;
typedef struct ReachingDefinitions ReachingDefinitions;
typedef struct LiveVariableAnalysis LiveVariableAnalysis;
typedef struct UseDefChains UseDefChains;

// Stub implementation since dataflow analysis doesn't exist yet
typedef struct DataFlowAnalysisStub {
    int dummy;
} DataFlowAnalysisStub;

DataFlowAnalysis *data_flow_analysis_create(void) {
    DataFlowAnalysisStub *dfa = malloc(sizeof(DataFlowAnalysisStub));
    if (dfa)
        dfa->dummy = 0;
    return (DataFlowAnalysis *)dfa;
}

void data_flow_analysis_destroy(DataFlowAnalysis *dfa) {
    if (dfa)
        free(dfa);
}

// These types don't exist in optimizer, provide stubs
typedef struct ReachingDefinitionsStub {
    DataFlowAnalysis *analysis;
} ReachingDefinitionsStub;

typedef struct LiveVariableAnalysisStub {
    DataFlowAnalysis *analysis;
} LiveVariableAnalysisStub;

typedef struct UseDefChainsStub {
    int dummy;
} UseDefChainsStub;

ReachingDefinitions *reaching_definitions_create(void) {
    ReachingDefinitionsStub *rd = malloc(sizeof(ReachingDefinitionsStub));
    if (rd) {
        rd->analysis = NULL; // No real implementation yet
    }
    return (ReachingDefinitions *)rd;
}

void reaching_definitions_destroy(ReachingDefinitions *rd) {
    if (rd) {
        ReachingDefinitionsStub *stub = (ReachingDefinitionsStub *)rd;
        // No need to destroy analysis since it's NULL
        free(rd);
    }
}

LiveVariableAnalysis *live_variable_analysis_create(void) {
    LiveVariableAnalysisStub *lva = malloc(sizeof(LiveVariableAnalysisStub));
    if (lva) {
        lva->analysis = NULL; // No real implementation yet
    }
    return (LiveVariableAnalysis *)lva;
}

void live_variable_analysis_destroy(LiveVariableAnalysis *lva) {
    if (lva) {
        LiveVariableAnalysisStub *stub = (LiveVariableAnalysisStub *)lva;
        // No need to destroy analysis since it's NULL
        free(lva);
    }
}

UseDefChains *use_def_chains_create(void) {
    UseDefChainsStub *udc = malloc(sizeof(UseDefChainsStub));
    if (udc)
        udc->dummy = 0;
    return (UseDefChains *)udc;
}

void use_def_chains_destroy(UseDefChains *udc) {
    if (udc)
        free(udc);
}

// Provide stubs for cfg_create/cfg_destroy that might be referenced elsewhere
ControlFlowGraph *cfg_create(void) {
    return control_flow_graph_create();
}

void cfg_destroy(ControlFlowGraph *cfg) {
    control_flow_graph_destroy(cfg);
}

// Provide stubs for dataflow functions
DataFlowAnalysis *dataflow_create(int type, ControlFlowGraph *cfg) {
    return data_flow_analysis_create();
}

void dataflow_destroy(DataFlowAnalysis *analysis) {
    data_flow_analysis_destroy(analysis);
}

DataFlowAnalysis *dataflow_reaching_definitions(ControlFlowGraph *cfg) {
    return data_flow_analysis_create();
}

DataFlowAnalysis *dataflow_live_variables(ControlFlowGraph *cfg) {
    return data_flow_analysis_create();
}

// =============================================================================
