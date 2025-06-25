#ifndef ASTHRA_AI_CONCURRENCY_RULES_H
#define ASTHRA_AI_CONCURRENCY_RULES_H

#include "../ai_lint_core.h"

// Concurrency rule check functions
bool check_data_race_potential(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result);
bool check_deadlock_potential(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result);
bool check_improper_spawn_usage(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result);
bool check_channel_misuse(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result);
bool check_missing_synchronization(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result);
bool check_blocking_in_async(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result);

// Registration function
void register_concurrency_rules(AILinter *linter);

#endif // ASTHRA_AI_CONCURRENCY_RULES_H
