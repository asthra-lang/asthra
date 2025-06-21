/*
 * Common Header for Data Flow Analysis Tests
 * Shared definitions, types, and function declarations for data flow analysis tests
 * 
 * Part of test_data_flow_analysis_minimal.c split (585 lines -> 6 focused modules)
 * Provides comprehensive data flow analysis test infrastructure
 */

#ifndef TEST_DATA_FLOW_ANALYSIS_COMMON_H
#define TEST_DATA_FLOW_ANALYSIS_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// ============================================================================
// MINIMAL TEST FRAMEWORK
// ============================================================================

#define DEFINE_TEST(name) static bool name(void)

#define RUN_TEST_SUITE(suite_name, ...) \
    do { \
        printf("Running %s test suite...\n", #suite_name); \
        typedef bool (*test_func_t)(void); \
        test_func_t tests[] = {__VA_ARGS__}; \
        int total = sizeof(tests) / sizeof(tests[0]); \
        int passed = 0; \
        for (int i = 0; i < total; i++) { \
            if (tests[i]()) { \
                passed++; \
                printf("  ✓ Test %d passed\n", i + 1); \
            } else { \
                printf("  ✗ Test %d failed\n", i + 1); \
            } \
        } \
        printf("%s: %d/%d tests passed\n", #suite_name, passed, total); \
        return passed == total; \
    } while (0)

// ============================================================================
// DATA FLOW ANALYSIS TYPE DEFINITIONS
// ============================================================================

typedef enum {
    OP_MOV, OP_ADD, OP_SUB, OP_CMP, OP_JMP, OP_JGT, OP_JGE, OP_RET
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
    bool is_definition;
    bool is_use;
} Instruction;

// Forward declarations
typedef struct DataFlowAnalysis DataFlowAnalysis;
typedef struct InstructionBuffer InstructionBuffer;
typedef struct ControlFlowGraph ControlFlowGraph;
typedef struct ReachingDefinitions ReachingDefinitions;
typedef struct LiveVariableAnalysis LiveVariableAnalysis;
typedef struct UseDefChains UseDefChains;
typedef struct BasicBlock BasicBlock;
typedef struct DefinitionSet DefinitionSet;

// ============================================================================
// DATA STRUCTURE DEFINITIONS
// ============================================================================

struct InstructionBuffer {
    Instruction* instructions;
    size_t count;
    size_t capacity;
};

struct BasicBlock {
    int id;
    size_t start_instruction;
    size_t end_instruction;
};

struct ControlFlowGraph {
    BasicBlock** blocks;
    size_t block_count;
};

struct DefinitionSet {
    int* definitions;
    size_t count;
    size_t capacity;
};

struct DataFlowAnalysis {
    bool initialized;
    ControlFlowGraph* cfg;
};

struct ReachingDefinitions {
    bool initialized;
    DefinitionSet** block_sets;
    size_t num_blocks;
};

struct LiveVariableAnalysis {
    bool initialized;
    int* live_vars;
    size_t num_vars;
};

struct UseDefChains {
    bool initialized;
    int* chains;
    size_t num_chains;
};

// ============================================================================
// CORE DATA FLOW ANALYSIS FUNCTIONS
// ============================================================================

/**
 * Create and destroy data flow analysis context
 */
DataFlowAnalysis* data_flow_analysis_create(void);
void data_flow_analysis_destroy(DataFlowAnalysis* dfa);

// ============================================================================
// INSTRUCTION BUFFER FUNCTIONS
// ============================================================================

/**
 * Create and manage instruction buffers
 */
InstructionBuffer* instruction_buffer_create(size_t capacity);
void instruction_buffer_destroy(InstructionBuffer* buffer);
bool instruction_buffer_add(InstructionBuffer* buffer, const Instruction* instr);

// ============================================================================
// CONTROL FLOW GRAPH FUNCTIONS
// ============================================================================

/**
 * Create and manage control flow graphs
 */
ControlFlowGraph* control_flow_graph_create(void);
void control_flow_graph_destroy(ControlFlowGraph* cfg);
bool control_flow_graph_build(ControlFlowGraph* cfg, InstructionBuffer* buffer);
BasicBlock* control_flow_graph_get_block_by_index(ControlFlowGraph* cfg, size_t index);

// ============================================================================
// REACHING DEFINITIONS ANALYSIS FUNCTIONS
// ============================================================================

/**
 * Create and manage reaching definitions analysis
 */
ReachingDefinitions* reaching_definitions_create(void);
void reaching_definitions_destroy(ReachingDefinitions* rd);
bool reaching_definitions_analyze(ReachingDefinitions* rd, ControlFlowGraph* cfg);
DefinitionSet* reaching_definitions_get_reaching_set(ReachingDefinitions* rd, BasicBlock* block);

/**
 * Definition set utilities
 */
size_t definition_set_size(DefinitionSet* set);

// ============================================================================
// LIVE VARIABLE ANALYSIS FUNCTIONS
// ============================================================================

/**
 * Create and manage live variable analysis
 */
LiveVariableAnalysis* live_variable_analysis_create(void);
void live_variable_analysis_destroy(LiveVariableAnalysis* lva);
bool live_variable_analysis_analyze(LiveVariableAnalysis* lva, ControlFlowGraph* cfg);
bool live_variable_analysis_is_live(LiveVariableAnalysis* lva, int var_id, BasicBlock* block);

// ============================================================================
// USE-DEF CHAINS ANALYSIS FUNCTIONS
// ============================================================================

/**
 * Create and manage use-def chains analysis
 */
UseDefChains* use_def_chains_create(void);
void use_def_chains_destroy(UseDefChains* udc);
bool use_def_chains_build(UseDefChains* udc, ControlFlowGraph* cfg, ReachingDefinitions* rd);
size_t use_def_chains_get_chain_count(UseDefChains* udc);

// ============================================================================
// TEST FUNCTION DECLARATIONS
// ============================================================================

/**
 * Individual test functions are defined using DEFINE_TEST macro
 * in their respective test files
 */

#endif // TEST_DATA_FLOW_ANALYSIS_COMMON_H 
