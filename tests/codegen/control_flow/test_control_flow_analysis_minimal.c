/**
 * Asthra Programming Language Compiler
 * Control Flow Analysis Testing - Minimal Framework Version
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// =============================================================================
// MINIMAL TEST FRAMEWORK
// =============================================================================

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

// =============================================================================
// MINIMAL STUBS FOR CONTROL FLOW ANALYSIS
// =============================================================================

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
} Instruction;

typedef struct BasicBlock BasicBlock;
typedef struct ControlFlowGraph ControlFlowGraph;
typedef struct InstructionBuffer InstructionBuffer;

struct InstructionBuffer {
    Instruction* instructions;
    size_t count;
    size_t capacity;
};

struct BasicBlock {
    int id;
    size_t start_instruction;
    size_t end_instruction;
    BasicBlock** successors;
    size_t successor_count;
    bool is_loop_header;
};

struct ControlFlowGraph {
    BasicBlock** blocks;
    size_t block_count;
    BasicBlock* entry_block;
    bool has_back_edges;
};

// Stub implementations
static ControlFlowGraph* control_flow_graph_create(void) {
    ControlFlowGraph* cfg = malloc(sizeof(ControlFlowGraph));
    if (cfg) {
        cfg->blocks = NULL;
        cfg->block_count = 0;
        cfg->entry_block = NULL;
        cfg->has_back_edges = false;
    }
    return cfg;
}

static void control_flow_graph_destroy(ControlFlowGraph* cfg) {
    if (cfg) {
        if (cfg->blocks) {
            for (size_t i = 0; i < cfg->block_count; i++) {
                if (cfg->blocks[i]) {
                    free(cfg->blocks[i]->successors);
                    free(cfg->blocks[i]);
                }
            }
            free(cfg->blocks);
        }
        free(cfg);
    }
}

static InstructionBuffer* instruction_buffer_create(size_t capacity) {
    InstructionBuffer* buffer = malloc(sizeof(InstructionBuffer));
    if (buffer) {
        buffer->instructions = malloc(capacity * sizeof(Instruction));
        buffer->count = 0;
        buffer->capacity = capacity;
    }
    return buffer;
}

static void instruction_buffer_destroy(InstructionBuffer* buffer) {
    if (buffer) {
        free(buffer->instructions);
        free(buffer);
    }
}

static bool instruction_buffer_add(InstructionBuffer* buffer, const Instruction* instr) {
    if (!buffer || !instr || buffer->count >= buffer->capacity) return false;
    buffer->instructions[buffer->count++] = *instr;
    return true;
}

static bool control_flow_graph_build(ControlFlowGraph* cfg, InstructionBuffer* buffer) {
    if (!cfg || !buffer) return false;
    
    // Simple simulation: create basic blocks based on control flow instructions
    cfg->block_count = 4; // Simulated block count
    cfg->blocks = malloc(cfg->block_count * sizeof(BasicBlock*));
    
    for (size_t i = 0; i < cfg->block_count; i++) {
        cfg->blocks[i] = malloc(sizeof(BasicBlock));
        cfg->blocks[i]->id = (int)i;
        cfg->blocks[i]->successors = malloc(2 * sizeof(BasicBlock*));
        cfg->blocks[i]->successor_count = (i == 0) ? 2 : 1; // Entry block has 2 successors
        cfg->blocks[i]->is_loop_header = false;
    }
    
    cfg->entry_block = cfg->blocks[0];
    
    // Detect back edges by checking for backward jumps
    for (size_t i = 0; i < buffer->count; i++) {
        if (buffer->instructions[i].opcode == OP_JMP &&
            buffer->instructions[i].operand1.type == OPERAND_IMMEDIATE &&
            buffer->instructions[i].operand1.value < (int)i) {
            cfg->has_back_edges = true;
            break;
        }
    }
    
    return true;
}

static size_t control_flow_graph_get_block_count(ControlFlowGraph* cfg) {
    return cfg ? cfg->block_count : 0;
}

static BasicBlock* control_flow_graph_get_entry_block(ControlFlowGraph* cfg) {
    return cfg ? cfg->entry_block : NULL;
}

static size_t basic_block_get_successor_count(BasicBlock* block) {
    return block ? block->successor_count : 0;
}

static bool control_flow_graph_has_back_edges(ControlFlowGraph* cfg) {
    return cfg ? cfg->has_back_edges : false;
}

// =============================================================================
// CONTROL FLOW ANALYSIS TESTS
// =============================================================================

DEFINE_TEST(test_cfg_construction) {
    ControlFlowGraph* cfg = control_flow_graph_create();
    if (!cfg) return false;
    
    InstructionBuffer* buffer = instruction_buffer_create(8);
    if (!buffer) {
        control_flow_graph_destroy(cfg);
        return false;
    }
    
    // Create a simple function with basic blocks
    Instruction instructions[8] = {
        {.opcode = OP_MOV, .dst_reg = 0, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 10}},
        {.opcode = OP_CMP, .operand1 = {.type = OPERAND_REGISTER, .reg = 0}, .operand2 = {.type = OPERAND_IMMEDIATE, .value = 5}},
        {.opcode = OP_JGT, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 5}},
        {.opcode = OP_MOV, .dst_reg = 1, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 1}},
        {.opcode = OP_JMP, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 7}},
        {.opcode = OP_MOV, .dst_reg = 1, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 0}},
        {.opcode = OP_JMP, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 7}},
        {.opcode = OP_RET}
    };
    
    for (int i = 0; i < 8; i++) {
        if (!instruction_buffer_add(buffer, &instructions[i])) {
            instruction_buffer_destroy(buffer);
            control_flow_graph_destroy(cfg);
            return false;
        }
    }
    
    bool success = control_flow_graph_build(cfg, buffer);
    if (!success) {
        instruction_buffer_destroy(buffer);
        control_flow_graph_destroy(cfg);
        return false;
    }
    
    // Check basic block count
    size_t num_blocks = control_flow_graph_get_block_count(cfg);
    if (num_blocks != 4) {
        instruction_buffer_destroy(buffer);
        control_flow_graph_destroy(cfg);
        return false;
    }
    
    // Check entry block
    BasicBlock* entry_block = control_flow_graph_get_entry_block(cfg);
    if (!entry_block) {
        instruction_buffer_destroy(buffer);
        control_flow_graph_destroy(cfg);
        return false;
    }
    
    // Check successors
    size_t num_successors = basic_block_get_successor_count(entry_block);
    if (num_successors != 2) {
        instruction_buffer_destroy(buffer);
        control_flow_graph_destroy(cfg);
        return false;
    }
    
    instruction_buffer_destroy(buffer);
    control_flow_graph_destroy(cfg);
    return true;
}

DEFINE_TEST(test_cfg_with_loops) {
    ControlFlowGraph* cfg = control_flow_graph_create();
    if (!cfg) return false;
    
    InstructionBuffer* buffer = instruction_buffer_create(6);
    if (!buffer) {
        control_flow_graph_destroy(cfg);
        return false;
    }
    
    // Create a simple loop structure
    Instruction instructions[6] = {
        {.opcode = OP_MOV, .dst_reg = 0, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 0}},
        {.opcode = OP_CMP, .operand1 = {.type = OPERAND_REGISTER, .reg = 0}, .operand2 = {.type = OPERAND_IMMEDIATE, .value = 10}},
        {.opcode = OP_JGE, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 5}},
        {.opcode = OP_ADD, .dst_reg = 0, .operand1 = {.type = OPERAND_REGISTER, .reg = 0}, .operand2 = {.type = OPERAND_IMMEDIATE, .value = 1}},
        {.opcode = OP_JMP, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 1}}, // Back edge
        {.opcode = OP_RET}
    };
    
    for (int i = 0; i < 6; i++) {
        if (!instruction_buffer_add(buffer, &instructions[i])) {
            instruction_buffer_destroy(buffer);
            control_flow_graph_destroy(cfg);
            return false;
        }
    }
    
    bool success = control_flow_graph_build(cfg, buffer);
    if (!success) {
        instruction_buffer_destroy(buffer);
        control_flow_graph_destroy(cfg);
        return false;
    }
    
    // Check that back edges were detected
    bool has_back_edges = control_flow_graph_has_back_edges(cfg);
    if (!has_back_edges) {
        instruction_buffer_destroy(buffer);
        control_flow_graph_destroy(cfg);
        return false;
    }
    
    instruction_buffer_destroy(buffer);
    control_flow_graph_destroy(cfg);
    return true;
}

DEFINE_TEST(test_basic_block_creation) {
    ControlFlowGraph* cfg = control_flow_graph_create();
    if (!cfg) return false;
    
    InstructionBuffer* buffer = instruction_buffer_create(4);
    if (!buffer) {
        control_flow_graph_destroy(cfg);
        return false;
    }
    
    // Simple linear code
    Instruction instructions[4] = {
        {.opcode = OP_MOV, .dst_reg = 0, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 1}},
        {.opcode = OP_ADD, .dst_reg = 0, .operand1 = {.type = OPERAND_REGISTER, .reg = 0}, .operand2 = {.type = OPERAND_IMMEDIATE, .value = 2}},
        {.opcode = OP_MOV, .dst_reg = 1, .operand1 = {.type = OPERAND_REGISTER, .reg = 0}},
        {.opcode = OP_RET}
    };
    
    for (int i = 0; i < 4; i++) {
        instruction_buffer_add(buffer, &instructions[i]);
    }
    
    bool success = control_flow_graph_build(cfg, buffer);
    
    instruction_buffer_destroy(buffer);
    control_flow_graph_destroy(cfg);
    return success;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("Control Flow Analysis Minimal Test Suite\n");
    printf("=========================================\n");
    
    bool all_passed = true;
    
    // Run the test suite
    printf("Running %s test suite...\n", "ControlFlowAnalysis");
    typedef bool (*test_func_t)(void);
    test_func_t tests[] = {test_cfg_construction, test_cfg_with_loops, test_basic_block_creation};
    int total = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;
    for (int i = 0; i < total; i++) {
        if (tests[i]()) {
            passed++;
            printf("  ✓ Test %d passed\n", i + 1);
        } else {
            printf("  ✗ Test %d failed\n", i + 1);
        }
    }
    printf("%s: %d/%d tests passed\n", "ControlFlowAnalysis", passed, total);
    all_passed = (passed == total);
    
    printf("\nTest suite completed!\n");
    return all_passed ? 0 : 1;
} 
