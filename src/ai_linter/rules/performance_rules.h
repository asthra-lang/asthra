#ifndef ASTHRA_AI_PERFORMANCE_RULES_H
#define ASTHRA_AI_PERFORMANCE_RULES_H

#include "../ai_lint_core.h"

// Performance rule check functions
bool check_string_concat_in_loop(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result);
bool check_collection_preallocation(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result);
bool check_unnecessary_cloning(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result);
bool check_manual_indexing_performance(ASTNode *node, AsthraSemanticsAPI *api,
                                       AILintResult **result);

// Registration function
void register_performance_rules(AILinter *linter);

#endif // ASTHRA_AI_PERFORMANCE_RULES_H
