/**
 * Common header for data flow analysis tests
 * Shared fixtures and utilities for reaching definitions, live variables, and use-def chains
 */

#ifndef TEST_DATA_FLOW_COMMON_H
#define TEST_DATA_FLOW_COMMON_H

#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Forward declarations to avoid including conflicting headers
typedef struct DataFlowAnalysis DataFlowAnalysis;
typedef struct InstructionBuffer InstructionBuffer;
typedef struct ControlFlowGraph ControlFlowGraph;
typedef struct ReachingDefinitions ReachingDefinitions;
typedef struct LiveVariableAnalysis LiveVariableAnalysis;
typedef struct UseDefChains UseDefChains;
typedef struct BasicBlock BasicBlock;
typedef struct DefinitionSet DefinitionSet;
typedef struct LivenessSet LivenessSet;
typedef struct DefList DefList;
typedef struct Definition {
    int instruction_index;
    int variable;
} Definition;
typedef struct ASTNode ASTNode;

// Minimal instruction types for testing
typedef enum {
    OP_MOV, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_CMP, OP_JMP, OP_JGT, OP_JGE, OP_JZ, OP_RET, OP_NOP
} OpCode;

typedef enum {
    OPERAND_REGISTER, OPERAND_IMMEDIATE
} OperandType;

typedef struct {
    OperandType type;
    union {
        int reg;
        int value;
    };
} Operand;

typedef struct {
    OpCode opcode;
    int dst_reg;
    Operand operand1;
    Operand operand2;
} Instruction;

// Test fixture for data flow analysis testing
typedef struct {
    DataFlowAnalysis* data_flow;
    InstructionBuffer* instruction_buffer;
    ControlFlowGraph* cfg;
    ReachingDefinitions* reaching_defs;
    LiveVariableAnalysis* live_vars;
    UseDefChains* use_def_chains;
    ASTNode* test_ast;
} DataFlowAnalysisTestFixture;

// Function prototypes
DataFlowAnalysisTestFixture* setup_data_flow_analysis_fixture(void);
void cleanup_data_flow_analysis_fixture(DataFlowAnalysisTestFixture* fixture);

// Stub function declarations
DataFlowAnalysis* data_flow_analysis_create(void);
void data_flow_analysis_destroy(DataFlowAnalysis* analysis);
InstructionBuffer* instruction_buffer_create(size_t capacity);
void instruction_buffer_destroy(InstructionBuffer* buffer);
bool instruction_buffer_add(InstructionBuffer* buffer, Instruction* instruction);
ControlFlowGraph* control_flow_graph_create(void);
void control_flow_graph_destroy(ControlFlowGraph* cfg);
bool control_flow_graph_build(ControlFlowGraph* cfg, InstructionBuffer* buffer);
BasicBlock* control_flow_graph_get_entry_block(ControlFlowGraph* cfg);
BasicBlock* control_flow_graph_get_block_by_index(ControlFlowGraph* cfg, int index);
ReachingDefinitions* reaching_definitions_create(void);
void reaching_definitions_destroy(ReachingDefinitions* rd);
bool reaching_definitions_analyze(ReachingDefinitions* rd, ControlFlowGraph* cfg);
DefinitionSet* reaching_definitions_get_reaching_set(ReachingDefinitions* rd, BasicBlock* block);
size_t definition_set_size(DefinitionSet* set);
LiveVariableAnalysis* live_variable_analysis_create(void);
void live_variable_analysis_destroy(LiveVariableAnalysis* lva);
bool live_variable_analysis_analyze(LiveVariableAnalysis* lva, ControlFlowGraph* cfg);
LivenessSet* live_variable_analysis_get_live_in(LiveVariableAnalysis* lva, BasicBlock* block, int instruction);
LivenessSet* live_variable_analysis_get_live_out(LiveVariableAnalysis* lva, BasicBlock* block);
bool liveness_set_contains(LivenessSet* set, int reg);
UseDefChains* use_def_chains_create(void);
void use_def_chains_destroy(UseDefChains* udc);
bool use_def_chains_build(UseDefChains* udc, ControlFlowGraph* cfg);
DefList* use_def_chains_get_defs_for_use(UseDefChains* udc, int instruction, int operand);
size_t def_list_size(DefList* list);
Definition* def_list_get(DefList* list, int index);
void ast_free_node(ASTNode* node);

#endif // TEST_DATA_FLOW_COMMON_H