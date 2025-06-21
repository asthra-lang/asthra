#ifndef ASTHRA_AI_MAINTAINABILITY_RULES_H
#define ASTHRA_AI_MAINTAINABILITY_RULES_H

#include "../ai_lint_core.h"

// Maintainability rule check functions
bool check_missing_error_handling(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result);
bool check_missing_documentation(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result);
bool check_too_many_parameters(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result);
bool check_magic_numbers(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result);
bool check_deep_nesting(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result);
bool check_descriptive_names(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result);

// Registration function
void register_maintainability_rules(AILinter *linter);

#endif // ASTHRA_AI_MAINTAINABILITY_RULES_H 
