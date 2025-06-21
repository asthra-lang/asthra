/**
 * Asthra Codegen Test Stubs Header
 * 
 * Declarations for test utility stub functions
 */

#ifndef ASTHRA_CODEGEN_TEST_STUBS_H
#define ASTHRA_CODEGEN_TEST_STUBS_H

#include <stdbool.h>
#include "ast.h"
#include "code_generator.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct PatternEngineContext PatternEngineContext;
typedef struct Pattern Pattern;
typedef struct MatchExpression MatchExpression;
typedef struct CodegenFixture CodegenFixture;
typedef struct ELFWriterFixture ELFWriterFixture;
typedef struct Variant Variant;
typedef struct StatementTestSuiteConfig StatementTestSuiteConfig;

// Pattern Engine Functions
// Note: Most pattern functions are in test_pattern_engine_common.c
// Only init_pattern_engine_context is provided here
PatternEngineContext* init_pattern_engine_context(void);

// Parser Integration
ASTNode* parse_test_expression(const char* expr_str);
// Note: parse_test_source and ast_free_node are in existing files

// Codegen Fixtures
// Note: setup/cleanup functions are in statement_generation_common.c
void cleanup_elf_writer_fixture(ELFWriterFixture* fixture);

// Variant Creation
// Note: variant creation functions are in test_variant_types_common.c

// Code Generation Test Functions
char* test_code_generate_expression(CodeGenerator* gen, ASTNode* expr);
char* test_code_generate_enum_variant_construction(CodeGenerator* gen, const char* enum_name, 
                                             const char* variant_name, ASTNode* payload);
char* test_code_generate_impl_block(CodeGenerator* gen, ASTNode* impl_block);

// Note: test_generate_while_loops is in test_control_flow_statements.c

// Test Suite Configuration
// Note: create_statement_test_suite_config is in statement_generation_common.c

// Test Framework Extensions
// Note: asthra_test_statistics_sync_compat_fields is in test_statistics.o

// Type System
// Note: TypeDescriptor is defined in semantic_types.h, don't redeclare here

// Control Flow Analysis
typedef struct ControlFlowGraph ControlFlowGraph;
typedef struct DominatorAnalysis DominatorAnalysis;
typedef struct LoopAnalysis LoopAnalysis;
typedef struct BasicBlock BasicBlock;

ControlFlowGraph* control_flow_graph_create(void);
void control_flow_graph_destroy(ControlFlowGraph* cfg);
bool control_flow_graph_build(ControlFlowGraph* cfg, void* instructions, size_t count);
size_t control_flow_graph_get_block_count(ControlFlowGraph* cfg);
BasicBlock* control_flow_graph_get_entry_block(ControlFlowGraph* cfg);
size_t basic_block_get_successor_count(BasicBlock* block);
bool control_flow_graph_has_back_edges(ControlFlowGraph* cfg);

DominatorAnalysis* dominator_analysis_create(void);
void dominator_analysis_destroy(DominatorAnalysis* da);
LoopAnalysis* loop_analysis_create(void);
void loop_analysis_destroy(LoopAnalysis* la);
size_t loop_analysis_get_loop_count(LoopAnalysis* la);
bool loop_analysis_has_natural_loops(LoopAnalysis* la);
bool loop_analysis_detect_loops(LoopAnalysis* la, ControlFlowGraph* cfg);
int loop_analysis_get_max_nesting_depth(LoopAnalysis* la);

// Data Flow Analysis
typedef struct DataFlowAnalysis DataFlowAnalysis;
typedef struct ReachingDefinitions ReachingDefinitions;
typedef struct LiveVariableAnalysis LiveVariableAnalysis;
typedef struct UseDefChains UseDefChains;

DataFlowAnalysis* data_flow_analysis_create(void);
void data_flow_analysis_destroy(DataFlowAnalysis* dfa);
ReachingDefinitions* reaching_definitions_create(void);
void reaching_definitions_destroy(ReachingDefinitions* rd);
LiveVariableAnalysis* live_variable_analysis_create(void);
void live_variable_analysis_destroy(LiveVariableAnalysis* lva);
UseDefChains* use_def_chains_create(void);
void use_def_chains_destroy(UseDefChains* udc);

// ELF Debug Information
typedef struct ELFLineNumberEntry ELFLineNumberEntry;
typedef struct ELFDebugSection ELFDebugSection;
typedef struct ELFSegment ELFSegment;

typedef enum {
    VAR_LOCATION_REGISTER,
    VAR_LOCATION_STACK,
    VAR_LOCATION_MEMORY,
    VAR_LOCATION_COMPLEX
} VarLocationType;

ELFWriterFixture* setup_elf_writer_fixture(void);
bool elf_writer_add_line_number_entry(void* writer, const ELFLineNumberEntry* entry);
bool elf_writer_generate_line_number_table(void* writer);
size_t elf_writer_get_line_table_size(void* writer);
bool elf_writer_sort_line_entries(void* writer);
bool elf_writer_validate_line_entry_ordering(void* writer);
void elf_writer_set_debug_enabled(void* writer, bool enabled);
bool elf_writer_add_debug_section(void* writer, const ELFDebugSection* section);
bool elf_writer_generate_debug_sections(void* writer);
size_t elf_writer_get_debug_section_count(void* writer);
void elf_writer_set_debug_compression(void* writer, bool enabled);
bool elf_writer_generate_compressed_debug_info(void* writer);
size_t elf_writer_get_uncompressed_debug_size(void* writer);
size_t elf_writer_get_compressed_debug_size(void* writer);
bool elf_writer_validate_debug_info(void* writer);
bool elf_writer_generate_variable_debug_info(void* writer);
size_t elf_writer_get_debug_info_size(void* writer);
bool elf_writer_generate_variable_locations(void* writer);
bool elf_writer_validate_variable_locations(void* writer);
bool elf_writer_lookup_variables_at_address(void* writer, uint64_t address);
bool elf_writer_lookup_source_file(void* writer, int file_index);
bool elf_writer_generate_header(void* writer);
bool elf_writer_generate_section_header(void* writer);
bool elf_writer_generate_program_header(void* writer, void* segments, void* headers);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CODEGEN_TEST_STUBS_H