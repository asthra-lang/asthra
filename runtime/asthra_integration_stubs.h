#ifndef ASTHRA_INTEGRATION_STUBS_H
#define ASTHRA_INTEGRATION_STUBS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "core/asthra_runtime_core.h"
#include "types/asthra_runtime_result.h"

// Forward declarations to avoid conflicts
typedef struct Lexer Lexer;
typedef struct Parser Parser;
typedef struct ASTNode ASTNode;
typedef struct SemanticAnalyzer SemanticAnalyzer;

#ifdef __cplusplus
extern "C" {
#endif

AsthraResult Asthra_register_c_thread(void);
void Asthra_unregister_c_thread(void);
void asthra_test_context_init(AsthraTestContext* context);
Lexer* lexer_create_stub(const char* source);
Parser* parser_create_stub(Lexer* lexer);
void parser_destroy_stub(Parser* parser);
ASTNode* parser_parse_program_stub(Parser* parser);
int semantic_analyze_program_stub(SemanticAnalyzer* analyzer, ASTNode* ast);
SemanticAnalyzer* semantic_analyzer_create_stub(void);
void semantic_analyzer_destroy_stub(SemanticAnalyzer* analyzer);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_INTEGRATION_STUBS_H
