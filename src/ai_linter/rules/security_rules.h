#ifndef ASTHRA_AI_SECURITY_RULES_H
#define ASTHRA_AI_SECURITY_RULES_H

#include "../ai_lint_core.h"

// Security rule check functions
bool check_unsafe_function_usage(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result);
bool check_buffer_overflow_risk(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result);
bool check_unvalidated_input(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result);
bool check_hardcoded_secrets(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result);
bool check_unsafe_pointer_operations(ASTNode *node, AsthraSemanticsAPI *api, AILintResult **result);
bool check_missing_error_handling_security(ASTNode *node, AsthraSemanticsAPI *api,
                                           AILintResult **result);

// Registration function
void register_security_rules(AILinter *linter);

#endif // ASTHRA_AI_SECURITY_RULES_H
