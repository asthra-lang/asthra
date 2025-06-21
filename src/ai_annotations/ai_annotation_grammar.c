#include "ai_annotation_grammar.h"
#include "../parser/parser_token.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// AI annotation name to type mapping
static const struct {
    const char *name;
    AIAnnotationType type;
} ai_annotation_map[] = {
    {"ai_confidence", AI_ANNOTATION_CONFIDENCE},
    {"ai_generated_confidence", AI_ANNOTATION_CONFIDENCE},
    {"ai_hypothesis", AI_ANNOTATION_HYPOTHESIS},
    {"ai_review_needed", AI_ANNOTATION_REVIEW_NEEDED},
    {"ai_todo", AI_ANNOTATION_TODO},
    {"ai_optimize", AI_ANNOTATION_OPTIMIZE},
    {"ai_test_coverage", AI_ANNOTATION_TEST_COVERAGE},
    {"ai_security_review", AI_ANNOTATION_SECURITY},
    {"ai_pattern", AI_ANNOTATION_PATTERN},
    {"ai_complexity", AI_ANNOTATION_COMPLEXITY},
    {"ai_refinement_step", AI_ANNOTATION_REFINEMENT},
    {NULL, 0}
};

bool is_ai_annotation(const char *annotation_name) {
    if (!annotation_name) return false;
    
    for (int i = 0; ai_annotation_map[i].name; i++) {
        if (strcmp(annotation_name, ai_annotation_map[i].name) == 0) {
            return true;
        }
    }
    return false;
}

AIAnnotationType resolve_ai_annotation_type(const char *annotation_name) {
    if (!annotation_name) return 0;
    
    for (int i = 0; ai_annotation_map[i].name; i++) {
        if (strcmp(annotation_name, ai_annotation_map[i].name) == 0) {
            return ai_annotation_map[i].type;
        }
    }
    return 0;
}

// Parse AI annotation parameters according to PEG grammar:
// AnnotationParams <- AnnotationParam (',' AnnotationParam)* / 'none'
// AnnotationParam <- IDENT '=' AnnotationValue
// AnnotationValue <- STRING / IDENT / INT / BOOL
AIAnnotationParam *parse_ai_annotation_parameters(Parser *parser, size_t *param_count) {
    if (!parser || !param_count) return NULL;
    
    *param_count = 0;
    
    if (!match_token(parser, TOKEN_LEFT_PAREN)) {
        return NULL; // No parameters
    }
    advance_token(parser);
    
    // Check for 'none' marker
    if (match_token(parser, TOKEN_IDENTIFIER) && 
        strcmp(parser->current_token.data.identifier.name, "none") == 0) {
        advance_token(parser);
        if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
            return NULL;
        }
        return NULL; // Explicit 'none' - no parameters
    }
    
    // Parse parameter list
    AIAnnotationParam *params = NULL;
    size_t capacity = 4;
    params = malloc(sizeof(AIAnnotationParam) * capacity);
    if (!params) return NULL;
    
    while (!match_token(parser, TOKEN_RIGHT_PAREN) && !at_end(parser)) {
        if (*param_count > 0) {
            if (!expect_token(parser, TOKEN_COMMA)) {
                free_ai_annotation_params(params, *param_count);
                return NULL;
            }
        }
        
        // Expand capacity if needed
        if (*param_count >= capacity) {
            capacity *= 2;
            AIAnnotationParam *new_params = realloc(params, sizeof(AIAnnotationParam) * capacity);
            if (!new_params) {
                free_ai_annotation_params(params, *param_count);
                return NULL;
            }
            params = new_params;
        }
        
        AIAnnotationParam *param = &params[*param_count];
        memset(param, 0, sizeof(AIAnnotationParam));
        
        // Parse IDENT '=' AnnotationValue
        if (!match_token(parser, TOKEN_IDENTIFIER)) {
            report_error(parser, "Expected parameter name");
            free_ai_annotation_params(params, *param_count);
            return NULL;
        }
        
        param->name = strdup(parser->current_token.data.identifier.name);
        advance_token(parser);
        
        if (!expect_token(parser, TOKEN_ASSIGN)) {
            free_ai_annotation_params(params, *param_count);
            return NULL;
        }
        
        // Parse AnnotationValue <- STRING / IDENT / INT / BOOL
        if (match_token(parser, TOKEN_STRING)) {
            param->type = ANNOTATION_PARAM_STRING;
            param->value.string_value = strdup(parser->current_token.data.string.value);
            advance_token(parser);
        } else if (match_token(parser, TOKEN_IDENTIFIER)) {
            param->type = ANNOTATION_PARAM_IDENT;
            param->value.ident_value = strdup(parser->current_token.data.identifier.name);
            advance_token(parser);
        } else if (match_token(parser, TOKEN_INTEGER)) {
            param->type = ANNOTATION_PARAM_INT;
            param->value.int_value = (int)parser->current_token.data.integer.value;
            advance_token(parser);
        } else if (match_token(parser, TOKEN_FLOAT)) {
            // Support float values for confidence scores
            param->type = ANNOTATION_PARAM_STRING; // Store as string for now
            char float_str[32];
            snprintf(float_str, sizeof(float_str), "%.2f", parser->current_token.data.float_val.value);
            param->value.string_value = strdup(float_str);
            param->value.float_value = (float)parser->current_token.data.float_val.value;
            advance_token(parser);
        } else if (match_token(parser, TOKEN_BOOL_TRUE) || match_token(parser, TOKEN_BOOL_FALSE)) {
            param->type = ANNOTATION_PARAM_BOOL;
            param->value.bool_value = (parser->current_token.type == TOKEN_BOOL_TRUE);
            advance_token(parser);
        } else {
            report_error(parser, "Expected annotation value (string, identifier, integer, float, or boolean)");
            free_ai_annotation_params(params, *param_count);
            return NULL;
        }
        
        (*param_count)++;
    }
    
    if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
        free_ai_annotation_params(params, *param_count);
        return NULL;
    }
    
    return params;
}

AIAnnotationParseResult *parse_ai_annotation(Parser *parser, const char *annotation_name) {
    if (!parser || !annotation_name || !is_ai_annotation(annotation_name)) {
        return NULL;
    }
    
    AIAnnotationParseResult *result = malloc(sizeof(AIAnnotationParseResult));
    if (!result) return NULL;
    
    result->name = strdup(annotation_name);
    result->type = resolve_ai_annotation_type(annotation_name);
    result->is_valid = false;
    result->error_message = NULL;
    
    // Parse annotation parameters
    size_t param_count = 0;
    AIAnnotationParam *params = parse_ai_annotation_parameters(parser, &param_count);
    
    result->params = params;
    result->param_count = param_count;
    
    // Validate annotation syntax
    result->is_valid = validate_ai_annotation_syntax(annotation_name, params, param_count);
    if (!result->is_valid && !result->error_message) {
        result->error_message = strdup("Invalid annotation parameters");
    }
    
    return result;
}

bool validate_ai_confidence_annotation(AIAnnotationParam *params, size_t param_count) {
    if (param_count != 1) return false;
    
    // Must be a float between 0.0 and 1.0
    if (params[0].type == ANNOTATION_PARAM_STRING) {
        // Try to parse as float
        float confidence = (float)atof(params[0].value.string_value);
        return confidence >= 0.0f && confidence <= 1.0f;
    }
    
    return false;
}

bool validate_ai_hypothesis_annotation(AIAnnotationParam *params, size_t param_count) {
    if (param_count != 1) return false;
    
    // Must be a non-empty string
    if (params[0].type != ANNOTATION_PARAM_STRING) return false;
    
    return params[0].value.string_value && 
           strlen(params[0].value.string_value) > 0;
}

bool validate_ai_review_annotation(AIAnnotationParam *params, size_t param_count) {
    if (param_count != 1) return false;
    
    // Must be a string describing the review area
    return params[0].type == ANNOTATION_PARAM_STRING &&
           params[0].value.string_value &&
           strlen(params[0].value.string_value) > 0;
}

bool validate_ai_todo_annotation(AIAnnotationParam *params, size_t param_count) {
    if (param_count != 1) return false;
    
    // Must be a string describing the task
    return params[0].type == ANNOTATION_PARAM_STRING &&
           params[0].value.string_value &&
           strlen(params[0].value.string_value) > 0;
}

bool validate_ai_optimize_annotation(AIAnnotationParam *params, size_t param_count) {
    if (param_count != 1) return false;
    
    // Must be a string describing the optimization suggestion
    return params[0].type == ANNOTATION_PARAM_STRING &&
           params[0].value.string_value &&
           strlen(params[0].value.string_value) > 0;
}

bool validate_ai_test_coverage_annotation(AIAnnotationParam *params, size_t param_count) {
    if (param_count != 1) return false;
    
    // Must be a string describing the test coverage level
    return params[0].type == ANNOTATION_PARAM_STRING &&
           params[0].value.string_value &&
           strlen(params[0].value.string_value) > 0;
}

bool validate_ai_security_annotation(AIAnnotationParam *params, size_t param_count) {
    if (param_count != 1) return false;
    
    // Must be a string describing the security priority
    return params[0].type == ANNOTATION_PARAM_STRING &&
           params[0].value.string_value &&
           strlen(params[0].value.string_value) > 0;
}

bool validate_ai_pattern_annotation(AIAnnotationParam *params, size_t param_count) {
    if (param_count != 1) return false;
    
    // Must be a string describing the pattern name
    return params[0].type == ANNOTATION_PARAM_STRING &&
           params[0].value.string_value &&
           strlen(params[0].value.string_value) > 0;
}

bool validate_ai_complexity_annotation(AIAnnotationParam *params, size_t param_count) {
    if (param_count != 1) return false;
    
    // Must be a string describing the complexity level
    return params[0].type == ANNOTATION_PARAM_STRING &&
           params[0].value.string_value &&
           strlen(params[0].value.string_value) > 0;
}

bool validate_ai_refinement_annotation(AIAnnotationParam *params, size_t param_count) {
    if (param_count != 1) return false;
    
    // Must be an integer representing the refinement step
    return params[0].type == ANNOTATION_PARAM_INT &&
           params[0].value.int_value > 0;
}

bool validate_ai_annotation_syntax(const char *annotation_name, AIAnnotationParam *params, size_t param_count) {
    AIAnnotationType type = resolve_ai_annotation_type(annotation_name);
    
    switch (type) {
        case AI_ANNOTATION_CONFIDENCE:
            return validate_ai_confidence_annotation(params, param_count);
        case AI_ANNOTATION_HYPOTHESIS:
            return validate_ai_hypothesis_annotation(params, param_count);
        case AI_ANNOTATION_REVIEW_NEEDED:
            return validate_ai_review_annotation(params, param_count);
        case AI_ANNOTATION_TODO:
            return validate_ai_todo_annotation(params, param_count);
        case AI_ANNOTATION_OPTIMIZE:
            return validate_ai_optimize_annotation(params, param_count);
        case AI_ANNOTATION_TEST_COVERAGE:
            return validate_ai_test_coverage_annotation(params, param_count);
        case AI_ANNOTATION_SECURITY:
            return validate_ai_security_annotation(params, param_count);
        case AI_ANNOTATION_PATTERN:
            return validate_ai_pattern_annotation(params, param_count);
        case AI_ANNOTATION_COMPLEXITY:
            return validate_ai_complexity_annotation(params, param_count);
        case AI_ANNOTATION_REFINEMENT:
            return validate_ai_refinement_annotation(params, param_count);
        default:
            return false;
    }
}

// Integration with existing annotation system
ASTNode *parse_ai_semantic_annotation(Parser *parser) {
    if (!parser) return NULL;
    
    SourceLocation start_loc = parser->current_token.location;
    
    // Expect #[annotation_name(...)]
    if (!expect_token(parser, TOKEN_HASH)) {
        return NULL;
    }
    
    if (!expect_token(parser, TOKEN_LEFT_BRACKET)) {
        return NULL;
    }
    
    if (!match_token(parser, TOKEN_IDENTIFIER)) {
        report_error(parser, "Expected AI annotation name");
        return NULL;
    }
    
    char *annotation_name = strdup(parser->current_token.data.identifier.name);
    advance_token(parser);
    
    // Check if this is an AI annotation
    if (!is_ai_annotation(annotation_name)) {
        free(annotation_name);
        report_error(parser, "Unknown AI annotation");
        return NULL;
    }
    
    // Parse parameters
    size_t param_count = 0;
    AIAnnotationParam *params = parse_ai_annotation_parameters(parser, &param_count);
    
    if (!expect_token(parser, TOKEN_RIGHT_BRACKET)) {
        free(annotation_name);
        free_ai_annotation_params(params, param_count);
        return NULL;
    }
    
    // Validate the annotation
    if (!validate_ai_annotation_syntax(annotation_name, params, param_count)) {
        free(annotation_name);
        free_ai_annotation_params(params, param_count);
        report_error(parser, "Invalid AI annotation syntax");
        return NULL;
    }
    
    // Create a semantic tag node for the AI annotation
    ASTNode *node = ast_create_node(AST_SEMANTIC_TAG, start_loc);
    if (!node) {
        free(annotation_name);
        free_ai_annotation_params(params, param_count);
        return NULL;
    }
    
    node->data.semantic_tag.name = annotation_name;
    
    // Convert AI parameters to AST node list
    if (param_count > 0) {
        node->data.semantic_tag.params = ast_node_list_create(param_count);
        if (node->data.semantic_tag.params) {
            for (size_t i = 0; i < param_count; i++) {
                ASTNode *param_node = NULL;
                
                switch (params[i].type) {
                    case ANNOTATION_PARAM_STRING:
                        param_node = ast_create_node(AST_STRING_LITERAL, start_loc);
                        if (param_node) {
                            param_node->data.string_literal.value = strdup(params[i].value.string_value);
                        }
                        break;
                    case ANNOTATION_PARAM_INT:
                        param_node = ast_create_node(AST_INTEGER_LITERAL, start_loc);
                        if (param_node) {
                            param_node->data.integer_literal.value = params[i].value.int_value;
                        }
                        break;
                    case ANNOTATION_PARAM_BOOL:
                        param_node = ast_create_node(AST_BOOL_LITERAL, start_loc);
                        if (param_node) {
                            param_node->data.bool_literal.value = params[i].value.bool_value;
                        }
                        break;
                    case ANNOTATION_PARAM_IDENT:
                        param_node = ast_create_node(AST_IDENTIFIER, start_loc);
                        if (param_node) {
                            param_node->data.identifier.name = strdup(params[i].value.ident_value);
                        }
                        break;
                }
                
                if (param_node) {
                    ast_node_list_add(&node->data.semantic_tag.params, param_node);
                }
            }
        }
    } else {
        node->data.semantic_tag.params = NULL;
    }
    
    free_ai_annotation_params(params, param_count);
    return node;
}

bool is_ai_semantic_annotation(const char *annotation_name) {
    return is_ai_annotation(annotation_name);
}

// Memory management functions
void free_ai_annotation_params(AIAnnotationParam *params, size_t count) {
    if (!params) return;
    
    for (size_t i = 0; i < count; i++) {
        free(params[i].name);
        
        switch (params[i].type) {
            case ANNOTATION_PARAM_STRING:
                free(params[i].value.string_value);
                break;
            case ANNOTATION_PARAM_IDENT:
                free(params[i].value.ident_value);
                break;
            default:
                break;
        }
    }
    
    free(params);
}

void free_ai_annotation_parse_result(AIAnnotationParseResult *result) {
    if (!result) return;
    
    free(result->name);
    free(result->error_message);
    free_ai_annotation_params(result->params, result->param_count);
    free(result);
} 
